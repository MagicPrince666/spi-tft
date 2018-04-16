#ifndef __LCD_H
#define __LCD_H

/*****************************************************************************************
 * 
 * 文件名  LCD.h
 * 描述    ：
 * 平台    ：linux
 * 版本    ：V1.0.0
 * 作者    ：小王子与木头人  QQ：846863428
 *修改时间  ：2017-07-4
*****************************************************************************************/

#include <inttypes.h>
#include "sys.h"

 //定义LCD的尺寸	
extern uint16_t SPI_LCD_RAM[320*480];//缓存大小

//LCD重要参数集
typedef struct  
{										    
	uint16_t width;			//LCD 宽度
	uint16_t height;		//LCD 高度
	uint16_t id;			//LCD ID
	uint8_t  dir;			//横屏还是竖屏控制：0，竖屏；1，横屏。	
	uint16_t	wramcmd;		//开始写gram指令
	uint16_t  setxcmd;		//设置x坐标指令
	uint16_t  setycmd;		//设置y坐标指令	 
}_lcd_dev; 

extern _lcd_dev lcddev;	//管理LCD重要参数
//IO连接  

#define LCD_DC_1 mt76x8_gpio_set_pin_value(11, 1)
#define LCD_DC_0 mt76x8_gpio_set_pin_value(11, 0)

//扫描方向定义
#define L2R_U2D  0 //从左到右,从上到下
#define L2R_D2U  1 //从左到右,从下到上
#define R2L_U2D  2 //从右到左,从上到下
#define R2L_D2U  3 //从右到左,从下到上

#define U2D_L2R  4 //从上到下,从左到右
#define U2D_R2L  5 //从上到下,从右到左
#define D2U_L2R  6 //从下到上,从左到右
#define D2U_R2L  7 //从下到上,从右到左	 

#define DFT_SCAN_DIR  L2R_U2D  //默认的扫描方向

extern  uint16_t BACK_COLOR, POINT_COLOR;   //背景色，画笔色

#define READ_LCD_ID 0x04
#define READ_LCD_STATUS 0x09
#define READ_LCD_POWER_MODE 0x0a
#define READ_LCD_MADCTL 0x0b
#define READ_LCD_PIX_FORMAT 0x0c
#define LCD_DIS_INVERSION 0x21  //反显
#define LCD_MEM_ACCESS 0x36  //存储访问控制


//LCD地址结构体
void Lcd_Init(void); 
void LCD_Clear(uint16_t Color);
void Address_set(unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2);
void LCD_WR_DATA8(char da); //发送数据-8位参数
void LCD_Fast_WR_Color_DATA16(uint16_t Color ,uint32_t len);
void LCD_Display_Dir(uint8_t dir);
void LCD_WR_DATA(uint16_t da);
void LCD_Fast_WR_DATA(uint8_t* Color ,uint32_t len);
void LCD_Fast_WR_DATA16(uint16_t* Color ,uint32_t len);
void LCD_WR_REG(uint8_t da);
void LCD_Color_Fill8(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint8_t *color);
void LCD_Color_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t *color);
//void LCD_DrawPoint(uint16_t x,uint16_t y,uint16_t color);
void LCD_DrawPoint(uint16_t x,uint16_t y);
void LCD_Fast_DrawPoint(uint16_t x,uint16_t y,uint16_t color);
uint16_t LCD_ReadPoint(uint16_t x,uint16_t y);
//void LCD_DrawPoint(uint16_t x,uint16_t y);//画点
void LCD_DrawPoint_big(uint16_t x,uint16_t y);//画一个大点
uint16_t  LCD_ReadPoint(uint16_t x,uint16_t y); //读点
//void Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r);
void LCD_Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r);
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);		   
void LCD_Fill(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color);
void LCD_ShowChar(uint16_t x,uint16_t y,uint8_t num,uint8_t size,uint8_t mode);//显示一个字符
//void LCD_ShowChar(uint16_t x,uint16_t y,uint8_t num,uint8_t size,uint8_t mode);
void LCD_ShowNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size);//显示数字
void LCD_Show2Num(uint16_t x,uint16_t y,uint16_t num,uint8_t len,uint8_t size);//显示2个数字
//void LCD_ShowString(uint16_t x,uint16_t y,uint8_t size,char *p);		 //显示一个字符串,16字体
void LCD_ShowString(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint8_t size,char *p);
//   LCD_ShowString
 
//void showhanzi(unsigned int x,unsigned int y,unsigned char index);
void showhanzi(unsigned int x,unsigned int y,char *p,uint8_t size);
void showimage(uint16_t x,uint16_t y); //显示40*40图片
void test_color(void);


//画笔颜色
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE         	 0x001F  
#define BRED             0XF81F
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40 //棕色
#define BRRED 			 0XFC07 //棕红色
#define GRAY  			 0X8430 //灰色
//GUI颜色

#define DARKBLUE      	 0X01CF	//深蓝色
#define LIGHTBLUE      	 0X7D7C	//浅蓝色  
#define GRAYBLUE       	 0X5458 //灰蓝色
//以上三色为PANEL的颜色 
 
#define LIGHTGREEN     	 0X841F //浅绿色
#define LGRAY 			 0XC618 //浅灰色(PANNEL),窗体背景色

#define LGRAYBLUE        0XA651 //浅灰蓝色(中间层颜色)
#define LBBLUE           0X2B12 //浅棕蓝色(选择条目的反色)
		  		 
#endif  

