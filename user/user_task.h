#ifndef _USER_TASK_H
#define _USER_TASK_H
#include <stdint.h>

#include "os_task.h"
#include "os_msg_q.h"

#include "co_printf.h"
#include "button.h"

#include "string.h"

enum user_event_t {
    USER_EVT_AT_COMMAND,
    USER_EVT_BUTTON,
};

void user_task_test(void);
static int task1_fun(os_event_t *param);

#endif  // _USER_TASK_H

