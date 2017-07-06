/*****************************************************************************************

 * 本程序只供学习使用，未经作者许可，不得用于其它任何用途 *
 * 文件名  main.cpp
 * 描述    ：此处硬件初始化和线程控制
 * 平台    ：linux
 * 版本    ：V1.0.0
 * 作者    ：小王子与木头人  QQ：846863428
 *修改时间  ：2017-07-4

*****************************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "LCD.h"
#include "font.h"
#include "touch.h"
#include "pwmconfig.h"
#include "key.h"
#include "piclib.h"
#include "spi.h"


uint8_t ref = 0;//刷新显示



uint8_t jiance() //检测触摸和按键

{

	if (tpstate() == 0) //如果触摸按下，则进入绘图程序

	 {

	 	LCD_Clear(WHITE); //清屏

		BACK_COLOR = WHITE;

		POINT_COLOR = RED;

	 	point();

		return 1;

	 }	



	if(key == 0)	//如果按键1按下,进入校准程序 

	{

		Touch_Adjust();//校准			

		return 1;

	}

	 

	   return 0;

}

void xianshi()//显示信息

{   

	uint16_t lx,ly;

	BACK_COLOR = WHITE;

	POINT_COLOR = RED;	

	showhanzi(10,0,0);  //晶

	showhanzi(45,0,1);  //耀

	LCD_ShowString(80,16,"Magic Prince");

    LCD_ShowString(10,35,"2.4 TFT SPI 240*320");

	LCD_ShowString(10,55,"LCD_W:");	LCD_ShowNum(70,55,lcddev.width,3);

	LCD_ShowString(110,55,"LCD_H:");LCD_ShowNum(160,55,lcddev.height,3);	

	lx=10;

	ly=75;			

	LCD_ShowString(lx,ly,"VX:");lx+=40;LCD_ShowNum(lx,ly,vx,5);					

	lx+=60;

	LCD_ShowString(lx,ly,"Vy:");lx+=40;LCD_ShowNum(lx,ly,vy,5);					

	lx=10;ly=95; 

	LCD_ShowString(lx,ly,"CHX:");lx+=40;LCD_ShowNum(lx,ly,chx,5);					

	lx+=60; 

	LCD_ShowString(lx,ly,"CHY:");lx+=40;LCD_ShowNum(lx,ly,chy,5);

}

void showimage() //显示40*40图片

{

  	int i,j,k; 

	LCD_Clear(WHITE); //清屏  

	xianshi(); //显示信息

	for(k=3;k<8;k++)

	{

	   	for(j=0;j<6;j++)

		{	

			Address_set(40*j,40*k,40*j+39,40*k+39);		//坐标设置

//			LCD_Fast_WR_DATA16((uint16_t *)image,1600);

			LCD_DC_1;

		    for(i=0;i<1600;i++)

			 { 	

				 	

				write(g_SPI_Fd, &(image[i*2 + 1]),1);

				write(g_SPI_Fd, &(image[i*2]),1);		 				

			 }	

			 if(jiance()) //检测触摸和按键

			{	

			ref = 1;

			return;	

			}

		 }

	}

	ref = 0;				

}



static void sigint_handler(int sig)

{   

	pwm_disable(1);

	pwm_disable(2);

	pwm_disable(3);

	pwm_disable(4);

	mt76x8_gpio_set_pin_value(11, 1);

	mt76x8_gpio_set_pin_value(14, 1);

	mt76x8_gpio_set_pin_value(15, 1);

	mt76x8_gpio_set_pin_value(16, 1);

	mt76x8_gpio_set_pin_value(17, 1);

	mt76x8_gpio_set_pin_value(39, 1);

	mt76x8_gpio_set_pin_value(40, 1);

	mt76x8_gpio_set_pin_value(41, 1);

	mt76x8_gpio_set_pin_value(42, 1);

	close(gpio_mmap_fd);

	SPI_Close();

	run = 0;

    printf("-----@@@@@ sigint_handler  is over !\n"); 

}



void * thread_1 (void *arg) 

{

	printf("ctrl + c to stop!\n");

	while(run)

	{

		if(jiance()) //检测触摸和按键

	  		showimage(); //显示40*40图片

	  	if(ref)

	  		showimage(); //显示40*40图片

    }

	pthread_exit(NULL);

}



pthread_mutex_t mut;//声明互斥变量 



int main(int argc, char *argv[])

{ 

	pthread_t pthread_id[3];//线程ID

    pthread_mutex_init(&mut,NULL);



	pwm_init();

	Lcd_Init();   //tft初始化

	Init_Key();

	

	LCD_Clear(RED);

	usleep(500*1000);

	LCD_Clear(GREEN);

	usleep(500*1000);

	LCD_Clear(BLUE);

	usleep(500*1000);

	LCD_Clear(WHITE); //刷屏测试

	usleep(500*1000);

	LCD_Clear(RED);

	LCD_Clear(GREEN);

	LCD_Clear(BLUE);

	LCD_Clear(WHITE); //刷屏测试



	BACK_COLOR = WHITE;

	POINT_COLOR = BLUE; 

	

//	printf("Init piclib\n");

	piclib_init();				//piclib初始化	

	

	printf("show image\n");



	//LCD_ReadPoint(1,1);//读取不了点数据



	ai_load_picfile((uint8_t*)"test.jpg",0,0,240,320,0,T_JPG);//显示当前目录jpg图片

	ai_load_picfile((uint8_t*)"test.bmp",0,0,240,320,0,T_BMP);//显示当前目录bmp图片

	ai_load_picfile((uint8_t*)"test.gif",(lcddev.width - 160)/2,(lcddev.height - 120)/2,160,120,0,T_GIF);//显示当前目录gif图片



	signal(SIGINT, sigint_handler);//信号处理



	if (pthread_create(&pthread_id[0], NULL, thread_1 , NULL))

        printf("Create thread_1 error!\n");



    if(pthread_id[0] !=0) {                   

            pthread_join(pthread_id[0],NULL);

            printf("thread_1 %ld exit!\n",pthread_id[0]);

	}

	return 0;

}
