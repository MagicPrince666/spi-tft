#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <fcntl.h>             
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
//#include <signal.h>
//#include <execinfo.h>

#include <asm/types.h>         
#include <linux/videodev2.h>
#include "capture.h"
#include "LCD.h"


#define VIDEO_WIDTH 640   
#define VIDEO_HEIGHT 480  
#define BUFFER_COUNT 4   

#define CLEAR(x) memset (&(x), 0, sizeof (x))


struct buffer {
	void * 		start;		//记录缓冲帧地址
	size_t 		length;		//一帧的大小
};


//pic process---------------------------------------------------------------------------------------------------------------------------------------------------------------

#pragma pack(1)  
typedef struct BITMAPFILEHEADER{
      unsigned short bfType;//位图文件的类型,  
      unsigned long bfSize;//位图文件的大小，以字节为单位  
      unsigned short bfReserved1;//位图文件保留字，必须为0  
      unsigned short bfReserved2;//同上  
      unsigned long bfOffBits;//位图阵列的起始位置，以相对于位图文件   或者说是头的偏移量表示，以字节为单位  
} BITMAPFILEHEADER;
#pragma pack() 

typedef struct BITMAPINFOHEADER{//位图信息头类型的数据结构，用于说明位图的尺寸      
      unsigned long biSize;//位图信息头的长度，以字节为单位  
      unsigned long biWidth;//位图的宽度，以像素为单位  
      unsigned long biHeight;//位图的高度，以像素为单位  
      unsigned short biPlanes;//目标设备的级别,必须为1  
      unsigned short biBitCount;//每个像素所需的位数，必须是1(单色),4(16色),8(256色)或24(2^24色)之一  
      unsigned long biCompression;//位图的压缩类型，必须是0-不压缩，1-BI_RLE8压缩类型或2-BI_RLE4压缩类型之一  
      unsigned long biSizeImage;//位图大小，以字节为单位  
      unsigned long biXPelsPerMeter;//位图目标设备水平分辨率，以每米像素数为单位  
      unsigned long biYPelsPerMeter;//位图目标设备垂直分辨率，以每米像素数为单位  
      unsigned long biClrUsed;//位图实际使用的颜色表中的颜色变址数  
      unsigned long biClrImportant;//位图显示过程中被认为重要颜色的变址数  
} BITMAPINFOHEADER;
 

void create_bmp_header(struct BITMAPFILEHEADER * bfh,struct BITMAPINFOHEADER * bih){
      bfh->bfType = (unsigned short)0x4D42;
      bfh->bfSize = (unsigned long)(14 + 40 + VIDEO_WIDTH * VIDEO_HEIGHT*3);
      bfh->bfReserved1 = 0;
      bfh->bfReserved2 = 0;
      bfh->bfOffBits = (unsigned long)(14 + 40);


      bih->biBitCount = 24;
      bih->biWidth = VIDEO_WIDTH;
      bih->biHeight = VIDEO_HEIGHT;
      bih->biSizeImage = VIDEO_WIDTH * VIDEO_HEIGHT * 3;
      bih->biClrImportant = 0;
      bih->biClrUsed = 0;
      bih->biCompression = 0;
      bih->biPlanes = 1;
      bih->biSize = 40;//sizeof(bih);  
      bih->biXPelsPerMeter = 0x00000ec4;
      bih->biYPelsPerMeter = 0x00000ec4;

     printf("----create bmp header successfully !----\n");
}

void store_bmp(FILE * fd,char * file_name,unsigned char * newBuf,int bmp_len,struct BITMAPFILEHEADER * bfh,struct BITMAPINFOHEADER * bih){
    
    fd = fopen(file_name, "wb");
    if (fd < 0) {
            printf("open frame data file failed\n");
            return;
    }
    fwrite(bfh,sizeof(*bfh),1,fd);
    fwrite(bih,sizeof(*bih),1,fd);
    fwrite(newBuf, 1, bmp_len, fd);
    fclose(fd);
    printf("----store bmp successfuylly ,bmp name is :  %s----\n", file_name);
    return;
}

/*
* YUYV 转 RGB 算法
*/

void yuyv2rgb(unsigned int index, struct buffer * buffers,unsigned char * newBuf){
	unsigned char YUYV[4],RGB[6];
    int j,k;
    unsigned int i;
	unsigned int location  = 0;
	unsigned char * starter;
	
	starter                = (unsigned char *)buffers[index].start;  /* starter 代表当前数据帧在用户空间的首地址*/
	j=0;
        for(i=0;i < buffers[index].length;i+=4){
		YUYV[0]=starter[i];//Y0  
		YUYV[1]=starter[i+1];//U  
		YUYV[2]=starter[i+2];//Y1  
		YUYV[3]=starter[i+3];//V  
		if(YUYV[0]<1){	
			RGB[0]=0;
			RGB[1]=0;
			RGB[2]=0;
		}else{
			RGB[0]=YUYV[0]+1.772*(YUYV[1]-128);//b  
			RGB[1]=YUYV[0]-0.34413*(YUYV[1]-128)-0.71414*(YUYV[3]-128);//g  
			RGB[2]=YUYV[0]+1.402*(YUYV[3]-128);//r  
		}	
		if(YUYV[2]<0){	
			RGB[3]=0;
			RGB[4]=0;
			RGB[5]=0;
		}else{
			RGB[3]=YUYV[2]+1.772*(YUYV[1]-128);//b  
			RGB[4]=YUYV[2]-0.34413*(YUYV[1]-128)-0.71414*(YUYV[3]-128);//g  
			RGB[5]=YUYV[2]+1.402*(YUYV[3]-128);//r  
		}
		for(k=0;k<6;k++){
			if(RGB[k]<0)
				RGB[k]=0;
			if(RGB[k]>255)
				RGB[k]=255;
		}		
		//请记住：扫描行在位图文件中是反向存储的！  
		if(j%(VIDEO_WIDTH*3)==0){//定位存储位置
			location=(VIDEO_HEIGHT-j/(VIDEO_WIDTH*3))*(VIDEO_WIDTH*3);
		}
		bcopy(RGB,newBuf+location+(j%(VIDEO_WIDTH*3)),sizeof(RGB));
		j+=6;
        }
	
	//LCD_Color_Fill(0,0,lcddev.width - 1,lcddev.height - 1,buffer);
	printf("---- yuyv 2 rgb CHANGE DONE ! ----");
	return;
}

/*
 *去噪算法
*/
void move_noise(unsigned char * newBuf){//双滤波器  
	int i,j,k,temp[3],temp1[3];
	unsigned char BGR[13*3];
	unsigned int sq,sq1,loc,loc1;
	int h=VIDEO_HEIGHT,w=VIDEO_WIDTH;

	for(i=2;i<h-2;i++){
		for(j=2;j<w-2;j++){
			memcpy(BGR,newBuf+(i-1)*w*3+3*(j-1),9);
			memcpy(BGR+9,newBuf+i*w*3+3*(j-1),9);
			memcpy(BGR+18,newBuf+(i+1)*w*3+3*(j-1),9);
			memcpy(BGR+27,newBuf+(i-2)*w*3+3*j,3);
			memcpy(BGR+30,newBuf+(i+2)*w*3+3*j,3);
			memcpy(BGR+33,newBuf+i*w*3+3*(j-2),3);
			memcpy(BGR+36,newBuf+i*w*3+3*(j+2),3);
			
			memset(temp,0,4*3);
			for(k=0;k<9;k++){
				temp[0]+=BGR[k*3];
				temp[1]+=BGR[k*3+1];
				temp[2]+=BGR[k*3+2];
			}

			temp1[0]=temp[0];
			temp1[1]=temp[1];
			temp1[2]=temp[2];
			
			for(k=9;k<13;k++){
				temp1[0]+=BGR[k*3];
				temp1[1]+=BGR[k*3+1];
				temp1[2]+=BGR[k*3+2];
			}

			for(k=0;k<3;k++){
				temp[k]/=9;
				temp1[k]/=13;
			}
	
			sq=0xffffffff;loc=0;
			sq1=0xffffffff;loc1=0;
			unsigned int a;
			
			for(k=0;k<9;k++){
				a=abs(temp[0]-BGR[k*3])+abs(temp[1]-BGR[k*3+1])+abs(temp[2]-BGR[k*3+2]);
				if(a<sq){
					sq=a;
					loc=k;
				}
			}
			for(k=0;k<13;k++){
				a=abs(temp1[0]-BGR[k*3])+abs(temp1[1]-BGR[k*3+1])+abs(temp1[2]-BGR[k*3+2]);
				if(a<sq1){
					sq1=a;
					loc1=k;
				}
			}
			newBuf[i*w*3+3*j]=(unsigned char)((BGR[3*loc]+BGR[3*loc1])/2);
              		newBuf[i*w*3+3*j+1]=(unsigned char)((BGR[3*loc+1]+BGR[3*loc1+1])/2);
             	        newBuf[i*w*3+3*j+2]=(unsigned char)((BGR[3*loc+2]+BGR[3*loc1+2])/2);
             		 /*还是有些许的噪点  
   		 	temp[0]=(BGR[3*loc]+BGR[3*loc1])/2;  
    			temp[1]=(BGR[3*loc+1]+BGR[3*loc1+1])/2;  
    			temp[2]=(BGR[3*loc+2]+BGR[3*loc1+2])/2;  
    			sq=abs(temp[0]-BGR[loc*3])+abs(temp[1]-BGR[loc*3+1])+abs(temp[2]-BGR[loc*3+2]);  
    			sq1=abs(temp[0]-BGR[loc1*3])+abs(temp[1]-BGR[loc1*3+1])+abs(temp[2]-BGR[loc1*3+2]);  
    			if(sq1<sq) loc=loc1;  
    			newBuf[i*w*3+3*j]=BGR[3*loc];  
    			newBuf[i*w*3+3*j+1]=BGR[3*loc+1];  
    			newBuf[i*w*3+3*j+2]=BGR[3*loc+2];*/
   		}
    	}
	printf("----move noise successfully---- !\n");
    	return;
}

//end of pic process------------------------------------------------------------------------------------------------------------------------------------------------------


//v4l2 -------------------------------------------------------------------------------------------------------------------------------------------------------------------

/*
查询设备能力，v4l2 规定这一步是必须（暂时这里不指定特别的功能） */
void do_cap(int fd,struct v4l2_capability * cap ){
  if( ioctl(fd,VIDIOC_QUERYCAP,cap) != -1 ){
    printf("  1:  step1:device capbility checed succssfully\n");
    return;
  }
  exit(EXIT_FAILURE);
}


/*
规定设备帧捕获格式的信息
*/
void init_fmt(struct  v4l2_format  * fmt){
  CLEAR(*fmt);
  fmt->type                     = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt->fmt.pix.width            = VIDEO_WIDTH;
  fmt->fmt.pix.height           = VIDEO_HEIGHT;
  fmt->fmt.pix.pixelformat      = V4L2_PIX_FMT_YUYV;  
  //fmt->fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;       
  fmt->fmt.pix.field            = V4L2_FIELD_INTERLACED;   
  return;
}
/*
设置设备帧捕获格式
*/
void do_set_fmt(int fd,struct  v4l2_format  * fmt){
    if(ioctl(fd,VIDIOC_S_FMT,fmt) != -1){
       printf("  2:  Set devic format  succssfully\n");
       return;
    }
    exit(EXIT_FAILURE);
}

/*
初始化缓冲帧请求表单
*/
void init_req(struct v4l2_requestbuffers * req){;
  CLEAR(*req);
  req->count                   = BUFFER_COUNT;
  req->type                    = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req->memory                  = V4L2_MEMORY_MMAP;
}

/*
 *发送命令申请缓冲帧
*/
void do_reqBuf(int fd,struct v4l2_requestbuffers * req ){
  if(ioctl(fd,VIDIOC_REQBUFS,req) != -1){
       printf("  3:  request buffers successfully \n");
       return;
    }
    exit(EXIT_FAILURE);
}

/*
 *获得驱动开辟的缓冲区的物理地址，并将此地址映射到用户空间可用的虚拟地址
*/
void process_bufferAddr(int fd,int num, struct buffer * buffers){
        
	int i;

        for (i = 0; i < num; ++i)
        {
           struct v4l2_buffer buf;   
           CLEAR (buf);
           buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
           buf.memory      = V4L2_MEMORY_MMAP;
           buf.index       = i;            //记录当前帧是buffers中的第及帧  

           if (-1 == ioctl (fd, VIDIOC_QUERYBUF, &buf)){ //获得缓冲帧的物理地址
                printf ("VIDIOC_QUERYBUF error\n");
                exit(EXIT_FAILURE);
           }
           buffers[i].length = buf.length;
      
           buffers[i].start =            //将内存中缓冲帧的物理地址映射为用户空间地址
           mmap (NULL,
                buf.length, 
                PROT_READ | PROT_WRITE ,
                MAP_SHARED,
                fd, buf.m.offset);
           if (MAP_FAILED == buffers[i].start){
                printf ("mmap failed\n");
		exit(EXIT_FAILURE);
	   }     
        }//end of for
	
       printf("  4:  address processed successfully! \n");
       return;
}


/*
 *操作内存中已开辟好空间的缓冲帧入队列，准备记录数据信息
*/
void in_queue(int fd, int num){
	int i;
	for(i = 0; i < num; ++i){
		struct v4l2_buffer buf;         //工作数据帧，用户填好index和type字段后，传给驱动，驱动根据这些信息去处理内存中实际存在的数据帧
		CLEAR(buf);
		buf.index     =  i;
		buf.type      =  V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory    = V4L2_MEMORY_MMAP;
			
	        if(-1 == ioctl(fd, VIDIOC_QBUF,&buf)){
			printf("VIDIOC_QBUF error !\n");
			exit(EXIT_FAILURE);
		}
	}
	
        printf("  5:  in queue successfully !\n");
	return;
}


/*
 *开始采集数据 
*/
void do_capture(int fd){
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if( -1 == ioctl(fd,VIDIOC_STREAMON,&type)){
		printf("VIDIOC_STREAMON error ! \n");
		exit(EXIT_FAILURE);
	}
	
        printf("  6:  do capture  successfully !\n");
	return;
}

/*
 *将数据帧存储为jpg图片
*/

void save_asJpg(__u32 index, struct buffer * buffers){
	
	/*
           采集到的图片相关的变量
        */
        FILE *           pic_fd;		       		//图片文件描述符
        //unsigned long    pic_length;		                //文件大小
        char *           pic_fileName        = (char*)"bin/test.jpg";      //图像文件名

	pic_fd = fopen(pic_fileName,"wb");
	fwrite(buffers[index].start, buffers[index].length, 1,pic_fd);
	
	fclose(pic_fd);                                         /*关闭文件*/
	
        printf("----  save pic as jpg  successfully !----\n");
	return;
}

void save_asBmp(unsigned int index, struct buffer * buffers){

	FILE *         bmp_fd;         /*新创建的bmp文件的文件描述副*/
	char *         bmp_name =  (char*)"bin/testBMP.bmp";

	unsigned char * bmpBuf;        /*为了处理jpg 转 bmp ，需要在内存中开辟一段工作空间*/
	int bmp_len;                   /*bmp 图片大小*/
        
	struct BITMAPFILEHEADER bfh;   /*bmp 文件结构体*/
	struct BITMAPINFOHEADER bih;   

	bmp_len			=  buffers[index].length*3/2;
        bmpBuf			=  (unsigned char*)calloc((unsigned int)bmp_len,sizeof(unsigned char));
	
        if(!bmpBuf){
   		 printf("cannot assign the memory for bmp!\n");
    		 exit(EXIT_FAILURE);
        }

	yuyv2rgb(index,buffers,bmpBuf);   /*将 buffers[index]代表的jpg格式的数据转换成bmp格式*/
	
	move_noise(bmpBuf);
	
	create_bmp_header(&bfh,&bih);
	store_bmp(bmp_fd,bmp_name,bmpBuf,bmp_len,&bfh,&bih);
	
}

/*
 * 将一帧放入数据缓冲队列
*/
int put_oneFrame_inQue(int camera_fd,struct v4l2_buffer * buf){
	ioctl(camera_fd,VIDIOC_QBUF,buf);
	return buf->index;
}

/*
*此函数由monitor_queue 函数调用，用来从设备文件中读取视频数据
*/
int read_frame (int camera_fd,struct buffer * buffers)
{
	struct v4l2_buffer buf;
	//unsigned int i;

	CLEAR (buf);
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	ioctl (camera_fd, VIDIOC_DQBUF, &buf);       /*从数据缓冲队列中取出数据帧*/

	//assert (buf.index < num);   
    
    save_asBmp(buf.index,buffers);
	
	save_asJpg(buf.index,buffers);              /* 将读取到的帧存为jpg格式的图片  */
	
	

	put_oneFrame_inQue(camera_fd,&buf);         /*重新把帧插入队列*/

        printf("  7:  read frame  successfully !\n");
	return 1;
}


/*
*监听数据队列数据帧的状态，状态发生改变（说明采集到了数据）,则调用read_frame函数返回数据帧
*/
void monitor_queue(int fd,struct buffer * buffers){
	
	while (1) 
        {
           fd_set fds;           //为select函数设置监听文件集，即：fds文件集合中的所有文件的状态将会被监听
           struct timeval tv;    //设置等待超时时间
           int ret;

           FD_ZERO (&fds);
           FD_SET (fd, &fds);

           /* Timeout. */
           tv.tv_sec = 2;
           tv.tv_usec = 0;

           ret = select (fd + 1, &fds, NULL, NULL, &tv);
          
           //监听出错
           if (-1 == ret) {
          	 if (EINTR == errno)
                	 continue;
                 printf ("select err\n");
           }
	   //等待超时
           if (0 == ret) {
           	fprintf (stderr, "select timeout\n");
                exit (EXIT_FAILURE);
           }
           //当ret != -1 && != 0时，则ret的值表示状态改变的文件个数
           if (read_frame (fd,buffers))
                break;
        } 

        printf("  8:  do capture  successfully !\n");
	return;
}



/*
 *将mmap映射的地址空间还原
*/

void release_mmap_buffer(int num,struct buffer * buffers){

	int i;

	for (i = 0; i < num; ++i){
		if (-1 == munmap (buffers[i].start, buffers[i].length)){
			printf ("munmap error !\n");
			exit(EXIT_FAILURE);
		}
	}
	printf("  9:  mmap buffer released successfuly !\n");
	return;
}
/*
初始化程序的上下s，然后开始视频采s
 */
void init_context(){
	/*
	   摄像头设备相关变量 
        */
	char *          camera_fileName      = (char*)"/dev/video0";   // linux下摄像头设备的文件名
	int             camera_fd            = -1;              // 摄像头的文件描述符
	
	/*
         视频采集相关变量              
        */
	struct v4l2_capability   cap;                  //查询设备的能力
    struct v4l2_format       fmt;                  //设置设备当前驱动的帧捕获格式
    //enum   v4l2_buf_type     type;                 //定义设备的采集方式
	struct v4l2_requestbuffers req;
	
    struct buffer *          buffers;              //向系统内存申请缓冲区，用于在内存中保存数据帧信息

	/*
	 * 图片采集处理相关变量
	*/
    //unsigned char *          bmp_picBuf;
	//int 			 bmp_len;


	/*
         启动采集工作
       */

 	//以阻塞方式打开一个设备文件
	camera_fd = open(camera_fileName,O_RDWR,0);  
      
    //1、v4l2 规定此步必须有
    do_cap(camera_fd,&cap);
    
    //2、设置设备捕获数据格式
    init_fmt(&fmt);
    do_set_fmt(camera_fd,&fmt);
    
    //3、申请缓冲区
    init_req(&req);
    do_reqBuf(camera_fd,&req);
        
	//4、获得缓冲帧地址，并且将其映射到用户空间      
    buffers = (buffer*)calloc (req.count, sizeof (*buffers));   //系统为为应用程序分配缓冲区，并清零
    process_bufferAddr(camera_fd,req.count,buffers);   //处理地址问题，函数返回后，buffers中即可获得缓冲帧的虚拟地址（应用程序可操作）	

	//5、通知驱动把内存中的数据帧入队，准备记录数据信息
	in_queue(camera_fd,req.count);

	//6、通知驱动开始视频采集
	do_capture(camera_fd);	
	
	//7、监听数据队列，并从队列中取出数据帧
	monitor_queue(camera_fd,buffers);

	//8、图片处理



    //9、清场
	/* 解除地址映射 */
	release_mmap_buffer(req.count,buffers); 
	/* 关闭设备文件*/
    close(camera_fd);
	/* 关闭图像文件*/
}

// static void _signal_handler(int signum)  
// {  
//     void *array[10];  
//     size_t size;  
//     char **strings;  
//     size_t i;  
  
//     signal(signum, SIG_DFL); /* 还原默认的信号处理handler */  
  
//     size = backtrace (array, 10);  
//     strings = (char **)backtrace_symbols (array, size);  
  
//     fprintf(stderr, "widebright received SIGSEGV! Stack trace:\n");  
//     for (i = 0; i < size; i++) {  
//         fprintf(stderr, "%d %s \n",i,strings[i]);  
//     }  
      
//     free (strings);  
//     exit(1);  
// } 

// int main(int argc,char** argv){
//     signal(SIGPIPE, _signal_handler);    // SIGPIPE，管道破裂。
//     signal(SIGSEGV, _signal_handler);    // SIGSEGV，非法内存访问
//     signal(SIGFPE, _signal_handler);     // SIGFPE，数学相关的异常，如被0除，浮点溢出，等等
//     signal(SIGABRT, _signal_handler);    // SIGABRT，由调用abort函数产生，进程非正常退出
//     init_context();
// }








