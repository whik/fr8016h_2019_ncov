#include "app_esp8266.h"

uint8_t rx_buf[RX_BUF_SIZE];
uint16_t rx_sta = 0;

uint32_t TIME = 10000;

__attribute__((weak)) __attribute__((section("ram_code"))) void uart0_isr_ram(void)
{
    uint8_t int_id;
    uint8_t c;
    volatile struct uart_reg_t *uart_reg = (volatile struct uart_reg_t *)UART0_BASE;
    int_id = uart_reg->u3.iir.int_id;
    if(int_id == 0x04 || int_id == 0x0c )   
    {
        c = uart_reg->u1.data;
        
        if((rx_sta & (1 << 15)) == 0) //�������һ������,��û�б�����,���ٽ�����������
        {
            if(rx_sta < RX_BUF_SIZE)	//�����Խ�������
            {
                if(rx_sta == 0) 				
                {
                    timer_init(TIMER0, TIME, TIMER_PERIODIC);
//                    timer_reload(TIMER0);                    
                    timer_run(TIMER0);
                }
                rx_buf[rx_sta++] = c;	//��¼���յ���ֵ
            }
            else
            {
                rx_sta |= 1 << 15;				//ǿ�Ʊ�ǽ������
            }
        }
    }
    else if(int_id == 0x06)
    {
        volatile uint32_t line_status = uart_reg->lsr;
    }
}

__attribute__((weak)) __attribute__((section("ram_code"))) void timer0_isr_ram(void)
{
    rx_sta |= 1<<15;	//��ǽ������
    timer_stop(TIMER0);
    timer_clear_interrupt(TIMER0);
//    co_printf("timer0 enter interrrupt\r\n");
}

void LOG(char *fmt,...)
{
	unsigned char UsartPrintfBuf[296];
	va_list ap;
	unsigned char *pStr = UsartPrintfBuf;
	
	va_start(ap, fmt);
	vsnprintf((char *)UsartPrintfBuf, sizeof(UsartPrintfBuf), fmt, ap);							//��ʽ��
	va_end(ap);
	
	while(*pStr != 0)
		uart_putc_noint(UART1, *pStr++);
}

void app_esp8266_init(void)
{
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_4, PORTD4_FUNC_UART0_RXD);
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_5, PORTD5_FUNC_UART0_TXD);
    uart_init(UART0, BAUD_RATE_115200);  
    NVIC_EnableIRQ(UART0_IRQn);   

    //10ms�ж�
    timer_init(TIMER0, TIME, TIMER_PERIODIC); 
    timer_stop(TIMER0);
    NVIC_EnableIRQ(TIMER0_IRQn);
}

void esp8266_printf(char *fmt,...)
{
	unsigned char UsartPrintfBuf[296];
	va_list ap;
	unsigned char *pStr = UsartPrintfBuf;
	
	va_start(ap, fmt);
	vsnprintf((char *)UsartPrintfBuf, sizeof(UsartPrintfBuf), fmt, ap);							//��ʽ��
	va_end(ap);
	
	while(*pStr != 0)
		uart_putc_noint(UART0, *pStr++);
}


