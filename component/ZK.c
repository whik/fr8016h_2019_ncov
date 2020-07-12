#include "lcd.h"
#include "delay.h"

u8 FontBuf[128];//�ֿ⻺��	
/******************************************************************************
      ����˵�������ֿ�д������
      ������ݣ�dat  Ҫд�������
      ����ֵ��  ��
******************************************************************************/
void ZK_command(u8 dat)
{
	u8 i; 
	for(i=0;i<8;i++ )
	{
		OLED_SCLK_Clr();  //�ֿ�ʱ������ 
		if(dat&0x80)
			OLED_SDIN_Set();
		else 
			OLED_SDIN_Clr();
		dat = dat<<1;
		OLED_SCLK_Set();  //�ֿ�ʱ������
	}
 }

/******************************************************************************
      ����˵�������ֿ��ȡ����
      ������ݣ���
      ����ֵ��  ret_data ��ȡ������
******************************************************************************/
u8  get_data_from_ROM(void)
{
	u8 i;
	u8 ret_data=0; //�������ݳ�ʼ��
	OLED_SCLK_Set();//�ֿ�ʱ������
	for(i=0;i<8;i++)
	{
		OLED_SCLK_Clr();  //�ֿ�ʱ������  
		ret_data=ret_data<<1;
		if(ZK_OUT())
			{
				ret_data=ret_data+1;
			}
		else
			{
				ret_data=ret_data;
			}
		OLED_SCLK_Set(); //�ֿ�ʱ������
	}
	return(ret_data);    //���ض�����һ���ֽ�
}
 
/******************************************************************************
      ����˵������ȡN������
      ������ݣ�AddrHigh  д��ַ���ֽ�
                AddrMid   д��ַ���ֽ�
                AddrLow   д��ַ���ֽ�
                *pBuff    ��ȡ������
                DataLen   ��ȡ���ݵĳ���
      ����ֵ��  ��
******************************************************************************/
void get_n_bytes_data_from_ROM(u8 AddrHigh,u8 AddrMid,u8 AddrLow,u8 *pBuff,u8 DataLen)
{
 	u8 i;
	OLED_CS_Set();
	ZK_CS_Clr(); //�ֿ�Ƭѡ
	OLED_SCLK_Clr();  //�ֿ�ʱ������

	ZK_command(0x03);//дָ��
	ZK_command(AddrHigh);//д��ַ���ֽ�
	ZK_command(AddrMid);//д��ַ���ֽ�
	ZK_command(AddrLow);//д��ַ���ֽ�
	for(i = 0; i < DataLen; i++ )
	{
	   *(pBuff+i) =get_data_from_ROM();//��һ���ֽ�����
	}

	ZK_CS_Set();//ȡ���ֿ�Ƭѡ
}

/******************************************************************************
      ����˵������ʾ����
      ������ݣ�x,y      д�������
                zk_num   1:12*12,  2:15*16,  3:24*24,  4:32*32
                color    ������ɫ
      ����ֵ��  ��
******************************************************************************/
void Display_GB2312(u16 x,u16 y,u8 zk_num,u16 color)
{
  u8 i,k;
	switch(zk_num)
	{
		case 1:
			 {
		 	   LCD_Address_Set(x,y,x+15,y+11);
			   for(i=0;i<24;i++)		 
	       {
					for(k=0;k<8;k++)
					{ 		     
						if((FontBuf[i]&(0x80>>k))!=0)
						{
							LCD_WR_DATA(color);
						} 
						else
						{
							LCD_WR_DATA(BACK_COLOR);
						}   
					}
	
	       }
			 }break;  // 12*12  

		case 2: 
			 {
		 	    LCD_Address_Set(x,y,x+15,y+15);
			    for(i=0;i<32;i++)		 
	       {
						for(k=0;k<8;k++)
						 { 		     
							if((FontBuf[i]&(0x80>>k))!=0)
							 {
								LCD_WR_DATA(color);
							 } 
							else
							 {
								LCD_WR_DATA(BACK_COLOR);
							 }   
						 }
	       }
		  }break;     // 15*16 

	    case 3:  
			 {
		 	    LCD_Address_Set(x,y,x+23,y+23);
			    for(i=0;i<72;i++)		 
						{
							for(k=0;k<8;k++)
							{ 		     
								if((FontBuf[i]&(0x80>>k))!=0)
							  {
									LCD_WR_DATA(color);
								} 
								else
								{
									LCD_WR_DATA(BACK_COLOR);
								}   
							}
						
						 }
		  }break;     // 24*24  

	    case 4:  
			 {
		 	    LCD_Address_Set(x,y,x+31,y+31);
			    for(i=0;i<128;i++)		 
	       {
						for(k=0;k<8;k++)
						{ 		     
							if((FontBuf[i]&(0x80>>k))!=0)
						  {
								LCD_WR_DATA(color);
							} 
							else
							{
								LCD_WR_DATA(BACK_COLOR);
							}   
						}
	       }
		  }break;    // 32*32  
	}
}

 
/******************************************************************************
      ����˵������ʾ����
      ������ݣ�zk_num    1:12*12,  2:15*16,  3:24*24,  4:32*32
                x,y       ����
                text[]    Ҫ��ʾ�ĺ���
                color     ������ɫ
      ����ֵ��  ��
******************************************************************************/
void Display_GB2312_String(u16 x,u16 y,u8 zk_num,u8 text[],u16 color)
{
   
	u8 i= 0;
	u8 AddrHigh,AddrMid,AddrLow ; //�ָߡ��С��͵�ַ
	
	u32 FontAddr=0; //�ֵ�ַ
	u32 BaseAdd=0; //�ֿ����ַ	
	u8 n,h,w,d,p;// ��ͬ�����ֿ�ļ������
	switch(zk_num)
		{  // n������h���ָ߶ȣ�w���ֿ�ȣ� d���ּ�࣬c��ҳ��С
		  case 1 :  BaseAdd=0x00;    n=24;  h=12; w=12; d=12; p=2;  break;  // 12*12  
		  case 2 :  BaseAdd=0x2C9D0; n=32;  h=16; w=16; d=16; p=2;  break;   // 15*16  
	    case 3 :  BaseAdd=0x68190; n=72;  h=24; w=24; d=24; p=3;  break;   // 24*24  
	    case 4 :  BaseAdd=0xEDF00; n=128; h=32; w=32; d=32; p=4;  break;   // 32*32  
 		}

		while((text[i]>0x00))
		{
		    if(x>(LCD_W-d))
		      {  y=y+p; 
			     x=0; 
				 	 }//�������
		
			if(((text[i]>=0xA1) &&(text[i]<=0xA9))&&(text[i+1]>=0xA1))
			{						
				//������壨GB2312�������� �ֿ�IC�еĵ�ַ�����¹�ʽ�����㣺//
				//Address = ((MSB - 0xA1) * 94 + (LSB - 0xA1))*n+ BaseAdd; ������ȡ��ַ///
			 	FontAddr = (text[i]- 0xA1)*94; 
				FontAddr += (text[i+1]-0xA1);
				FontAddr = (unsigned long)((FontAddr*n)+BaseAdd);
				
				AddrHigh = (FontAddr&0xff0000)>>16;  //��ַ�ĸ�8λ,��24λ//
				AddrMid = (FontAddr&0xff00)>>8;      //��ַ����8λ,��24λ//
				AddrLow = FontAddr&0xff;	     //��ַ�ĵ�8λ,��24λ//
				get_n_bytes_data_from_ROM(AddrHigh,AddrMid,AddrLow,FontBuf,n );//ȡһ�����ֵ����ݣ��浽"FontBuf[]"
				Display_GB2312(x,y,zk_num,color);//��ʾһ�����ֵ�OLED��/ 
	 		}
	
			else if(((text[i]>=0xB0) &&(text[i]<=0xF7))&&(text[i+1]>=0xA1))
		 	{
				//������壨GB2312�� �ֿ�IC�еĵ�ַ�����¹�ʽ�����㣺//
				//Address = ((MSB - 0xB0) * 94 + (LSB - 0xA1)+846)*n+ BaseAdd; ������ȡ��ַ//
				FontAddr = (text[i]- 0xB0)*94; 
				FontAddr += (text[i+1]-0xA1)+846;
				FontAddr = (unsigned long)((FontAddr*n)+BaseAdd);
				
				AddrHigh = (FontAddr&0xff0000)>>16;  //��ַ�ĸ�8λ,��24λ//
				AddrMid = (FontAddr&0xff00)>>8;      //��ַ����8λ,��24λ//
				AddrLow = FontAddr&0xff;	     //��ַ�ĵ�8λ,��24λ//
				get_n_bytes_data_from_ROM(AddrHigh,AddrMid,AddrLow,FontBuf,n );//ȡһ�����ֵ����ݣ��浽"FontBuf[ ]"
				Display_GB2312(x,y,zk_num,color);//��ʾһ�����ֵ�LCD��/
		 	}
			 x=x+d; //��һ��������
		 	 i+=2;  //�¸��ַ�
			}
} 
 
/******************************************************************************
      ����˵������ʾASCII��
      ������ݣ�x,y      д�������
                zk_num   1:5*7   2:5*8   3:6*12,  4:8*16,  5:12*24,  6:16*32
                color    ������ɫ
      ����ֵ��  ��
******************************************************************************/
void Display_Asc(u16 x,u16 y,u8 zk_num,u16 color)
{
    
    unsigned char i,k;
 
		
	switch(zk_num)
	{
	    case 1:   
			 {
		 	LCD_Address_Set(x,y,x+7,y+7);
			 for(i=0;i<7;i++)		 
	{
		for(k=0;k<8;k++)
		{ 		     
		 	if((FontBuf[i]&(0x80>>k))!=0)
		{
				LCD_WR_DATA(color);
			} 
			else
			{
				LCD_WR_DATA(BACK_COLOR);
			}   
		}
	
	 }
			 }break;    //	  5x7 ASCII

	 	case 2:   	
			 {
		 	LCD_Address_Set(x,y,x+7,y+7);
			 for(i=0;i<8;i++)		 
	{
		for(k=0;k<8;k++)
		{ 		     
		 	if((FontBuf[i]&(0x80>>k))!=0)
		{
				LCD_WR_DATA(color);
			} 
			else
			{
				LCD_WR_DATA(BACK_COLOR);
			}   
		}
	
	 }
			 }break;   //	  7x8 ASCII

	    case 3:   
			 {
		 	LCD_Address_Set(x,y,x+7,y+11);
			 for(i=0;i<12;i++)		 
	{
		for(k=0;k<8;k++)
		{ 		     
		 	if((FontBuf[i]&(0x80>>k))!=0)
		{
				LCD_WR_DATA(color);
			} 
			else
			{
				LCD_WR_DATA(BACK_COLOR);
			}   
		}
	
	 }
			 }break;  //  6x12 ASCII

	    case 4:  
	 {
		 	LCD_Address_Set(x,y,x+7,y+15);
			 for(i=0;i<16;i++)		 
	{
		for(k=0;k<8;k++)
		{ 		     
		 	if((FontBuf[i]&(0x80>>k))!=0)
		{
				LCD_WR_DATA(color);
			} 
			else
			{
				LCD_WR_DATA(BACK_COLOR);
			}   
		}
	
	 }
			 }break;     //  8x16 ASCII

	    case 5:  
	 {
		 	LCD_Address_Set(x,y,x+15,y+24);
			 for(i=0;i<48;i++)		 
	{
		for(k=0;k<8;k++)
		{ 		     
		 	if((FontBuf[i]&(0x80>>k))!=0)
		{
				LCD_WR_DATA(color);
			} 
			else
			{
				LCD_WR_DATA(BACK_COLOR);
			}   
		}
	
	 }
			 }break;    //  12x24 ASCII

	    case 6:  	 
	 {
		 	LCD_Address_Set(x,y,x+15,y+31);
			 for(i=0;i<64;i++)		 
	{
		for(k=0;k<8;k++)
		{ 		     
		 	if((FontBuf[i]&(0x80>>k))!=0)
		{
				LCD_WR_DATA(color);
			} 
			else
			{
				LCD_WR_DATA(BACK_COLOR);
			}   
		}
	
	 }
			 }break;   //  16x32 ASCII
			}
}

/******************************************************************************
      ����˵������ʾASCII��
      ������ݣ�x,y      д�������
                zk_num   1:5*7   2:5*8   3:6*12,  4:8*16,  5:12*24,  6:16*32
                text[]   Ҫ��ʾ���ַ���
                color    ������ɫ
      ����ֵ��  ��
******************************************************************************/
void Display_Asc_String(u16 x,u16 y,u16 zk_num,u8 text[],u16 color)
{
	u8 i= 0;
	u8 AddrHigh,AddrMid,AddrLow ; //�ָߡ��С��͵�ַ

	u32 FontAddr=0; //�ֵ�ַ
	u32 BaseAdd=0; //�ֿ����ַ	
  u8 n,h,w,d,c;// ��ͬ�����ֿ�ļ������
	switch(zk_num)
	{	// n������h���ָ߶ȣ�w���ֿ�ȣ� d���ּ�࣬c��ҳ��С
		case 1:  BaseAdd=0x1DDF80; n=8;  h=7;  w=5;  d=6 ; c=1;  break;	 //	  5x7 ASCII
		case 2:  BaseAdd=0x1DE280; n=8;  h=8;  w=7;  d=8;  c=1;  break;	 //   7x8 ASCII
		case 3:  BaseAdd=0x1DBE00; n=12; h=12; w=6;  d=6;  c=2;  break;	 //  6x12 ASCII
	  case 4:  BaseAdd=0x1DD780; n=16; h=16; w=8;  d=8;  c=2;  break;	 //  8x16 ASCII	
	  case 5:  BaseAdd=0x1DFF00; n=48; h=24; w=12; d=12; c=3;  break;	 //  12x24 ASCII
	 	case 6:  BaseAdd=0x1E5A50; n=64; h=32; w=16; d=16; c=4;  break;	 //  16x32 ASCII

			}

	while((text[i]>0x00))
	{	
	   if(x>(LCD_W-d))   //�������
	      {  y=y+c; 
		     x=0; 
			 	 }
	   if((text[i] >= 0x20) &&(text[i] <= 0x7E))
		{						
		    FontAddr = 	text[i]-0x20;
			FontAddr = (unsigned long)((FontAddr*n)+BaseAdd);
			
			AddrHigh = (FontAddr&0xff0000)>>16;  /*��ַ�ĸ�8λ,��24λ*/
			AddrMid = (FontAddr&0xff00)>>8;      /*��ַ����8λ,��24λ*/
			AddrLow = FontAddr&0xff;	     /*��ַ�ĵ�8λ,��24λ*/
			get_n_bytes_data_from_ROM(AddrHigh,AddrMid,AddrLow,FontBuf,n );/*ȡһ�����ֵ����ݣ��浽"FontBuf[]"*/
			Display_Asc(x,y,zk_num,color);/*��ʾһ��ascii��OLED�� */
  
		}
    i+=1;  //�¸�����
	x+=d;//��һ�������� 
	}	    
}



