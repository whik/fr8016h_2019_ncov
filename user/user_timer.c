#include "user_timer.h"
#include "lcd.h"
#include "driver_pmu.h"

extern uint16_t task1_id;

os_timer_t timer;       //必须是全局变量

struct NCOV_DATA rec;

void user_timer_test(void)
{
    os_timer_init(&timer, user_timer_fun, 0);
    os_timer_start(&timer, 1000, true);   //max = 0x3FFFFF = 4194303ms = 69min
}

void user_timer_fun(void *parg)
{
    os_event_t event;

    struct NCOV_DATA param;
    
    param.data1 = 111;
    param.data2 = 222;
    param.data3 = "hello event!";
    param.data4 = 124;
    
    event.event_id = 12;
    event.param = &param;
    event.param_len = sizeof(param);
    event.src_task_id = task1_id;
    
    //向task1发送一条消息
    os_msg_post(task1_id, &event);

    co_printf("this is tiemr test\r\n");    
}
