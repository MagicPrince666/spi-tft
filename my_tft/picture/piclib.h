#ifndef __PICLIB_H
#define __PICLIB_H	  		  
#include "sys.h" 
#include "LCD.h"
#include "malloc.h"
#include "bmp.h"
#include "tjpgd.h"
#include "gif.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK MiniSTM32开发板
//图片解码 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2014/3/14
//版本：V2.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved
//********************************************************************************
//升级说明
//V2.0
//1,将jpeg解码库换成了TJPGD,支持更多的jpg/jpeg文件,支持小尺寸图片快速jpeg显示 
//2,pic_phy里面新增fillcolor函数,用于填充显示,以提高小尺寸jpg图片的显示速度
//3,ai_load_picfile函数,新增一个参数:fast,用于设置是否使能jpeg/jpg快速显示
//注意:这里的小尺寸是指:jpg/jpeg图片尺寸小于等于LCD尺寸.
//////////////////////////////////////////////////////////////////////////////////

#define PIC_FORMAT_ERR		0x27	//格式错误
#define PIC_SIZE_ERR		0x28	//图片尺寸错误
#define PIC_WINDOW_ERR		0x29	//窗口设定错误
#define PIC_MEM_ERR			0x11	//内存错误

//f_typetell返回的类型定义
//根据表FILE_TYPE_TBL获得.在exfuns.c里面定义
#define T_BIN		0X00	//bin文件
#define T_LRC		0X10	//lrc文件
#define T_NES		0X20	//nes文件
#define T_TEXT		0X30	//.txt文件
#define T_C			0X31	//.c文件
#define T_H			0X32    //.h文件
#define T_FLAC		0X4C	//flac文件
#define T_BMP		0X50	//bmp文件
#define T_JPG		0X51	//jpg文件
#define T_JPEG		0X52	//jpeg文件		 
#define T_GIF		0X53	//gif文件  
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef TRUE
#define TRUE    1
#endif
#ifndef FALSE
#define FALSE   0
#endif 

//图片显示物理层接口  
//在移植的时候,必须由用户自己实现这几个函数
typedef struct 
{
	uint16_t(*read_point)(uint16_t,uint16_t);				//uint16_t read_point(uint16_t x,uint16_t y)						读点函数
	void(*draw_point)(uint16_t,uint16_t,uint16_t);			//void draw_point(uint16_t x,uint16_t y,uint16_t color)		    画点函数
 	void(*fill)(uint16_t,uint16_t,uint16_t,uint16_t,uint16_t);		///void fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t color) 单色填充函数 	 
 	void(*draw_hline)(uint16_t,uint16_t,uint16_t,uint16_t);		//void draw_hline(uint16_t x0,uint16_t y0,uint16_t len,uint16_t color)  画水平线函数	 
 	void(*fillcolor)(uint16_t,uint16_t,uint16_t,uint16_t,uint16_t*);	//void piclib_fill_color(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint16_t *color) 颜色填充
}_pic_phy; 

extern _pic_phy pic_phy;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//图像信息
typedef struct
{		
	uint16_t lcdwidth;	//LCD的宽度
	uint16_t lcdheight;	//LCD的高度
	uint32_t ImgWidth; 	//图像的实际宽度和高度
	uint32_t ImgHeight;

	uint32_t Div_Fac;  	//缩放系数 (扩大了8192倍的)
	
	uint32_t S_Height; 	//设定的高度和宽度
	uint32_t S_Width;
	
	uint32_t	S_XOFF;	  	//x轴和y轴的偏移量
	uint32_t S_YOFF;

	uint32_t staticx; 	//当前显示到的ｘｙ坐标
	uint32_t staticy;																 	
}_pic_info;
extern _pic_info picinfo;//图像信息
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void piclib_init(void);								//初始化画图
uint16_t piclib_alpha_blend(uint16_t src,uint16_t dst,uint8_t alpha);	//alphablend处理
void ai_draw_init(void);							//初始化智能画图
uint8_t is_element_ok(uint16_t x,uint16_t y,uint8_t chg);				//判断像素是否有效
uint8_t ai_load_picfile(const uint8_t *filename,uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint8_t fast,uint8_t temp);//智能画图
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
