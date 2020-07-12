#ifndef __APP_ESP8266_NCOV_H__
#define __APP_ESP8266_NCOV_H__

#include "app_esp8266_wifista.h"
#include "sys_utils.h"

struct ncov_data{
    long currentConfirmedCount;
    long currentConfirmedIncr;
    long confirmedCount;
    long confirmedIncr;
    long curedCount;
    long curedIncr;
    long seriousCount;
    long seriousIncr;
    long deadCount;
    long deadIncr;
    char updateTime[20];
};

uint8_t build_ssl_connect(char *type, char *ip, char *port);
uint8_t get_ncov_api(char *api_addr, uint8_t (*parse_fun)(void));
uint8_t parse_ncov_data(void);

#endif


