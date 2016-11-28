#include "daicc.h"

#include <string.h>

/***********************************************************************************/
/*                                   ��������                                      */
/***********************************************************************************/
int elf_hash(char* key);                      //���ݹؼ��ּ����ϣ��ַ
void *mallocz(int size);                      //������ڴ棬����ʼ��Ϊ0


/*********************************
     ���ݹؼ��ּ����ϣ��ַ
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
    ��������ؼ��֣�������ʶ���ȷ��뵥�ʱ�
*********************************************/
TKWord *tkword_direct_insert(TKWord *tp)
{
	int keyno;
	dynarray_add(& tktable,tp);   //�����ʷ��뵥�ʱ�
	keyno=elf_hash(tp->spelling); //���ݵ��ʵ�ƴд�������Ӧ�Ĺ�ϣ��
	//����й�ϣ��ͻ������ǰ���ʷ��ڹ�ϣ��ͻ���еĶ�ͷλ��
	tp->next=tk_hashtable[keyno]; 
	tk_hashtable[keyno]=tp;       //��Ҫ����ĵ��ʵĵ�ַ���ڹ�ϣ����
	return tp;
}

/******************************************
             �ڵ��ʱ��в��ҵ���
*******************************************/
TKWord * tkword_find(char *p,int keyno)
{
	TKWord *tp=NULL, *tp1;
	for(tp1=tk_hashtable[keyno];tp1!=NULL;tp1=tp1->next)
	{
		if(strcmp(p,tp1->spelling)==0) //������ҳɹ�������token�����ص���ָ��
		{
			token=tp1->tkcode;
			tp=tp1;
		}
	}
	return tp;
}
/***************************************
    ������ڴ棬�������ݳ�ʼ��Ϊ0
****************************************/
void *mallocz(int size)
{
	void *ptr;
	ptr=malloc(size);
	if(!ptr&&size)
		error("�ڴ����ʧ��");
	memset(ptr,0,size);
	return ptr;
}
/************************************************
           ��ʶ�����뵥�ʱ�
		   �Ȳ��ң����Ҳ����ٲ��뵥�ʱ�
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
		//�����������ŵ��ʵ�ƴд�ĵ�ַ�Ǹ��ڽṹ�ĵ�ַ����ģ��������Լ����ڴ���Ƭ��������ṹ�ĵ�ַ�ռ�ʱһͬ����
		tp=(TKWord*)mallocz(sizeof(TKWord)+lenth+1);
		//��ʼ������
		tp->tkcode=tktable.count-1;
		s=(char*)tp+sizeof(TKWord);
		tp->spelling=(char*)s;
		for(end=p+lenth;p<end;)
			*s++=*p++;
		*s=(char)'\0';
		//�����ʵ�ַ�����ϣ��
		tp->next=tk_hashtable[keyno];
		tk_hashtable[keyno]=tp;
		//�����ʷ��뵥�ʱ�
		dynarray_add(&tktable,tp);
	}
	return tp;
}