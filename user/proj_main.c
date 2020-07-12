#include "proj_main.h"

/* 1_只能设置系统参数 */
void user_custom_parameters(void)
{
    /* 设置本机蓝牙MAC地址 */
    __jump_table.addr.addr[0] = 0xBD;
    __jump_table.addr.addr[1] = 0xAD;
    __jump_table.addr.addr[2] = 0xD0;
    __jump_table.addr.addr[3] = 0xF0;
    __jump_table.addr.addr[4] = 0x80;
    __jump_table.addr.addr[5] = 0x10;
    __jump_table.system_clk = SYSTEM_SYS_CLK_48M;
    
    /* 取消sleep模式 */
    __jump_table.system_option &= ~(SYSTEM_OPTION_SLEEP_ENABLE);
	jump_table_set_static_keys_store_offset(0x7d000);
}

/* 2_只能调用外设驱动组件函数 */
void user_entry_before_ble_init(void)
{    
    /* 设置芯片供电选择 */
    pmu_set_sys_power_mode(PMU_SYS_POW_BUCK);
    /* 使能PMU的一些中断类型 */
    pmu_enable_irq(PMU_ISR_BIT_ACOK
                   | PMU_ISR_BIT_ACOFF
                   | PMU_ISR_BIT_ONKEY_PO
                   | PMU_ISR_BIT_OTP
                   | PMU_ISR_BIT_LVD
                   | PMU_ISR_BIT_BAT
                   | PMU_ISR_BIT_ONKEY_HIGH);
    NVIC_EnableIRQ(PMU_IRQn);
    
    pmu_set_aldo_voltage(PMU_ALDO_MODE_NORMAL, PMU_ALDO_VOL_3_3);
    
    /* 设置PA2/3为串口1功能, 115200 */
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_2, PORTA2_FUNC_UART1_RXD);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_3, PORTA3_FUNC_UART1_TXD);
    uart_init(UART1, BAUD_RATE_115200); 
    NVIC_EnableIRQ(UART1_IRQn);    
    
    if(__jump_table.system_option & SYSTEM_OPTION_ENABLE_HCI_MODE)
    {
        system_set_port_pull(GPIO_PA4, true);
        system_set_port_mux(GPIO_PORT_A, GPIO_BIT_4, PORTA4_FUNC_UART0_RXD);
        system_set_port_mux(GPIO_PORT_A, GPIO_BIT_5, PORTA5_FUNC_UART0_TXD);
    }

    /* used for debug, reserve 3S for j-link once sleep is enabled. */
    if(__jump_table.system_option & SYSTEM_OPTION_SLEEP_ENABLE)
    {
        co_delay_100us(10000);
        co_delay_100us(10000);
        co_delay_100us(10000);
    }
}

/* 3_能调用所有组件的函数 */
extern struct ncov_data dataChina;
extern struct ncov_data dataGlobal;
os_timer_t timer_ncov;
void user_entry_after_ble_init(void)
{
    LOG("新冠肺炎疫情实时数据监控平台\r\n");
    LOG("    基于FreqChip FR8016H\r\n");
    
    app_lcd_init(BLACK);
    gui_show_ncov_data(dataChina, dataGlobal);

//    app_sht3x_init();
    app_button_init();
    
    app_esp8266_init();
    app_esp8266_wifista_config(WIFI_SSID, WIFI_PWD);
    build_ssl_connect(data_cip_type, data_api_ip, data_api_port);
    get_ncov_api(api_data, parse_ncov_data);

    os_timer_init(&timer_ncov, ncov_update, 0);
    os_timer_start(&timer_ncov, 1000, true);  //10min

//    user_task_test();
//    user_timer_test();
//    simple_peripheral_init();
}

void ncov_update(void *parg)
{
    static uint16_t cnt = 0;
    int ret;
    
    cnt++;
    LOG("cnt: %d, k1:%d\r\n", cnt, read_btn_k1());
    if(cnt >= 60*5)
    {
        cnt = 0;
        ret = get_ncov_api(api_data, parse_ncov_data);
        if(ret == 1)
        {
            ret = build_ssl_connect(data_cip_type, data_api_ip, data_api_port);
            if(ret == 1)
                app_esp8266_wifista_config(WIFI_SSID, WIFI_PWD);
        }
    }
}



