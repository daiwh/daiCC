#include "daicc.h"

/**************************************************
           �����ŷ������ջ��
**************************************************/
Symbol *sym_direct_push(Stack *ss, int v, Type *type, int c)
{
	Symbol s, *p;
	s.v = v;
	s.type.t = type->t;
	s.type.ref = type->ref;
	s.c = c;
	s.nest = NULL;
	p = (Symbol *)stack_push(ss, &s, sizeof(Symbol));
	return p;
}  

/***************************************************
�����ŷ������ջ�У���̬�ж��Ƿ���ȫ�ַ���ջ���Ǿֲ�����ջ
****************************************************/
Symbol *sym_push(int v,Type *type, int r, int c)
{
	Symbol *ps, **pps;
	TKWord *ts;
	Stack *ss;
	if(stack_is_empty(&local_sym_stack)==0)
		ss = &local_sym_stack;
	else
		ss = &global_sym_stack;
	ps = sym_direct_push(ss, v, type, c);
	ps->r = r;

	//����¼�ṹ���Ա����������
	if((v & SC_STRUCT) || v < SC_ANOM)
	{
		//���µ���sym_struct �� sym_identifier�ֶ�
		ts = (TKWord *)tktable.data[(v &~SC_STRUCT)];
		if(v & SC_STRUCT)
			pps = &ts->sym_struct;
		else
			pps = &ts->sym_identifier;
		ps->prev_tok = *pps;
	    *pps = ps;
	}
	return ps;
}

/**********************************************************
        ���������ŷ���ȫ�ַ��ű���
***********************************************************/
Symbol *func_sym_push(int v, Type *type)
{
	Symbol *s, **ps;
	s = sym_direct_push(&global_sym_stack, v, type, 0);
	ps = &((TKWord *)tktable.data[v])->sym_identifier;
	//ͬ�����ţ��������ŷ������
	while(*ps!=NULL)
		ps = &(*ps)->prev_tok;
	s->prev_tok=NULL;
	*ps = s;
	return s;
}
/**************************************************************

**************************************************************/
Symbol *var_sym_put(Type *type, int r, int v, int addr)
{
	Symbol *sym = NULL;
	if((r & SC_VALMASK) ==SC_LOCAL) //�ֲ�����
		sym = sym_push(v, type, r, addr);
	else if(v && (r & SC_VALMASK) == SC_GLOBAL)  //ȫ�ֱ���
	{
		sym = sym_search(v);
		if(sym)
			error("%s�ض���\n", ((TKWord *)tktable.data[v])->spelling);
		else
			sym = sym_push(v, type, r|SC_SYM, 0);
	}
	//else �ַ�������
	return sym;
}

/*************************************************************
             �������Ʒ���ȫ�ַ��ű�
*************************************************************/
Symbol *sec_sym_put(char *sec, int c)
{
	TKWord *tp;
	Symbol *s;
	Type type;
	type.t = T_INT;
	tp = tkword_insert(sec);
	token = tp->tkcode;
	s = sym_push(token, &type, SC_GLOBAL, c);
	return s;
}

/***********************************************************
                  ���ŵ�ɾ��
***********************************************************/
void sym_pop(Stack *ptop, Symbol *b)
{
	Symbol *s, **ps;
	TKWord *ts;
	int v;

	s = (Symbol *)stack_get_top(ptop);
	while(s!=b)
	{
		v = s->v;
		//���µ��ʱ��� sym_struct sym_identifier
		if(( v & SC_STRUCT) || v <SC_ANOM)
		{
			ts = (TKWord *)tktable.data[(v & ~SC_STRUCT)];
			if( v & SC_STRUCT)
				ps = &ts->sym_struct;
			else
				ps = &ts->sym_identifier;
			*ps = s->prev_tok;
		}
		stack_pop(ptop);
		s = (Symbol *)stack_get_top(ptop);
	}
}

/************************************************************
                 ���ҽṹ����
************************************************************/
Symbol *struct_search(int v)
{
	if(v>tktable.count)
		return NULL;
	else
		return ((TKWord *)tktable.data[v])->sym_struct;
}

/************************************************************
                 ���ұ�ʶ������
************************************************************/
Symbol *sym_search(int v)
{
	if(v>tktable.count)
		return NULL;
	else
		return ((TKWord *)tktable.data[v])->sym_identifier;
}