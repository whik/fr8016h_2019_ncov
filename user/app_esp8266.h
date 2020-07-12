#ifndef __APP_ESP8266_H__
#define __APP_ESP8266_H__

#include "driver_uart.h"
#include "driver_system.h"
#include "driver_timer.h"

#include "os_timer.h"
#include "co_printf.h"

#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#define RX_BUF_SIZE     2000

extern uint8_t rx_buf[RX_BUF_SIZE];
extern uint16_t rx_sta;
extern uint32_t TIME;

void LOG(char *fmt,...);
void app_esp8266_init(void);
void esp8266_printf(char *fmt,...);

#endif


