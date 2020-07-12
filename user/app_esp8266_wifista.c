#include "app_esp8266_wifista.h"
#include "stdlib.h"
#include "string.h"
#include "sys_utils.h"

//将收到的AT指令应答数据返回给电脑串口
//mode:0,不清零rx_sta;
//     1,清零rx_sta;
void app_esp8266_at_response(uint8_t mode)
{
    if (rx_sta & 0X8000)     //接收到一次数据了
    {
        rx_buf[rx_sta & 0X7FFF] = 0; //添加结束符
        LOG("%s", rx_buf); //发送到串口
        if (mode)
        rx_sta = 0;
    }
}
//ESP8266发送命令后,检测接收到的应答
//str:期待的应答结果
//返回值:0,没有得到期待的应答结果
//    其他,期待应答结果的位置(str的位置)
uint8_t *app_esp8266_check_cmd(uint8_t *str)
{
    char *strx = 0;
    if (rx_sta & 0X8000)     //接收到一次数据了
    {
        rx_buf[rx_sta & 0X7FFF] = 0; //添加结束符
        strx = strstr((const char *)rx_buf, (const char *)str);
    }
    return (uint8_t *)strx;
}
//向 ESP8266发送命令
//cmd:发送的命令字符串
//ack:期待的应答结果,如果为空,则表示不需要等待应答
//waittime:等待时间(单位:10ms)
//返回值:0,发送成功(得到了期待的应答结果)
//       1,发送失败
uint8_t app_esp8266_send_cmd(uint8_t *cmd, uint8_t *ack, uint16_t waittime)
{
    uint8_t res = 0;
    rx_sta = 0;
    esp8266_printf("%s\r\n", cmd);   //发送命令
    if (ack && waittime)    //需要等待应答
    {
        while (--waittime)  //等待倒计时
        {
            co_delay_100us(100);
            if (rx_sta & 0X8000) //接收到期待的应答结果
            {
                if (app_esp8266_check_cmd(ack))
                {
                    LOG("发送：%s 回应:%s\r\n", cmd, (uint8_t *)ack);
                    break;//得到有效数据
                }
                rx_sta = 0;
            }
        }
        if (waittime == 0)res = 1;
    }
    return res;
}
// ESP8266退出透传模式
//返回值:0,退出成功;
//       1,退出失败
uint8_t app_esp8266_quit_trans(void)
{
    uart_putc_noint(UART0, '+');
    co_delay_100us(150);
    uart_putc_noint(UART0, '+');
    co_delay_100us(150);
    uart_putc_noint(UART0, '+');
    co_delay_100us(1000);
    
    return app_esp8266_send_cmd("AT", "OK", 20); //退出透传判断.
}

//配置ESP8266位sta模式，并连接到路由器
uint8_t app_esp8266_wifista_config(char *ssid, char *pwd)
{
    uint8_t p[200];
    uint8_t TryConnectTime = 0;
    
	LOG("准备连接\r\n");
    while (app_esp8266_send_cmd("AT", "OK", 20)) //检查WIFI模块是否在线
    {
        app_esp8266_quit_trans();//退出透传
        co_delay_100us(10000);
        LOG("未检测到模块\r\n");
    }
    
    app_esp8266_send_cmd("AT+RESTORE", "OK", 200); 
    co_delay_100us(10000);
    co_delay_100us(10000);
    co_delay_100us(10000);
    co_delay_100us(10000);
    
    app_esp8266_send_cmd("AT+CIPMODE=0", "OK", 200); //关闭透传模式
    while (app_esp8266_send_cmd("ATE0", "OK", 20)); //关闭回显
    app_esp8266_send_cmd("AT+CWMODE=1", "OK", 50);     //设置WIFI STA模式
//    app_esp8266_send_cmd("AT+RST", "OK", 20);      //DHCP服务器关闭(仅AP模式有效)
    co_delay_100us(10000);
    co_delay_100us(10000);
    co_delay_100us(10000);
    co_delay_100us(10000);

    app_esp8266_send_cmd("AT+CIPMUX=0", "OK", 20); 
    sprintf((char *)p, "AT+CWJAP=\"%s\",\"%s\"", ssid, pwd);

	while (app_esp8266_send_cmd(p, "WIFI GOT IP", 300))
    {
        if (TryConnectTime >= 10)
        {
            TryConnectTime = 0;
            return 1;
        }
        LOG("WiFi连接失败,正在尝试第 %d 次连接\r\n", TryConnectTime++);
        co_delay_100us(1000);
    };                  //连接目标路由器,并且获得IP
    LOG("WiFi连接成功\r\n");
    return 0;
}

