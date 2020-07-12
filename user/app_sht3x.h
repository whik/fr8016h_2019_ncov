#ifndef __APP_SHT3X_H__
#define __APP_SHT3X_H__

#include "sht3x.h"
#include "co_printf.h"
#include "os_timer.h"

void sht3x_update(void *parg);

int16_t app_sht3x_init(void);
int16_t app_sht3x_get(int32_t *temperature, int32_t *humidity);

#endif
