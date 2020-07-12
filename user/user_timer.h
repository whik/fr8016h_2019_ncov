#ifndef _USER_TIMER_H
#define _USER_TIMER_H

#include <stdint.h>

#include "os_task.h"
#include "os_msg_q.h"
#include "os_timer.h"

#include "co_printf.h"
#include "user_task.h"
#include "button.h"

struct NCOV_DATA
{
    uint16_t data1;
    uint16_t data2;
    char *data3;
    uint8_t data4;
};

void user_timer_test(void);
void user_timer_fun(void *parg);

#endif 

