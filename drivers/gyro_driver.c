/**
 * Copyright (c) 2019, Freqchip
 * 
 * All rights reserved.
 * 
 * 
 */
//***********gyro driver**************

/*
 * INCLUDES
 */
#include <stdio.h>
#include <stdint.h>

#include "os_timer.h"
#include "driver_iic.h"
#include "driver_iomux.h"
#include "gyro_alg.h"
#include "driver_system.h"
#include "sys_utils.h"

#ifdef FOR_GYRO_DRIVER
os_timer_t gyroscope_loop_timer;
uint32_t gyroscope_loop_count = 0;

/******************************************************************************
      ����˵�� ����OS timer
      ������ݣ���
      ����ֵ��  ��

******************************************************************************/
void start_gyroscope_timer(void)
{
	os_timer_start(&gyroscope_loop_timer,10,1);
}

/******************************************************************************
      ����˵�� ����OS timer
      ������ݣ���
      ����ֵ��  ��

******************************************************************************/
void gyroscope_timer_loop(void * arg)
{
	gyroscope_i2c_init(GYRO_IIC_CHL);
	gyroscope_loop_count++;
	gyroscope_loop();
}

/******************************************************************************
      ����˵�� ����OS timer
      ������ݣ���
      ����ֵ��  ��

******************************************************************************/
void gyroscope_timer_init()
{
	os_timer_init(&gyroscope_loop_timer,gyroscope_timer_loop,NULL);
}

/******************************************************************************
      ����˵�� get_dt
      ������ݣ���
      ����ֵ��  ��

******************************************************************************/
uint32_t get_dt(void)
{
	uint32_t cur_tick = 0;
	cur_tick = gyroscope_loop_count;
	gyroscope_loop_count = 0;
	return cur_tick;
}

/******************************************************************************
      ����˵�� ��ʱ����
      ������ݣ�
      uint32_t delayTime  ��ʱ����  /ms
      ����ֵ��  ��

******************************************************************************/
void delay_ms(uint32_t delayTime)
{
	co_delay_100us((10*delayTime));
}

/******************************************************************************
      ����˵�� I2C��������
      ������ݣ�
      uint8_t deviceAddr,I2C�豸��ַ
      uint8_t regAddr,�Ĵ�����ַ
      uint8_t readLen,��ȡ����
      uint8_t *readBuf ��ȡbufferָ��
      ����ֵ��  ��

******************************************************************************/
void I2C_Read_NBytes(uint8_t deviceAddr,uint8_t regAddr,uint8_t readLen,uint8_t *readBuf)
{
	uint8_t i = 0;

	for(i = 0;i < readLen;i++)
	{
		iic_read_byte(GYRO_IIC_CHL,deviceAddr,(regAddr+i),&readBuf[i]);
	}
}

/******************************************************************************
      ����˵�� I2Cд������I2C_Write_NBytes_imp�޸Ĺ���I2C д����
      ������ݣ���
      ����ֵ��  ��

******************************************************************************/
void I2C_Write_NBytes(uint8_t deviceAddr,uint8_t regAddr,uint8_t writeLen,uint8_t *writeBuf)
{
	iic_write_bytes(GYRO_IIC_CHL,deviceAddr,regAddr,writeBuf,writeLen);
}

void I2C_Write_NBytes_imp(uint8_t deviceAddr,uint8_t regAddr,uint8_t writeLen,uint8_t *writeBuf)
{
	iic_write_bytes_imp(GYRO_IIC_CHL,deviceAddr,regAddr,writeBuf,writeLen);
}

void gyroscope_i2c_init(uint8_t chl)
{
	system_set_port_mux(GPIO_PORT_C,GPIO_BIT_6,PORTC6_FUNC_I2C1_CLK);
	system_set_port_mux(GPIO_PORT_C,GPIO_BIT_7,PORTC7_FUNC_I2C1_DAT);
	system_set_port_pull(GPIO_PC6,true);
	system_set_port_pull(GPIO_PC7,true);
	iic_init(GYRO_IIC_CHL,350,GYRO_ADDRESS);
}

/******************************************************************************
      ����˵�� g-sensor��ʼ����
      ������ݣ���
      ����ֵ��  ��

******************************************************************************/
void gyro_dev_init(void)
{
	printf("=gyroscope start=\r\n");
	gyroscope_i2c_init(GYRO_IIC_CHL);//I2C��ʼ��
	gyroscope_init();//g-sensor init
	gyroscope_timer_init();//����os_timer
	start_gyroscope_timer();//����os_timer
}
#endif


