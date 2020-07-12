#include "app_lcd.h"

/* 240*240 */

void app_lcd_init(uint16_t bg_clr)
{
    BACK_COLOR = bg_clr;
    Lcd_Init();
    LCD_Clear(bg_clr);
}
