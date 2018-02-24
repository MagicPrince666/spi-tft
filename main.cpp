/*****************************************************************************************
 * 本程序只供学习使用，未经作者许可，不得用于其它任何用途 *
 * 文件名  main.cpp
 * 描述    ：此处硬件初始化和线程控制
 * 平台    ：linux
 * 版本    ：V1.0.0
 * 作者    ：小王子与木头人  QQ：846863428
 * 修改时间  ：2017-09-19
 * 修复找不到图片文件会发生段错误的bug
 * 增加外部字库文件 以显示中文汉字
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
#include "iic.h"
#include "mpu6050.h"
#include "text.h"
#include "guix.h"

uint8_t ref = 0;//刷新显示

void my_quit()
{
	// pwm_disable(1);
	// pwm_disable(2);
	// pwm_disable(3);
	// pwm_disable(4);
	//close(gpio_mmap_fd);
	//SPI_Close();
	//I2C_close();
	gifdecoding = 0;
	run = 0;
}
//清空屏幕并在右上角显示"RST"
void Load_Drow_Dialog(void)
{
	LCD_Clear(WHITE);//清屏   
 	POINT_COLOR=BLUE;//设置字体为蓝色 
	LCD_ShowString(lcddev.width-32,0,200,16,16,(char *)"RST");//显示清屏区域
  	POINT_COLOR=RED;//设置画笔蓝色 
}

//两个数之差的绝对值 
//x1,x2：需取差值的两个数
//返回值：|x1-x2|
uint16_t my_abs(uint16_t x1,uint16_t x2)
{			 
	if(x1>x2)return x1-x2;
	else return x2-x1;
}  
//画一条粗线
//(x1,y1),(x2,y2):线条的起始坐标
//size：线条的粗细程度
//color：线条的颜色
void lcd_draw_bline(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint8_t size,uint16_t color)
{
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	if(x1<size|| x2<size||y1<size|| y2<size)return; 
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
		gui_fill_circle(uRow,uCol,size,color);//画点 
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
////////////////////////////////////////////////////////////////////////////////
//5个触控点的颜色(电容触摸屏用)												 
const uint16_t POINT_COLOR_TBL[5]={RED,GREEN,BLUE,BROWN,GRED};  
//电阻触摸屏测试函数
void rtp_test(void)
{	  
	while(run)
	{
		tp_dev.scan(0); 		 
		if(tp_dev.sta&TP_PRES_DOWN)			//触摸屏被按下
		{	
		 	if(tp_dev.x[0]<lcddev.width&&tp_dev.y[0]<lcddev.height)
			{	
				if(tp_dev.x[0]>(lcddev.width-24)&&tp_dev.y[0]<16)Load_Drow_Dialog();//清除
				else TP_Draw_Big_Point(tp_dev.x[0],tp_dev.y[0],RED);		//画图	  			   
			}
		}else usleep(10*1000);	//没有按键按下的时候 	    
		if(KEY_1 == 0)	//如果按键2按下,退出程序 
		break;
	}
}
//电容触摸屏测试函数
void ctp_test(void)
{
	uint8_t t=0;	  	    
 	uint16_t lastpos[5][2];		//最后一次的数据 
	while(run)
	{
		tp_dev.scan(0);
		for(t=0;t<5;t++)
		{
			if((tp_dev.sta)&(1<<t))
			{
				if(tp_dev.x[t]<lcddev.width&&tp_dev.y[t]<lcddev.height)
				{
					if(lastpos[t][0]==0XFFFF)
					{
						lastpos[t][0] = tp_dev.x[t];
						lastpos[t][1] = tp_dev.y[t];
					}
					lcd_draw_bline(lastpos[t][0],lastpos[t][1],tp_dev.x[t],tp_dev.y[t],2,POINT_COLOR_TBL[t]);//画线
					lastpos[t][0]=tp_dev.x[t];
					lastpos[t][1]=tp_dev.y[t];
					if(tp_dev.x[t]>(lcddev.width-24)&&tp_dev.y[t]<20)
					{
						Load_Drow_Dialog();//清除
					}
				}
			}else lastpos[t][0]=0XFFFF;
		}
		usleep(5*1000);
	}	
}

uint8_t jiance() //检测触摸和按键
{	
	tp_dev.scan(0);
	if(tp_dev.sta&TP_PRES_DOWN)			//触摸屏被按下
	{
		Load_Drow_Dialog();
		rtp_test();
	}
		
	if(KEY_1 == 0)	//如果按键1按下,进入校准程序 
	{		
		//LCD_ReadPoint(120,160);//读取不了点数据		
		
		// LCD_Clear(WHITE); //清屏  
		//Show_Str(60,0,lcddev.width,lcddev.height,(uint8_t*)"小王子与木头人",16,0x00);
		usleep(20*1000);
		LCD_ShowString(20,15,lcddev.width,lcddev.height,16,(char *)"Test mpu6050");
		while(!KEY_1)
		{
			LCD_ShowString(10,55,lcddev.width,lcddev.height,16,(char *)"ACCE_X:");
			LCD_ShowNum(10 + 16*7,55,GetData(ACCEL_XOUT_H),5,16);

			LCD_ShowString(10,55 + 20*1,lcddev.width,lcddev.height,16,(char *)"ACCE_Y:");
			LCD_ShowNum(10 + 16*7,55 + 20*1,GetData(ACCEL_YOUT_H),5,16);

			LCD_ShowString(10,55 + 20*2,lcddev.width,lcddev.height,16,(char *)"ACCE_Z:");
			LCD_ShowNum(10 + 16*7,55 + 20*2,GetData(ACCEL_ZOUT_H),5,16);

			LCD_ShowString(10,55 + 20*3,lcddev.width,lcddev.height,16,(char *)"GYRO_X:");
			LCD_ShowNum(10 + 16*7,55 + 20*3,GetData(GYRO_XOUT_H),5,16);

			LCD_ShowString(10,55 + 20*4,lcddev.width,lcddev.height,16,(char *)"GYRO_Y:");
			LCD_ShowNum(10 + 16*7,55 + 20*4,GetData(GYRO_YOUT_H),5,16);

			LCD_ShowString(10,55 + 20*5,lcddev.width,lcddev.height,16,(char *)"GYRO_Z:");
			LCD_ShowNum(10 + 16*7,55 + 20*5,GetData(GYRO_ZOUT_H),5,16);	
		}
			
		return 1;
	}
	 
	return 0;
}
void xianshi()//显示信息
{   
	BACK_COLOR = WHITE;
	POINT_COLOR = RED;	

	Show_Str(0,0,lcddev.width,lcddev.height,(const uint8_t *)"小王子与木头人",24,0x00);
	Show_Str(0,26,lcddev.width,lcddev.height,(const uint8_t *)"小王子与木头人",16,0x00);
	Show_Str(0,44,lcddev.width,lcddev.height,(const uint8_t *)"小王子与木头人",12,0x00);
}
void showqq()
{ 
	uint16_t x,y; 
	x=0;
	y=120;
	while(y<lcddev.height-39)
	{
		x=0;
		while(x<lcddev.width-39)
		{
			showimage(x,y);	
			x+=40;
		}
		y+=40;
	 }	  
} 

void showimage() //显示40*40图片
{
	LCD_Clear(WHITE); //清屏  
	showqq();
	xianshi(); //显示信息 中文显示有bug
	ref = 0;				
}

static void sigint_handler(int sig)
{   
	my_quit();
    printf("-----@@@@@ sigint_handler  is over !\n"); 
}

void * thread_1 (void *arg) 
{
	printf("ctrl + c to stop!\n"); 
	while(run)
	{							
		if(jiance()) //检测触摸和按键
		{
			ai_load_picfile((uint8_t*)"test.bmp",0,0,240,319,0,T_BMP);
			//LCD_DrawRectangle(12,16,228,304);
		}
	  	if(ref)
		  	ai_load_picfile((uint8_t*)"test.bmp",0,0,240,319,0,T_BMP);
		usleep(1000);
    }
	pthread_exit(NULL);
}

pthread_mutex_t mut;//声明互斥变量 

int main(int argc, char *argv[])
{ 
	pthread_t pthread_id[3];//线程ID
    pthread_mutex_init(&mut,NULL);

	//pwm_init();
	Lcd_Init();   //tft初始化
	Init_Key();
	gui_init();
	//I2C_open();
	//MPU6050_Init();

	time_t timer;//time_t就是long int 类型
	int times = 0;
    timer = time(NULL);
    printf("start time is: %ld\n", timer);

	for(int i = 0 ; i < 5 ; i++)
	{
		LCD_Clear(RED);times++;
		LCD_Clear(GREEN);times++;
		LCD_Clear(BLUE);times++;
		LCD_Clear(WHITE);times++; //刷屏测试
	}

	timer = time(NULL);
    printf("end time is:   %ld\n", timer);
	printf("times is: %d\n", times);

	BACK_COLOR = WHITE;
	POINT_COLOR = BLUE; 
	
	piclib_init();				//piclib初始化	
	printf("Init piclib\n");

	printf("show jpg\n");
	ai_load_picfile((uint8_t*)"test.jpg",0,0,240,320,0,T_JPG);//显示当前目录jpg图片
	usleep(100*1000);
	LCD_Clear(WHITE);
	printf("show bmp\n");
	ai_load_picfile((uint8_t*)"test.bmp",12,16,228,304,1,T_BMP);//显示当前目录bmp图片
	usleep(100*1000);
	LCD_Clear(WHITE);
	
	tp_dev.init();//触摸初始化
	printf("touch init\n");
	showimage();


	signal(SIGINT, sigint_handler);//信号处理

	if (pthread_create(&pthread_id[0], NULL, thread_1 , NULL))
        printf("Create thread_1 error!\n");

    if(pthread_id[0] != 0) {                   
        pthread_join(pthread_id[0],NULL);
        printf("thread_1 %ld exit!\n",pthread_id[0]);
	}
	return 0;
}