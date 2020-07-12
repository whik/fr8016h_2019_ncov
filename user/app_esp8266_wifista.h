#ifndef __APP_ESP8266_WIFISTA_H__
#define __APP_ESP8266_WIFISTA_H__

#include "app_esp8266.h"


void app_esp8266_at_response(uint8_t mode);
uint8_t* app_esp8266_check_cmd(uint8_t *str);
uint8_t app_esp8266_send_data(uint8_t *data,uint8_t *ack,uint16_t waittime);
uint8_t app_esp8266_send_cmd(uint8_t *cmd,uint8_t *ack,uint16_t waittime);
uint8_t app_esp8266_quit_trans(void);
//uint8_t app_esp8266_consta_check(void);
//void app_esp8266_get_wanip(uint8_t* ipbuf);
//void app_esp8266_get_ip(uint8_t x,uint8_t y);
uint8_t app_esp8266_wifista_config(char *ssid, char *pwd);
 
extern const uint8_t* wifista_ssid;		//WIFI STA SSID
extern const uint8_t* wifista_encryption;//WIFI STA º”√‹∑Ω Ω
extern const uint8_t* wifista_password; 	//WIFI STA √‹¬Î

extern const uint8_t* ATK_ESP8266_ECN_TBL[5];

#endif

