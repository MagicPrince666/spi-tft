#ifndef __TOUCH_H
#define __TOUCH_H

/*****************************************************************************************

 * 本程序只供学习使用，未经作者许可，不得用于其它任何用途 *
 * 文件名  touch.h
 * 描述    ：
 * 平台    ：linux
 * 版本    ：V1.0.0
 * 作者    ：小王子与木头人  QQ：846863428
 *修改时间  ：2017-07-4

*****************************************************************************************/

#include <inttypes.h>
#include "sys.h"	
/* touch panel interface define */
#define  CMD_RDX  0xD0	 //触摸IC读坐标积存器
#define  CMD_RDY  0x90
//IO连接
#define DCLK_1 mt76x8_gpio_set_pin_value(15, 1)
#define DCLK_0 mt76x8_gpio_set_pin_value(15, 0)

#define CS_1 mt76x8_gpio_set_pin_value(41, 1)
#define CS_0 mt76x8_gpio_set_pin_value(41, 0)

#define DIN_1 mt76x8_gpio_set_pin_value(14, 1)
#define DIN_0 mt76x8_gpio_set_pin_value(14, 0)

#define DOUT mt76x8_gpio_get_pin(16)

#define Penirq mt76x8_gpio_get_pin(40) //检测触摸屏响应信号

struct tp_pix_
{
	uint16_t x;
	uint16_t y;
};
struct tp_pixuint32_t_
{
	uint32_t x;
	uint32_t y;
};

extern int run; 
extern struct tp_pix_  tp_pixad,tp_pixlcd;	 //当前触控坐标的AD值,前触控坐标的像素值   
extern uint16_t vx,vy;  //比例因子，此值除以1000之后表示多少个AD值代表一个像素点
extern uint16_t chx,chy;//默认像素点坐标为0时的AD起始值
uint8_t tpstate(void);
void spistar(void);  
void Touch_Adjust(void);
void point(void); //绘图函数
uint16_t ReadFromCharFrom7843();         //SPI 读数据

#endif  
	 
	 
