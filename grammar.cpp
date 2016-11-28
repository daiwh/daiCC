#include "daicc.h"




/***********************************************************************************/
/*                                   函数申明                                      */
/***********************************************************************************/
int calc_align(int n, int align);                                  //计算字节对齐位置


/*****************************************************************************/
/*                                 外部定义                                  */
/*****************************************************************************/
/***********************************************
              语法分析入口函数
***********************************************/
void translation_unit()
{
	while(token!=TK_EOF)
	{
		external_declaration();
	}
}

/**********************************************
              解析外部申明
***********************************************/
void external_declaration(int l) //l指明存储类型是局部的还是全局的
{
	Type btype, type;
	int v, has_init, r, addr;
	Symbol *sym;
	if(!type_specifier(&btype))   //解析类型区分符，如果解析不成功返回0，报缺少成分错
	{
		expect("类型区分符");
	}
	if(btype.t==T_STRUCT && token==TK_SEMICOLON) //如果是分号
	{
		get_token();
		return;
	}
	while(1)                //逐个分析申明或函数定义
	{
		type = btype;
		declarator(&type, &v, NULL);
		if(token==TK_BEGIN)  //函数定义
		{
			if(l==SC_LOCAL)
				error("不支持函数的嵌套定义");
			if((type.t & T_BTYPE)!=T_FUNC)
				expect("函数定义");
			sym = sym_search(v);
			if(sym)        //函数前面申明过，现在给出函数的定义
			{
				if((sym->type.t & T_BTYPE) != T_FUNC)
					error("'%s'重定义",get_tkstr(v));
				sym->type = type;
			}
			else
			{
				sym = func_sym_push(v, &type);
			}
			sym->r = SC_SYM | SC_GLOBAL;
			funcbody(sym);
			break;
		}
		else
		{
			if((type.t & T_BTYPE) ==T_FUNC)
			{
				if(sym_search(v) == NULL)
				{
					sym = sym_push(v, &type, SC_GLOBAL|SC_SYM, 0);
				}
			}
			else
			{
				r = 0;
				if(!(type.t & T_ARRAY))
					r |= SC_LVAL;
				r |= 1;
				has_init = (token == TK_ASSIGN);
				if(has_init)
				{
					get_token();
					initializer(&type);
				}
				sym = var_sym_put(&type, r, v, addr);
			}
			if(token==TK_COMMA)
			{
				get_token();
			}
			else
			{
				if(print)
				syntax_state=SNTX_LF_HT;
				skip(TK_SEMICOLON);
				break;
			}
		}
	}
}
/***********************************************************
                    解析类型区分符
				解析成功返回1，不成功返回0
************************************************************/
int type_specifier(Type *type)
{
	int t, type_found;
	Type type;
	t = 0;
	type_found = 0;
	switch(token)
	{
		case KW_CHAR:
			t = T_CHAR;
			type_found=1;
			syntax_state=SNTX_SP;
			get_token();
			break;
		case KW_SHORT:
			t = T_SHORT;
			type_found=1;
			syntax_state=SNTX_SP;
			get_token();
			break;	
		case KW_VOID:
			t = T_VOID;
			type_found=1;
			syntax_state=SNTX_SP;
			get_token();
			break;	
		case KW_INT:
			t = T_INT;
			syntax_state=SNTX_SP;
			type_found=1;
			get_token();
			break;	
		case KW_STRUCT:
			t = T_STRUCT;
			syntax_state=SNTX_SP;
			struct_specifier(); //struct结构解析
			type_found=1;
			break;		
		default: 
			break;
	  }
	type->t = t;
	return type_found;
}
/*************************************************
              结构区分符
*************************************************/
void struct_specifier(Type *type)
{
	int v;
	Symbol *s;
	Type type1;

	get_token(); //取出关键字"struct"后边的单词
	v=token;
	get_token();

    if(print)
	{
		syntax_state=SNTX_DELAY;   //延迟到取出下一个单词后确定输出格式
		get_token();
		if(token==TK_BEGIN)        //如果结构体名后边跟的是大括号，则为结构体的定义，换行并缩进
			syntax_state=SNTX_LF_HT;
		else if(token==TK_CLOSEPA) //如果结构体名后边跟的是右小括号（sizeof(struct struct_name)的形式），空处理
			syntax_state=SNTX_NUL;
		else                       //结构体变量的申明
			syntax_state=SNTX_SP; 
		syntax_indent();
	}
	if(v<TK_IDENT)
		error("关键字不能作为结构体名");
	s = struct_search(v);
	if(!s)
	{
		type1.t = KW_STRUCT;
		s = sym_push(v|SC_STRUCT, &type1, 0, -1); //-1赋值给s->c，标识结构体尚未定义
		s->r = 0;
	}

	type->t = T_STRUCT;
	type->ref = s;
	if(token==TK_BEGIN)    //如果关键字"struct"后边跟的是左大括号，则为结构的申明
	{
		struct_declaration_list(type);
	}
}

/*********************************************************
                      结构申明符表
*********************************************************/
void struct_declaration_list(Type* type)
{
	int maxalign, offset;
	Symbol *s, **ps;
	s = type->ref;
	if(print)
	{
		syntax_state=SNTX_LF_HT;
		syntax_level++;
	}
	get_token();
	if(s->c != -1)   //s->c记录结构体尺寸
		error("结构体已定义");
	maxalign = 1;
	ps = &s->nest;
	offset = 0;
	while(token!=TK_END)
	{
		struct_declaration(&maxalign, &offset, &ps);
	}
	skip(TK_END);
	s->c = calc_align(offset, maxalign);
	s->r = maxalign;
	if(print)
		syntax_state=SNTX_LF_HT;  
}

/*************************************************************
                   计算字节对齐位置
*************************************************************/
int calc_align(int n, int align)
{
	return ((n+align-1) & (~(align-1)));
}














/*************************************************************
                     结构体成员申明
*************************************************************/
void struct_declaration(int *maxalign, int *offset, Symbol ***ps)
{
	type_specifier(); //解析类型区分符
	while(1)
	{
		declarator();

		if(token==TK_SEMICOLON)
			break;
		skip(TK_COMMA);
	}
	if(print)
		syntax_state=SNTX_LF_HT;
	skip(TK_SEMICOLON);
}

/*******************************************************
         用于函数申明上，用在数据申明上忽略掉
********************************************************/
void function_calling_convention(int *fc)
 {
	*fc=KW_CDECL;
	if(token==KW_CDECL||token==KW_STDCALL)
	{
		*fc=token;
		if(print)
		syntax_state=SNTX_SP;
		get_token();
	}
}

/********************************************************
                     结构成员对齐
*********************************************************/
void struct_member_alignment()
{
	if(token==KW_ALIGN)
	{
		get_token();
		skip(TK_OPENPA);
		if(token==TK_CINT)
		{
			get_token();
		}
		else
			expect("<整数常量>");
		skip(TK_CLOSEPA);
	}
}

/********************************************************
                    申明符
*********************************************************/
void declarator()
{
	int fc;
	while(token==TK_STAR)
	{
		get_token();
	}
	function_calling_convention(&fc);
	struct_member_alignment();
	direct_declarator();
}

/*******************************************************
                   直接申明符
*******************************************************/
void direct_declarator()
{
	if(token>=TK_IDENT)
	{
		get_token();
	}
	else
	{
		expect("标识符");
	}
	direct_declarator_postfix();
}

/*******************************************************
                 直接申明后缀
*******************************************************/
void direct_declarator_postfix()
{
	int n;
	if(token==TK_OPENPA)
	{
		parameter_type_list();
	}
	else if(token==TK_OPENBR)
	{
		get_token();
		if(token==TK_CINT)
		{
			get_token();
			n=tkvalue;
		}
		skip(TK_CLOSEBR);
		direct_declarator_postfix();
	}
}

/*******************************************************************
                      形参类型表
*******************************************************************/
void parameter_type_list(int func_call)
{
	get_token();
	while(token!=TK_CLOSEPA)
	{
		if(!type_specifier())
		{
			error("无效标识符");
		}
		declarator();
		if(token==TK_CLOSEPA)
			break;
		skip(TK_COMMA);
		if(token==TK_ELLIPSIS)
		{
			func_call = KW_CDECL;
			get_token();
			break;
		}
	}
	skip(TK_CLOSEPA);
	if(print)
	{
		syntax_state=SNTX_DELAY;
		if(token==TK_BEGIN)
			syntax_state=SNTX_LF_HT;
		else
			syntax_state=SNTX_NUL;
		syntax_indent();
	}
}

/***************************************************
                   函数体
***************************************************/
 void funcbody()
{
	compound_statement();
}

/***************************************************
                   初值符
****************************************************/
void initializer()
{
	assignment_expression();
}


/****************************************************************************************/
/*                                 表达式                                             */
/****************************************************************************************/

/***************************************************
                语句分析入口
****************************************************/
void statement(int *bsym, int *csym)
{
	switch(token)
	{
	case TK_BEGIN:
		compound_statement(bsym, csym);
		break;
	case KW_IF:
	    if_statement(bysm, csym);
		break;
	case KW_RETURN:
	    return_statement();
		break;
	case KW_BREAK:
	    break_statement(bsym);
		break;
	case KW_CONTINUE:
	    continue_statement(bsym);
		break;
	case KW_FOR:
	    for_statement(bsym, csym);
	default:
	    expression_statement();
		break;
	}
}

/************************************************************
                       复合语句
*************************************************************/
void compound_statement()
{
	if(print)
	{
		syntax_state=SNTX_LF_HT;
		syntax_level++;            //复合语句，缩进增加一级
	}
	
	get_token();
	
	while(is_type_specifier(token))
	{
		external_declaration();
	}
	while(token!=TK_END)
	{
		statement();
	}
	if(print)
	syntax_state=SNTX_LF_HT;
	get_token();
}

/******************************************************
               判断是否为类型区分符
******************************************************/
int is_type_specifier(int v)
{
	switch(v)
	{
		case KW_CHAR:
		case KW_SHORT:
		case KW_INT:
		case KW_VOID:
		case KW_STRUCT:
		   return 1;
		default:
		   break;
	}
	return 0;
}

/***********************************************
                  表达式语句
************************************************/
void expression_statement()
{
	if(token!=TK_SEMICOLON)
	{
		expression();
	}
	if(print)
		syntax_state=SNTX_LF_HT;
	skip(TK_SEMICOLON);
}

/***********************************************
                 if-else选择语句
***********************************************/
void if_statement()
{
	if(print)
		syntax_state=SNTX_SP;
	get_token();
	skip(TK_OPENPA);
	expression();
	if(print)
		syntax_state=SNTX_LF_HT;
	skip(TK_CLOSEPA);
	statement();
	if(token==KW_ELSE)
	{
		if(print)
			syntax_state=SNTX_LF_HT;
		get_token();
		statement();
	}
}

/**************************************************************
                         for循环语句
**************************************************************/
void for_statement()
{
	get_token();
	skip(TK_OPENPA);
	if(token!=TK_SEMICOLON)
	{
		expression();
	}
	skip(TK_SEMICOLON);
	if(token!=TK_SEMICOLON)
	{
		expression();
	}
	skip(TK_SEMICOLON);
	if(token!=TK_CLOSEPA)
	{
		expression();
	}
	if(print)
		syntax_state=SNTX_LF_HT;
	skip(TK_CLOSEPA);
	statement();
}

/******************************************************************
                         continue跳转语句
******************************************************************/
void continue_statement()
{
	get_token();
	if(print)
		syntax_state=SNTX_LF_HT;
	skip(TK_SEMICOLON);
}

/******************************************************************
                         break跳转语句
*******************************************************************/
void break_statement()
{
	get_token();
	if(print)
		syntax_state=SNTX_LF_HT;
	skip(TK_SEMICOLON);
}

/******************************************************************
                        return跳转语句
*******************************************************************/
void return_statement()
{    
	if(print)
		syntax_state=SNTX_DELAY;
	get_token();
	if(print)
	{
		if(token==TK_SEMICOLON)     //适用于return；
			syntax_state=SNTX_NUL;
		else                        //适用于return <expression>
			syntax_state=SNTX_SP;
		syntax_indent();
	}
	if(token!=TK_SEMICOLON)
	{
		expression();
	}
	if(print)
		syntax_state=SNTX_LF_HT;
	skip(TK_SEMICOLON);
}


/****************************************************************************************/
/*                                 表达式定义                                           */
/****************************************************************************************/
/******************************************************
                   表达式分析入口
******************************************************/
void expression()
{
	while(1)
	{
		assignment_expression();
		if(token!=TK_COMMA)
			break;
		get_token();
	}
}

/*******************************************************
                       赋值表达式
*******************************************************/
void assignment_expression()
{
	equality_expression();
	if(token==TK_ASSIGN)
	{
		get_token();
		assignment_expression();
	}
}

/*******************************************************
                     相等类表达式
*******************************************************/
void equality_expression()
{
	relational_expression();
	while(token==TK_EQ||token==TK_NEQ)
	{
		get_token();
		relational_expression();
	}
}

/******************************************************
                    关系表达式
******************************************************/
void relational_expression()
{
	additive_expression();
	while((token==TK_LT||token==TK_LEQ)||token==TK_GT||token==TK_GEQ)
	{
		get_token();
		additive_expression();
	}
}

/******************************************************
                 加减类表达式
******************************************************/
void additive_expression()
{
	multiplicative_expression();
	while(token==TK_PLUS||token==TK_MINUS)
	{
		get_token();
		multiplicative_expression();
	}
}

/*****************************************************
               乘除类表达式
*****************************************************/
void multiplicative_expression()
{
	unary_expression();
	while(token==TK_STAR||token==TK_DIVIDE||token==TK_MOD)
	{
		get_token();
		unary_expression();
	}
}

/****************************************************
               一元表达式
****************************************************/
void unary_expression()
{
	switch(token)
	{
	case TK_AND:
		get_token();
		unary_expression();
		break;
	case TK_STAR:
		get_token();
		unary_expression();
		break;
	case TK_PLUS:
		get_token();
		unary_expression();
		break;
	case TK_MINUS:
		get_token();
		unary_expression();
		break;
	case KW_SIZEOF:
		sizeof_expression();
		break;
	default:
	    postfix_expression();
		break;
	}
}

/************************************************
              sizeof表达式
************************************************/
void sizeof_expression()
{
	get_token();
	skip(TK_OPENPA);
	type_specifier();
	skip(TK_CLOSEPA);
}

/***********************************************
                 后缀表达式
***********************************************/
void postfix_expression()
{
	primary_expression();
	while(1)
	{
		if(token==TK_DOT||token==TK_POINTSTO)
		{
			get_token();
			token |=SC_MEMBER;
			get_token();
		}
		else if(token==TK_OPENBR)
		{
			get_token();
			expression();
			skip(TK_CLOSEBR);
		}
		else if(token==TK_OPENPA)
		{
			argument_expression_list();
		}
		else
			break;
	}
}

/******************************************************
                  初值表达式
******************************************************/
void primary_expression()
 {
	 int t;
	 switch(token)
	 {
	 case TK_CINT:
	 case TK_CCHAR:
		 get_token();
		 break;
	 case TK_CSTR:
	     get_token();
		 break;
	 case TK_OPENPA:
		 get_token();
		 expression();
         skip(TK_CLOSEPA);
         break;
     default:
         t=token;
         get_token();
         if(t<TK_IDENT)
           expect("标识符或常量");
         break;	   
	 }
 }

/*******************************************************
                  实参表达式
*******************************************************/
void argument_expression_list()
{
	get_token();
	if(token!=TK_CLOSEPA)
	{
		for(;;)
		{
			assignment_expression();
			if(token==TK_CLOSEPA)
				break;
			skip(TK_COMMA);
		}
	}
	skip(TK_CLOSEPA);
}
 