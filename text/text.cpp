#include "sys.h" 
#include "LCD.h"
#include "text.h"	
#include <sys/stat.h>  
#include <string.h>
#include "guix.h"

extern "C" {
#include "gbk_utf8.h"	
#include <iconv.h> 		
}									   
					                        
//code 字符指针开始
//从字库中查找出字模
//code 字符串的开始地址,GBK码
//mat  数据存放地址 (size/8+((size%8)?1:0))*(size) bytes大小	
//size:字体大小
int Get_HzMat(unsigned char *code,unsigned char *mat,uint8_t size)
{		    
	unsigned char qh,ql;
	unsigned char i;					  
	unsigned long foffset; 
	FILE *FONT = NULL;
	uint8_t csize=(size/8+((size%8)?1:0))*(size);//得到字体一个字符对应点阵集所占的字节数	 
	qh =* code;
	ql =* (++code);
	if(qh < 0x81||ql < 0x40||ql == 0xff||qh == 0xff)//非 常用汉字
	{   		    
	    for(i = 0 ; i < csize ; i++) * mat++= 0x00;//填充满格
	    return 1; //结束访问
	}          
	if( ql < 0x7f ) ql -= 0x40;//注意!
	else ql -= 0x41;
	qh -= 0x81;   
	foffset = ((unsigned long)190 * qh + ql) * csize;	//得到字库中的字节偏移量  		  
	switch(size)
	{
		case 12:
			FONT = fopen("gbk12.fon","r"); 
			if(FONT == NULL)return 1;
			else
			{
				fseek(FONT,foffset,SEEK_SET);
				fread(mat,1,24,FONT);
			}
			break;
		case 16:
			FONT = fopen("gbk16.fon","r"); 
			if(FONT == NULL)return 1;
			else
			{
				fseek(FONT,foffset,SEEK_SET);
				fread(mat,1,32,FONT);
			}		
			break;
		case 24:
			FONT = fopen("gbk24.fon","r"); 
			if(FONT == NULL)return 1;
			else
			{
				fseek(FONT,foffset,SEEK_SET);
				fread(mat,1,72,FONT);
			}
			break;			
	}  
	fclose(FONT);   	
	return 0;											    
}  

//显示一个指定大小的汉字
//x,y :汉字的坐标
//font:汉字GBK码
//size:字体大小
//mode:0,正常显示,1,叠加显示	   
int Show_Font(uint16_t x,uint16_t y,const uint8_t *font,uint8_t size,uint8_t mode)
{
	uint8_t temp,t,t1;
	uint16_t x0=x,y0=y;
	uint8_t dzk[72];   
	uint8_t csize=(size/8+((size%8)?1:0))*(size);//得到字体一个字符对应点阵集所占的字节数	 
	if(size!=12&&size!=16&&size!=24)return 1;	//不支持的size

	if(size == 12)
	{
		for (int i=0;i<3;i++)
			printf("%2d",font[i]);
	}
	printf(" ");

	uint8_t str[2]; 
	u2g((const char*)font,3,(char *)str,2);

	

	if(Get_HzMat(str,dzk,size))//得到相应大小的点阵数据 
	return 1;	

	for(t = 0; t < csize ;t++)
	{   												   
		temp = dzk[t];			//得到点阵数据                          
		for(t1=0;t1<8;t1++)
		{
			if(temp & 0x80)
			{
				SPI_LCD_RAM[(y-y0)*size + (x-x0)] = POINT_COLOR;
			}
			else if(mode == 0)
			{
				SPI_LCD_RAM[(y-y0)*size + (x-x0)] = BACK_COLOR;
			}
			temp<<=1;
			y++;
			if((y-y0)==size)
			{
				y=y0;
				x++;
				break;
			}
		}  	 
	} 
	LCD_Color_Fill(x0,y0,x0 + size - 1,y0 + size - 1,SPI_LCD_RAM); 
	return 0;
}
//在指定位置开始显示一个字符串	    
//支持自动换行
//(x,y):起始坐标
//width,height:区域
//str  :字符串
//size :字体大小
//mode:0,非叠加方式;1,叠加方式    	   		   
void Show_Str(uint16_t x,uint16_t y,uint16_t width,uint16_t height,const uint8_t *str,uint8_t size,uint8_t mode)
{					
	uint16_t x0=x;
	uint16_t y0=y;							  	  
    uint8_t bHz=0;     //字符或者中文  	    				    				  	  
    while(*str!=0)//数据未结束
    { 
        if(!bHz)
        {
	        if(*str>0x80)bHz=1;//中文 
	        else              //字符
	        {      
                if(x>(x0+width-size/2))//换行
				{				   
					y+=size;
					x=x0;	   
				}							    
		        if(y>(y0+height-size))break;//越界返回      
		        if(*str==13)//换行符号
		        {         
		            y+=size;
					x=x0;
		            str++; 
		        }  
		        else LCD_ShowChar(x,y,*str,size,mode);//有效部分写入 
				str++; 
		        x+=size/2; //字符,为全字的一半 
	        }
        }else//中文 
        {     
            bHz=0;//有汉字库    
            if(x>(x0+width-size))//换行
			{	    
				y+=size;
				x=x0;		  
			}
	        if(y>(y0+height-size))break;//越界返回  						     
	        Show_Font(x,y,str,size,mode); //显示这个汉字,空心显示 
	        str+=3; 
	        x+=size;//下一个汉字偏移	    
        }						 
    }   
}  			 		 
//在指定宽度的中间显示字符串
//如果字符长度超过了len,则用Show_Str显示
//len:指定要显示的宽度			  
void Show_Str_Mid(uint16_t x,uint16_t y,uint8_t *str,uint8_t size,uint8_t len)
{
	uint16_t strlenth=0;
   	strlenth=strlen((const char*)str);
	strlenth*=size/2;
	if(strlenth>len)Show_Str(x,y,lcddev.width,lcddev.height,(const uint8_t *)str,size,1);
	else
	{
		strlenth=(len-strlenth)/2;
	    Show_Str(strlenth+x,y,lcddev.width,lcddev.height,(const uint8_t *)str,size,1);
	}
}   

//PC2LCD2002字体取模方法:逐列式,顺向(高位在前),阴码.C51格式.																		    
//特殊字体:
//数码管字体:ASCII集+℃(' '+95)
//普通字体:ASCII集
// char *const APP_ASCII_S6030=(char *const)"fonts60.fon";	//数码管字体60*30大数字字体路径 
// char *const APP_ASCII_5427=(char *const)"font54.fon";		//普通字体54*27大数字字体路径 
// char *const APP_ASCII_3618=(char *const)"font36.fon";		//普通字体36*18大数字字体路径
// char *const APP_ASCII_2814=(char *const)"font28.fon";		//普通字体28*14大数字字体路径 
char *const APP_ASCII_2412=(char *const)"font24.fon";		//普通字体28*14大数字字体路径
char *const APP_ASCII_1608=(char *const)"font16.fon";		//普通字体28*14大数字字体路径
char *const APP_ASCII_1206=(char *const)"font12.fon";		//普通字体28*14大数字字体路径

// uint8_t* asc2_s6030=0;	//数码管字体60*30大字体点阵集
// uint8_t* asc2_5427=0;	//普通字体54*27大字体点阵集
// uint8_t* asc2_3618=0;	//普通字体36*18大字体点阵集
// uint8_t* asc2_2814=0;	//普通字体28*14大字体点阵集
uint8_t* asc2_2412=0;	//普通字体24*12大字体点阵集
uint8_t* asc2_1608=0;	//普通字体16*08大字体点阵集
uint8_t* asc2_1206=0;	//普通字体12*06大字体点阵集


//在指定地址开始显示一个ASCII字符
//x,y:显示开始坐标.
//xend,yend:x,y 方向的终点坐标
//offset:开始显示的偏移
//color:文字颜色.
//size:文字大小
//chr:字符
//mode:0,非叠加显示;1,叠加显示.2,大点叠加(只适用于叠加模式)
// void gui_show_ptchar(uint16_t x,uint16_t y,uint16_t xend,uint16_t yend,uint16_t offset,uint16_t color,uint16_t size,uint8_t chr,uint8_t mode)
// {
//     uint8_t temp;
//     uint8_t t1,t;
// 	uint16_t tempoff;
// 	uint16_t y0=y;		
// 	uint16_t csize=(size/8+((size%8)?1:0))*(size/2);		//得到字体一个字符对应点阵集所占的字节数	
// 	unsigned long filesize = -1;      
//     struct stat statbuff;  			     
// 	if(chr>' ')chr=chr-' ';//得到偏移后的值
// 	else chr=0;//小于空格的一律用空格代替,比如TAB键(键值为9)
// 	FILE *FONT;	
//     if(stat((char*)APP_ASCII_S6030, &statbuff) > 0){   
//         filesize = statbuff.st_size;  
//     }  
// 	FONT = fopen(APP_ASCII_S6030,"r");//打开文件 
// 	fread(asc2_s6030,1,filesize,FONT);	//一次读取整个文件
// 	for(t=0;t<csize;t++)
// 	{   
// 		switch(size)
// 		{
// 			case 12:
// 				if(asc2_1206==NULL)return;		//非法的数组
// 				temp=asc2_1206[chr*csize+t];	//调用1206字体
// 				break;
// 			case 16:
// 				if(asc2_1608==NULL)return;		//非法的数组
// 				temp=asc2_1608[chr*csize+t];	//调用1608字体
// 				break;
// 			case 24:
// 				if(asc2_2412==NULL)return;		//非法的数组
// 				temp=asc2_2412[chr*csize+t];	//调用2412字体
// 				break;
// 			case 28:
// 				if(asc2_2814==NULL)return;		//非法的数组
// 				temp=asc2_2814[chr*csize+t];	//调用2814字体
// 				break;
// 			case 36:
// 				if(asc2_3618==NULL)return;		//非法的数组
// 				temp=asc2_3618[chr*csize+t];	//调用3618字体
// 				break;
// 			case 54:
// 				if(asc2_5427==NULL)return;		//非法的数组
// 				temp=asc2_5427[chr*csize+t];	//调用5427字体
// 				break;
// 			case 60:
// 				if(asc2_s6030==NULL)return;		//非法的数组
// 				temp=asc2_s6030[chr*csize+t];	//调用6030字体
// 				break;
// 			default://不支持的字体
// 				return;
// 		} 
// 		tempoff=offset;
// 		if(x>xend)return;	//超区域了 	
// 		if(tempoff==0)	//偏移地址到了 
// 		{			
// 			for(t1=0;t1<8;t1++)
// 			{		    
// 				if(y<=yend)
// 				{
// 					if(temp&0x80)
// 					{ 
// 						if(mode==0x02)gui_draw_bigpoint(x,y,color);	 
// 						else LCD_Fast_DrawPoint(x,y,color);
// 					}else if(mode==0)LCD_Fast_DrawPoint(x,y,BACK_COLOR); 
// 				}
// 				temp<<=1;
// 				y++;
// 				if((y-y0)==size)
// 				{
// 					y=y0;
// 					x++;
// 					break;
// 				}
// 			}
// 		}else
// 		{
// 				y+=8;
// 				if((y-y0)>=size)//大于一个字的高度了
// 				{
// 					y=y0;		//y坐标归零
//  					tempoff--;   
// 				}
// 		}	 
// 	}						     
// }
























		  






