#include "daicc.h"

/***********************************************************************************/
/*                                  ȫ�ֱ�������                                   */
/***********************************************************************************/
//�﷨���������ʼ��ʱҪ����д�뵥�ʱ�(��������ָ������ؼ��֣��ͳ�����ʶ��
//�ֶκ���ֱ�Ϊ �����ʱ��룬ָ���ϣ��ͻ��ͬ��ʣ������ַ�������������ʾ�Ľṹ���壬��������ʾ�ı�ʶ��
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

		{TK_CINT,		NULL,	"���γ���",		NULL,	NULL},
		{TK_CCHAR,		NULL,	"�ַ�����",		NULL,	NULL},
		{TK_CSTR,		NULL,	"�ַ�������",	NULL,	NULL},

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
/*                                   ��������                                      */
/***********************************************************************************/
int elf_hash(char* key);                 //���ݹؼ��ּ����ϣ��ַ(Ϊ�˿��ٲ��ҵ��ʵĴ洢λ��)
void getch();                            //��Դ�ļ��ж�ȡһ���ַ�����
void skip_white_space();                 //�����հ��ַ����ո� ��tab���س����س��ӻ��У�
void preprocess();                       //Ԥ�������Կհ׺�ע��
void parse_comment();                    //����ע��
int is_nodigit(char c);                  //�ж�c�Ƿ�Ϊ��ĸ�����»���
int is_digit(char c);                    //�ж�c�Ƿ�Ϊ����
void parse_identifier();                 //������ʶ��
void parse_num();                        //�������γ���
void parse_string(char sep);             //�����ַ���

/*********************
��Դ�ļ���ȡһ���ַ�
*********************/
void getch()
{
	ch=getc(fin);
}

/*******************************************
              �����հ��ַ�
��Ϊֻ���windows������Ĭ��ÿ�еĽ�����\r\n
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
			line_num++;   //�������У��к�+1
		}
		getch();
	}
}

/**************************
Ԥ�������Կհ��ַ���ע��
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
			else //���/��߸��Ĳ���*�����������ַ��Ż����������ָ��ֳ�
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
      ����ע��
*********************************/
void parse_comment()
{
	getch();
	while(1)
	{
		while(1) //ѭ����ȡ������ÿһ���ַ���ֱ���������У�*�������ļ�������
		{
			if(ch=='\n'||ch=='*'||ch==EOF)
				break;
			else
				getch();
		}
		if(ch=='\n') //����������У��кż�һ
		{
			line_num++;
			getch();
		}
		else if(ch=='*') //�����һ�����������ַ�ʱ*�������һ���ǲ���/������ǣ�ע�Ϳ������������ǣ����Ž���
		{
			getch();
			if(ch=='/')
			{
				getch();
			    return;
			}
		}
		else   //�����û������ע�ͽ�������ļ�����
		{
			error("һֱ���ļ�ĩβ������Ե�ע�ͽ�����");
			return;
		}
	}
}
/******************************
     �ж� c�Ƿ�Ϊ��ĸ�����»���
	 ���ڱ�ʶ���ĺϷ��Լ��
*******************************/
int is_nodigit(char c)
{
	return(c>='a'&&c<='z'||c>='A'&&c<='Z'||c=='_');
}

/************************
   �ж�c�Ƿ�Ϊ����
************************/
int is_digit(char c)
{
	return c>='0'&&c<='9';
}

/**********************************
           �ʷ�������ʼ��
***********************************/
void init_lex()
{
	TKWord *tp;
	dynarray_init(&tktable,8);   //��ʼ���䵥�ʱ�Ŀռ�Ϊ8��8��ָ�룩
	for(tp=&keywords[0];tp->spelling!=NULL;tp++) //���ν����뵥�ʱ�
		tkword_direct_insert(tp);
}

/***********************************
             ������ʶ��
***********************************/
void parse_identifier()
{
	dynstring_reset(&tkstr);     //��ʼ����ǰҪ�����ı�ʶ���ĵ�ַ�ռ�
	dynstring_chcat(&tkstr,ch);  
	getch();
	while(is_nodigit(ch)||is_digit(ch))//��ĸ���»��߿�ʼ����߸���ĸ���ֻ��»���
	{
		dynstring_chcat(&tkstr,ch);
		getch();
	}
	dynstring_chcat(&tkstr,'\0');    //��β0
}
/***********************************
              �������γ���
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
	tkvalue=atoi(tkstr.data);       //�����������ַ�����ֵ
}

/***********************************
              �����ַ���
***********************************/
void parse_string(char sep)//secΪ���ţ��ַ�������������sec��ʼ����sec����
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
		else if(ch=='\\')//����ת���ַ�
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
				default:                   //�Ƿ�ת���ַ�����
					c=ch;
					if(c>='!'&&c<='~')
						error("�Ƿ�ת���ַ�:\'\\%c\'",c);
					else
						error("�Ƿ�ת���ַ�:\\\0x%x\'",c);
					break;
			}
			dynstring_chcat(&tkstr,ch);
			dynstring_chcat(&sourcestr,ch);
		 getch();
		}
		else //����Ȳ���ת���ַ���Ҳ�����ַ���������
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
          ��ȡ����
***********************************/
void get_token()
{
	preprocess();       //Ԥ�������Կհ׺�ע��
	if(is_nodigit(ch))  //��ĸ�»��߿�ͷ
	{

		TKWord *tp;
		parse_identifier();
		tp=tkword_insert(tkstr.data); //�����������ĵ��ʷ��뵥�ʱ�
		token=tp->tkcode;   //token�洢������ֵ����Ϊ��ö���������Ǵ�0��ʼ�ģ�����������ֵҲ���ǽṹ��һ���ֶε�ֵ
		                    //����daicc.h�е�ö�ٶ������Ҫ��lex.cpp�е�ȫ������Ԫ��˳��һ��
	}
	else if(is_digit(ch))   //���ֿ�ͷ
	{
		parse_num();
		token=TK_CINT;      //���ֳ������÷��뵥�ʱ�
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
	else if(ch=='/')  //���/ֻ�п����ǳ���
	{
		token=TK_DIVIDE;
		getch();
	}
	else if(ch=='%') //��֧�ָ�ʽ�������%ֻ������ģ�����
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
			error("��֧��'!'(�ǲ�������");
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
			    error("����ƴд����..��");
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
		tkvalue=*(char*)tkstr.data;//tkvalue��ŵ��ַ���asciiֵ
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
		error("�����д���δ������ַ�");
		getch();
	}
	syntax_indent();//�﷨����ʱ�ж��﷨����
}