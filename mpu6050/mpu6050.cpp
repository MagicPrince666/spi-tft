/************************************************************/
//文件名：mpu6050.c
//功能:测试linux下iic读写mpu6050程序
//使用说明: (1)
//          (2)
//          (3)
//          (4)
//作者:huangea
//日期:2016-10-03
/************************************************************/
#include "mpu6050.h"
#include "iic.h"

//MPU6050初始化
uint8_t MPU6050_Init()
{
	if(ioctl(fd_iic, I2C_SLAVE, Address)<0) {    //set i2c address 
		printf("fail to set i2c device slave address!\n");
		close(fd_iic);
		return -1;
	}
	printf("set slave address to 0x%x success!\n", Address);

	i2c_write(fd_iic,PWR_MGMT_1,0X00);    
	i2c_write(fd_iic,SMPLRT_DIV,0X07); 
	i2c_write(fd_iic,CONFIG,0X06); 
	i2c_write(fd_iic,ACCEL_CONFIG,0X01); 

	return 1;
}


//get data
uint16_t GetData(unsigned char REG_Address)
{
	uint8_t H,L;
	i2c_read(fd_iic, REG_Address, &H);
	i2c_read(fd_iic, REG_Address + 1, &L);

	return (H<<8)+L;
}

// main
void *  mpu_thread(void *arg) 
{
	MPU6050_Init();
	usleep(1000*100);
	while(1)
	{
		printf("\033[2J");
		usleep(1000*200);
		printf("ACCE_X:%6d\n ",GetData(ACCEL_XOUT_H));
		printf("ACCE_Y:%6d\n ",GetData(ACCEL_YOUT_H));
		printf("ACCE_Z:%6d\n ",GetData(ACCEL_ZOUT_H));
		printf("GYRO_X:%6d\n ",GetData(GYRO_XOUT_H));
		printf("GYRO_Y:%6d\n ",GetData(GYRO_YOUT_H));
		printf("GYRO_Z:%6d\n ",GetData(GYRO_ZOUT_H));
	}
	close(fd_iic);
	pthread_exit(NULL);
}

uint8_t MPU_Write_Len(uint8_t addr,uint8_t reg,uint8_t len,uint8_t *buf)
{
	addr = addr<<1|0;
	write(fd_iic, &addr, 1);
	write(fd_iic, &reg, 1);
	write(fd_iic, buf, len);
	
	return 0;
} 

uint8_t MPU_Read_Len(uint8_t addr,uint8_t reg,uint8_t len,uint8_t *buf)
{ 
	write(fd_iic, &reg, 1);
	addr = addr<<1|1;
	write(fd_iic, &addr, 1);
	read(fd_iic, buf, len);
	
	return 0;
}