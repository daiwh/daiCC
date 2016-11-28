#include "daicc.h"

#include <string.h>

/***********************************************************************************/
/*                                   函数申明                                      */
/***********************************************************************************/
int elf_hash(char* key);                      //根据关键字计算哈希地址
void *mallocz(int size);                      //分配堆内存，并初始化为0


/*********************************
     根据关键字计算哈希地址
**********************************/
int elf_hash(char* key)
{
	int h=0,g;
	while(*key)
	{
		h=(h<<4)+*key++;
		g=h&0xf0000000;
		if(g)
			h^=g>>24;
	}
	return h%MAXKEY;
}

/********************************************
    运算符，关键字，常量标识优先放入单词表
*********************************************/
TKWord *tkword_direct_insert(TKWord *tp)
{
	int keyno;
	dynarray_add(& tktable,tp);   //将单词放入单词表
	keyno=elf_hash(tp->spelling); //根据单词的拼写求出所对应的哈希码
	//如果有哈希冲突，将当前单词放在哈希冲突队列的队头位置
	tp->next=tk_hashtable[keyno]; 
	tk_hashtable[keyno]=tp;       //将要插入的单词的地址放在哈希表中
	return tp;
}

/******************************************
             在单词表中查找单词
*******************************************/
TKWord * tkword_find(char *p,int keyno)
{
	TKWord *tp=NULL, *tp1;
	for(tp1=tk_hashtable[keyno];tp1!=NULL;tp1=tp1->next)
	{
		if(strcmp(p,tp1->spelling)==0) //如果查找成功，更新token，返回单词指针
		{
			token=tp1->tkcode;
			tp=tp1;
		}
	}
	return tp;
}
/***************************************
    分配堆内存，并将数据初始化为0
****************************************/
void *mallocz(int size)
{
	void *ptr;
	ptr=malloc(size);
	if(!ptr&&size)
		error("内存分配失败");
	memset(ptr,0,size);
	return ptr;
}
/************************************************
           标识符插入单词表
		   先查找，查找不到再插入单词表
************************************************/
TKWord * tkword_insert(char *p)
{
	TKWord *tp;
	int keyno;
	char *s;
	char *end;
	int lenth;
	keyno=elf_hash(p);
	tp=tkword_find(p,keyno);
	if(tp==NULL)
	{
		lenth=strlen(p);
		//申请的用来存放单词的拼写的地址是跟在结构的地址后面的，这样可以减少内存碎片化，申请结构的地址空间时一同申请
		tp=(TKWord*)mallocz(sizeof(TKWord)+lenth+1);
		//初始化单词
		tp->tkcode=tktable.count-1;
		s=(char*)tp+sizeof(TKWord);
		tp->spelling=(char*)s;
		for(end=p+lenth;p<end;)
			*s++=*p++;
		*s=(char)'\0';
		//将单词地址放入哈希表
		tp->next=tk_hashtable[keyno];
		tk_hashtable[keyno]=tp;
		//将单词放入单词表
		dynarray_add(&tktable,tp);
	}
	return tp;
}