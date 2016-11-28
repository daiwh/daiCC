#include "daicc.h"

/***********************************************************************************/
/*                                  全局变量定义                                   */
/***********************************************************************************/
//语法分析程序初始化时要优先写入单词表(运算符，分隔符，关键字，和常量标识）
//字段含义分别为 ：单词编码，指向哈希冲突的同义词，单词字符串，单词所标示的结构定义，单词所表示的标识符
static TKWord keywords[]={
		{TK_PLUS,		NULL,	"+",			NULL,	NULL},
		{TK_MINUS,		NULL,	"-",			NULL,	NULL},
		{TK_STAR,		NULL,	"*",			NULL,	NULL},
		{TK_DIVIDE,		NULL,	"/",			NULL,	NULL},
		{TK_MOD,		NULL,	"%",			NULL,	NULL},

		{TK_EQ,			NULL,	"==",			NULL,	NULL},
		{TK_NEQ,		NULL,	"!=",			NULL,	NULL},
		{TK_LT,			NULL,	"<",			NULL,	NULL},
		{TK_LEQ,		NULL,	"<=",			NULL,	NULL},
		{TK_GT,			NULL,	">",			NULL,	NULL},

		{TK_GEQ,		NULL,	">=",			NULL,	NULL},
		{TK_ASSIGN,		NULL,	"=",			NULL,	NULL},
		{TK_POINTSTO,	NULL,	"->",			NULL,	NULL},
		{TK_DOT,		NULL,	".",			NULL,	NULL},
		{TK_AND,		NULL,	"&",			NULL,	NULL},

		{TK_OPENPA,		NULL,	"(",			NULL,	NULL},
		{TK_CLOSEPA,	NULL,	")",			NULL,	NULL},
		{TK_OPENBR,		NULL,	"[",			NULL,	NULL},
		{TK_CLOSEBR,	NULL,	"]",			NULL,	NULL},
		{TK_BEGIN,		NULL,	"{",			NULL,	NULL},

		{TK_END,		NULL,	"}",			NULL,	NULL},
		{TK_SEMICOLON,	NULL,	";",			NULL,	NULL},
		{TK_COMMA,		NULL,	",",			NULL,	NULL},
		{TK_ELLIPSIS,	NULL,	"...",			NULL,	NULL},
		{TK_EOF,		NULL,	"END_OF_FILE",	NULL,	NULL},

		{TK_CINT,		NULL,	"整形常数",		NULL,	NULL},
		{TK_CCHAR,		NULL,	"字符常量",		NULL,	NULL},
		{TK_CSTR,		NULL,	"字符串常量",	NULL,	NULL},

		{KW_CHAR,		NULL,	"char",			NULL,	NULL},
		{KW_SHORT,		NULL,	"short",		NULL,	NULL},
		{KW_INT,		NULL,	"int",			NULL,	NULL},
		{KW_VOID,		NULL,	"void",			NULL,	NULL},
		{KW_STRUCT,		NULL,	"struct",		NULL,	NULL},

		{KW_IF,			NULL,	"if",			NULL,	NULL},
		{KW_ELSE,		NULL,	"else",			NULL,	NULL},
		{KW_FOR,		NULL,	"for",			NULL,	NULL},
		{KW_CONTINUE,	NULL,	"continue",		NULL,	NULL},
		{KW_BREAK,		NULL,	"break",		NULL,	NULL},

		{KW_RETURN,		NULL,	"return",		NULL,	NULL},
		{KW_SIZEOF,		NULL,	"sizeof",		NULL,	NULL},
		{KW_ALIGN,		NULL,	"align",		NULL,	NULL},
		{KW_CDECL,		NULL,	"cdecl",		NULL,	NULL},
		{KW_STDCALL,	NULL,	"stdcall",		NULL,	NULL},

		{0,				NULL,	NULL,			NULL,	NULL},
	};
/***********************************************************************************/
/*                                   函数申明                                      */
/***********************************************************************************/
int elf_hash(char* key);                 //根据关键字计算哈希地址(为了快速查找单词的存储位置)
void getch();                            //从源文件中读取一个字符分析
void skip_white_space();                 //解析空白字符（空格 ，tab，回车，回车加换行）
void preprocess();                       //预处理，忽略空白和注释
void parse_comment();                    //解析注释
int is_nodigit(char c);                  //判断c是否为字母或者下划线
int is_digit(char c);                    //判断c是否为数字
void parse_identifier();                 //解析标识符
void parse_num();                        //解析整形常量
void parse_string(char sep);             //解析字符串

/*********************
从源文件读取一个字符
*********************/
void getch()
{
	ch=getc(fin);
}

/*******************************************
              解析空白字符
因为只针对windows，所以默认每行的结束是\r\n
********************************************/
void skip_white_space()
{
	while(ch==' '||ch=='\t'||ch=='\r')
	{
		if(ch=='\r')
		{
			getch();
			if(ch!='\n') 
				return;
			line_num++;   //遇到换行，行号+1
		}
		getch();
	}
}

/**************************
预处理，忽略空白字符及注释
***************************/
void preprocess()
{
	while(1)
	{
		if(ch==' '||ch=='\t'||ch=='\r')
			skip_white_space();
		else if(ch=='/')
		{
			getch();
			if(ch=='*')
			{
				parse_comment();
			}
			else //如果/后边跟的不是*，将读出的字符放回输入流，恢复现场
			{
				ungetc(ch,fin);
				ch='/';
				break;
			}
		}
		else
			break;
	}
}


/********************************
      解析注释
*********************************/
void parse_comment()
{
	getch();
	while(1)
	{
		while(1) //循环读取并处理每一个字符，直到遇到换行，*，或者文件结束符
		{
			if(ch=='\n'||ch=='*'||ch==EOF)
				break;
			else
				getch();
		}
		if(ch=='\n') //如果遇到换行，行号加一
		{
			line_num++;
			getch();
		}
		else if(ch=='*') //如果下一个解析到的字符时*，检测下一个是不是/，如果是，注释块结束，如果不是，接着解析
		{
			getch();
			if(ch=='/')
			{
				getch();
			    return;
			}
		}
		else   //如果好没有遇到注释结束标记文件就了
		{
			error("一直到文件末尾看到配对的注释结束符");
			return;
		}
	}
}
/******************************
     判断 c是否为字母或者下划线
	 用于标识符的合法性检测
*******************************/
int is_nodigit(char c)
{
	return(c>='a'&&c<='z'||c>='A'&&c<='Z'||c=='_');
}

/************************
   判断c是否为数字
************************/
int is_digit(char c)
{
	return c>='0'&&c<='9';
}

/**********************************
           词法分析初始化
***********************************/
void init_lex()
{
	TKWord *tp;
	dynarray_init(&tktable,8);   //初始分配单词表的空间为8（8个指针）
	for(tp=&keywords[0];tp->spelling!=NULL;tp++) //依次将放入单词表
		tkword_direct_insert(tp);
}

/***********************************
             解析标识符
***********************************/
void parse_identifier()
{
	dynstring_reset(&tkstr);     //初始化当前要解析的标识符的地址空间
	dynstring_chcat(&tkstr,ch);  
	getch();
	while(is_nodigit(ch)||is_digit(ch))//字母或下划线开始，后边跟字母数字或下划线
	{
		dynstring_chcat(&tkstr,ch);
		getch();
	}
	dynstring_chcat(&tkstr,'\0');    //补尾0
}
/***********************************
              解析整形常量
************************************/
void parse_num()
{
	dynstring_reset(&tkstr);
	dynstring_reset(&sourcestr);
	while(is_digit(ch))
	{
		dynstring_chcat(&tkstr,ch);
		dynstring_chcat(&sourcestr,ch);
		getch();
	}
	dynstring_chcat(&tkstr,'\0');
	dynstring_chcat(&sourcestr,'\0');
	tkvalue=atoi(tkstr.data);       //解析出数字字符串的值
}

/***********************************
              解析字符串
***********************************/
void parse_string(char sep)//sec为引号，字符串解析过程以sec开始，以sec结束
{
	char c;
	dynstring_reset(&tkstr);
	dynstring_reset(&sourcestr);
	dynstring_chcat(&sourcestr,sep);
	getch();
	for(;;)
	{
		if(ch==sep)
			break;
		else if(ch=='\\')//解析转义字符
		{
			dynstring_chcat(&sourcestr,ch);
			getch();
			switch(ch)
			{
				case '0':c='\0';break;
				case 'a':c='\a';break;
				case 'b':c='\b';break;
				case 't':c='\t';break;
				case 'n':c='\n';break;
				case 'v':c='\v';break;
				case 'f':c='\f';break;
				case 'r':c='\r';break;
				case '\"':c='\"';break;
				case '\'':c='\'';break;
				case '\\':c='\\';break;
				default:                   //非法转义字符处理
					c=ch;
					if(c>='!'&&c<='~')
						error("非法转义字符:\'\\%c\'",c);
					else
						error("非法转义字符:\\\0x%x\'",c);
					break;
			}
			dynstring_chcat(&tkstr,ch);
			dynstring_chcat(&sourcestr,ch);
		 getch();
		}
		else //如果既不是转义字符，也不是字符串结束符
		{
			dynstring_chcat(&tkstr,ch);
		    dynstring_chcat(&sourcestr,ch);
			getch();
		}
	}
	dynstring_chcat(&tkstr,'\0');
    dynstring_chcat(&sourcestr,sep);
	dynstring_chcat(&sourcestr,'\0');
	getch();
}


/***********************************
          获取单词
***********************************/
void get_token()
{
	preprocess();       //预处理，忽略空白和注释
	if(is_nodigit(ch))  //字母下划线开头
	{

		TKWord *tp;
		parse_identifier();
		tp=tkword_insert(tkstr.data); //将解析出来的单词放入单词表
		token=tp->tkcode;   //token存储索引的值，因为在枚举中索引是从0开始的，所以索引的值也就是结构第一个字段的值
		                    //所以daicc.h中的枚举定义必须要和lex.cpp中的全局数组元素顺序一致
	}
	else if(is_digit(ch))   //数字开头
	{
		parse_num();
		token=TK_CINT;      //数字常量不用放入单词表
	}
	else if(ch=='+')
	{
		getch();
		token=TK_PLUS;
	}
	else if(ch=='-')
	{
		getch();
		if(ch=='>')
		{
			token=TK_POINTSTO;
			getch();
		}
		else
		{
			token=TK_MINUS;
		}
	}
	else if(ch=='/')  //裸的/只有可能是除号
	{
		token=TK_DIVIDE;
		getch();
	}
	else if(ch=='%') //不支持格式化输出，%只可能是模运算符
	{
		token=TK_MOD;
		getch();
	}
	else if(ch=='=')
	{
		getch();
		if(ch=='=')
		{
			token=TK_EQ;
			getch();
		}
		else
			token=TK_ASSIGN;
	}
	else if(ch=='!')
	{
		getch();
		if(ch=='=')
		{
			token=TK_NEQ;
		    getch();
		}
		else
			error("不支持'!'(非操作符）");
	}
	else if(ch=='<')
	{
		getch();
		if(ch=='=')
		{
			token=TK_LEQ;
			getch();
		}
		else
			token=TK_LT;
	}
	else if(ch=='>')
	{
		getch();
		if(ch=='=')
		{
			token=TK_GEQ;
			getch();
		}
		else
			token=TK_GT;
	}
	else if(ch=='.')
	{
		getch();
		if(ch=='.')
		{
			getch();
		    if(ch!='.')
			    error("符号拼写错误（..）");
		    else
			     token=TK_ELLIPSIS;
		getch();
		}
		else
			token=TK_DOT;
	}
	else if(ch=='&')
	{
		token=TK_AND;
		getch();
	}
	else if(ch==';')
	{
		token=TK_SEMICOLON;
		getch();
	}
	else if(ch==']')
	{
		token=TK_CLOSEBR;
		getch();
	}
	else if(ch=='}')
	{
		token=TK_END;
		getch();
	}
	else if(ch==')')
	{
		token=TK_CLOSEPA;
		getch();
	}
	else if(ch=='[')
	{
		token=TK_OPENBR;
		getch();
	}
	else if(ch=='{')
	{
		token=TK_BEGIN;
		getch();
	}
	else if(ch=='(')
	{
		token=TK_OPENPA;
		getch();
	}
	else if(ch==',')
	{
		token=TK_COMMA;
		getch();
	}
	else if(ch=='*')
	{
		token=TK_STAR;
		getch();
	}
	else if(ch=='\'')
	{
		parse_string(ch);
		token=TK_CCHAR;
		tkvalue=*(char*)tkstr.data;//tkvalue存放单字符的ascii值
	}
	else if(ch=='\"')
	{
		parse_string(ch);
		token=TK_CSTR;
	}
	else if(ch==EOF)
	{
		token=TK_EOF;
	}
	else
	{
		error("代码中存在未定义的字符");
		getch();
	}
	syntax_indent();//语法分析时判断语法缩进
}