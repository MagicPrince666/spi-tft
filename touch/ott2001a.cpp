#include "ott2001a.h"
#include "touch.h"
#include "ctiic.h"

//向OTT2001A写入一次数据
//reg:起始寄存器地址
//buf:数据缓缓存区
//len:写数据长度
//返回值:0,成功;1,失败.
uint8_t OTT2001A_WR_Reg(uint16_t reg,uint8_t *buf,uint8_t len)
{
	uint8_t cmd;
	cmd = OTT_CMD_WR;
 	write(fd_iic,&cmd,1);   //发送写命令 	 
	//write(fd_iic,reg>>8,1);   	//发送高8位地址
	write(fd_iic,&reg,2);   	//发送低8位地址
	
	if(len == write(fd_iic, buf, len))	    
	return 0; 
	else return 1;
}
//从OTT2001A读出一次数据
//reg:起始寄存器地址
//buf:数据缓缓存区
//len:读数据长度			  
void OTT2001A_RD_Reg(uint16_t reg,uint8_t *buf,uint8_t len)
{
	uint8_t cmd;
	cmd = OTT_CMD_WR;
 	write(fd_iic,&cmd,1);   //发送写命令 	 
	//write(fd_iic,reg>>8,1);   	//发送高8位地址
	write(fd_iic,&reg,1);   	//发送低8位地址
	cmd = OTT_CMD_RD;
	write(fd_iic,&cmd,1);   //发送读命令		   
	
	read(fd_iic,buf,len); //发数据  
}
//传感器打开/关闭操作
//cmd:1,打开传感器;0,关闭传感器
void OTT2001A_SensorControl(uint8_t cmd)
{
	uint8_t regval=0X00;
	if(cmd)regval=0X80;
	OTT2001A_WR_Reg(OTT_CTRL_REG,&regval,1); 
} 
//初始化触摸屏
//返回值:0,初始化成功;1,初始化失败 
uint8_t OTT2001A_Init(void)
{
 	uint8_t regval=0; 
	 
	OTT_RST_0;				//复位
	usleep(100*1000);
 	OTT_RST_1;				//释放复位		    
	usleep(100*1000); 
	OTT2001A_SensorControl(1);	//打开传感器 
	OTT2001A_RD_Reg(OTT_CTRL_REG,&regval,1);//读取传感器运行寄存器的值来判断I2C通信是否正常
	printf("CTP ID:%x\r\n",regval);
    if(regval==0x80)return 0;
	return 1;
}

const uint16_t OTT_TPX_TBL[5]={OTT_TP1_REG,OTT_TP2_REG,OTT_TP3_REG,OTT_TP4_REG,OTT_TP5_REG};
//扫描触摸屏(采用查询方式)
//mode:0,正常扫描.
//返回值:当前触屏状态.
//0,触屏无触摸;1,触屏有触摸
uint8_t OTT2001A_Scan(uint8_t mode)
{
	uint8_t buf[4];
	uint8_t i=0;
	uint8_t res=0;
	static uint8_t t=0;//控制查询间隔,从而降低CPU占用率   
	t++;
	if((t%10)==0||t<10)//空闲时,每进入10次CTP_Scan函数才检测1次,从而节省CPU使用率
	{
		OTT2001A_RD_Reg(OTT_GSTID_REG,&mode,1);//读取触摸点的状态  
		if(mode&0X1F)
		{
			tp_dev.sta=(mode&0X1F)|TP_PRES_DOWN|TP_CATH_PRES;
			for(i=0;i<5;i++)
			{
				if(tp_dev.sta&(1<<i))	//触摸有效?
				{
					OTT2001A_RD_Reg(OTT_TPX_TBL[i],buf,4);	//读取XY坐标值
					if(tp_dev.touchtype&0X01)//横屏
					{
						tp_dev.y[i]=(((uint16_t)buf[2]<<8)+buf[3])*OTT_SCAL_Y;
						tp_dev.x[i]=800-((((uint16_t)buf[0]<<8)+buf[1])*OTT_SCAL_X);
					}else
					{
						tp_dev.x[i]=(((uint16_t)buf[2]<<8)+buf[3])*OTT_SCAL_Y;
						tp_dev.y[i]=(((uint16_t)buf[0]<<8)+buf[1])*OTT_SCAL_X;
					}  
					//printf("x[%d]:%d,y[%d]:%d\r\n",i,tp_dev.x[i],i,tp_dev.y[i]);
				}			
			} 
			res=1;
			if(tp_dev.x[0]==0 && tp_dev.y[0]==0)mode=0;	//读到的数据都是0,则忽略此次数据
			t=0;		//触发一次,则会最少连续监测10次,从而提高命中率
		}
	}
	if((mode&0X1F)==0)//无触摸点按下
	{ 
		if(tp_dev.sta&TP_PRES_DOWN)	//之前是被按下的
		{
			tp_dev.sta&=~(1<<7);	//标记按键松开
		}else						//之前就没有被按下
		{ 
			tp_dev.x[0]=0xffff;
			tp_dev.y[0]=0xffff;
			tp_dev.sta&=0XE0;	//清除点有效标记	
		}	 
	} 	
	if(t>240)t=10;//重新从10开始计数
	return res;
}
 