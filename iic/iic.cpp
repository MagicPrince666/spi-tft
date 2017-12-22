#include "iic.h"

int fd_iic = -1;

int I2C_open()
{
    fd_iic = open("/dev/i2c-0", O_RDWR);   // open file and enable read and  write

	if(fd_iic < 0)
	{
		perror("Can't open /dev/MPU6050 \n"); // open i2c dev file fail
		exit(1);
	}
	printf("open /dev/i2c-0 success !\n");   // open i2c dev file succes
    return 0;
}

//MPU6050 wirte byte
uint8_t i2c_write(int fd, uint8_t reg, uint8_t val)
{
	int retries;
	uint8_t data[2];

	data[0] = reg;
	data[1] = val;
	for(retries=5; retries; retries--) {
		if(write(fd, data, 2)==2)
			return 0;
		usleep(1000*10);
	}
	return -1;
}

//MPU6050 read byte
uint8_t i2c_read(int fd, uint8_t reg, uint8_t *val)
{
	int retries;

	for(retries=5; retries; retries--)
		if(write(fd, &reg, 1)==1)
			if(read(fd, val, 1)==1)
				return 0;
	return -1;
}

void I2C_close()
{
	if(fd_iic > 0)
		close(fd_iic);
}