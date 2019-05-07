/*
* 说明：SPI通讯实现
* 方式一： 同时发送与接收实现函数： SPI_Transfer()
* 方式二：发送与接收分开来实现
* SPI_Write() 只发送
* SPI_Read() 只接收
* 两种方式不同之处：方式一，在发的过程中也在接收，第二种方式，收与发单独进行
* Created on: 2013-5-28
* Author: lzy
*/

#include "spi.h"

//#include "Debug.h"
#define SPI_DEBUG 0


static const char *device = "/dev/spidev32766.1";
static uint8_t mode = 0; /* SPI通信使用全双工，设置CPOL＝0，CPHA＝0。 */
static uint8_t bits = 8; /* 8ｂiｔｓ读写，MSB first。*/
static uint32_t speed = 96 * 1000 * 1000;/* 设置96M传输速度 */
int g_SPI_Fd = 0;


static void pabort(const char *s)
{
    perror(s);
    abort();
}

/**
* 功 能：发送数据
* 入口参数 ：
* TxBuf -> 发送数据首地址
＊len -> 发送与长度
＊返回值：0 成功
* 开发人员：Lzy 2013－5－22
*/
int SPI_Write(uint8_t *TxBuf, int len)
{
    int ret = 0;
    int fd = g_SPI_Fd;
    
    if( len<= 32)
        ret = write(fd, TxBuf, len);
    else
    {
        printf("buffer too long\n");
    }

    if (ret < 0)
    printf("SPI Write error\n");
    else
    {
#if SPI_DEBUG
        int i;
        printf("nSPI Write [Len:%d]: ", len);
        for (i = 0; i < len; i++)
        {
            if (i % 8 == 0)
            printf("nt");
            printf("0x%02X ", TxBuf[i]);
        }
        printf("\n");
#endif
    }

    return ret;
}


/**
* 功 能：接收数据
* 出口参数：
* RxBuf -> 接收数据缓冲区
* rtn -> 接收到的长度
* 返回值：>=0 成功
* 开发人员：Lzy 2013－5－22
*/
int SPI_Read(uint8_t *RxBuf, int len)
{
    int ret;
    int fd = g_SPI_Fd;
    ret = read(fd, RxBuf, len);
    if (ret < 0)
    printf("SPI Read errorn\n");
    else
    {
    #if SPI_DEBUG
        int i;
        printf("SPI Read [len:%d]:", len);
        for (i = 0; i < len; i++)
        {
            if (i % 8 == 0)
            printf("nt");
            printf("0x%02X ", RxBuf[i]);
        }
        printf("\n");
    #endif
    }

    return ret;
}


/**
* 功 能：打开设备 并初始化设备
* 入口参数 ：
* 出口参数：
* 返回值：0 表示已打开 0XF1 表示SPI已打开 其它出错
* 开发人员：Lzy 2013－5－22
*/
int SPI_Open(void)
{
    int fd;
    int ret = 0;


    if (g_SPI_Fd != 0) /* 设备已打开 */
    return 0xF1;


    fd = open(device, O_RDWR);
    if (fd < 0)
        pabort("can't open device");
    else
        printf("SPI %s - Open Succeed. Start Init SPI...\n",device);


    g_SPI_Fd = fd;
    /*
    * spi mode
    */
    ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
    if (ret == -1)
        pabort("can't set spi mode");


    ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
    if (ret == -1)
        pabort("can't get spi mode");


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


    printf("spi mode: %d\n", mode);
    printf("bits per word: %d\n", bits);
    printf("max speed: %d KHz (%d MHz)\n", speed / 1000, speed / 1000 / 1000);


    return ret;
}


/**
* 功 能：关闭SPI模块
*/
int SPI_Close(void)
{
    if (g_SPI_Fd == 0) /* SPI是否已经打开*/
    return 0;
    close(g_SPI_Fd);

    return 0;
}

