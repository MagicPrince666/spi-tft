/***************************************************************************************

 * 本程序只供学习使用，未经作者许可，不得用于其它任何用途 *
 * 文件名  LCD.cpp
 * 描述    ：液晶驱动
 * 平台    ：linux
 * 版本    ：V1.0.0
 * 作者    ：小王子与木头人  QQ：846863428
 *修改时间  ：2017-07-4

*****************************************************************************************/
#include <unistd.h>
#include <stdio.h>
#include "font.h"
#include "LCD.h"
#include "spi.h"
#include "pwmconfig.h"
#include "touch.h"

#define SPI



uint8_t *SPI_LCD_RAM;



_lcd_dev lcddev;



uint16_t BACK_COLOR, POINT_COLOR;   //背景色，画笔色

//写8bit数据

void LCD_Writ_Bus(uint8_t da)   //串行数据写入

{		

#ifdef SPI

//	SPI_Write(&da,1);

	write(g_SPI_Fd, &da, 1);

#else

	for(int i = 0; i < 8; i++)

	{

		if((da << i) & 0x80)

			LCD_SDI_1;

		else

			LCD_SDI_0;	

		

		LCD_SCK_0;

		LCD_SCK_1;		

	}

#endif

} 



#ifndef SPI

//写16bit数据

void LCD_Writ_Bus16(uint16_t da)   //串行数据写入

{		

	for(int i = 0; i < 16; i++)

	{

		if((da << i) & 0x8000)

			LCD_SDI_1;

		else

			LCD_SDI_0;	

		

		LCD_SCK_0;

		LCD_SCK_1;	

	}

} 

#endif

//读8bit数据

uint8_t LCD_Read_Bus()   

{

	

	uint8_t ret = 0;		

#ifdef SPI

	SPI_Read(&ret,1);

#else

	LCD_CS_0;

	for(int i = 0; i < 8; i++)

	{	

		LCD_SCK_1;	

		LCD_SCK_0;

		ret <<= 1;

		if(LCD_SDO) ret |= 0x01;			

	}

	LCD_CS_1;

#endif

	return ret;

} 



#ifndef SPI 

//写16bit数据

uint16_t LCD_Read_Bus16(void) 

{

	LCD_CS_0;

	uint16_t ret = 0;		

	for(int i = 0; i < 16; i++)

	{	

		LCD_SCK_1;

		LCD_SCK_0;	

		ret <<= 1;

		if(LCD_SDO) ret |= 0x01;		

	}

	LCD_CS_1;

	return ret;

} 

#endif



void LCD_WR_DATA8(char da) //发送数据-8位参数

{

#ifdef SPI

	LCD_DC_1;

    LCD_Writ_Bus(da); 

#else

	LCD_CS_0;

    LCD_DC_1;

	LCD_Writ_Bus(da);  

	LCD_CS_1;

#endif

} 



//#define LCD_WR_DATA8 LCD_WR_DATA



 void LCD_WR_DATA(uint16_t da)

{

#ifdef SPI

	LCD_DC_1;



	/* 小端系统没办法 */

	write(g_SPI_Fd, (uint8_t *)&da + 1,1);

	write(g_SPI_Fd, (uint8_t *)&da,1);



	/* 大端系统可以使用这条进行优化 */

//	write(g_SPI_Fd, (uint8_t *)&da,2);

#else

	LCD_CS_0;

	LCD_DC_1;

	LCD_Writ_Bus16(da);

	LCD_CS_1;

#endif	

}	



#ifdef SPI

void LCD_Fast_WR_DATA16(uint16_t* Color ,int len)

{

	uint16_t j,k;

	SPI_LCD_RAM = (uint8_t *)malloc(2 * len);	

	k = len*2 / 32;

	for (uint16_t i = 0;i < len; i++)

	{

		SPI_LCD_RAM[ 2*i ] = Color[i] >> 8;//高位

		SPI_LCD_RAM[ 2*i + 1 ] = Color[i]; //低位

	}

	LCD_DC_1;

	for(j = 0; j < k ;j++)//刷新一行数据

		write(g_SPI_Fd, SPI_LCD_RAM + 32*j, 32);	 

	write(g_SPI_Fd, SPI_LCD_RAM + 32*j, len*2 % 32);//最后写入不满32字节的包



	free(SPI_LCD_RAM);		 

}  

#endif



void LCD_WR_REG(char da)	 

{	

#ifdef SPI

	LCD_DC_0;

	LCD_Writ_Bus(da);

#else

	LCD_CS_0;

    LCD_DC_0;

	LCD_Writ_Bus(da);

	LCD_CS_1;

#endif

}

 void LCD_WR_REG_DATA(uint16_t reg,uint16_t da)

{

    LCD_WR_REG(reg);

	LCD_WR_DATA(da);

}

//读LCD数据

uint16_t LCD_RD_DATA(void)

{

#ifndef SPI 

	return LCD_Read_Bus16();

#else

	uint16_t ret = LCD_Read_Bus() <<8 ;

	ret |= LCD_Read_Bus();

	return ret;

#endif

}

//读寄存器数据

uint16_t LCD_ReadReg(uint16_t LCD_Reg)

{

	LCD_WR_REG(LCD_Reg);

	usleep(20);

	return LCD_RD_DATA();

}



//开始写GRAM

void LCD_WriteRAM_Prepare(void)

{

	LCD_WR_REG(lcddev.wramcmd);  

}



//LCD写GRAM

void LCD_WriteRAM(uint16_t RGB_Code)

{

	LCD_WR_DATA(RGB_Code);  

}



//从ILI93xx读出的数据为GBR格式，而我们写入的时候为RGB格式。

//通过该函数转换

//c:GBR格式的颜色值

//返回值：RGB格式的颜色值

uint16_t LCD_BGR2RGB(uint16_t c)

{

	uint16_t  r,g,b,rgb;   

	b=(c>>0)&0x1f;

	g=(c>>5)&0x3f;

	r=(c>>11)&0x1f;	 

	rgb=(b<<11)+(g<<5)+(r<<0);		 

	return(rgb);

} 



void Address_set(unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2)

{ 

	LCD_WR_REG(0x2a);

	LCD_WR_DATA(x1);

	LCD_WR_DATA(x2);

	

	LCD_WR_REG(0x2b);

	LCD_WR_DATA(y1);

	LCD_WR_DATA(y2);



	LCD_WR_REG(0x2C);					 						 

}

//设置光标位置

//Xpos:横坐标

//Ypos:纵坐标

void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)

{

    LCD_WR_REG(lcddev.setxcmd); 

	LCD_WR_DATA(Xpos);

	//LCD_WR_DATA8(Xpos >> 8); 

	//LCD_WR_DATA8(Xpos & 0XFF);	 

	LCD_WR_REG(lcddev.setycmd); 

	LCD_WR_DATA(Ypos);

	//LCD_WR_DATA8(Ypos >> 8); 

	//LCD_WR_DATA8(Ypos & 0XFF);

} 



//设置LCD的自动扫描方向

//注意:其他函数可能会受到此函数设置的影响(尤其是9341/6804这两个奇葩),

//所以,一般设置为L2R_U2D即可,如果设置为其他扫描方式,可能导致显示不正常.

//dir:0~7,代表8个方向(具体定义见lcd.h)

//9320/9325/9328/4531/4535/1505/b505/8989/5408/9341/5310/5510等IC已经实际测试	   	   

void LCD_Scan_Dir(uint8_t dir)

{

	uint16_t regval=0;

	uint16_t dirreg=0;

	uint16_t temp;  

	switch(dir)

	{

		case L2R_U2D://从左到右,从上到下

			regval|=(0<<7)|(0<<6)|(0<<5); 

			break;

		case L2R_D2U://从左到右,从下到上

			regval|=(1<<7)|(0<<6)|(0<<5); 

			break;

		case R2L_U2D://从右到左,从上到下

			regval|=(0<<7)|(1<<6)|(0<<5); 

			break;

		case R2L_D2U://从右到左,从下到上

			regval|=(1<<7)|(1<<6)|(0<<5); 

			break;	 

		case U2D_L2R://从上到下,从左到右

			regval|=(0<<7)|(0<<6)|(1<<5); 

			break;

		case U2D_R2L://从上到下,从右到左

			regval|=(0<<7)|(1<<6)|(1<<5); 

			break;

		case D2U_L2R://从下到上,从左到右

			regval|=(1<<7)|(0<<6)|(1<<5); 

			break;

		case D2U_R2L://从下到上,从右到左

			regval|=(1<<7)|(1<<6)|(1<<5); 

			break;	 

			  

		LCD_WR_REG_DATA(dirreg,regval);

 		if((regval&0X20)||lcddev.dir==1)

		{

			if(lcddev.width<lcddev.height)//交换X,Y

			{

				temp=lcddev.width;

				lcddev.width=lcddev.height;

				lcddev.height=temp;

 			}

		}else  

		{

			if(lcddev.width>lcddev.height)//交换X,Y

			{

				temp=lcddev.width;

				lcddev.width=lcddev.height;

				lcddev.height=temp;

 			}

		}  



		LCD_WR_REG(lcddev.setxcmd); 

		LCD_WR_DATA(0);

		LCD_WR_DATA(lcddev.width-1);

		LCD_WR_REG(lcddev.setycmd); 

		LCD_WR_DATA(0);

		LCD_WR_DATA(lcddev.height-1);  

  	}

}   



//画点

//x,y:坐标

//POINT_COLOR:此点的颜色

void LCD_DrawPoint(uint16_t x,uint16_t y,uint16_t color)

{

	LCD_SetCursor(x,y);		//设置光标位置 

	LCD_WriteRAM_Prepare();	//开始写入GRAM

	LCD_WR_DATA(color); 

} 



//快速画点

//x,y:坐标

//color:颜色

void LCD_Fast_DrawPoint(uint16_t x,uint16_t y,uint16_t color)

{	   

	LCD_WR_REG(lcddev.setxcmd); 

	LCD_WR_DATA(x);

//	LCD_WR_DATA8(x>>8); 

//	LCD_WR_DATA8(x&0XFF);	 

	LCD_WR_REG(lcddev.setycmd); 

	LCD_WR_DATA(y);

//	LCD_WR_DATA8(y>>8); 

//	LCD_WR_DATA8(y&0XFF);

	

	LCD_WR_REG_DATA(lcddev.wramcmd,color);		 

}	



//设置LCD显示方向

//dir:0,竖屏；1,横屏

void LCD_Display_Dir(uint8_t dir)

{

	if(dir==0)			//竖屏

	{

		lcddev.dir=0;	//竖屏

		lcddev.width=240;

		lcddev.height=320;

		

		lcddev.wramcmd=0X2C;

		lcddev.setxcmd=0X2A;

		lcddev.setycmd=0X2B;  	 

		

	}else 				//横屏

	{	  				

		lcddev.dir=1;	//横屏

		lcddev.width=320;

		lcddev.height=240;

		

		lcddev.wramcmd=0X2C;

		lcddev.setxcmd=0X2A;

		lcddev.setycmd=0X2B;  	 		

		

	} 

	LCD_Scan_Dir(DFT_SCAN_DIR);	//默认扫描方向

}



//设置窗口,并自动设置画点坐标到窗口左上角(sx,sy).

//sx,sy:窗口起始坐标(左上角)

//width,height:窗口宽度和高度,必须大于0!!

//窗体大小:width*height.

//68042,横屏时不支持窗口设置!! 

void LCD_Set_Window(uint16_t sx,uint16_t sy,uint16_t width,uint16_t height)

{    

	width=sx+width-1;

	height=sy+height-1;

	

	LCD_WR_REG(lcddev.setxcmd); 

	LCD_WR_DATA(sx);	 

	LCD_WR_DATA(width);   

	LCD_WR_REG(lcddev.setycmd); 

	LCD_WR_DATA(sy); 

	LCD_WR_DATA(height); 

	

} 

//读取个某点的颜色值	 

//x,y:坐标

//返回值:此点的颜色

uint16_t LCD_ReadPoint(uint16_t x,uint16_t y)

{

 	uint16_t r=0,g=0,b=0;

	uint16_t LCD_RAM;

	if(x>=lcddev.width||y>=lcddev.height)return 0;	//超过了范围,直接返回		   

	LCD_SetCursor(x,y);	    

	LCD_WR_REG(0X2E);//9341/6804/3510 发送读GRAM指令



	LCD_RAM = LCD_RD_DATA();	

	printf("point date:0x%x\n",LCD_RAM);   



	if(LCD_RAM)r=0;							//dummy Read	     

 	r=LCD_RAM;  		  						//实际坐标颜色

 		  

	b = LCD_RAM; 

	g = r&0XFF;		//对于9341/5310/5510,第一次读取的是RG的值,R在前,G在后,各占8位

	g <<= 8;

	

	return (((r>>11)<<11)|((g>>10)<<5)|(b>>11));

}	



//LCD开启显示

void LCD_DisplayOn(void)

{					   

	LCD_WR_REG(0x29);

}	 

//LCD关闭显示

void LCD_DisplayOff(void)

{	   

	LCD_WR_REG(0x28);

}  



void Lcd_Init(void)

{

   //GPIO参考 佐须之男 的博客

    int MyPeriod = 1000000; //period 设置 10ms

	float rate = 0.5;

    int MyDuty = MyPeriod * rate;



	/*export corresponding PWM Channel*/

	if(pwm_export(1) < 0){

		printf("PWM export error!\n");

	}

	if(pwm_disable(1) < 0){

		printf("PWM disable error!\n");

	}

	/* set period and duty cycle time in ns */

	if(pwm_config(1, MyPeriod, MyDuty) < 0){

		printf("PWM configure error!\n");

	}

	/* enable corresponding PWM Channel */

	if(pwm_enable(1) < 0){

		printf("PWM enable error!\n");

	}

	//printf("PWM_A successfully enabled with period - %dms, duty cycle - %2.1f%%\n", MyPeriod/1000000, rate*100);



	if (gpio_mmap())

		printf("error\n");



	mt76x8_gpio_set_pin_direction(42, 1);

	mt76x8_gpio_set_pin_direction(11, 1);

	mt76x8_gpio_set_pin_direction(17, 1);

//	mt76x8_gpio_set_pin_direction(14, 1);

//	mt76x8_gpio_set_pin_direction(15, 1);

//	mt76x8_gpio_set_pin_direction(16, 0);

//	mt76x8_gpio_set_pin_value(16, 1);



	LCD_SCK_0;



	spistar();//模拟SPI初始化



	SPI_Open();



	lcddev.dir = 0;	//竖屏

	lcddev.width = 240;

	lcddev.height = 320;

	lcddev.id = 0X9341;

	lcddev.wramcmd = 0X2C;

	lcddev.setxcmd = 0X2A;

	lcddev.setycmd = 0X2B;



	LCD_REST_0;

	usleep(5 * 1000);

	LCD_REST_1;



	usleep(50 * 1000);

	LCD_WR_REG_DATA(0x00,0x0001);

	usleep(50 * 1000);

	lcddev.id = LCD_ReadReg(0x00);//SPI频死活读不了ID......



	if(lcddev.id<0XFF||lcddev.id==0XFFFF||lcddev.id==0X9300)//读到ID不正确,新增lcddev.id==0X9300判断，因为9341在未被复位的情况下会被读成9300

	{	

 		//尝试9341 ID的读取		

		LCD_WR_REG(0XD3);				   

		LCD_Read_Bus(); 				//dummy read 	

 		LCD_Read_Bus();   	    	//读到0X00

  		lcddev.id=LCD_Read_Bus();   	//读取93								   

 		lcddev.id<<=8;

		lcddev.id|=LCD_Read_Bus();  	//读取41 	   			   

 		if(lcddev.id!=0X9341)		//非9341,尝试是不是6804

		{	

			printf("LCD ID:%x not 0x9341\n",lcddev.id);

 			LCD_WR_REG(0XBF);				   

			LCD_Read_Bus(); 			//dummy read 	 

	 		LCD_Read_Bus();   	    //读回0X01			   

	 		LCD_Read_Bus(); 			//读回0XD0 			  	

	  		lcddev.id=LCD_Read_Bus();//这里读回0X68 

			lcddev.id<<=8;

	  		lcddev.id|=LCD_Read_Bus();//这里读回0X04	  

			if(lcddev.id!=0X6804)	//也不是6804,尝试看看是不是NT35310

			{ 

				printf("LCD ID:%x not 0x6804\n",lcddev.id);

				LCD_WR_REG(0XD4);				   

				LCD_Read_Bus(); 				//dummy read  

				LCD_Read_Bus();   			//读回0X01	 

				lcddev.id=LCD_Read_Bus();	//读回0X53	

				lcddev.id<<=8;	 

				lcddev.id|=LCD_Read_Bus();	//这里读回0X10	



				if(lcddev.id!=0X5310)		//也不是NT35310,尝试看看是不是NT35510

				{

					printf("LCD ID:%x not 0x5310\n",lcddev.id);

					LCD_WR_REG(0XDA);	

					LCD_WR_REG(0X00);

					LCD_Read_Bus();   		//读回0X00	 

					LCD_WR_REG(0XDB);	

					LCD_WR_REG(0X00);	

					lcddev.id=LCD_Read_Bus();//读回0X80

					lcddev.id<<=8;	

					LCD_WR_REG(0XDC);	

					LCD_WR_REG(0X00);	

					lcddev.id|=LCD_Read_Bus();//读回0X00		

					if(lcddev.id==0x8000)lcddev.id=0x5510;//NT35510读回的ID是8000H,为方便区分,我们强制设置为5510

				}

			}

 		}  	

	}



	printf("LCD ID:%x\n",lcddev.id);



	LCD_WR_REG(0xCF);  

    LCD_WR_DATA8(0x00); 

    LCD_WR_DATA8(0XC1); 

    LCD_WR_DATA8(0X30); 



	LCD_WR_REG(0xED);  

    LCD_WR_DATA8(0x64); 

    LCD_WR_DATA8(0x03); 

    LCD_WR_DATA8(0X12); 

    LCD_WR_DATA8(0X81); 



	LCD_WR_REG(0xE8);  

    LCD_WR_DATA8(0x85); 

    LCD_WR_DATA8(0x00); 

    LCD_WR_DATA8(0x78); 



 	LCD_WR_REG(0xCB);  

    LCD_WR_DATA8(0x39); 

    LCD_WR_DATA8(0x2C); 

    LCD_WR_DATA8(0x00); 

    LCD_WR_DATA8(0x34); 

    LCD_WR_DATA8(0x02); 

 	

	LCD_WR_REG(0xF7);  

    LCD_WR_DATA8(0x20); 



	LCD_WR_REG(0xEA);  

    LCD_WR_DATA8(0x00); 

    LCD_WR_DATA8(0x00); 

 	

    LCD_WR_REG(0xC0);    //Power control 

    LCD_WR_DATA8(0x23);   //VRH[5:0] 

   

    LCD_WR_REG(0xC1);    //Power control 

    LCD_WR_DATA8(0x10);   //SAP[2:0];BT[3:0] 

 

    LCD_WR_REG(0xC5);    //VCM control 

    LCD_WR_DATA8(0x3e); //对比度调节

    LCD_WR_DATA8(0x28); 

 

    LCD_WR_REG(0xC7);    //VCM control2 

    LCD_WR_DATA8(0x86);  //--

 

    LCD_WR_REG(0x36);    // Memory Access Control 

    LCD_WR_DATA8(0x48); //	   //48 68竖屏//28 E8 横屏



    LCD_WR_REG(0x3A);    

    LCD_WR_DATA8(0x55); 



    LCD_WR_REG(0xB1);    

    LCD_WR_DATA8(0x00);  

    LCD_WR_DATA8(0x18); 

 

    LCD_WR_REG(0xB6);    // Display Function Control 

    LCD_WR_DATA8(0x08); 

    LCD_WR_DATA8(0x82);

    LCD_WR_DATA8(0x27);  

 	

	LCD_WR_REG(0xF2);    // 3Gamma Function Disable 

    LCD_WR_DATA8(0x00); 

 	

	LCD_WR_REG(0x26);    //Gamma curve selected 

    LCD_WR_DATA8(0x01); 

 

 	LCD_WR_REG(0xE0);    //Set Gamma 

    LCD_WR_DATA8(0x0F); 

    LCD_WR_DATA8(0x31); 

    LCD_WR_DATA8(0x2B); 

    LCD_WR_DATA8(0x0C); 

    LCD_WR_DATA8(0x0E); 

    LCD_WR_DATA8(0x08); 

    LCD_WR_DATA8(0x4E); 

    LCD_WR_DATA8(0xF1); 

    LCD_WR_DATA8(0x37); 

    LCD_WR_DATA8(0x07); 

    LCD_WR_DATA8(0x10); 

    LCD_WR_DATA8(0x03); 

    LCD_WR_DATA8(0x0E); 

    LCD_WR_DATA8(0x09); 

    LCD_WR_DATA8(0x00); 



    LCD_WR_REG(0XE1);    //Set Gamma 
   LCD_WR_DATA8(0x00); 

    LCD_WR_DATA8(0x0E); 

    LCD_WR_DATA8(0x14); 

    LCD_WR_DATA8(0x03); 

    LCD_WR_DATA8(0x11); 

    LCD_WR_DATA8(0x07); 

    LCD_WR_DATA8(0x31); 

    LCD_WR_DATA8(0xC1); 

    LCD_WR_DATA8(0x48); 

    LCD_WR_DATA8(0x08); 

    LCD_WR_DATA8(0x0F); 

    LCD_WR_DATA8(0x0C); 

    LCD_WR_DATA8(0x31); 

    LCD_WR_DATA8(0x36); 

    LCD_WR_DATA8(0x0F); 



	LCD_WR_REG(0x2B); 

	LCD_WR_DATA8(0x00);

	LCD_WR_DATA8(0x00);

	LCD_WR_DATA8(0x01);

	LCD_WR_DATA8(0x3f);

	

	LCD_WR_REG(0x2A); 

	LCD_WR_DATA8(0x00);

	LCD_WR_DATA8(0x00);

	LCD_WR_DATA8(0x00);

	LCD_WR_DATA8(0xef);



    LCD_WR_REG(0x11);    //Exit Sleep 

    

	usleep(120 * 1000); 



    LCD_WR_REG(0x29);    //Display on 

    LCD_WR_REG(0x2c); 



	LCD_Display_Dir(0);

}



//清屏函数

//Color:要清屏的填充色

#ifdef SPI

void LCD_Clear(uint16_t Color)

{

	uint16_t i,j,k;

	SPI_LCD_RAM = (uint8_t *)malloc(2 * lcddev.width);	



	Address_set(0,0,lcddev.width - 1,lcddev.height - 1);

	LCD_DC_1;

    for(i = 0;i < lcddev.height; i++)//逐行刷新

	{

	  for (j = 0 ;j < lcddev.width; j++)

	  	{

			SPI_LCD_RAM[ 2*j ] = Color >> 8;//高位

			SPI_LCD_RAM[ 2*j + 1 ] = Color; //低位

		}

		for(k = 0; k < 15 ;k++)//刷新一行数据

			write(g_SPI_Fd, SPI_LCD_RAM + 32*k, 32);

	}	 



	free(SPI_LCD_RAM);		 

}

#else

void LCD_Clear(uint16_t Color)

{

	uint16_t i,j;  	

	Address_set(0,0,lcddev.width-1,lcddev.height-1);

    for(i = 0;i < lcddev.width;i++)

	  for (j = 0;j < lcddev.height;j++)

    	LCD_WR_DATA(Color);	 			 

}

#endif

//在指定位置显示一个汉字(32*33大小)

//dcolor为内容颜色，gbcolor为背静颜色

void showhanzi(unsigned int x,unsigned int y,unsigned char index)	

{  

	unsigned char i,j;

	unsigned char *temp = hanzi;    

    Address_set(x,y,x+31,y+31); //设置区域      

	temp+=index*128;	

	for(j=0;j<128;j++)

	{

		for(i=0;i<8;i++)

		{ 		     

		 	if((*temp&(1<<i))!=0)	

				LCD_WR_DATA(POINT_COLOR);

			else

				LCD_WR_DATA(BACK_COLOR); 

		}

		temp++;

	 }

}

//画点

//POINT_COLOR:此点的颜色

void LCD_DrawPoint(uint16_t x,uint16_t y)

{

	Address_set(x,y,x,y);//设置光标位置 

	LCD_WR_DATA(POINT_COLOR); 	    

} 	 

//画一个大点

//POINT_COLOR:此点的颜色

void LCD_DrawPoint_big(uint16_t x,uint16_t y)

{

	LCD_Fill(x-1,y-1,x+1,y+1,POINT_COLOR);

} 

//在指定区域内填充指定颜色

//区域大小:

//  (xend-xsta)*(yend-ysta)

void LCD_Fill(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color)

{          

	uint16_t i,j; 

	Address_set(xsta,ysta,xend,yend);      //设置光标位置 

	for(i=ysta;i<=yend;i++)													   	 	

		for(j=xsta;j<=xend;j++)

			LCD_WR_DATA(color);//设置光标位置 	    				  	    

} 

//在指定区域内填充指定颜色块			 

//(sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex-sx+1)*(ey-sy+1)   

//color:要填充的颜色

void LCD_Color_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t *color)

{  

	uint16_t height,width;

	uint16_t i,j;

	width=ex-sx+1; 		//得到填充的宽度

	height=ey-sy+1;		//高度

 	for(i=0;i<height;i++)

	{

 		LCD_SetCursor(sx,sy+i);   	//设置光标位置 

		LCD_WR_REG(lcddev.wramcmd);     //开始写入GRAM

		for(j=0;j<width;j++) LCD_WR_DATA(color[i*height+j]);

	}	  

}  

//画线

//x1,y1:起点坐标

//x2,y2:终点坐标  

void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)

{

	uint16_t t; 

	int xerr=0,yerr=0,delta_x,delta_y,distance; 

	int incx,incy,uRow,uCol; 



	delta_x=x2-x1; //计算坐标增量 

	delta_y=y2-y1; 

	uRow=x1; 

	uCol=y1; 

	if(delta_x>0)incx=1; //设置单步方向 

	else if(delta_x==0)incx=0;//垂直线 

	else {incx=-1;delta_x=-delta_x;} 

	if(delta_y>0)incy=1; 

	else if(delta_y==0)incy=0;//水平线 

	else{incy=-1;delta_y=-delta_y;} 

	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 

	else distance=delta_y; 

	for(t=0;t<=distance+1;t++ )//画线输出 

	{  

		LCD_DrawPoint(uRow,uCol);//画点 

		xerr+=delta_x ; 

		yerr+=delta_y ; 

		if(xerr>distance) 

		{ 

			xerr-=distance; 

			uRow+=incx; 

		} 

		if(yerr>distance) 

		{ 

			yerr-=distance; 

			uCol+=incy; 

		} 

	}  

}    

//画矩形

void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)

{

	LCD_DrawLine(x1,y1,x2,y1);

	LCD_DrawLine(x1,y1,x1,y2);

	LCD_DrawLine(x1,y2,x2,y2);

	LCD_DrawLine(x2,y1,x2,y2);

}

//在指定位置画一个指定大小的圆

//(x,y):中心点

//r    :半径

void Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r)

{

	int a,b;

	int di;

	a=0;b=r;	  

	di=3-(r<<1);             //判断下个点位置的标志

	while(a<=b)

	{

		LCD_DrawPoint(x0-b,y0-a);             //3           

		LCD_DrawPoint(x0+b,y0-a);             //0           

		LCD_DrawPoint(x0-a,y0+b);             //1       

		LCD_DrawPoint(x0-b,y0-a);             //7           

		LCD_DrawPoint(x0-a,y0-b);             //2             

		LCD_DrawPoint(x0+b,y0+a);             //4               

		LCD_DrawPoint(x0+a,y0-b);             //5

		LCD_DrawPoint(x0+a,y0+b);             //6 

		LCD_DrawPoint(x0-b,y0+a);             

		a++;

		//使用Bresenham算法画圆     

		if(di<0)di +=4*a+6;	  

		else

		{

			di+=10+4*(a-b);   

			b--;

		} 

		LCD_DrawPoint(x0+a,y0+b);

	}

} 

//在指定位置显示一个字符



//num:要显示的字符:" "--->"~"

//mode:叠加方式(1)还是非叠加方式(0)

//在指定位置显示一个字符



//num:要显示的字符:" "--->"~"



//mode:叠加方式(1)还是非叠加方式(0)

void LCD_ShowChar(uint16_t x,uint16_t y,uint8_t num,uint8_t mode)

{

    uint8_t temp;

    uint8_t pos,t;

	uint16_t x0=x;

	uint16_t colortemp=POINT_COLOR;      

    if(x>lcddev.width-16||y>lcddev.height-16)return;	    

	//设置窗口		   

	num=num-' ';//得到偏移后的值

	Address_set(x,y,x+8-1,y+16-1);      //设置光标位置 

	if(!mode) //非叠加方式

	{

		for(pos=0;pos<16;pos++)

		{ 

			temp=asc2_1608[(uint16_t)num*16+pos];		 //调用1608字体

			for(t=0;t<8;t++)

		    {                 

		        if(temp&0x01)POINT_COLOR=colortemp;

				else POINT_COLOR=BACK_COLOR;

				LCD_WR_DATA(POINT_COLOR);	

				temp>>=1; 

				x++;

		    }

			x=x0;

			y++;

		}	

	}else//叠加方式

	{

		for(pos=0;pos<16;pos++)

		{

		    temp=asc2_1608[(uint16_t)num*16+pos];		 //调用1608字体

			for(t=0;t<8;t++)

		    {                 

		        if(temp&0x01)LCD_DrawPoint(x+t,y+pos);//画一个点     

		        temp>>=1; 

		    }

		}

	}

	POINT_COLOR=colortemp;	    	   	 	  

}   

//m^n函数

uint32_t mypow(uint8_t m,uint8_t n)

{

	uint32_t result=1;	 

	while(n--)result*=m;    

	return result;

}			 

//显示2个数字

//x,y :起点坐标	 

//len :数字的位数

//color:颜色

//num:数值(0~4294967295);	

void LCD_ShowNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len)

{         	

	uint8_t t,temp;

	uint8_t enshow=0;

	num=(uint16_t)num;

	for(t=0;t<len;t++)

	{

		temp=(num/mypow(10,len-t-1))%10;

		if(enshow==0&&t<(len-1))

		{

			if(temp==0)

			{

				LCD_ShowChar(x+8*t,y,' ',0);

				continue;

			}else enshow=1; 

		 	 

		}

	 	LCD_ShowChar(x+8*t,y,temp+48,0); 

	}

} 

//显示2个数字

//x,y:起点坐标

//num:数值(0~99);	 

void LCD_Show2Num(uint16_t x,uint16_t y,uint16_t num,uint8_t len)

{         	

	uint8_t t,temp;						   

	for(t=0;t<len;t++)

	{

		temp=(num/mypow(10,len-t-1))%10;

	 	LCD_ShowChar(x+8*t,y,temp+'0',0); 

	}

} 

//显示字符串

//x,y:起点坐标  

//*p:字符串起始地址

//用16字体

void LCD_ShowString(uint16_t x,uint16_t y,const char *p)

{         

    while(*p!='\0')

    {       

        if(x>lcddev.width-16){x=0;y+=16;}

        if(y>lcddev.height-16){y=x=0;LCD_Clear(RED);}

        LCD_ShowChar(x,y,*p,0);

        x+=8;

        p++;

    }  

}











