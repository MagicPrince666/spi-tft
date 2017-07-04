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
	 
//	if(key == 1)	//如果按键1按下,进入校准程序 
//	   {
//			Touch_Adjust();//校准
//			return 1;
//	   }
	 
	   return 0;
}
void xianshi()//显示信息
{   
	uint16_t lx,ly;
	BACK_COLOR = WHITE;
	POINT_COLOR = RED;	
	showhanzi(10,0,0);  //晶
	showhanzi(45,0,1);  //耀
    LCD_ShowString(10,35,"2.4 TFT SPI 240*320");
	LCD_ShowString(10,55,"LCD_W:");	LCD_ShowNum(70,55,LCD_W,3);
	LCD_ShowString(110,55,"LCD_H:");LCD_ShowNum(160,55,LCD_H,3);	
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
		    for(i=0;i<1600;i++)
			 { 	
				 if(jiance()) //检测触摸和按键
				 {	
				 	ref = 1;
				  	return;	
				 }		
			  	 LCD_WR_DATA(image[i*2+1]<<8 | image[i*2]);	 
				 //LCD_WR_DATA8(image[i*2]);				
			 }	
		 }
	}
	ref = 0;				
}

int run = 1;

static void sigint_handler(int sig)
{
    close(gpio_mmap_fd);
	pwm_disable(1);
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
	
	LCD_Clear(WHITE); //清屏

	BACK_COLOR = WHITE;
	POINT_COLOR = BLUE; 
	
//	printf("Init piclib\n");
	piclib_init();				//piclib初始化	
	
	printf("show image\n");

	//LCD_ReadPoint(1,1);//读取不了点数据

	ai_load_picfile((uint8_t*)"test.jpg",0,0,240,320,0,T_JPG);//显示图片

	signal(SIGINT, sigint_handler);//信号处理

	if (pthread_create(&pthread_id[0], NULL, thread_1 , NULL))
        printf("Create thread_1 error!\n");

    if(pthread_id[0] !=0) {                   
            pthread_join(pthread_id[0],NULL);
            printf("thread_1 %ld exit!\n",pthread_id[0]);
	}
	return 0;
}