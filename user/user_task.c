#include "user_task.h"
#include "user_timer.h"

uint16_t task1_id;

extern void esp8266_printf(char *fmt,...);
extern void LOG(char *fmt,...);

/* task 测试，可传参数过来 */
static int task1_fun(os_event_t *event)
{
    uint16_t id = event->event_id;
    struct NCOV_DATA *param;
    
    param = event->param;
//    param = (struct NCOV_DATA *)event->param;

    switch(id)
    {
        case 12:
            LOG("data1:%d, data2:%d, data3: %s, data4: %d\r\n",
                param->data1, param->data2, param->data3, param->data4);
            esp8266_printf("data1:%d, data2:%d, data3: %s, data4: %d\r\n",
                param->data1, param->data2, param->data3, param->data4);
            
            break;
        default: break;
    }

//    co_printf("event trigger\r\n");
    return EVT_CONSUMED;
}

void user_task_test(void)
{
    task1_id = os_task_create(task1_fun);
    if(task1_id != 0xFF)
        co_printf("task 1 create ok: %d\r\n", task1_id);
}
