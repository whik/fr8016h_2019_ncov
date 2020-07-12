#ifndef __PROJ_MAIN_H__
#define __PROJ_MAIN_H__

#include <stdio.h>
#include <string.h>

#include "gap_api.h"
#include "gatt_api.h"

#include "os_timer.h"
#include "os_mem.h"
#include "sys_utils.h"
#include "button.h"
#include "jump_table.h"


#include "driver_plf.h"
#include "driver_system.h"
#include "driver_i2s.h"
#include "driver_pmu.h"
#include "driver_uart.h"
#include "driver_rtc.h"

#include "driver_timer.h"

#include "ble_simple_peripheral.h"
#include "simple_gatt_service.h"

#include "user_task.h"
#include "user_timer.h"

#include "app_esp8266.h"
#include "app_esp8266_wifista.h"
#include "app_esp8266_ncov.h"

#include "app_lcd.h"
#include "app_lcd_gui.h"

#include "app_sht3x.h"

#include "app_button.h"

#define api_data        "https://lab.isaaclin.cn/nCoV/api/overall"
//#define api_news        "https://lab.isaaclin.cn/nCoV/api/news?page=1&num=3"
//#define api_province    "https://lab.isaaclin.cn/nCoV/api/area?province=%E5%8C%97%E4%BA%AC%E5%B8%82"

#define data_cip_type   "SSL"
#define data_api_port   "443"
#define data_api_ip     "47.102.117.253"

#define WIFI_SSID   "fr8016h_2019_ncov"
#define WIFI_PWD    "www.wangchaochao.top"


void ncov_update(void *parg);


#endif

