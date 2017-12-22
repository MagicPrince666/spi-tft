#include "ft5206.h"
#include "touch.h"
#include "ctiic.h"
#include "string.h" 

 
//向FT5206写入一次数据
//reg:起始寄存器地址
//buf:数据缓缓存区
//len:写数据长度
//返回值:0,成功;1,失败.
uint8_t FT5206_WR_Reg(uint16_t reg,uint8_t *buf,uint8_t len){
	reg &= 0x00ff;
	reg |= FT_CMD_WR << 8;
	write(fd_iic,&reg,2);   	//发送低8位地址
	if(len == write(fd_iic, buf, len))
	return 0; 
	else return 1; 
}
//从FT5206读出一次数据
//reg:起始寄存器地址
//buf:数据缓缓存区
//len:读数据长度			  
void FT5206_RD_Reg(uint16_t reg,uint8_t *buf,uint8_t len)
{	
	uint8_t cmd;
	//cmd = FT_CMD_WR;
 	//write(fd_iic,&cmd,1);   	//发送写命令 
	reg &= 0x00ff;
	reg |= FT_CMD_WR << 8;	 	 										  		   
 	write(fd_iic,&reg,2);   	//发送低8位地址
	cmd = FT_CMD_RD;
	write(fd_iic,&cmd,1);   	//发送读命令		   
    read(fd_iic,buf,len); //发数据	     
} 
//初始化FT5206触摸屏
//返回值:0,初始化成功;1,初始化失败 
uint8_t FT5206_Init(void)
{
	uint8_t temp[2];  							 

	FT_RST_0;				//复位
	usleep(20*1000);
 	FT_RST_1;				//释放复位		    
	usleep(50*1000);  	
	temp[0]=0;
	FT5206_WR_Reg(FT_DEVIDE_MODE,temp,1);	//进入正常操作模式 
	FT5206_WR_Reg(FT_ID_G_MODE,temp,1);		//查询模式 
	temp[0]=22;								//触摸有效值，22，越小越灵敏	
	FT5206_WR_Reg(FT_ID_G_THGROUP,temp,1);	//设置触摸有效值
	temp[0]=12;								//激活周期，不能小于12，最大14
	FT5206_WR_Reg(FT_ID_G_PERIODACTIVE,temp,1); 
	//读取版本号，参考值：0x3003
	FT5206_RD_Reg(FT_ID_G_LIB_VERSION,&temp[0],2);  
	if(temp[0]==0X30&&temp[1]==0X03)//版本:0X3003
	{ 
		printf("CTP ID:%x\r\n",((uint16_t)temp[0]<<8)+temp[1]);
		return 0;
	} 
	return 1;
}
const uint16_t FT5206_TPX_TBL[5]={FT_TP1_REG,FT_TP2_REG,FT_TP3_REG,FT_TP4_REG,FT_TP5_REG};
//扫描触摸屏(采用查询方式)
//mode:0,正常扫描.
//返回值:当前触屏状态.
//0,触屏无触摸;1,触屏有触摸
uint8_t FT5206_Scan(uint8_t mode)
{
	uint8_t buf[4];
	uint8_t i=0;
	uint8_t res=0;
	uint8_t temp;
	static uint8_t t=0;//控制查询间隔,从而降低CPU占用率   
	t++;
	if((t%10)==0||t<10)//空闲时,每进入10次CTP_Scan函数才检测1次,从而节省CPU使用率
	{
		FT5206_RD_Reg(FT_REG_NUM_FINGER,&mode,1);//读取触摸点的状态  
		if((mode&0XF)&&((mode&0XF)<6))
		{
			temp=0XFF<<(mode&0XF);//将点的个数转换为1的位数,匹配tp_dev.sta定义 
			tp_dev.sta=(~temp)|TP_PRES_DOWN|TP_CATH_PRES; 
			for(i=0;i<5;i++)
			{
				if(tp_dev.sta&(1<<i))	//触摸有效?
				{
					FT5206_RD_Reg(FT5206_TPX_TBL[i],buf,4);	//读取XY坐标值 
					if(tp_dev.touchtype&0X01)//横屏
					{
						tp_dev.y[i]=((uint16_t)(buf[0]&0X0F)<<8)+buf[1];
						tp_dev.x[i]=((uint16_t)(buf[2]&0X0F)<<8)+buf[3];
					}else
					{
						tp_dev.x[i]=480-(((uint16_t)(buf[0]&0X0F)<<8)+buf[1]);
						tp_dev.y[i]=((uint16_t)(buf[2]&0X0F)<<8)+buf[3];
					}  
					if((buf[0]&0XF0)!=0X80)tp_dev.x[i]=tp_dev.y[i]=0;//必须是contact事件，才认为有效
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
 
