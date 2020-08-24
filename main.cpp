/*****************************************************************************************
 * 
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
//#include "touch.h"
//#include "key.h"
#include "piclib.h"
#include "spi.h"

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
	//gifdecoding = 0;
	//run = 0;
}
//清空屏幕并在右上角显示"RST"
void Load_Drow_Dialog(void)
{
	LCD_Clear(WHITE);//清屏   
 	POINT_COLOR=BLUE;//设置字体为蓝色 
	LCD_ShowString(lcddev.width-32,0,200,16,16,(char *)"RST");//显示清屏区域
  	POINT_COLOR=RED;//设置画笔蓝色 
}
//电容触摸屏专有部分
//画水平线
//x0,y0:坐标
//len:线长度
//color:颜色
void gui_draw_hline(uint16_t x0,uint16_t y0,uint16_t len,uint16_t color)
{
	if(len==0)return;
	LCD_Fill(x0,y0,x0+len-1,y0,color);	
}
//画实心圆
//x0,y0:坐标
//r:半径
//color:颜色
void gui_fill_circle(uint16_t x0,uint16_t y0,uint16_t r,uint16_t color)
{											  
	uint32_t i;
	uint32_t imax = ((uint32_t)r*707)/1000+1;
	uint32_t sqmax = (uint32_t)r*(uint32_t)r+(uint32_t)r/2;
	uint32_t x=r;
	gui_draw_hline(x0-r,y0,2*r,color);
	for (i=1;i<=imax;i++) 
	{
		if ((i*i+x*x)>sqmax)// draw lines from outside  
		{
 			if (x>imax) 
			{
				gui_draw_hline (x0-i+1,y0+x,2*(i-1),color);
				gui_draw_hline (x0-i+1,y0-x,2*(i-1),color);
			}
			x--;
		}
		// draw lines from inside (center)  
		gui_draw_hline(x0-x,y0+i,2*x,color);
		gui_draw_hline(x0-x,y0-i,2*x,color);
	}
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
#if 0
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
		//if(KEY_1 == 0)	//如果按键2按下,退出程序 
		//break;
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
/*	
	if(KEY_1 == 0)	//如果按键1按下,进入校准程序 
	{		
		usleep(20*1000);
		while(!KEY_1)
		{	
		}
			
		return 1;
	}
*/ 
	return 0;
}
#endif
void xianshi()//显示信息
{   
	BACK_COLOR = WHITE;
	POINT_COLOR = RED;	
	//LCD_ShowString(0,20,lcddev.width,lcddev.height,16,(char *)"https://github.com/MagicPrince666/spi-tft.git");
	//LCD_ShowString(0,12+16+24,lcddev.width,lcddev.height,16,(char *)"Designer:me ! is me!");
	LCD_ShowString(30,252,lcddev.width,lcddev.height,16,(char *)"Nmae:leo");
	LCD_ShowString(30,268,lcddev.width,lcddev.height,16,(char *)"Embedded engineer");
	LCD_ShowString(30,284,lcddev.width,lcddev.height,16,(char *)"Company:XAG CO., LTD.");
	LCD_ShowString(30,300,lcddev.width,lcddev.height,16,(char *)"Email:huangliquan@xa.com");
	
	LCD_Backlight(0x00);
}

void showqq()
{ 
	uint16_t x,y; 
	x=0;
	y=80;
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
//	showqq();
	xianshi(); //显示信息 
	ref = 0;		
	//LCD_Display_Dir(L2R_U2D);		
}
#if 0
static void sigint_handler(int sig)
{   
	my_quit();
    printf("-----@@@@@ sigint_handler  is over !\n"); 
}
#endif
void * thread_tft (void *arg) 
{
	Lcd_Init();   //tft初始化
	//Init_Key();
	//gui_init();
	
	BACK_COLOR = WHITE;
	POINT_COLOR = BLUE; 
	
	piclib_init();				//piclib初始化	
	printf("Init piclib\n");

	// printf("show jpg\n");
	//ai_load_picfile((uint8_t*)"/root/test.jpg",0,0,240,320,0,T_JPG);//显示当前目录jpg图片
	// usleep(100*1000);
	// LCD_Clear(WHITE);
	// printf("show bmp\n");
	// ai_load_picfile((uint8_t*)"test.bmp",12,16,228,304,1,T_BMP);//显示当前目录bmp图片
	// usleep(100*1000);
	// LCD_Clear(WHITE);

	// printf("show gif\n");
	// LCD_Display_Dir(D2U_L2R);//横屏显示
	// ai_load_picfile((uint8_t*)"test.gif",0,0,lcddev.width,lcddev.height,1,T_GIF);//显示当前目录bmp图片
	// usleep(100*1000);
	// LCD_Display_Dir(DFT_SCAN_DIR);
	// LCD_Clear(WHITE);
	
	//tp_dev.init();//触摸初始化
	//printf("touch init\n");
	
	LCD_Display_Dir(D2U_L2R);
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

	printf("Did you see that? demo!!\n");
	
	LCD_Display_Dir(DFT_SCAN_DIR);

	ai_load_picfile((uint8_t*)"/root/test.jpg",0,0,240,320,0,T_JPG);//显示当前目录jpg图片
	sleep(1);
	LCD_Clear(WHITE);
	//showimage();
	int count = 0;
	char str[64] = {0};

	while(1)
	{
#if 0						
		if(jiance()) //检测触摸和按键
		{
			LCD_Display_Dir(L2R_U2D);
			//ai_load_picfile((uint8_t*)"test.bmp",0,0,240,319,0,T_BMP);
			LCD_Display_Dir(D2U_L2R);
			//LCD_DrawRectangle(12,16,228,304);
		}
#endif
		if(count >= 3) {
			count = 0;
		}
		int len = sprintf(str, "/root/test_%d.jpg", ++count);
		str[len] = 0;
		//LCD_Display_Dir(L2R_U2D);
		ai_load_picfile((uint8_t*)str, 0, 0, 240, 320, 0, T_JPG);//显示当前目录jpg图片
		//LCD_ShowString(30,252,lcddev.width,lcddev.height,16,(char *)"Nmae:leo");
		//LCD_ShowString(30,268,lcddev.width,lcddev.height,16,(char *)"Embedded engineer");
		//LCD_ShowString(30,284,lcddev.width,lcddev.height,16,(char *)"Company:XAG CO., LTD.");
		//LCD_ShowString(30,300,lcddev.width,lcddev.height,16,(char *)"Email:huangliquan@xa.com");
		//LCD_Display_Dir(D2U_L2R);
		sleep(3);
		LCD_Clear(WHITE);
    }

	//close(gpio_mmap_fd);
	SPI_Close();
	pthread_exit(NULL);
}


pthread_mutex_t mut;//声明互斥变量 

int main(int argc, char *argv[])
{ 
	pthread_t pthread_id[3];//线程ID
    pthread_mutex_init(&mut,NULL);

	//signal(SIGINT, sigint_handler);//信号处理

	if (pthread_create(&pthread_id[0], NULL, thread_tft , NULL))
        printf("Create thread_tft error!\n");
	// if (pthread_create(&pthread_id[1], NULL, pwm_thread , NULL))
    //     printf("Create pwm_thread error!\n");
	// if (pthread_create(&pthread_id[2], NULL, oled_thread , NULL))
    //     printf("Create oled_thread error!\n");

	printf("ctrl + c to stop!\n"); 

    if(pthread_id[0] != 0) {                   
        pthread_join(pthread_id[0],NULL);
        printf("thread_tft %ld exit!\n",pthread_id[0]);
	}
	// if(pthread_id[1] != 0) {                   
    //     pthread_join(pthread_id[1],NULL);
    //     printf("pwm_thread %ld exit!\n",pthread_id[1]);
	// }
	// if(pthread_id[2] != 0) {                   
    //     pthread_join(pthread_id[2],NULL);
    //     printf("oled_thread %ld exit!\n",pthread_id[2]);
	// }

	return 0;
}