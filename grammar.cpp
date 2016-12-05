#include "daicc.h"




/***********************************************************************************/
/*                                   ��������                                      */
/***********************************************************************************/
int calc_align(int n, int align);                                  //�����ֽڶ���λ��


/*****************************************************************************/
/*                                 �ⲿ����                                  */
/*****************************************************************************/
/***********************************************
              �﷨������ں���
***********************************************/
void translation_unit()
{
	while(token!=TK_EOF)
	{
		external_declaration();
	}
}

/**********************************************
              �����ⲿ����
***********************************************/
void external_declaration(int l) //lָ���洢�����Ǿֲ��Ļ���ȫ�ֵ�
{
	Type btype, type;
	int v, has_init, r, addr;
	Symbol *sym;
	if(!type_specifier(&btype))   //�����������ַ�������������ɹ�����0����ȱ�ٳɷִ�
	{
		expect("�������ַ�");
	}
	if(btype.t==T_STRUCT && token==TK_SEMICOLON) //����Ƿֺ�
	{
		get_token();
		return;
	}
	while(1)                //�������������������
	{
		type = btype;
		declarator(&type, &v, NULL);
		if(token==TK_BEGIN)  //��������
		{
			if(l==SC_LOCAL)
				error("��֧�ֺ�����Ƕ�׶���");
			if((type.t & T_BTYPE)!=T_FUNC)
				expect("��������");
			sym = sym_search(v);
			if(sym)        //����ǰ�������������ڸ��������Ķ���
			{
				if((sym->type.t & T_BTYPE) != T_FUNC)
					error("'%s'�ض���",get_tkstr(v));
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
                    �����������ַ�
				�����ɹ�����1�����ɹ�����0
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
			struct_specifier(); //struct�ṹ����
			type_found=1;
			break;		
		default: 
			break;
	  }
	type->t = t;
	return type_found;
}
/*************************************************
              �ṹ���ַ�
*************************************************/
void struct_specifier(Type *type)
{
	int v;
	Symbol *s;
	Type type1;

	get_token(); //ȡ���ؼ���"struct"��ߵĵ���
	v=token;
	get_token();

    if(print)
	{
		syntax_state=SNTX_DELAY;   //�ӳٵ�ȡ����һ�����ʺ�ȷ�������ʽ
		get_token();
		if(token==TK_BEGIN)        //����ṹ������߸����Ǵ����ţ���Ϊ�ṹ��Ķ��壬���в�����
			syntax_state=SNTX_LF_HT;
		else if(token==TK_CLOSEPA) //����ṹ������߸�������С���ţ�sizeof(struct struct_name)����ʽ�����մ���
			syntax_state=SNTX_NUL;
		else                       //�ṹ�����������
			syntax_state=SNTX_SP; 
		syntax_indent();
	}
	if(v<TK_IDENT)
		error("�ؼ��ֲ�����Ϊ�ṹ����");
	s = struct_search(v);
	if(!s)
	{
		type1.t = KW_STRUCT;
		s = sym_push(v|SC_STRUCT, &type1, 0, -1); //-1��ֵ��s->c����ʶ�ṹ����δ����
		s->r = 0;
	}

	type->t = T_STRUCT;
	type->ref = s;
	if(token==TK_BEGIN)    //����ؼ���"struct"��߸�����������ţ���Ϊ�ṹ������
	{
		struct_declaration_list(type);
	}
}

/*********************************************************
                      �ṹ��������
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
	if(s->c != -1)   //s->c��¼�ṹ��ߴ�
		error("�ṹ���Ѷ���");
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
                   �����ֽڶ���λ��
				   align:��������
*************************************************************/
int calc_align(int n, int align)
{
	return ((n+align-1) & (~(align-1)));
}


/*************************************************************
                     �ṹ���Ա����
*************************************************************/
void struct_declaration(int *maxalign, int *offset, Symbol ***ps)
{
	int v, size, align;
	Symbol *ss;
	Type type1, btype;
	int force_align;
	type_specifier(&btype); //�����������ַ�
	while(1)
	{
		v = 0;
		type1 = btype;
		declarator(&type1, &v, &force_align);
		size = type_size(&type1, &align);

		if(force_align & ALIGN_SET)
			align = force_align &~ALIGN_SET;
		*offset = calc_align(*offset, align);
		if(align > *maxalign)
			*maxalign = align;
		ss = sym_push(v|SC_MEMBER, &type1, 0, *offset);
		*offset += size;
		**ps = ss;
		*ps = &ss->nest;

		if(token==TK_SEMICOLON)
			break;
		skip(TK_COMMA);
	}
	if(print)
		syntax_state=SNTX_LF_HT;
	skip(TK_SEMICOLON);
}

/********************************************************
                     �ṹ��Ա����
					 force_align:ǿ�ƶ�������
*********************************************************/
void struct_member_alignment(int * force_align)
{
	int align = 1;
	if(token==KW_ALIGN)
	{
		get_token();
		skip(TK_OPENPA);
		if(token==TK_CINT)
		{
			get_token();
			align=tkvalue;
		}
		else
			expect("<��������>");
		skip(TK_CLOSEPA);
		if(align!=1 && align!=2 && align!=4)
			align=1;
		align |= ALIGN_SET;
		*force_align = align;
	}
	else
		*force_align = 1;
}

/********************************************************
                    ������
*********************************************************/
void declarator(Type *type, int *v, int *force_align)
{
	int fc;
	while(token==TK_STAR)
	{
		mk_pointer(type);
		get_token();
	}
	function_calling_convention(&fc);
	struct_member_alignment(force_align);
	direct_declarator(type, v, fc);
}
/*******************************************************
         ������������Լ��
		 ���ں��������ϣ��������������Ϻ��Ե�
********************************************************/
void function_calling_convention(int *fc)
 {
	*fc=KW_CDECL;
	if(token==KW_CDECL||token==KW_STDCALL)
	{
		*fc=token;
		get_token();
		if(print)
			syntax_state=SNTX_SP;
	}
}

/*******************************************************
                   ֱ��������
*******************************************************/
void direct_declarator(Type *type, int *v, int func_call)
{
	if(token>=TK_IDENT)
	{
		*v = token;
		get_token();
	}
	else
	{
		expect("��ʶ��");
	}
	direct_declarator_postfix(type, func_call);
}

/*******************************************************
                 ֱ��������׺
*******************************************************/
void direct_declarator_postfix(Type *type, int func_call)
{
	int n;
	Symbol *s;
	if(token==TK_OPENPA)
	{
		parameter_type_list(type, func_call);
	}
	else if(token==TK_OPENBR)
	{
		get_token();
		n = -1;
		if(token==TK_CINT)
		{
			get_token();
			n=tkvalue;
		}
		skip(TK_CLOSEBR);
		direct_declarator_postfix(type, func_call);
		s = sym_push(SC_ANOM, type, 0, n);
		type->t = T_ARRAY|T_PTR;
		type->ref = s;
	}
}

/*******************************************************************
                      �β����ͱ�
*******************************************************************/
void parameter_type_list(Type *type, int func_call)
{
	int n;
	Symbol **plast, *s, *first;
	Type pt;
	get_token();
	first = NULL;
	plast=&first;
	while(token!=TK_CLOSEPA)
	{
		if(!type_specifier(&pt))
		{
			error("��Ч��ʶ��");
		}
		declarator(&pt, &n, NULL);
		s = sym_push(n|SC_PARAMS, &pt, 0, 0);
		*plast=s;
		plast=&s->nest;
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
	//�˴��������������ʹ洢��Ȼ��ָ����������type��Ϊ�������ͣ�Ӧ�õ������Ϣ����ref��
	s = sym_push(SC_ANOM, type, func_call, 0);
	s->nest = first;
	type->t = T_FUNC;
	type->ref = s;
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
                   ������
***************************************************/
 void funcbody(Symbol *sym)
{
	//��һ���������ھֲ����ű���
	sym_direct_push(&local_sym_stack, SC_ANOM, &int_type, 0);
	compound_statement(NULL, NULL);
	//��վֲ�����ջ
	sym_pop(&local_sym_stack, NULL);
}

/***************************************************
                   ������ʼ��
				   ��ֵ��
****************************************************/
void initializer(Type *type)
{
	if(type->t & T_ARRAY)
		get_token();
	else
		assignment_expression();
}


/****************************************************************************************/
/*                                 ���ʽ                                             */
/****************************************************************************************/

/***************************************************
                ���������
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
                       �������
					   bsym:break��תλ��
					   csym:continue��תλ��
*************************************************************/
void compound_statement(int *bsym, int *csym)
{
	Symbol *s;
	s = (Symbol *)stack_get_top(&local_sym_stack);

	if(print)
	{
		syntax_state=SNTX_LF_HT;
		syntax_level++;            //������䣬��������һ��
	}
	
	get_token();
	
	while(is_type_specifier(token))
	{
		external_declaration(SC_LOCAL);
	}
	while(token!=TK_END)
	{
		statement(bsym, csym);
	}
	sym_pop(&local_sym_stack, s);
	if(print)
		syntax_state=SNTX_LF_HT;
	get_token();
}

/************************************************
              sizeof���ʽ
************************************************/
void sizeof_expression()
{
	int align,size;
	Type type;

	get_token();
	skip(TK_OPENPA);
	type_specifier(&type);
	skip(TK_CLOSEPA);

	size = type_size(&type, &align);
	if(size < 0)
		error("size �������ͳߴ�ʧ��");
}
/******************************************************
				���㷵�����ͳ���
				t:��������ָ��
				a:����ֵ
******************************************************/
int type_size(Type *t, int *a)
{
	Symbol *s;
	int bt;
	int PTR_SIZE = 4;

	bt = t->t & T_BTYPE;
	switch(bt)
	{
	case T_STRUCT:
		s = t->ref;
		*a = s->r;
		return s->c;
	case T_PTR:
		if(t->t & T_ARRAY)
		{
			s = t->ref;
			return type_size(&s->type, a) * s->c;
		}
		else
		{
			*a = PTR_SIZE;
			return PTR_SIZE;
		}
	case T_INT:
		*a = 4;
		return 4;
	case T_SHORT:
		*a = 2;
		return 2;
	default:         //char, function, void
		*a = 1;
		return 1;
	}
}

/******************************************************
                  ��ֵ���ʽ
******************************************************/
void primary_expression()
{
	 int t, addr;
	 Type type;
	 Symbol *s;
	 switch(token)
	 {
	 case TK_CINT:
	 case TK_CCHAR:
		 get_token();
		 break;
	 case TK_CSTR:
		 t = T_CHAR;
		 type.t = t;
		 mk_pointer(&type);
		 type.t |= T_ARRAY;
		 var_sym_put(&type, SC_GLOBAL, 0, addr);
		 initializer(&type);
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
           expect("��ʶ������");
		 s = sym_search(t);
		 if(!s)
		 {
			 if(token != TK_OPENPA)
				 error("'%s'δ����\n", get_tkstr(t));
			 s = func_sym_push(t, &default_func_type); //������������ֱ������
			 s->r = SC_GLOBAL|SC_SYM;
		 }
         break;	   
	 }
 }

/******************************************************
               �ж��Ƿ�Ϊ�������ַ�
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
                  ���ʽ���
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
                 if-elseѡ�����
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
                         forѭ�����
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
                         continue��ת���
******************************************************************/
void continue_statement()
{
	get_token();
	if(print)
		syntax_state=SNTX_LF_HT;
	skip(TK_SEMICOLON);
}

/******************************************************************
                         break��ת���
*******************************************************************/
void break_statement()
{
	get_token();
	if(print)
		syntax_state=SNTX_LF_HT;
	skip(TK_SEMICOLON);
}

/******************************************************************
                        return��ת���
*******************************************************************/
void return_statement()
{    
	if(print)
		syntax_state=SNTX_DELAY;
	get_token();
	if(print)
	{
		if(token==TK_SEMICOLON)     //������return��
			syntax_state=SNTX_NUL;
		else                        //������return <expression>
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
/*                                 ���ʽ����                                           */
/****************************************************************************************/
/******************************************************
                   ���ʽ�������
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
                       ��ֵ���ʽ
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
                     �������ʽ
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
                    ��ϵ���ʽ
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
                 �Ӽ�����ʽ
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
               �˳�����ʽ
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
               һԪ���ʽ
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

/***********************************************
                 ��׺���ʽ
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

/*******************************************************
                  ʵ�α��ʽ
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
 