#ifndef __IIC_H
#define __IIC_H

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

int I2C_open();
uint8_t i2c_write(int fd, uint8_t reg, uint8_t val);
uint8_t i2c_read(int fd, uint8_t reg, uint8_t *val);
void I2C_close();

#endif