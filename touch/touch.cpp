#include <unistd.h>
#include "touch.h" 
#include "LCD.h"
#include "stdlib.h"
#include "math.h"
#include "sys.h"
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>


_m_tp_dev tp_dev=
{
	TP_Init,
	TP_Scan,
	TP_Adjust,
	0,
	0, 
	0,
	0,
	0,
	0,	  	 		
	0,
	0,	  	 		
};
					
//默认为touchtype=0的数据.
uint8_t CMD_RDX=0XD0;
uint8_t CMD_RDY=0X90;
 	 			    					   
#define SPI_Control


#ifndef SPI_Control	 			    					   
//SPI写数据
//向触摸屏IC写入1byte数据    
//num:要写入的数据
void TP_Write_Byte(uint8_t num)    
{  
	uint8_t count=0;   
	for(count=0;count<8;count++)  
	{ 	  
		if(num&0x80)TDIN_1;  
		else TDIN_0;   
		num<<=1;    
		TCLK_0; 
		//usleep(1);	 
		TCLK_1;		//上升沿有效	        
	}		 			    
}
#endif

#ifdef SPI_Control
static const char *device = "/dev/spidev32766.2";
static uint8_t mode = 0; /* SPI通信使用全双工，设置CPOL＝0，CPHA＝0。 */
static uint8_t bits = 8; /* 8ｂiｔｓ读写，MSB first。*/
static uint32_t speed = 4 * 1000 * 1000;/* 设置96M传输速度 */
static int t_SPI_Fd = -1; 

static void pabort(const char *s)
{
    perror(s);
    abort();
}

int T_SPI_Open(void)
{
    int fd;
    int ret = 0;

    fd = open(device, O_RDWR);
    if (fd < 0)
        pabort("can't open device");
    else
        printf("SPI2 %s - Open Succeed. Start Init SPI...\n",device);


    t_SPI_Fd = fd;
    /*
    * spi mode
    */
    ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
    if (ret == -1)
        pabort("can't set spi2 mode");


    ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
    if (ret == -1)
        pabort("can't get spi2 mode");


    /*
    * bits per word
    */
    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1)
        pabort("can't set bits per word");


    ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
    if (ret == -1)
        pabort("can't get bits per word");


    /*
    * max speed hz
    */
    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1)
        pabort("can't set max speed hz");


    ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
    if (ret == -1)
        pabort("can't get max speed hz");


    printf("spi2 mode: %d\n", mode);
    printf("bits per word: %d\n", bits);
    printf("max speed: %d KHz (%d MHz)\n", speed / 1000, speed / 1000 / 1000);


    return ret;
}


/**
* 功 能：关闭SPI模块
*/
int T_SPI_Close(void)
{
    if (t_SPI_Fd == 0) /* SPI是否已经打开*/
    return 0;
    close(t_SPI_Fd);

    return 0;
}

#endif
//SPI读数据 
//从触摸屏IC读取adc值
//CMD:指令
//返回值:读到的数据	   
uint16_t TP_Read_AD(uint8_t CMD)	  
{
#ifndef SPI_Control	
	uint8_t count=0; 	  
	uint16_t Num=0; 		
	TCLK_0;		//先拉低时钟 	 
	TDIN_0; 	//拉低数据线
	TCS_0; 		//选中触摸屏IC
	TP_Write_Byte(CMD);//发送命令字
	usleep(6);//ADS7846的转换时间最长为6us
	TCLK_0; 	     	    
	usleep(1);    	   
	TCLK_1;		//给1个时钟，清除BUSY	
	usleep(1);    	    
	TCLK_0; 		
	for(count = 0; count < 16; count++)//读出16位数据,只有高12位有效 
	{ 				  
		Num<<=1; 	 
		TCLK_0;	//下降沿有效    
		//usleep(1);  	   
		TCLK_1;
		if(DOUT)Num++; 		 
	}   	//只有高12位有效.
	Num >>= 4;
	TCS_1;		//释放片选	

#else	  
	uint16_t Num=0;
	char str[2];
	  	 

	write(t_SPI_Fd, &CMD, 1);
	usleep(6);//ADS7846的转换时间最长为6us	
	read(t_SPI_Fd, str, 2);
	Num = str[0]<<8 | str[1] >> 4;   	//只有高12位有效.
	//printf("Num=%d\n",Num);

#endif

	return(Num);   
}
//读取一个坐标值(x或者y)
//连续读取READ_TIMES次数据,对这些数据升序排列,
//然后去掉最低和最高LOST_VAL个数,取平均值 
//xy:指令（CMD_RDX/CMD_RDY）
//返回值:读到的数据
#define READ_TIMES 5 	//读取次数
#define LOST_VAL 1	  	//丢弃值
uint16_t TP_Read_XOY(uint8_t xy)
{
	uint16_t i, j;
	uint16_t buf[READ_TIMES];
	uint16_t sum=0;
	uint16_t temp;
	for(i=0;i<READ_TIMES;i++)buf[i]=TP_Read_AD(xy);		 		    
	for(i=0;i<READ_TIMES-1; i++)
	{
		for(j=i+1;j<READ_TIMES;j++)
		{
			if(buf[i]>buf[j])
			{
				temp=buf[i];
				buf[i]=buf[j];
				buf[j]=temp;
			}
		}
	}	  
	sum=0;
	for(i=LOST_VAL;i<READ_TIMES-LOST_VAL;i++)sum+=buf[i];
	temp=sum/(READ_TIMES-2*LOST_VAL);
	return temp;   
} 
//读取x,y坐标
//最小值不能少于100.
//x,y:读取到的坐标值
//返回值:0,失败;1,成功。
uint8_t TP_Read_XY(uint16_t *x,uint16_t *y)
{
	uint16_t xtemp,ytemp;	
	xtemp=TP_Read_XOY(CMD_RDX);
	ytemp=TP_Read_XOY(CMD_RDY);	 												   
	//if(xtemp<100||ytemp<100)return 0;//读数失败
	*x=xtemp;
	*y=ytemp;
	return 1;//读数成功
}
//连续2次读取触摸屏IC,且这两次的偏差不能超过
//ERR_RANGE,满足条件,则认为读数正确,否则读数错误.	   
//该函数能大大提高准确度
//x,y:读取到的坐标值
//返回值:0,失败;1,成功。
#define ERR_RANGE 50 //误差范围 
uint8_t TP_Read_XY2(uint16_t *x,uint16_t *y) 
{
	uint16_t x1,y1;
 	uint16_t x2,y2;
 	uint8_t flag;    
    flag=TP_Read_XY(&x1,&y1);   
    if(flag==0)return(0);
    flag=TP_Read_XY(&x2,&y2);	   
    if(flag==0)return(0);   
    if(((x2<=x1&&x1<x2+ERR_RANGE)||(x1<=x2&&x2<x1+ERR_RANGE))//前后两次采样在+-50内
    &&((y2<=y1&&y1<y2+ERR_RANGE)||(y1<=y2&&y2<y1+ERR_RANGE)))
    {
        *x=(x1+x2)/2;
        *y=(y1+y2)/2;
        return 1;
    }else return 0;	  
}  
//////////////////////////////////////////////////////////////////////////////////		  
//与LCD部分有关的函数  
//画一个触摸点
//用来校准用的
//x,y:坐标
//color:颜色
void TP_Drow_Touch_Point(uint16_t x,uint16_t y,uint16_t color)
{
	POINT_COLOR = color;
	LCD_DrawLine(x-12,y,x+13,y);//横线
	LCD_DrawLine(x,y-12,x,y+13);//竖线
	LCD_DrawPoint(x+1,y+1);
	LCD_DrawPoint(x-1,y+1);
	LCD_DrawPoint(x+1,y-1);
	LCD_DrawPoint(x-1,y-1);
	LCD_Draw_Circle(x,y,6);//画中心圈
}	  
//画一个大点(2*2的点)		   
//x,y:坐标
//color:颜色
void TP_Draw_Big_Point(uint16_t x,uint16_t y,uint16_t color)
{	
	POINT_COLOR = color;    
	LCD_DrawPoint(x,y);//中心点 
	LCD_DrawPoint(x+1,y);
	LCD_DrawPoint(x,y+1);
	LCD_DrawPoint(x+1,y+1);	 	  	
}						  
//////////////////////////////////////////////////////////////////////////////////		  
//触摸按键扫描
//tp:0,屏幕坐标;1,物理坐标(校准等特殊场合用)
//返回值:当前触屏状态.
//0,触屏无触摸;1,触屏有触摸
uint8_t TP_Scan(uint8_t tp)
{			   
	if(PEN==0)
	{
		if(tp)TP_Read_XY2(&tp_dev.x[0],&tp_dev.y[0]);
		else if(TP_Read_XY2(&tp_dev.x[0],&tp_dev.y[0]))
		{
	 		tp_dev.x[0]=tp_dev.xfac*tp_dev.x[0]+tp_dev.xoff;
			tp_dev.y[0]=tp_dev.yfac*tp_dev.y[0]+tp_dev.yoff;  
	 	} 
		if((tp_dev.sta&TP_PRES_DOWN)==0)
		{		 
			tp_dev.sta=TP_PRES_DOWN|TP_CATH_PRES;
			tp_dev.x[4]=tp_dev.x[0];
			tp_dev.y[4]=tp_dev.y[0];  	   			 
		}	
		if(lcddev.dir)	//横屏
		{
			//tp_dev.sta |= 0x01;
			uint16_t temp;
			temp = tp_dev.y[0];
			tp_dev.y[0] = tp_dev.x[0];
			tp_dev.x[0] = lcddev.width - temp;
		}	   
	}else
	{
		if(tp_dev.sta&TP_PRES_DOWN)
		{
			tp_dev.sta&=~(1<<7);
		}else
		{
			tp_dev.x[4]=0;
			tp_dev.y[4]=0;
			tp_dev.x[0]=0xffff;
			tp_dev.y[0]=0xffff;
		}	    
	}
	return tp_dev.sta&TP_PRES_DOWN;
}	  
//////////////////////////////////////////////////////////////////////////	 
//保存在EEPROM里面的地址区间基址,占用13个字节(RANGE:SAVE_ADDR_BASE~SAVE_ADDR_BASE+12)
#define SAVE_ADDR_BASE 0X0
//保存校准参数										    
void TP_Save_Adjdata(void)
{
	int temp;
	uint8_t writeadj[18];
	uint8_t *str;

	FILE *file;
	file = fopen((char *)"tp-adj","w");//保存校准信息

	temp = tp_dev.xfac*100000000;//保存x校正因素
	//printf("write tp_dev.xfac = %f\n",tp_dev.xfac);
	str = (uint8_t *)&temp;
	writeadj[0] = str[3];//小端系统是低位在前高位在后
	writeadj[1] = str[2];
	writeadj[2] = str[1];
	writeadj[3] = str[0];
		 
	temp = tp_dev.yfac*100000000;//保存y校正因素
	//printf("write tp_dev.yfac = %f\n",tp_dev.yfac);
	writeadj[4] = str[3];
	writeadj[5] = str[2];
	writeadj[6] = str[1];
	writeadj[7] = str[0];	

	//保存x偏移量
	temp = tp_dev.xoff;
	//printf("write tp_dev.xoff = %d\n",tp_dev.xoff);
	writeadj[8] = str[3];
	writeadj[9] = str[2];
	writeadj[10] = str[1];
	writeadj[11] = str[0];


	//保存y偏移量
	temp = 0x0000FFFF&(tp_dev.yoff);
	//printf("write tp_dev.yoff = %d\n",tp_dev.yoff);
	writeadj[12] = str[3];
	writeadj[13] = str[2];
	writeadj[14] = str[1];
	writeadj[15] = str[0];

	//保存触屏类型
	writeadj[16] = tp_dev.touchtype;
	writeadj[17] = 0X0A;//标记校准过了 

	fwrite((uint8_t *)writeadj,1,18,file);
	// printf("write adj:");	
	// for(int i = 0 ;i < 14; i++)
	// 	printf("%x ",writeadj[i]);
	// printf("\n");
	fclose(file);
}
//得到保存在EEPROM里面的校准值
//返回值：1，成功获取数据
//        0，获取失败，要重新校准
uint8_t TP_Get_Adjdata(void)
{
	FILE *file;
	file = fopen((char *)"tp-adj","r");
	if(file != NULL)
	{					  
		int tempfac;
		tempfac = 0;
		uint8_t readadj[18];
		uint8_t *str = (uint8_t *)&tempfac;

		fread((uint8_t *)readadj,1,18,file);	
		// printf("read  adj:");	
		// for(int i = 0;i < 14; i++)
		// 	printf("%x ",readadj[i]);
		// printf("\n");

		if(readadj[17] == 0X0A)//触摸屏已经校准过了			   
		{    												 
			str[3] = readadj[0];
			str[2] = readadj[1];
			str[1] = readadj[2];
			str[0] = readadj[3];		  
			tp_dev.xfac = (float)tempfac/100000000;//得到x校准参数
			//printf(" read tp_dev.xfac = %f\n",tp_dev.xfac);

			str[3] = readadj[4];
			str[2] = readadj[5];
			str[1] = readadj[6];
			str[0] = readadj[7];
			tp_dev.yfac = (float)tempfac/100000000;//得到y校准参数
			//printf(" read tp_dev.yfac = %f\n",tp_dev.yfac);

			//得到x偏移量
			str[3] = readadj[8];
			str[2] = readadj[9];
			str[1] = readadj[10];
			str[0] = readadj[11];
			tp_dev.xoff = tempfac;	
			//printf(" read tp_dev.xoff = %d\n",tp_dev.xoff);

			//得到y偏移量
			str[3] = readadj[12];
			str[2] = readadj[13];
			str[1] = readadj[14];
			str[0] = readadj[15];
			tp_dev.yoff = tempfac;		
			//printf(" read tp_dev.yoff = %d\n",tp_dev.yoff);			
			tp_dev.touchtype = readadj[16];//读取触屏类型标记
			if(tp_dev.touchtype)//X,Y方向与屏幕相反
			{
				CMD_RDX=0X90;
				CMD_RDY=0XD0;	 
			}else				   //X,Y方向与屏幕相同
			{
				CMD_RDX=0XD0;
				CMD_RDY=0X90;	 
			}	
			fclose(file);	 
			return 1;	 
		}
		fclose(file);			
	} 
	return 0;	 
}	  
//提示字符串
uint8_t* const TP_REMIND_MSG_TBL=(uint8_t*)"Please use the stylus click the cross on the screen.The cross will always move until the screen adjustment is completed.";
 					  
//提示校准结果(各个参数)
void TP_Adj_Info_Show(uint16_t x0,uint16_t y0,uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t x3,uint16_t y3,uint16_t fac)
{	  
	POINT_COLOR=RED;
	LCD_ShowString(40,160,lcddev.width,lcddev.height,16,(char *)"x1:");
 	LCD_ShowString(40+80,160,lcddev.width,lcddev.height,16,(char *)"y1:");
 	LCD_ShowString(40,180,lcddev.width,lcddev.height,16,(char *)"x2:");
 	LCD_ShowString(40+80,180,lcddev.width,lcddev.height,16,(char *)"y2:");
	LCD_ShowString(40,200,lcddev.width,lcddev.height,16,(char *)"x3:");
 	LCD_ShowString(40+80,200,lcddev.width,lcddev.height,16,(char *)"y3:");
	LCD_ShowString(40,220,lcddev.width,lcddev.height,16,(char *)"x4:");
 	LCD_ShowString(40+80,220,lcddev.width,lcddev.height,16,(char *)"y4:");  
 	LCD_ShowString(40,240,lcddev.width,lcddev.height,16,(char *)"fac is:");     
	LCD_ShowNum(40+24,160,x0,4,16);		//显示数值
	LCD_ShowNum(40+24+80,160,y0,4,16);	//显示数值
	LCD_ShowNum(40+24,180,x1,4,16);		//显示数值
	LCD_ShowNum(40+24+80,180,y1,4,16);	//显示数值
	LCD_ShowNum(40+24,200,x2,4,16);		//显示数值
	LCD_ShowNum(40+24+80,200,y2,4,16);	//显示数值
	LCD_ShowNum(40+24,220,x3,4,16);		//显示数值
	LCD_ShowNum(40+24+80,220,y3,4,16);	//显示数值
 	LCD_ShowNum(40+56,240,fac,3,16); 	//显示数值,该数值必须在95~105范围之内.

}
int run = 1;  		 
//触摸屏校准代码
//得到四个校准参数
void TP_Adjust(void)
{								 
	uint16_t pos_temp[4][2];//坐标缓存值
	uint8_t  cnt=0;	
	uint16_t d1,d2;
	uint32_t tem1,tem2;
	double fac; 	
	uint16_t outtime=0;
 	cnt=0;				
	POINT_COLOR=BLUE;
	BACK_COLOR =WHITE;
	LCD_Clear(WHITE);//清屏   
	POINT_COLOR=RED;//红色 
	LCD_Clear(WHITE);//清屏 	   
	POINT_COLOR=BLACK;
	LCD_ShowString(40,40,160,100,16,(char *)TP_REMIND_MSG_TBL);//显示提示信息
	TP_Drow_Touch_Point(20,20,RED);//画点1 
	tp_dev.sta=0;//消除触发信号 
	tp_dev.xfac=0;//xfac用来标记是否校准过,所以校准之前必须清掉!以免错误	 
	while(run)//如果连续10秒钟没有按下,则自动退出
	{
		tp_dev.scan(1);//扫描物理坐标
		if((tp_dev.sta&0xc0)==TP_CATH_PRES)//按键按下了一次(此时按键松开了.)
		{	
			outtime=0;		
			tp_dev.sta&=~(1<<6);//标记按键已经被处理过了.
						   			   
			pos_temp[cnt][0]=tp_dev.x[0];
			pos_temp[cnt][1]=tp_dev.y[0];
			cnt++;	  
			switch(cnt)
			{			   
				case 1:						 
					TP_Drow_Touch_Point(20,20,WHITE);				//清除点1 
					TP_Drow_Touch_Point((lcddev.width) - 20,20,RED);	//画点2
					break;
				case 2:
 					TP_Drow_Touch_Point((lcddev.width) - 20,20,WHITE);	//清除点2
					TP_Drow_Touch_Point(20,(lcddev.height) - 20,RED);	//画点3
					break;
				case 3:
 					TP_Drow_Touch_Point(20,(lcddev.height) - 20,WHITE);			//清除点3
 					TP_Drow_Touch_Point((lcddev.width) - 20,(lcddev.height) - 20,RED);	//画点4
					break;
				case 4:	 //全部四个点已经得到
	    		    //对边相等
					tem1=abs(pos_temp[0][0]-pos_temp[1][0]);//x1-x2
					tem2=abs(pos_temp[0][1]-pos_temp[1][1]);//y1-y2
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);//得到1,2的距离
					
					tem1=abs(pos_temp[2][0]-pos_temp[3][0]);//x3-x4
					tem2=abs(pos_temp[2][1]-pos_temp[3][1]);//y3-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//得到3,4的距离
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05||d1==0||d2==0)//不合格
					{
						cnt=0;
 				    	TP_Drow_Touch_Point((lcddev.width)-20,(lcddev.height)-20,WHITE);	//清除点4
   	 					TP_Drow_Touch_Point(20,20,RED);								//画点1
 						TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//显示数据   
 						continue;
					}
					tem1=abs(pos_temp[0][0]-pos_temp[2][0]);//x1-x3
					tem2=abs(pos_temp[0][1]-pos_temp[2][1]);//y1-y3
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);//得到1,3的距离
					
					tem1=abs(pos_temp[1][0]-pos_temp[3][0]);//x2-x4
					tem2=abs(pos_temp[1][1]-pos_temp[3][1]);//y2-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//得到2,4的距离
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05)//不合格
					{
						cnt=0;
 				    	TP_Drow_Touch_Point((lcddev.width) - 20,(lcddev.height) - 20,WHITE);	//清除点4
   	 					TP_Drow_Touch_Point(20,20,RED);								//画点1
 						TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//显示数据   
						continue;
					}//正确了
								   
					//对角线相等
					tem1=abs(pos_temp[1][0]-pos_temp[2][0]);//x1-x3
					tem2=abs(pos_temp[1][1]-pos_temp[2][1]);//y1-y3
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);//得到1,4的距离
	
					tem1=abs(pos_temp[0][0]-pos_temp[3][0]);//x2-x4
					tem2=abs(pos_temp[0][1]-pos_temp[3][1]);//y2-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//得到2,3的距离
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05)//不合格
					{
						cnt=0;
 				    	TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);	//清除点4
   	 					TP_Drow_Touch_Point(20,20,RED);								//画点1
 						TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//显示数据   
						continue;
					}//正确了
					//计算结果
					tp_dev.xfac=(float)(lcddev.width-40)/(pos_temp[1][0]-pos_temp[0][0]);//得到xfac		 
					tp_dev.xoff=(lcddev.width-tp_dev.xfac*(pos_temp[1][0]+pos_temp[0][0]))/2;//得到xoff
						  
					tp_dev.yfac=(float)(lcddev.height-40)/(pos_temp[2][1]-pos_temp[0][1]);//得到yfac
					tp_dev.yoff=(lcddev.height-tp_dev.yfac*(pos_temp[2][1]+pos_temp[0][1]))/2;//得到yoff  
					if(abs(tp_dev.xfac)>2||abs(tp_dev.yfac)>2)//触屏和预设的相反了.
					{
						cnt=0;
 				    	TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);	//清除点4
   	 					TP_Drow_Touch_Point(20,20,RED);								//画点1
						LCD_ShowString(40,26,lcddev.width,lcddev.height,16,(char *)"TP Need readjust!");
						tp_dev.touchtype=!tp_dev.touchtype;//修改触屏类型.
						if(tp_dev.touchtype)//X,Y方向与屏幕相反
						{
							CMD_RDX=0X90;
							CMD_RDY=0XD0;	 
						}else				   //X,Y方向与屏幕相同
						{
							CMD_RDX=0XD0;
							CMD_RDY=0X90;	 
						}			    
						continue;
					}		
					POINT_COLOR=BLUE;
					LCD_Clear(WHITE);//清屏
					LCD_ShowString(35,110,lcddev.width,lcddev.height,16,(char *)"Touch Screen Adjust OK!");//校正完成
					usleep(1000*1000);
					TP_Save_Adjdata();  
 					LCD_Clear(WHITE);//清屏   
					return;//校正完成				 
			}
		}
		usleep(10*1000);
		outtime++;
		if(outtime>1000)
		{
			TP_Get_Adjdata();
			break;
	 	} 
 	}
}	 
//触摸屏初始化  		    
//返回值:0,没有进行校准
//       1,进行过校准
uint8_t TP_Init(void)
{	
	// if(lcddev.id == 0X5510)		//电容触摸屏
	// {
	// 	if(GT9147_Init() == 0)	//是GT9147
	// 	{ 
	// 		tp_dev.scan = GT9147_Scan;	//扫描函数指向GT9147触摸屏扫描
	// 	}else
	// 	{
	// 		OTT2001A_Init();
	// 		tp_dev.scan = OTT2001A_Scan;	//扫描函数指向OTT2001A触摸屏扫描
	// 	}
	// 	tp_dev.touchtype |= 0X80;	//电容屏 
	// 	tp_dev.touchtype |= lcddev.dir&0X01;//横屏还是竖屏 
	// 	return 0;
	// }else if(lcddev.id == 0X1963)
	// {
	// 	FT5206_Init();
	// 	tp_dev.scan = FT5206_Scan;		//扫描函数指向GT9147触摸屏扫描		
	// 	tp_dev.touchtype |= 0X80;			//电容屏 
	// 	tp_dev.touchtype |= lcddev.dir&0X01;//横屏还是竖屏 
	// 	return 0;
	// }else
	{
		//mt76x8_gpio_set_pin_direction(14, 1);
		//mt76x8_gpio_set_pin_direction(15, 1);
		mt76x8_gpio_set_pin_direction(16, 0);
		//mt76x8_gpio_set_pin_direction(17, 1);
		//mt76x8_gpio_set_pin_direction(41, 0);
		//mt76x8_gpio_set_pin_value(41, 1);
		//mt76x8_gpio_set_pin_value(16, 1);
		
		//TCS_1;
		//TDIN_1;
		//TCLK_1;
#ifdef SPI_Control
		printf("init spi2\n");
		T_SPI_Open();
#endif
		
		TP_Read_XY(&tp_dev.x[0],&tp_dev.y[0]);//第一次读取初始化	 
		if(TP_Get_Adjdata())return 0;//已经校准
		else			   //未校准?
		{ 										    
			LCD_Clear(WHITE);//清屏
			TP_Adjust();  //屏幕校准 	 
		}			
		TP_Get_Adjdata();	
	}
	return 1; 									 
}

