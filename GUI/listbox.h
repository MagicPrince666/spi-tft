#ifndef __LISTBOX_H
#define __LISTBOX_H 
#include "guix.h"  	 			  
#include "scrollbar.h"


//////listbox上的符号的默认颜色 
#define LBOX_DFT_LBKCOLOR		0XFFFF		//内部背景色
#define LBOX_DFT_LSELCOLOR		0XFFFF		//选中list后的字体颜色
#define LBOX_DFT_LSELBKCOLOR	0X001F		//选中list后的背景色
#define LBOX_DFT_LNCOLOR		0X0000		//未选中的list字体颜色
#define LBOX_DFT_RIMCOLOR		0XF800		//边框颜色

////listboxtype
#define LBOX_TYPE_RIM 			0X01		//有边框的 
//滚条宽度
#define LBOX_SCB_WIDTH 			0X0E		//滚动条宽度为15个像素

#pragma pack (1)    
//list结构体.链表结构
typedef struct 
{
	void * prevlist;
	void * nextlist;
	uint32_t id;
	uint8_t *name;
}_listbox_list;

//listbox结构体定义
typedef struct 
{
	uint16_t top; 				  		//listbox顶端坐标
	uint16_t left;                       //listbox左端坐标
	uint16_t width; 				  		//宽度
	uint16_t height;						//高度 必须为12/16的倍数

	uint8_t type;						//类型标记字
									//[bit7]:1,需要画滚动条出来(条件是totalitems>itemsperpage);0,不需要画出来.(此位由软件自动控制)
									//[bit6:0]:保留					 
								
	uint8_t sta;							//listbox状态,[bit7]:滑动标志;[bit6]:编号有效的标志;[bit5:0]:第一次按下的编号.	 
	uint8_t id;							//listbox 的id
	uint8_t dbclick;					   	//双击,
									//[7]:0,没有双击.1,有双击.
									//[6~0]:0,保留.

	uint8_t font;						//文字字体 12/16
	uint16_t selindex;					//选中的索引
 
	uint16_t lbkcolor;					//内部背景颜色
 	uint16_t lnselcolor; 				//list name 选中后的颜色
  	uint16_t lnselbkcolor; 				//list name 选中后的背景颜色
	uint16_t lncolor; 					//list name 未选中的颜色
	uint16_t rimcolor;					//边框颜色

	uint8_t *fname;						//当前选中的index的名字
	uint16_t namelen;					//name所占的点数.
	uint16_t curnamepos;					//当前的偏移
	uint32_t oldtime;					//上一次更新时间 

	_scrollbar_obj * scbv;			//垂直滚动条
	_listbox_list *list;			//链表	    
}_listbox_obj;
#pragma pack ()

_listbox_obj * listbox_creat(uint16_t left,uint16_t top,uint16_t width,uint16_t height,uint8_t type,uint8_t font);//创建listbox
_listbox_list * list_search(_listbox_list *listx,uint16_t index);//查找编号为index的list,并取得详细信息
void listbox_delete(_listbox_obj *listbox_del);	 			//删除listbox
uint8_t listbox_check(_listbox_obj * listbox,void * in_key);		//检查listbox的按下状态
uint8_t listbox_addlist(_listbox_obj * listbox,uint8_t *name);		//增加一个list
void listbox_draw_list(_listbox_obj *listbox);				//画list
void listbox_draw_listbox(_listbox_obj *listbox);			//重画listbox
void listbox_2click_hook(_listbox_obj *listbox);			//双击钩子函数
#endif




