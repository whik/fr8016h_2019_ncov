#include "app_lcd_gui.h"
#include "font.h"
#include "math.h"
#include "string.h"
#include "stdio.h"

/* ��ֱ�ߣ�ˮƽ��ֱ���� */
void gui_drawLine(uint16_t x0, uint16_t y0, uint16_t len, uint16_t direction, uint16_t color)
{
    uint16_t x, y;
    uint16_t size_x, size_y;

    if(direction == DIR_Y)
    {
        size_y = y0 + len;
        if(len > LCD_SIZE_X)
            size_y = LCD_SIZE_X;
        for(y = y0; y < size_y; y++)
            lcd_drawPoint(x0, y, color);
    }
    else if(direction == DIR_X)
    {
        size_x = x0 + len;
        if(len > LCD_SIZE_Y)
            size_x = LCD_SIZE_Y;
        for(x = x0; x < size_x; x++)
            lcd_drawPoint(x, y0, color);
    }
}

/* ���� */
void gui_draw_point(uint16_t x0, uint16_t y0, uint16_t en, uint16_t color)
{
    if(en)
        lcd_drawPoint(x0, y0, color);
}


#ifdef FONT_6X8
/* ��ʾ6X8������ַ���idx��ѯ�ֿ����� */
void gui_show_F6X8_Char(uint16_t x0, uint16_t y0, uint16_t idx, uint16_t color)
{
    uint16_t x, y;
    for(y = y0; y < y0+8; y++)
    {
        for(x = x0; x < x0+6; x++)
        {
            if(F6X8[idx][y-y0] & (0x80 >> (x-x0)))
                lcd_drawPoint(x, y, color);
        }
    }
}

/* ��ʾ6X8������ַ���: "06-24 11:56" */
void gui_show_F6X8_String(uint16_t x0, uint16_t y0, char *str, uint16_t color)
{
    uint16_t x = x0;
    while(*str)
    {
        if(*str >= '0' && *str <= '9')
            gui_show_F6X8_Char(x, y0, *str-'0', color);
        else if(*str == '-')    //11
            gui_show_F6X8_Char(x, y0, 11, color);
        else if(*str == ':')    //10
            gui_show_F6X8_Char(x, y0, 10, color);
        else if(*str == ' ')
            gui_show_F6X8_Char(x, y0, 12, color);
        str++;
        x += 6;
    }
}
#endif
/* ��ʾ8X16������ַ�, idx��ѯ�ֿ����� */
void gui_show_F8X16_Char(uint16_t x0, uint16_t y0, uint16_t idx, uint16_t color)
{
    uint16_t x, y;

    for(y = y0; y < y0 + 16; y++)
    {
        for(x = x0; x < x0 + 8; x++)
        {
//            gui_draw_point(x, y, F8X16[idx][y-y0] & (0x80>>(x-x0)), color);
            if(F8X16[idx][y - y0] & (0x80 >> (x - x0)))
                lcd_drawPoint(x, y, color);
        }
    }
}

/* ��ʾ8X16������ַ���: "06-24 11:56" */
void gui_show_F8X16_String(uint16_t x0, uint16_t y0, char *str, uint16_t color)
{
    uint16_t x = x0;
    while(*str)
    {
        if(*str >= '0' && *str <= '9')
            gui_show_F8X16_Char(x, y0, *str-'0', color);
        else if(*str == '-')    //11
            gui_show_F8X16_Char(x, y0, 11, color);
        else if(*str == ':')    //10
            gui_show_F8X16_Char(x, y0, 10, color);
        else if(*str == ' ')
            gui_show_F8X16_Char(x, y0, 12, color);
        str++;
        x += 8;
    }
}

uint32_t my_pow(uint16_t m, uint16_t n)
{
    uint32_t result = 1;
    while(n--)result *= m;
    return result;
}

/* ��ȡ����λ�������� */
void my_getNum(long num, uint16_t *cnt, uint8_t *minusFlag)
{
    if(num < 0)
    {
        *minusFlag = 1;
        *cnt = (int)log10(-num) + 1;
    }
    else
    {
        *minusFlag = 0;
        *cnt = (int)log10(num) + 1;    
    }
}

/* ��ʾ1��Nλ�����֣�֧���з����� */
void gui_show_num(uint16_t x, uint16_t y, long num, uint16_t color)
{
    uint16_t len;
    uint8_t minusFlag;
    long num_p;
    
    uint16_t t, temp;
    uint16_t enshow = 0;
    /* ��ȡ����λ����������־ */
    my_getNum(num, &len, &minusFlag);
    num_p = (minusFlag) ? -num : num;
    
    for(t = 0; t < len; t++)
    {
        temp = (num_p / my_pow(10, len - t - 1)) % 10;
        if(enshow == 0 && t < (len - 1))
        {
            if(temp == 0) //��λΪ0����ʾ�հ�
            {
                if(minusFlag == 1)
                    gui_show_F8X16_Char(x + 8 * t + 8, y, 12, color); //��ʾ�հ�
                else
                    gui_show_F8X16_Char(x + 8 * t, y, 12, color); //��ʾ�հ�
                continue;
            }
            else
                enshow = 1;
        }
        if(minusFlag == 1)
            gui_show_F8X16_Char(x + 8 * t + 8, y, temp, color);
        else 
            gui_show_F8X16_Char(x + 8 * t, y, temp, color);
    }
    if(minusFlag == 1)
        gui_show_F8X16_Char(x, y, 11, color);       //"-"
}

/* ��ʾ16X16������ַ� */
void gui_show_F16X16_Char(uint16_t x0, uint16_t y0, uint16_t idx, uint16_t color)
{
    uint16_t x, y;

    for(y = y0; y < y0 + 16; y++)
    {
        for(x = x0; x < x0 + 16; x++)
        {
            if(x - x0 < 8)      //0-7
                gui_draw_point(x, y, FONT_16X16_TABLE[idx].table[(y - y0) * 2] & (0x80 >> (x - x0)), color);
            else
                gui_draw_point(x, y, FONT_16X16_TABLE[idx].table[(y - y0) * 2 + 1] & (0x80 >> (x - x0 - 8)), color);
        }
    }
}


void gui_show_chn_string(uint16_t x0, uint16_t y0, char *str, uint16_t clr)
{
    int x = x0;
    char ch[2]; 
    
    while(*str)
    {
        ch[0] = *str;
        ch[1] = *(str+1);
        gui_show_chn(x, y0, ch, clr);
        str += 2;
        x += 16;
    }
}

void gui_show_ncov_data(struct ncov_data china, struct ncov_data global)
{
    uint8_t y0 = 20;
    
    uint16_t CLR_FONT = WHITE;
    uint16_t CLR_NUM  = YELLOW;
    uint16_t CLR_TIME = GREEN;
    
    gui_show_bar(CLR_FONT);
    
    /* "��������" */
    gui_drawLine(0, 18, LCD_SIZE_X, DIR_X, CLR_FONT);
    gui_drawLine(0, 38, LCD_SIZE_X, DIR_X, CLR_FONT);
    gui_drawLine(0, 138, LCD_SIZE_X, DIR_X, CLR_FONT);
    gui_drawLine(0, 220, LCD_SIZE_X, DIR_X, CLR_FONT);
    gui_drawLine(0, 220, LCD_SIZE_X, DIR_X, CLR_FONT);
    gui_drawLine(0, 158, LCD_SIZE_X, DIR_X, CLR_FONT);
    
    gui_show_chn_string((LCD_SIZE_X-16*4)/2, y0, "��������", CLR_FONT);
    gui_show_line_data(y0+20, "�ִ�ȷ��", china.currentConfirmedCount, "������", china.currentConfirmedIncr, CLR_FONT, CLR_NUM);
    gui_show_line_data(y0+40, "�ۼ�ȷ��", china.confirmedCount, "������", china.confirmedIncr, CLR_FONT, CLR_NUM);
    gui_show_line_data(y0+60, "�ۼ�����", china.curedCount, "������", china.curedIncr, CLR_FONT, CLR_NUM);
    gui_show_line_data(y0+80, "�ִ���֢", china.seriousCount, "������", china.seriousIncr, CLR_FONT, CLR_NUM);
    gui_show_line_data(y0+100, "�ۼ�����", china.deadCount, "������", china.deadIncr, CLR_FONT, CLR_NUM);

    /* ȫ������ */
    gui_show_chn_string((LCD_SIZE_X-16*4)/2, 140, "ȫ������", CLR_FONT);
    gui_show_line_data(y0+140, "�ִ�ȷ��", global.currentConfirmedCount, "������", global.currentConfirmedIncr, CLR_FONT, CLR_NUM);
    gui_show_line_data(y0+160, "�ۼ�����", global.curedCount, "������", global.curedIncr, CLR_FONT, CLR_NUM);
    gui_show_line_data(y0+180, "�ۼ�����", global.deadCount, "������", global.deadIncr, CLR_FONT, CLR_NUM);
    
    gui_show_chn_string(100, 222, "������", CLR_FONT);
    gui_show_F8X16_String(150, 222, china.updateTime, CLR_TIME);
}

/* "str1": num1   "str2": num2 */
void gui_show_line_data(uint16_t y, char *str1, long num1, char *str2, long num2, uint16_t clr_str, uint16_t clr_num)
{
    gui_show_chn_string(0, y, str1, clr_str);        //�ۼ�ȷ��
    gui_show_F8X16_Char(64, y, 10, clr_str);       //: -> 10
    gui_show_num(72, y, num1, clr_num);        //12345678
    gui_show_chn_string(136, y, str2, clr_str);      //������
    gui_show_F8X16_Char(184, y, 10, clr_str);       //: -> 10
    gui_show_num(192, y, num2, clr_num);       //12345
}

void gui_show_chn(uint16_t x0, uint16_t y0, char *chn, uint16_t clr)
{
    uint8_t idx = 0;
    char *code[2]; /* �������룺��=C4DA����=B9FA */
    uint8_t size = sizeof(FONT_16X16_TABLE) / sizeof(FONT_16X16_TABLE[0]);
    /* �������֣���ȡ���� */
    for(idx = 0; idx < size; idx++)
    {
        code[0] = FONT_16X16_TABLE[idx].chn;
        code[1] = FONT_16X16_TABLE[idx].chn + 1;
        //��������һ��
        if(strcmp(code[0], chn) == 0 && strcmp(code[1], chn+1) == 0)
            break;
    }
    gui_show_F16X16_Char(x0, y0, idx, clr);
}

void gui_show_code(void)
{
    int idx;
    int arrSize = sizeof(FONT_16X16_TABLE) / sizeof(FONT_16X16_TABLE[0]);
    for(idx = 0; idx < arrSize; idx++)
    {
        //�������룺��=C4DA����=B9FA
        printf("%d-%x%x\r\n", idx, *FONT_16X16_TABLE[idx].chn, *(FONT_16X16_TABLE[idx].chn+1));
    }
}

extern const uint8_t ICON_BAR[];
void gui_show_bar(uint16_t clr)
{
    uint16_t x, y;
    for(x = 0; x < 240; x++)
    {
        for(y = 0; y < 8; y++)
        {
            gui_draw_point(x, y, ICON_BAR[x*2] & (0x80 >> y), clr);
            gui_draw_point(x, y+8, ICON_BAR[x*2+1] & (0x80 >> y), clr);
        }        
    }
}

