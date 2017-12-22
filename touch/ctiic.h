#ifndef __MYCT_IIC_H
#define __MYCT_IIC_H

#include "sys.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include <inttypes.h>	    
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板
//4.3寸/7寸电容触摸屏驱动-IIC通信部分	  
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2015/1/15
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved	 
//////////////////////////////////////////////////////////////////////////////////
extern int fd_iic;
uint8_t CT_IIC_Send_Byte(int fd, uint8_t reg, uint8_t txd);
uint8_t CT_IIC_Read_Byte(int fd, uint8_t reg, uint8_t *val);
//uint8_t i2c_write(int fd, uint8_t reg, uint8_t val);
//uint8_t i2c_read(int fd, uint8_t reg, uint8_t *val);

#endif
