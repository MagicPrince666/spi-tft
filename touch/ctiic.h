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

extern int fd_iic;
uint8_t CT_IIC_Send_Byte(int fd, uint8_t reg, uint8_t txd);
uint8_t CT_IIC_Read_Byte(int fd, uint8_t reg, uint8_t *val);
//uint8_t i2c_write(int fd, uint8_t reg, uint8_t val);
//uint8_t i2c_read(int fd, uint8_t reg, uint8_t *val);

#endif
