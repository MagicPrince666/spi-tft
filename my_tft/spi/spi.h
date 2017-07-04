#ifndef __SPI_H
#define __SPI_H

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

int SPI_Transfer(const uint8_t *TxBuf, uint8_t *RxBuf, uint32_t len);
int SPI_Write(uint8_t *TxBuf, int len);
int SPI_Read(uint8_t *RxBuf, int len);
int SPI_Open(void);
int SPI_Close(void);

#endif