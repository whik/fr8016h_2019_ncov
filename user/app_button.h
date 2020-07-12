#ifndef __APP_BUTTON_H__
#define __APP_BUTTON_H__

#include "multi_button.h"
#include "driver_system.h"
#include "driver_gpio.h"
#include "driver_iomux.h"
#include "driver_pmu.h"
#include "co_printf.h"
#include "os_timer.h"

void app_button_init(void);
uint8_t read_btn_k1(void);
void button_callback(void *button);
void timer_k1_fun(void *parg);


#endif

