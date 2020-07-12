#include "app_esp8266_ncov.h"
#include "cJSON.h"
#include "stdio.h"
#include "time.h"

//struct ncov_data dataChina = {12345678, -100, 23456789, 456, 4567890, -920, 123445, 12324, 12324, 123412, "06-13 16:22"};
//struct ncov_data dataGlobal = {12345678, -100, 23456789, 456, 4567890, -920, 0, 0, 0, 0, NULL};

struct ncov_data dataChina = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "01-01 01:00"};
struct ncov_data dataGlobal = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "01-01 01:00"};

extern void gui_show_ncov_data(struct ncov_data china, struct ncov_data global);
extern void LCD_Clear(uint16_t Color);
extern uint16_t BACK_COLOR;   //背景色

uint8_t build_ssl_connect(char *type, char *ip, char *port)
{
    char str_tmp[200];
    uint8_t TryConnectTime = 0;

    app_esp8266_send_cmd("AT+CIPMUX=0", "OK", 100);
    app_esp8266_send_cmd("AT+CIPSSLSIZE=4096", "OK", 100);

    sprintf((char *)str_tmp, "AT+CIPSTART=\"%s\",\"%s\",%s", type, ip, port);

    while (app_esp8266_send_cmd((uint8_t *)str_tmp, "OK", 200))
    {
        LOG("api连接失败,正在尝试第 %d 次连接\r\n", TryConnectTime++);
        co_delay_100us(2000);
        if (TryConnectTime >= 10)
        {
            return 1;
        }
    };

    co_delay_100us(3000);
    app_esp8266_send_cmd("AT+CIPMODE=1", "OK", 100);    //传输模式为：透传

    rx_sta = 0;
    app_esp8266_send_cmd("AT+CIPSEND", "OK", 100);       //开始透传
    return 0;
}

uint8_t get_ncov_api(char *api_addr, uint8_t (*parse_fun)(void))
{
    LOG("获取疫情数据:GET %s\r\n", api_addr);

    TIME = 300000; //500ms,时间太短会导致接收不完整
    esp8266_printf("GET %s\n\n", api_addr);

    co_delay_100us(200);
    rx_sta = 0;
    co_delay_100us(1000);
    co_delay_100us(1000);
    co_delay_100us(1000);

    if (rx_sta & 0X8000)
        rx_buf[rx_sta & 0X7FFF] = 0; //添加结束符

    //    LOG("接收到的数据:%d\r\n%s\r\n", strlen((const char*)rx_buf), rx_buf);	//JSON原始数据
    LOG("接收到的数据:%d\r\n", strlen((const char *)rx_buf));	//JSON原始数据
    if(strlen((const char *)rx_buf) < 600)
    {
        TIME = 10000;
        rx_sta = 0;
        memset(rx_buf, 0, sizeof(rx_buf));
        return 1;
    }
    
    parse_fun();

    LCD_Clear(BACK_COLOR);
    gui_show_ncov_data(dataChina, dataGlobal);

    TIME = 10000;
    rx_sta = 0;
    memset(rx_buf, 0, sizeof(rx_buf));

    return 0;
}

uint8_t parse_ncov_data(void)
{
    cJSON *root;
    cJSON *results_arr;
    cJSON *results;
    cJSON *globalStatistics;
    time_t updateTime;
    struct tm *time;

    LOG("接收到的数据:%d\r\n", strlen((const char *)rx_buf));	//JSON原始数据
    root = cJSON_Parse((const char *)rx_buf);
    if(root)
    {
        LOG("数据格式正确，开始解析\r\n");

        results_arr = cJSON_GetObjectItem(root, "results");
        if(results_arr->type == cJSON_Array)
        {
            results = cJSON_GetArrayItem(results_arr, 0);
            if(results->type == cJSON_Object)
            {
                dataChina.currentConfirmedCount = cJSON_GetObjectItem(results, "currentConfirmedCount")->valueint;
                dataChina.currentConfirmedIncr  = cJSON_GetObjectItem(results, "currentConfirmedIncr")->valueint;
                dataChina.confirmedCount        = cJSON_GetObjectItem(results, "confirmedCount")->valueint;
                dataChina.confirmedIncr         = cJSON_GetObjectItem(results, "confirmedIncr")->valueint;
                dataChina.curedCount            = cJSON_GetObjectItem(results, "curedCount")->valueint;
                dataChina.curedIncr             = cJSON_GetObjectItem(results, "curedIncr")->valueint;
                dataChina.deadCount             = cJSON_GetObjectItem(results, "deadCount")->valueint;
                dataChina.deadIncr              = cJSON_GetObjectItem(results, "deadIncr")->valueint;
                dataChina.seriousCount          = cJSON_GetObjectItem(results, "seriousCount")->valueint;
                dataChina.seriousIncr           = cJSON_GetObjectItem(results, "seriousIncr")->valueint;

                LOG("------------国内疫情-------------\r\n");
                LOG("现存确诊:   %5d, 较昨日:%3d\r\n", dataChina.currentConfirmedCount, dataChina.currentConfirmedIncr);
                LOG("累计确诊:   %5d, 较昨日:%3d\r\n", dataChina.confirmedCount, dataChina.confirmedIncr);
                LOG("累计治愈:   %5d, 较昨日:%3d\r\n", dataChina.curedCount, dataChina.curedIncr);
                LOG("累计死亡:   %5d, 较昨日:%3d\r\n", dataChina.deadCount, dataChina.deadIncr);
                LOG("现存无症状: %5d, 较昨日:%3d\r\n\r\n", dataChina.seriousCount, dataChina.seriousIncr);

                globalStatistics = cJSON_GetObjectItem(results, "globalStatistics");
                if(globalStatistics->type == cJSON_Object)
                {
                    dataGlobal.currentConfirmedCount = cJSON_GetObjectItem(globalStatistics, "currentConfirmedCount")->valueint;
                    dataGlobal.confirmedCount        = cJSON_GetObjectItem(globalStatistics, "confirmedCount")->valueint;
                    dataGlobal.curedCount            = cJSON_GetObjectItem(globalStatistics, "curedCount")->valueint;
                    dataGlobal.deadCount             = cJSON_GetObjectItem(globalStatistics, "deadCount")->valueint;
                    dataGlobal.currentConfirmedIncr  = cJSON_GetObjectItem(globalStatistics, "currentConfirmedIncr")->valueint;
                    dataGlobal.confirmedIncr         = cJSON_GetObjectItem(globalStatistics, "confirmedIncr")->valueint;
                    dataGlobal.curedIncr             = cJSON_GetObjectItem(globalStatistics, "curedIncr")->valueint;
                    dataGlobal.deadIncr              = cJSON_GetObjectItem(globalStatistics, "deadIncr")->valueint;

                    LOG("------------全球疫情-------------\r\n");
                    LOG("现存确诊: %8d, 较昨日:%5d\r\n", dataGlobal.currentConfirmedCount, dataGlobal.currentConfirmedIncr);
                    LOG("累计确诊: %8d, 较昨日:%5d\r\n", dataGlobal.confirmedCount, dataGlobal.confirmedIncr);
                    LOG("累计死亡: %8d, 较昨日:%5d\r\n", dataGlobal.deadCount, dataGlobal.deadIncr);
                    LOG("累计治愈: %8d, 较昨日:%5d\r\n\r\n", dataGlobal.curedCount, dataGlobal.curedIncr);
                }

                /* 毫秒级时间戳转字符串 */
                updateTime = (time_t )(cJSON_GetObjectItem(results, "updateTime")->valuedouble / 1000);
                updateTime += 8 * 60 * 60; /* UTC8校正 */
                time = localtime(&updateTime);
                /* 格式化时间 */
                strftime(dataChina.updateTime, 20, "%m-%d %H:%M", time);
                LOG("更新于:%s\r\n", dataChina.updateTime);/* 06-24 11:21 */
            }
        }
        else
        {
            LOG("数据格式错误\r\n");
            return 0;
        }
    }

    cJSON_Delete(root);

    LOG("*********更新完成*********\r\n");
    return 1;
}


