#include "app_esp8266_wifista.h"
#include "stdlib.h"
#include "string.h"
#include "sys_utils.h"

//���յ���ATָ��Ӧ�����ݷ��ظ����Դ���
//mode:0,������rx_sta;
//     1,����rx_sta;
void app_esp8266_at_response(uint8_t mode)
{
    if (rx_sta & 0X8000)     //���յ�һ��������
    {
        rx_buf[rx_sta & 0X7FFF] = 0; //��ӽ�����
        LOG("%s", rx_buf); //���͵�����
        if (mode)
        rx_sta = 0;
    }
}
//ESP8266���������,�����յ���Ӧ��
//str:�ڴ���Ӧ����
//����ֵ:0,û�еõ��ڴ���Ӧ����
//    ����,�ڴ�Ӧ������λ��(str��λ��)
uint8_t *app_esp8266_check_cmd(uint8_t *str)
{
    char *strx = 0;
    if (rx_sta & 0X8000)     //���յ�һ��������
    {
        rx_buf[rx_sta & 0X7FFF] = 0; //��ӽ�����
        strx = strstr((const char *)rx_buf, (const char *)str);
    }
    return (uint8_t *)strx;
}
//�� ESP8266��������
//cmd:���͵������ַ���
//ack:�ڴ���Ӧ����,���Ϊ��,���ʾ����Ҫ�ȴ�Ӧ��
//waittime:�ȴ�ʱ��(��λ:10ms)
//����ֵ:0,���ͳɹ�(�õ����ڴ���Ӧ����)
//       1,����ʧ��
uint8_t app_esp8266_send_cmd(uint8_t *cmd, uint8_t *ack, uint16_t waittime)
{
    uint8_t res = 0;
    rx_sta = 0;
    esp8266_printf("%s\r\n", cmd);   //��������
    if (ack && waittime)    //��Ҫ�ȴ�Ӧ��
    {
        while (--waittime)  //�ȴ�����ʱ
        {
            co_delay_100us(100);
            if (rx_sta & 0X8000) //���յ��ڴ���Ӧ����
            {
                if (app_esp8266_check_cmd(ack))
                {
                    LOG("���ͣ�%s ��Ӧ:%s\r\n", cmd, (uint8_t *)ack);
                    break;//�õ���Ч����
                }
                rx_sta = 0;
            }
        }
        if (waittime == 0)res = 1;
    }
    return res;
}
// ESP8266�˳�͸��ģʽ
//����ֵ:0,�˳��ɹ�;
//       1,�˳�ʧ��
uint8_t app_esp8266_quit_trans(void)
{
    uart_putc_noint(UART0, '+');
    co_delay_100us(150);
    uart_putc_noint(UART0, '+');
    co_delay_100us(150);
    uart_putc_noint(UART0, '+');
    co_delay_100us(1000);
    
    return app_esp8266_send_cmd("AT", "OK", 20); //�˳�͸���ж�.
}

//����ESP8266λstaģʽ�������ӵ�·����
uint8_t app_esp8266_wifista_config(char *ssid, char *pwd)
{
    uint8_t p[200];
    uint8_t TryConnectTime = 0;
    
	LOG("׼������\r\n");
    while (app_esp8266_send_cmd("AT", "OK", 20)) //���WIFIģ���Ƿ�����
    {
        app_esp8266_quit_trans();//�˳�͸��
        co_delay_100us(10000);
        LOG("δ��⵽ģ��\r\n");
    }
    
    app_esp8266_send_cmd("AT+RESTORE", "OK", 200); 
    co_delay_100us(10000);
    co_delay_100us(10000);
    co_delay_100us(10000);
    co_delay_100us(10000);
    
    app_esp8266_send_cmd("AT+CIPMODE=0", "OK", 200); //�ر�͸��ģʽ
    while (app_esp8266_send_cmd("ATE0", "OK", 20)); //�رջ���
    app_esp8266_send_cmd("AT+CWMODE=1", "OK", 50);     //����WIFI STAģʽ
//    app_esp8266_send_cmd("AT+RST", "OK", 20);      //DHCP�������ر�(��APģʽ��Ч)
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
        LOG("WiFi����ʧ��,���ڳ��Ե� %d ������\r\n", TryConnectTime++);
        co_delay_100us(1000);
    };                  //����Ŀ��·����,���һ��IP
    LOG("WiFi���ӳɹ�\r\n");
    return 0;
}

