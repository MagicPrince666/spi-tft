#include "ctiic.h"
 					 				     
//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
uint8_t CT_IIC_Send_Byte(int fd, uint8_t reg, uint8_t txd)
{                        
    int retries;
	uint8_t data[2];

	data[0] = reg;
	data[1] = txd;
	for(retries=5; retries; retries--) {
		if(write(fd, data, 2)==2)
			return 0;
		usleep(1000*10);
	}
	return -1;
} 	    
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
uint8_t CT_IIC_Read_Byte(int fd, uint8_t reg, uint8_t *val)
{
	int retries;
	for(retries=5; retries; retries--)
		if(write(fd, &reg, 1)==1)
			if(read(fd, val, 1)==1)
				return 0;
	return -1;
}

