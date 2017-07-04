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
static uint8_t bits = 8; /* ８ｂiｔｓ读写，MSB first。*/
static uint32_t speed = 12 * 1000 * 1000;/* 设置12M传输速度 */
static uint16_t delay = 0;
static int g_SPI_Fd = 0;


static void pabort(const char *s)
{
    perror(s);
    abort();
}


/**
* 功 能：同步数据传输
* 入口参数 ：
* TxBuf -> 发送数据首地址
* len -> 交换数据的长度
* 出口参数：
* RxBuf -> 接收数据缓冲区
* 返回值：0 成功
* 开发人员：Lzy 2013－5－22
*/

int SPI_Transfer(const uint8_t *TxBuf, uint8_t *RxBuf, uint32_t len)
{
    int ret;
    int fd = g_SPI_Fd;
    
    struct spi_ioc_transfer tr ={
        .tx_buf = (unsigned long) TxBuf,
        .rx_buf = (unsigned long) RxBuf,
        .len = len,
        .speed_hz = speed,
        .delay_usecs = delay,
        .bits_per_word = bits,
        .cs_change = 0,
        .tx_nbits = 8,
        .rx_nbits = 8,
        .pad = 0,
    };

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1)
    printf("can't send spi message");
    else
    {
    #if SPI_DEBUG
        int i;
        printf("nsend spi message Succeed");
        printf("nSPI Send [Len:%d]: ", len);
        for (i = 0; i < len; i++)
        {
            if (i % 8 == 0)
            printf("nt");
            printf("0x%02X ", TxBuf[i]);
        }
        printf("n");


        printf("SPI Receive [len:%d]:", len);
        for (i = 0; i < len; i++)
        {
            if (i % 8 == 0)
            printf("nt");
            printf("0x%02X ", RxBuf[i]);
        }
        printf("n");
    #endif
    }
    return ret;
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
    int ret;
    int fd = g_SPI_Fd;

    ret = write(fd, TxBuf, len);
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
        printf("SPI - Open Succeed. Start Init SPI...\n");


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
    int fd = g_SPI_Fd;


    if (fd == 0) /* SPI是否已经打开*/
    return 0;
    close(fd);
    g_SPI_Fd = 0;


    return 0;
}


/**
* 功 能：自发自收测试程序
* 接收到的数据与发送的数据如果不一样 ，则失败
* 说明：
* 在硬件上需要把输入与输出引脚短跑
* 开发人员：Lzy 2013－5－22
*/
int SPI_LookBackTest(void)
{
    int ret, i;
    const int BufSize = 16;
    uint8_t tx[BufSize], rx[BufSize];


    bzero(rx, sizeof(rx));
    for (i = 0; i < BufSize; i++)
    tx[i] = i;

    printf("\nSPI - LookBack Mode Test...\n");
    ret = SPI_Transfer(tx, rx, BufSize);
    if (ret > 1)
    {
        ret = memcmp(tx, rx, BufSize);
        if (ret != 0)
        {
            printf("LookBack Mode Test error\n");
        //pabort("error");
        }
        else
            printf("SPI - LookBack Mode OK\n");
    }

    return ret;
}