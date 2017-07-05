#include "piclib.h"
#include "gif.h"	 
#include <unistd.h>	    
				    

const uint16_t _aMaskTbl[16] =
{
	0x0000, 0x0001, 0x0003, 0x0007,
	0x000f, 0x001f, 0x003f, 0x007f,
	0x00ff, 0x01ff, 0x03ff, 0x07ff,
	0x0fff, 0x1fff, 0x3fff, 0x7fff,
};	  
const uint8_t _aInterlaceOffset[]={8,8,4,2};
const uint8_t _aInterlaceYPos  []={0,4,2,1};
 
uint8_t gifdecoding=0;//标记GIF正在解码.


//定义是否使用malloc,这里我们选择使用malloc
#if GIF_USE_MALLOC==0 	
gif89a tgif89a;			//gif89a文件
FILE f_gfile;			//gif 文件
LZW_INFO tlzw;			//lzw
#endif

//检测GIF头
//返回值:0,是GIF89a/87a;非零,非GIF89a/87a
uint8_t gif_check_head(FILE *file)
{
	uint8_t gifversion[6];
//	uint32_t readed;
	uint8_t res;
	res = fread(gifversion,1,6,file);
	if(res)return 1;	   
	if((gifversion[0]!='G')||(gifversion[1]!='I')||(gifversion[2]!='F')||
	(gifversion[3]!='8')||((gifversion[4]!='7')&&(gifversion[4]!='9'))||
	(gifversion[5]!='a'))return 2;
	else return 0;	
}
//将RGB888转为RGB565
//ctb:RGB888颜色数组首地址.
//返回值:RGB565颜色.
uint16_t gif_getrgb565(uint8_t *ctb) 
{
	uint16_t r,g,b;
	r=(ctb[0]>>3)&0X1F;
	g=(ctb[1]>>2)&0X3F;
	b=(ctb[2]>>3)&0X1F;
	return b+(g<<5)+(r<<11);
}
//读取颜色表
//file:文件;
//gif:gif信息;
//num:tbl大小.
//返回值:0,OK;其他,失败;
uint8_t gif_readcolortbl(FILE *file,gif89a * gif,uint16_t num)
{
	uint8_t rgb[3];
	uint16_t t;
	uint8_t res;
//	uint32_t readed;
	for(t=0;t<num;t++)
	{
		res = fread(rgb,1,3,file);
		if(res)return 1;//读错误
		gif->colortbl[t]=gif_getrgb565(rgb);
	}
	return 0;
} 
//得到逻辑屏幕描述,图像尺寸等
//file:文件;
//gif:gif信息;
//返回值:0,OK;其他,失败;
uint8_t gif_getinfo(FILE *file,gif89a * gif)
{
//	uint32_t readed;	 
	uint8_t res;   
	res = fread((uint8_t*)&gif->gifLSD,1,7,file);
	if(res)return 1;
	if(gif->gifLSD.flag&0x80)//存在全局颜色表
	{
		gif->numcolors=2<<(gif->gifLSD.flag&0x07);//得到颜色表大小
		if(gif_readcolortbl(file,gif,gif->numcolors))return 1;//读错误	
	}	   
	return 0;
}
//保存全局颜色表	 
//gif:gif信息;
void gif_savegctbl(gif89a* gif)
{
	uint16_t i=0;
	for(i=0;i<256;i++)gif->bkpcolortbl[i]=gif->colortbl[i];//保存全局颜色.
}
//恢复全局颜色表	 
//gif:gif信息;
void gif_recovergctbl(gif89a* gif)
{
	uint16_t i=0;
	for(i=0;i<256;i++)gif->colortbl[i]=gif->bkpcolortbl[i];//恢复全局颜色.
}

//初始化LZW相关参数	   
//gif:gif信息;
//codesize:lzw码长度
void gif_initlzw(gif89a* gif,uint8_t codesize) 
{
 	memset((uint8_t *)gif->lzw, 0, sizeof(LZW_INFO));
	gif->lzw->SetCodeSize  = codesize;
	gif->lzw->CodeSize     = codesize + 1;
	gif->lzw->ClearCode    = (1 << codesize);
	gif->lzw->EndCode      = (1 << codesize) + 1;
	gif->lzw->MaxCode      = (1 << codesize) + 2;
	gif->lzw->MaxCodeSize  = (1 << codesize) << 1;
	gif->lzw->ReturnClear  = 1;
	gif->lzw->LastByte     = 2;
	gif->lzw->sp           = gif->lzw->aDecompBuffer;
}

//读取一个数据块
//gfile:gif文件;
//buf:数据缓存区
//maxnum:最大读写数据限制
uint16_t gif_getdatablock(FILE *gfile,uint8_t *buf,uint16_t maxnum) 
{
	uint8_t cnt;
//	uint32_t readed;
	uint32_t fpos;
	fread(&cnt,1,1,gfile);//得到LZW长度			 
	if(cnt) 
	{
		if (buf)//需要读取 
		{
			if(cnt>maxnum)
			{
				fpos=ftell(gfile);
				fseek(gfile,fpos+cnt,SEEK_SET);//跳过
				return cnt;//直接不读
			}
			fread(buf,1,cnt,gfile);//得到LZW长度	
		}else 	//直接跳过
		{
			fpos=ftell(gfile);
			fseek(gfile,fpos+cnt,SEEK_SET);//跳过
		}
	}
	return cnt;
}
//ReadExtension		 
//Purpose:
//Reads an extension block. One extension block can consist of several data blocks.
//If an unknown extension block occures, the routine failes.
//返回值:0,成功;
// 		 其他,失败
uint8_t gif_readextension(FILE *gfile,gif89a* gif, int *pTransIndex,uint8_t *pDisposal)
{
	uint8_t temp;
//	uint32_t readed;	 
	uint8_t buf[4];  
	fread(&temp,1,1,gfile);//得到长度		 
	switch(temp)
	{
		case GIF_PLAINTEXT:
		case GIF_APPLICATION:
		case GIF_COMMENT:
			while(gif_getdatablock(gfile,0,256)>0);			//获取数据块
			return 0;
		case GIF_GRAPHICCTL://图形控制扩展块
			if(gif_getdatablock(gfile,buf,4)!=4)return 1;	//图形控制扩展块的长度必须为4 
 		 	gif->delay=(buf[2]<<8)|buf[1];					//得到延时 
			*pDisposal=(buf[0]>>2)&0x7; 	    			//得到处理方法
			if((buf[0]&0x1)!=0)*pTransIndex=buf[3];			//透明色表 
			fread(&temp,1,1,gfile);	 		//得到LZW长度	
 			if(temp!=0)return 1;							//读取数据块结束符错误.
			return 0;
	}
	return 1;//错误的数据
}

//从LZW缓存中得到下一个LZW码,每个码包含12位
//返回值:<0,错误.
//		 其他,正常.
int gif_getnextcode(FILE *gfile,gif89a* gif) 
{
	int i,j,End;
	long Result;
	if(gif->lzw->ReturnClear)
	{
		//The first code should be a clearcode.
		gif->lzw->ReturnClear=0;
		return gif->lzw->ClearCode;
	}
	End=gif->lzw->CurBit+gif->lzw->CodeSize;
	if(End>=gif->lzw->LastBit)
	{
		int Count;
		if(gif->lzw->GetDone)return-1;//Error 
		gif->lzw->aBuffer[0]=gif->lzw->aBuffer[gif->lzw->LastByte-2];
		gif->lzw->aBuffer[1]=gif->lzw->aBuffer[gif->lzw->LastByte-1];
		if((Count=gif_getdatablock(gfile,&gif->lzw->aBuffer[2],300))==0)gif->lzw->GetDone=1;
		if(Count<0)return -1;//Error 
		gif->lzw->LastByte=2+Count;
		gif->lzw->CurBit=(gif->lzw->CurBit-gif->lzw->LastBit)+16;
		gif->lzw->LastBit=(2+Count)*8;
		End=gif->lzw->CurBit+gif->lzw->CodeSize;
	}
	j=End>>3;
	i=gif->lzw->CurBit>>3;
	if(i==j)Result=(long)gif->lzw->aBuffer[i];
	else if(i+1==j)Result=(long)gif->lzw->aBuffer[i]|((long)gif->lzw->aBuffer[i+1]<<8);
	else Result=(long)gif->lzw->aBuffer[i]|((long)gif->lzw->aBuffer[i+1]<<8)|((long)gif->lzw->aBuffer[i+2]<<16);
	Result=(Result>>(gif->lzw->CurBit&0x7))&_aMaskTbl[gif->lzw->CodeSize];
	gif->lzw->CurBit+=gif->lzw->CodeSize;
	return(int)Result;
}	
//得到LZW的下一个码
//返回值:<0,错误(-1,不成功;-2,读到结束符了)
//		 >=0,OK.(LZW的第一个码)
int gif_getnextbyte(FILE *gfile,gif89a* gif) 
{
	int i,Code,Incode;
	while((Code=gif_getnextcode(gfile,gif))>=0)
	{
		if(Code==gif->lzw->ClearCode)
		{
			//Corrupt GIFs can make this happen  
			if(gif->lzw->ClearCode>=(1<<MAX_NUM_LWZ_BITS))return -1;//Error 
			//Clear the tables 
			memset((uint8_t*)gif->lzw->aCode,0,sizeof(gif->lzw->aCode));
			for(i=0;i<gif->lzw->ClearCode;++i)gif->lzw->aPrefix[i]=i;
			//Calculate the'special codes' independence of the initial code size
			//and initialize the stack pointer 
			gif->lzw->CodeSize=gif->lzw->SetCodeSize+1;
			gif->lzw->MaxCodeSize=gif->lzw->ClearCode<<1;
			gif->lzw->MaxCode=gif->lzw->ClearCode+2;
			gif->lzw->sp=gif->lzw->aDecompBuffer;
			//Read the first code from the stack after clear ingand initializing*/
			do
			{
				gif->lzw->FirstCode=gif_getnextcode(gfile,gif);
			}while(gif->lzw->FirstCode==gif->lzw->ClearCode);
			gif->lzw->OldCode=gif->lzw->FirstCode;
			return gif->lzw->FirstCode;
		}
		if(Code==gif->lzw->EndCode)return -2;//End code
		Incode=Code;
		if(Code>=gif->lzw->MaxCode)
		{
			*(gif->lzw->sp)++=gif->lzw->FirstCode;
			Code=gif->lzw->OldCode;
		}
		while(Code>=gif->lzw->ClearCode)
		{
			*(gif->lzw->sp)++=gif->lzw->aPrefix[Code];
			if(Code==gif->lzw->aCode[Code])return Code;
			if((gif->lzw->sp-gif->lzw->aDecompBuffer)>= (uint8_t)sizeof(gif->lzw->aDecompBuffer))return Code;
			Code=gif->lzw->aCode[Code];
		}
		*(gif->lzw->sp)++=gif->lzw->FirstCode=gif->lzw->aPrefix[Code];
		if((Code=gif->lzw->MaxCode)<(1<<MAX_NUM_LWZ_BITS))
		{
			gif->lzw->aCode[Code]=gif->lzw->OldCode;
			gif->lzw->aPrefix[Code]=gif->lzw->FirstCode;
			++gif->lzw->MaxCode;
			if((gif->lzw->MaxCode>=gif->lzw->MaxCodeSize)&&(gif->lzw->MaxCodeSize<(1<<MAX_NUM_LWZ_BITS)))
			{
				gif->lzw->MaxCodeSize<<=1;
				++gif->lzw->CodeSize;
			}
		}
		gif->lzw->OldCode=Incode;
		if(gif->lzw->sp>gif->lzw->aDecompBuffer)return *--(gif->lzw->sp);
	}
	return Code;
}
//DispGIFImage		 
//Purpose:
//   This routine draws a GIF image from the current pointer which should point to a
//   valid GIF data block. The size of the desired image is given in the image descriptor.
//Return value:
//  0 if succeed
//  1 if not succeed
//Parameters:
//  pDescriptor  - Points to a IMAGE_DESCRIPTOR structure, which contains infos about size, colors and interlacing.
//  x0, y0       - Obvious.
//  Transparency - Color index which should be treated as transparent.
//  Disposal     - Contains the disposal method of the previous image. If Disposal == 2, the transparent pixels
//                 of the image are rendered with the background color.
uint8_t gif_dispimage(FILE *gfile,gif89a* gif,uint16_t x0,uint16_t y0,int Transparency, uint8_t Disposal) 
{
//	uint32_t readed;	   
   	uint8_t lzwlen;
	int Index,OldIndex,XPos,YPos,YCnt,Pass,Interlace,XEnd;
	int Width,Height,Cnt,ColorIndex;
	uint16_t bkcolor;
	uint16_t *pTrans;

	Width=gif->gifISD.width;
	Height=gif->gifISD.height;
	XEnd=Width+x0-1;
	bkcolor=gif->colortbl[gif->gifLSD.bkcindex];
	pTrans=(uint16_t*)gif->colortbl;
	fread(&lzwlen,1,1,gfile);//得到LZW长度	 
	gif_initlzw(gif,lzwlen);//Initialize the LZW stack with the LZW code size 
	Interlace=gif->gifISD.flag&0x40;//是否交织编码
	for(YCnt=0,YPos=y0,Pass=0;YCnt<Height;YCnt++)
	{
		Cnt=0;
		OldIndex=-1;
		for(XPos=x0;XPos<=XEnd;XPos++)
		{
			if(gif->lzw->sp>gif->lzw->aDecompBuffer)Index=*--(gif->lzw->sp);
		    else Index=gif_getnextbyte(gfile,gif);	   
			if(Index==-2)return 0;//Endcode     
			if((Index<0)||(Index>=gif->numcolors))
			{
				//IfIndex out of legal range stop decompressing
				return 1;//Error
			}
			//If current index equals old index increment counter
			if((Index==OldIndex)&&(XPos<=XEnd))Cnt++;
	 		else
			{
				if(Cnt)
				{
					if(OldIndex!=Transparency)
					{									    
						pic_phy.draw_hline(XPos-Cnt-1,YPos,Cnt+1,*(pTrans+OldIndex));
					}else if(Disposal==2)
					{
						pic_phy.draw_hline(XPos-Cnt-1,YPos,Cnt+1,bkcolor);
					}
					Cnt=0;
				}else
				{
					if(OldIndex>=0)
					{
						if(OldIndex!=Transparency)pic_phy.draw_point(XPos-1,YPos,*(pTrans+OldIndex));
					 	else if(Disposal==2)pic_phy.draw_point(XPos-1,YPos,bkcolor); 
					}
				}
			}
			OldIndex=Index;
		}
		if((OldIndex!=Transparency)||(Disposal==2))
		{
			if(OldIndex!=Transparency)ColorIndex=*(pTrans+OldIndex);
		    else ColorIndex=bkcolor;
	 		if(Cnt)
			{
				pic_phy.draw_hline(XPos-Cnt-1,YPos,Cnt+1,ColorIndex);
			}else pic_phy.draw_point(XEnd,YPos,ColorIndex);	 
		}
		//Adjust YPos if image is interlaced 
		if(Interlace)//交织编码
		{
			YPos+=_aInterlaceOffset[Pass];
			if((YPos-y0)>=Height)
			{
				++Pass;
				YPos=_aInterlaceYPos[Pass]+y0;
			}
		}else YPos++;	    
	}
	return 0;
}  			   
//恢复成背景色
//x,y:坐标
//gif:gif信息.
//pimge:图像描述块信息
void gif_clear2bkcolor(uint16_t x,uint16_t y,gif89a* gif,ImageScreenDescriptor pimge)
{
	uint16_t x0,y0,x1,y1;
	uint16_t color=gif->colortbl[gif->gifLSD.bkcindex];
	if(pimge.width==0||pimge.height==0)return;//直接不用清除了,原来没有图像!!
	if(gif->gifISD.yoff>pimge.yoff)
	{
   		x0=x+pimge.xoff;
		y0=y+pimge.yoff;
		x1=x+pimge.xoff+pimge.width-1;;
		y1=y+gif->gifISD.yoff-1;
		if(x0<x1&&y0<y1&&x1<320&&y1<320)pic_phy.fill(x0,y0,x1,y1,color); //设定xy,的范围不能太大.
	}
	if(gif->gifISD.xoff>pimge.xoff)
	{
   		x0=x+pimge.xoff;
		y0=y+pimge.yoff;
		x1=x+gif->gifISD.xoff-1;;
		y1=y+pimge.yoff+pimge.height-1;
		if(x0<x1&&y0<y1&&x1<320&&y1<320)pic_phy.fill(x0,y0,x1,y1,color);
	}
	if((gif->gifISD.yoff+gif->gifISD.height)<(pimge.yoff+pimge.height))
	{
   		x0=x+pimge.xoff;
		y0=y+gif->gifISD.yoff+gif->gifISD.height-1;
		x1=x+pimge.xoff+pimge.width-1;;
		y1=y+pimge.yoff+pimge.height-1;
		if(x0<x1&&y0<y1&&x1<320&&y1<320)pic_phy.fill(x0,y0,x1,y1,color);
	}
 	if((gif->gifISD.xoff+gif->gifISD.width)<(pimge.xoff+pimge.width))
	{
   		x0=x+gif->gifISD.xoff+gif->gifISD.width-1;
		y0=y+pimge.yoff;
		x1=x+pimge.xoff+pimge.width-1;;
		y1=y+pimge.yoff+pimge.height-1;
		if(x0<x1&&y0<y1&&x1<320&&y1<320)pic_phy.fill(x0,y0,x1,y1,color);
	}   
}

//画GIF图像的一帧
//gfile:gif文件.
//x0,y0:开始显示的坐标
uint8_t gif_drawimage(FILE *gfile,gif89a* gif,uint16_t x0,uint16_t y0)
{		  
	uint32_t readed;
	uint8_t res,temp;    
	uint16_t numcolors;
	ImageScreenDescriptor previmg;

	uint8_t Disposal;
	int TransIndex;
	uint8_t Introducer;
	TransIndex=-1;				  
	do
	{
		res=fread(&Introducer,1,1,gfile);//读取一个字节
		if(res)return 1;   
		switch(Introducer)
		{		 
			case GIF_INTRO_IMAGE://图像描述
				previmg.xoff=gif->gifISD.xoff;
 				previmg.yoff=gif->gifISD.yoff;
				previmg.width=gif->gifISD.width;
				previmg.height=gif->gifISD.height;

				res=fread((uint8_t*)&gif->gifISD,1,9,gfile);//读取一个字节
				if(res)return 1;			 
				if(gif->gifISD.flag&0x80)//存在局部颜色表
				{							  
					gif_savegctbl(gif);//保存全局颜色表
					numcolors=2<<(gif->gifISD.flag&0X07);//得到局部颜色表大小
					if(gif_readcolortbl(gfile,gif,numcolors))return 1;//读错误	
				}
				if(Disposal==2)gif_clear2bkcolor(x0,y0,gif,previmg); 
				gif_dispimage(gfile,gif,x0+gif->gifISD.xoff,y0+gif->gifISD.yoff,TransIndex,Disposal);
 				while(1)
				{
					fread(&temp,1,1,gfile);//读取一个字节
					if(temp==0)break;
					readed=ftell(gfile);//还存在块.	
					if(fseek(gfile,readed+temp,SEEK_SET))break;//继续向后偏移	 
			    }
				if(temp!=0)return 1;//Error 
				return 0;
			case GIF_INTRO_TERMINATOR://得到结束符了
				return 2;//代表图像解码完成了.
			case GIF_INTRO_EXTENSION:
				//Read image extension*/
				res=gif_readextension(gfile,gif,&TransIndex,&Disposal);//读取图像扩展块消息
				if(res)return 1;
	 			break;
			default:
				return 1;
		}
	}while(Introducer!=GIF_INTRO_TERMINATOR);//读到结束符了
	return 0;
}

//退出当前解码.
void gif_quit(void)
{
	gifdecoding=0;
}

//解码一个gif文件
//本例子不能显示尺寸大与给定尺寸的gif图片!!!
//filename:带路径的gif文件名字
//x,y,width,height:显示坐标及区域大小.
uint8_t gif_decode(const uint8_t *filename,uint16_t x,uint16_t y,uint16_t width,uint16_t height)
{
	uint8_t res=0;
	uint16_t dtime=0;//解码延时
	gif89a *mygif89a;
	FILE *gfile;
#if GIF_USE_MALLOC==1 	//定义是否使用malloc,这里我们选择使用malloc
	mygif89a=(gif89a*)malloc(sizeof(gif89a));
	if(mygif89a==NULL)res=PIC_MEM_ERR;//申请内存失败    
	mygif89a->lzw=(LZW_INFO*)malloc(sizeof(LZW_INFO));
	if(mygif89a->lzw==NULL)res=PIC_MEM_ERR;//申请内存失败 
#else
	mygif89a=&tgif89a;
	mygif89a->lzw=&tlzw;
#endif

	if(res==0)//OK
	{
		gfile = fopen((char *)filename,"r");
		if(gfile != NULL)//打开文件ok
		{
			if(gif_check_head(gfile))res=PIC_FORMAT_ERR;
			if(gif_getinfo(gfile,mygif89a))res=PIC_FORMAT_ERR;
			if(mygif89a->gifLSD.width>width||mygif89a->gifLSD.height>height)res=PIC_SIZE_ERR;//尺寸太大.
			else
			{
				x=(width-mygif89a->gifLSD.width)/2+x;
				y=(height-mygif89a->gifLSD.height)/2+y;
			}
			gifdecoding=1;
			while(gifdecoding&&res==0)//解码循环
			{	 
				res=gif_drawimage(gfile,mygif89a,x,y);//显示一张图片
				if(mygif89a->gifISD.flag&0x80)gif_recovergctbl(mygif89a);//恢复全局颜色表
				if(mygif89a->delay)dtime=mygif89a->delay;
				else dtime=10;//默认延时
				while(dtime--&&gifdecoding)usleep(10 * 1000);//延迟
				if(res==2)
				{
					res=0;
					break;
				}
			}
		}
		fclose(gfile);
	}   
#if GIF_USE_MALLOC==1 	//定义是否使用malloc,这里我们选择使用malloc
	free(mygif89a->lzw);
	free(mygif89a); 
#endif 
	return res;
}

