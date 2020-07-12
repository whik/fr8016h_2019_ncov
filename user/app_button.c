#include "app_button.h"
#include "proj_main.h"

struct Button btn_k1;
os_timer_t timer_k1;
extern uint8_t get_ncov_api(char *api_addr, uint8_t (*parse_fun)(void));

void app_button_init(void)
{
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_5, PORTC5_FUNC_C5);
    gpio_set_dir(GPIO_PORT_C, GPIO_BIT_5, GPIO_DIR_IN);
    system_set_port_pull(GPIO_PC5, true);
    
    mbutton_init(&btn_k1, read_btn_k1, 0);
    
//    button_attach(&btn_k1, PRESS_DOWN, button_callback);
//    button_attach(&btn_k1, PRESS_UP, button_callback);
    button_attach(&btn_k1, SINGLE_CLICK, button_callback);
   
    os_timer_init(&timer_k1, timer_k1_fun, 0);
    os_timer_start(&timer_k1, 10, true); 
    button_start(&btn_k1);
}

void timer_k1_fun(void *parg)
{
    button_ticks();
}

uint8_t read_btn_k1(void)
{
    uint8_t in;
    in = gpio_portc_read() & (1<<5);
    return in>>5;
}

void button_callback(void *button)
{
    uint32_t btn_event_val; 
    
    btn_event_val = get_button_event((struct Button *)button); 
    
    switch(btn_event_val)
    {
	    case PRESS_DOWN:
	        co_printf("---> key1 press down! <---\r\n"); 
	    	break; 
	    case PRESS_UP: 
	        co_printf("***> key1 press up! <***\r\n");
	    	break; 
	    case PRESS_REPEAT: 
	        co_printf("---> key1 press repeat! <---\r\n");
	    	break; 
	    case SINGLE_CLICK: 
	        co_printf("---> key1 single click! <---\r\n");
            get_ncov_api(api_data, parse_ncov_data);
	    	break; 
	    case DOUBLE_CLICK: 
	        co_printf("***> key1 double click! <***\r\n");
	    	break; 
	    case LONG_PRESS_START: 
	        co_printf("---> key1 long press start! <---\r\n");
	   		break; 
	    case LONG_PRESS_HOLD: 
	        co_printf("***> key1 long press hold! <***\r\n");
	    	break; 
	}
}


