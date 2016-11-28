#include "daicc.h"


void print_tab(int n)
{
	for(int i=0;i<n;i++)
		printf("\t");
}

void o_token(int level)
{
	char *p;
	p=get_tkstr(token);
	if(level==1)
	   printf("%s",p);
	else
		printf("%c",ch);
}
void syntax_indent()
{
	switch(syntax_state)
	{
	case SNTX_NUL:
		o_token(1);
		break;
	case SNTX_SP:
		printf(" ");
		o_token(1);
		break;
	case SNTX_LF_HT:
		if(token==TK_END)
			syntax_level--;
		printf("\n");
		print_tab(syntax_level);
		o_token(1);
		break;
	case SNTX_DELAY:
		break;
	}
	syntax_state=SNTX_NUL;
}

 void init()
{
	line_num=1;
	init_lex();
}