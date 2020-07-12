/**
 * Copyright (c) 2019, Freqchip
 * 
 * All rights reserved.
 * 
 * 
 */

/*
 * INCLUDES
 */
#include <stdbool.h>
#include <stdio.h>

#include "capb18-001.h"
#include "co_printf.h"
#include "sys_utils.h"
#include "lcd.h"
#include "os_mem.h"
#include "driver_gpio.h"
#include "driver_iomux.h"
#include "driver_iic.h"
#include "os_timer.h"
//os_timer_t timer_CAPB18;





int CompForm2TrueForm(int CompFormdata,uint8_t Bits);
uint8_t CAPB18_data_get(float *temperature,float *air_press);

static const uint8_t CAPB18_ADDRESS = (0x77<<1);
#define COMPENSATION_FACTOR 524288 //��������Ϊsingle��Ӧ�Ĳ����������
//Register Addr
#define PSR_B2   0x00
#define PSR_B1   0x01
#define PSR_B0   0x02
#define TMP_B2   0x03
#define TMP_B1   0x04
#define TMP_B0   0x05
#define PRS_CFG  0x06
#define TMP_CFG  0x07
#define MEAS_CFG 0x08
#define CFG_REG  0x09
#define INI_STS  0x0a
#define FIFO_STS 0x0B
#define RESET    0x0c
#define ID       0x0d
#define COEF_c0  0x10

//У׼ϵ��
enum
{
    c0 =0,
    c1,
    c00,
    c10,
    c01,
    c11,
    c20,
    c21,
    c30,
    cMax
};

int32_t  COFF_data_cxx[cMax];
uint8_t CAPB18_ReadData[32][3];
uint32_t Traw_Bn,Praw_Bn;
int Traw,Praw;
float  Traw_sc,Praw_sc;
//float Tcomp,Pcomp;


//int32_t c0,c1,c00,c10,c01,c11,c20,c21,c30;
/******************************************************************************
      ����˵����IIC��ʼ��
      ������ݣ���
      ����ֵ��  ��

******************************************************************************/

void CAPB18_I2C_init(void)
{
//  iic_init(enum iic_channel_t channel, uint16_t speed, uint16_t slave_addr)
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_6, PORTC6_FUNC_I2C1_CLK);//Pc6
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_7, PORTC7_FUNC_I2C1_DAT);//Pc7
    iic_init(IIC_CHANNEL_1,1000,0);


}
/******************************************************************************
      ����˵������ȡCAPB18 ID����,�鿴�Ƿ����
      ������ݣ���
      ����ֵ��
        true���ɹ�
        false��ʧ��
******************************************************************************/

uint8_t CAPB18_measure(void)
{
    uint8_t CAPB18_id =0,set=0;

    set = iic_read_byte(IIC_CHANNEL_1, CAPB18_ADDRESS, ID, &CAPB18_id);
    if((set == true)&&(CAPB18_id ==0x10))
    {
        co_printf(" CAPB18_id = %d\r\n",CAPB18_id);
        return set;
    }
    return false;
}
/******************************************************************************
      ����˵������ȡCAPB18У׼ϵ������
      ������ݣ���
      ����ֵ��
        true���ɹ�
        false��ʧ��
******************************************************************************/

uint8_t CAPB18_COFF_GET(void)
{

    uint8_t set=0,i=0;
    uint8_t Reg_Meas = 0;
    uint8_t *COFF_data = (uint8_t *)os_malloc(20);
    //set = iic_read_bytes(IIC_CHANNEL_1, CAPB18_ADDRESS, COEF_c0, COFF_data,18);//����Ῠ��  ������ԭ��2019-12-12
    if( iic_read_byte(IIC_CHANNEL_1, CAPB18_ADDRESS, MEAS_CFG, &Reg_Meas))
    {
        if((Reg_Meas & 0xc0) == 0xc0)
        {
            co_printf("CAPB18 init Ready  Reg_Meas =0x%x\r\n",Reg_Meas);
        }
        else
        {
            co_delay_100us(10);
            if(iic_read_byte(IIC_CHANNEL_1, CAPB18_ADDRESS, MEAS_CFG, &Reg_Meas))
            {
                if((Reg_Meas & 0xc0) == 0xc0)
                {
                    co_printf("CAPB18 init Ready  Reg_Meas =0x%x\r\n",Reg_Meas);
                }
                else
                {
                    co_printf("CAPB18 init fail  Reg_Meas =0x%x\r\n",Reg_Meas);
                }
            }
        }
    }
    else
    {
        co_printf("MEAS_CFG Read Fail\r\n");
    }

    for(i = 0; i < 18; i++)
    {
        set = iic_read_byte(IIC_CHANNEL_1, CAPB18_ADDRESS, COEF_c0+i, &COFF_data[i]);
        if(set == false)
            return set;
		//co_printf("Coff_data[%d] = %x\r\n",i,COFF_data[i]);
    }

    COFF_data_cxx[c0] = (COFF_data[0]<<4) | (COFF_data[1]>>4);
    COFF_data_cxx[c1] = ((COFF_data[1]&0x0f)<<8) | (COFF_data[2]);

    COFF_data_cxx[c00] = ((COFF_data[3])<<12) | (COFF_data[4]<<4)| (COFF_data[5]>>4);
    COFF_data_cxx[c10] = ((COFF_data[5]&0x0f)<<16) | (COFF_data[6]<<8) | COFF_data[7];
    COFF_data_cxx[c01] =  (COFF_data[8]<<8) | COFF_data[9];
    COFF_data_cxx[c11] =  (COFF_data[10]<<8) | COFF_data[11];
    COFF_data_cxx[c20] =  (COFF_data[12]<<8) | COFF_data[13];
    COFF_data_cxx[c21] =  (COFF_data[14]<<8) | COFF_data[15];
    COFF_data_cxx[c30] =  (COFF_data[16]<<8) | COFF_data[17];
    os_free(COFF_data);

    for( i = c0; i<cMax; i++)
    {
        if((i==c0) ||(i==c1))
            COFF_data_cxx[i] = CompForm2TrueForm(COFF_data_cxx[i],11);//����λ11
        else if((i==c00) ||(i==c10))
            COFF_data_cxx[i] = CompForm2TrueForm(COFF_data_cxx[i],19);//����λ19
        else
            COFF_data_cxx[i] = CompForm2TrueForm(COFF_data_cxx[i],15);//����λ15
    }
    return set;
}
/******************************************************************************
      ����˵������ȡCAPB18FIFO����
      ������ݣ���
      ����ֵ��
        bit[1]:  0-Not Full     1- Full
        bit[0]: 0- non-empty    1- empty
******************************************************************************/
uint8_t CAPB18_FIFO_STATE_GET(void)
{
    uint8_t Reg_FIFO_sts = 0;
    iic_read_byte(IIC_CHANNEL_1, CAPB18_ADDRESS, FIFO_STS, &Reg_FIFO_sts);
    return Reg_FIFO_sts;
}




/*
//CAPB18 ��ʱ��ȡ���� �ص�����

uint8_t LCD_ShowStringBuff_TmpPrs[32];
void timer_CAPB18_FUN(void *arg)
{
    CAPB18_data_get(&Tcomp,&Pcomp);
	sprintf((char*)LCD_ShowStringBuff_TmpPrs,"PRS:%7.5f",Pcomp);
    co_printf("%s\r\n",LCD_ShowStringBuff_TmpPrs);
    LCD_ShowString(80,135,LCD_ShowStringBuff_TmpPrs,BLUE);

	 sprintf((char*)LCD_ShowStringBuff_TmpPrs,"TMP:%7.5f",Tcomp);
    co_printf("%s\r\n",LCD_ShowStringBuff_TmpPrs);
    LCD_ShowString(80,90,LCD_ShowStringBuff_TmpPrs,BLUE);



}*/
/******************************************************************************
      ����˵����CAPB18_data_get����
      ������ݣ�temperature���¶�����ָ��  ��   air_press����ѹ����ָ��
      ����ֵ��

******************************************************************************/

uint8_t CAPB18_data_get(float *temperature,float *air_press)
{
    uint8_t i=0,j=0;
	CAPB18_I2C_init();
	 if(CAPB18_measure()==false)
    {
        co_printf("CAPB18 get ID false\r\n");
        return false;
    }
    while(!(CAPB18_FIFO_STATE_GET()&0x01)) //��ȡFIFOȫ�����ݣ�ֱ��FIFOΪ��
    {
				//	co_printf("CAPB18_FIFO_STATE_GET %x\r\n",CAPB18_FIFO_STATE_GET());
			for(i=0; i<3; i++)
        {
            iic_read_byte(IIC_CHANNEL_1, CAPB18_ADDRESS, PSR_B2+i, &CAPB18_ReadData[j][i]);
        }
        j++;
    }
	//	co_printf("CAPB18_data_get %d\r\n",j);
    for(i=0; i<j; i++)
    {
        if(CAPB18_ReadData[i][2] & 0x01) //��ѹ����
        {
            Praw_Bn = (CAPB18_ReadData[i][0]<<16) | (CAPB18_ReadData[i][1]<<8) | CAPB18_ReadData[i][2];
            Praw =  CompForm2TrueForm((int)Praw_Bn,23);
            Praw_sc = (float)Praw/COMPENSATION_FACTOR;
            *air_press =  COFF_data_cxx[c00] + Praw_sc*(COFF_data_cxx[c10] + Praw_sc*(COFF_data_cxx[c20] + Praw_sc*COFF_data_cxx[c30]))
                     + Traw_sc*COFF_data_cxx[c01] + Traw_sc*Praw_sc*(COFF_data_cxx[c11]+Praw_sc*COFF_data_cxx[c21]);

        }
        else  //�¶�����
        {
            Traw_Bn = (CAPB18_ReadData[i][0]<<16) | (CAPB18_ReadData[i][1]<<8) | CAPB18_ReadData[i][2];
            Traw = CompForm2TrueForm((int)Traw_Bn,23);
            Traw_sc = (float)Traw/COMPENSATION_FACTOR;
            *temperature = COFF_data_cxx[c0]*0.5;// + 10*Traw_sc*COFF_data_cxx[c1];//��֪��Ϊʲô���� �Ͳ�����
            *temperature = *temperature+Traw_sc*COFF_data_cxx[c1];

        }


    }
	return true;


}

/******************************************************************************
      ����˵����CAPB18demo��ں���������I2C�� CAPB18��ʼ��
      ������ݣ���
      ����ֵ��  ��ʼ���ɹ����� true    ���򷵻�false

******************************************************************************/

uint8_t demo_CAPB18_APP(void)
{
    uint8_t i = 0;

    CAPB18_I2C_init();//I2C��ʼ��
    if(CAPB18_measure()==false)//��ȡCAPB18��IDֵ���ж��Ƿ���ȷ
    {
        co_printf("CAPB18 get ID false\r\n");
        return false;
    }
    if(CAPB18_COFF_GET()==false)//��ȡCAPB18��У׼ϵ��
    {
        co_printf("CAPB18 get COFF false\r\n");
        return false;
    }
    else
    {
        for( i = 0; i<cMax; i++)
            co_printf("%x ",COFF_data_cxx[i]);
        co_printf("\r\n");
    }

	//CAPB18�Ĵ�������
    iic_write_byte(IIC_CHANNEL_1, CAPB18_ADDRESS, PRS_CFG,0);//��ѹ�������� 1 Hz   �������� Single
    iic_write_byte(IIC_CHANNEL_1, CAPB18_ADDRESS, TMP_CFG,0x80);//�¶Ȳ������� 1 Hz   �������� Single
    iic_write_byte(IIC_CHANNEL_1, CAPB18_ADDRESS, MEAS_CFG,0x07);//����ѹ�����¶Ȳ���
    iic_write_byte(IIC_CHANNEL_1, CAPB18_ADDRESS, CFG_REG,0x02);//ʹ��FIFO
    iic_write_byte(IIC_CHANNEL_1, CAPB18_ADDRESS, RESET,0x80);//���FIFO


 //   os_timer_init(&timer_CAPB18,timer_CAPB18_FUN,NULL);//ÿ1s����һ����ʪ��
 //   os_timer_start(&timer_CAPB18,1000,1);

    return true;

}



/******************************************************************************
      ����˵��������λ������ת��32bit���뺯��
      ������ݣ�
      CompFormdata:���룬
      Bits������λ��������λ
      ����ֵ��  32bit ����

******************************************************************************/
int CompForm2TrueForm(int CompFormdata,uint8_t Bits)
{
    int32_t Bit_symbol = 0;
    Bit_symbol = CompFormdata & BIT(Bits);
    if(Bit_symbol)
    {
        //return (  0-(Bit_symbol - (CompFormdata & (Bit_symbol-1))));//0-(Bit_symbol - (CompFormdata & (Bit_symbol-1)))
        return((0xffffffff<<Bits) | CompFormdata);
    }
    else
    {
        return  CompFormdata;
    }
}



