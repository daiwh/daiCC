#include "daicc.h"

#include <stdarg.h>  //va_start,va_end函数包含在此头文件中

/***********************************************************************************/
/*                                   函数申明                                      */
/***********************************************************************************/
void handle_exception(int stage, int level, char *fmt,va_list ap);    //异常处理程序
void expect(char *msg);                                               //语法成分缺少提示

/****************************************************
                编译警告处理
*****************************************************/
void warning(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	handle_exception(STAGE_COMPILE,LEVEL_WARNING,fmt,ap);
	va_end(ap);
}

/****************************************************
                编译错误处理
*****************************************************/
void error(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	handle_exception(STAGE_COMPILE,LEVEL_ERROR,fmt,ap);
	va_end(ap);
}

/**************************************************
            链接错误处理程序
***************************************************/
void link_error(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	handle_exception(STAGE_LINK,LEVEL_ERROR,fmt,ap);
	va_end(ap);
}

/*********************************************************
                  异常处理程序
**********************************************************/
void handle_exception(int stage, int level, char *fmt,va_list ap)
{
	char buf[1024];
	vsprintf(buf, fmt, ap); //将不确定参数列表按照fmt所定义的格式写入buf中
	if(stage==STAGE_COMPILE)
	{
		if(level==LEVEL_ERROR)
		{
			printf("%s(第%d行) 编译错误:%s",filename,line_num,buf);
			exit(-1);
		}
		else
			printf("%s(第%d行) 编译警告:%s",filename,line_num,buf);
	}
	else
	{
		printf("链接错误:%s",buf);
		exit(-1);
	}
}

/****************************************************
           语法成分缺少提示
****************************************************/
void expect(char *msg)
{
	error("缺少%s",msg);
}

/***************************************************
          跳过当前单词取下一单词c
		  如果当前单词不是c,报错
***************************************************/
void skip(int c)
{
	if(token != c)
		error("缺少%s",get_tkstr(c));
	get_token();
}

/**************************************************
           取单词所代表的源码字符串
**************************************************/
char *get_tkstr(int v)
{
	if(v>tktable.count)
		return NULL;
	else if(v>TK_CINT && v<=TK_CSTR)  //如果是常量
		return sourcestr.data;
	else
		return ((TKWord *)(tktable.data[v]))->spelling;
}
