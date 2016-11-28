#include "daicc.h"

#include <stdarg.h>  //va_start,va_end���������ڴ�ͷ�ļ���

/***********************************************************************************/
/*                                   ��������                                      */
/***********************************************************************************/
void handle_exception(int stage, int level, char *fmt,va_list ap);    //�쳣�������
void expect(char *msg);                                               //�﷨�ɷ�ȱ����ʾ

/****************************************************
                ���뾯�洦��
*****************************************************/
void warning(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	handle_exception(STAGE_COMPILE,LEVEL_WARNING,fmt,ap);
	va_end(ap);
}

/****************************************************
                ���������
*****************************************************/
void error(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	handle_exception(STAGE_COMPILE,LEVEL_ERROR,fmt,ap);
	va_end(ap);
}

/**************************************************
            ���Ӵ��������
***************************************************/
void link_error(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	handle_exception(STAGE_LINK,LEVEL_ERROR,fmt,ap);
	va_end(ap);
}

/*********************************************************
                  �쳣�������
**********************************************************/
void handle_exception(int stage, int level, char *fmt,va_list ap)
{
	char buf[1024];
	vsprintf(buf, fmt, ap); //����ȷ�������б���fmt������ĸ�ʽд��buf��
	if(stage==STAGE_COMPILE)
	{
		if(level==LEVEL_ERROR)
		{
			printf("%s(��%d��) �������:%s",filename,line_num,buf);
			exit(-1);
		}
		else
			printf("%s(��%d��) ���뾯��:%s",filename,line_num,buf);
	}
	else
	{
		printf("���Ӵ���:%s",buf);
		exit(-1);
	}
}

/****************************************************
           �﷨�ɷ�ȱ����ʾ
****************************************************/
void expect(char *msg)
{
	error("ȱ��%s",msg);
}

/***************************************************
          ������ǰ����ȡ��һ����c
		  �����ǰ���ʲ���c,����
***************************************************/
void skip(int c)
{
	if(token != c)
		error("ȱ��%s",get_tkstr(c));
	get_token();
}

/**************************************************
           ȡ�����������Դ���ַ���
**************************************************/
char *get_tkstr(int v)
{
	if(v>tktable.count)
		return NULL;
	else if(v>TK_CINT && v<=TK_CSTR)  //����ǳ���
		return sourcestr.data;
	else
		return ((TKWord *)(tktable.data[v]))->spelling;
}
