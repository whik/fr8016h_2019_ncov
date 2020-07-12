#include "app_sht3x.h"

extern void LOG(char *fmt,...);

os_timer_t timer_sht3x;
int16_t app_sht3x_init(void)
{
    int16_t ret = 0;
    sensirion_i2c_init();
    ret = sht3x_probe();
    if(ret != 0)
        LOG("sht3x is not detected\r\n");
//    os_timer_init(&timer_sht3x, sht3x_update, 0);
//    os_timer_start(&timer_sht3x, 250, true);
    return ret;
}

void sht3x_update(void *parg)
{
    int32_t temp, hum;
    int16_t ret;
    ret = sht3x_measure_blocking_read(&temp, &hum);
    if(ret == STATUS_OK)
        LOG("temperature: %.1f C, humidity: %.1f %\r\n", temp/1000.0, hum/1000.0);
    else 
        LOG("error reading measurement");
}

