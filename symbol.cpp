#include "daicc.h"

/**************************************************
           将符号放入符号栈中
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
将符号放入符号栈中，动态判断是放入全局符号栈还是局部符号栈
          v:符号编号
		  type:符号数据类型
		  r:符号存储类型
		  c:符号关联值
****************************************************/
Symbol *sym_push(int v,Type *type, int r , int c)
{
	Symbol *ps, **pps;
	TKWord *ts;     //符号所对应的单词
	Stack *ss;      //标记符号栈
	if(stack_is_empty(&local_sym_stack)==0)  //如果局部符号栈为空，就放入局部符号栈
		ss = &local_sym_stack;
	else                                     //否则放入全局符号栈
		ss = &global_sym_stack;
	ps = sym_direct_push(ss, v, type, c);    //将符号放入指定符号栈中，该函数返回栈顶元素
	ps->r = r;                      //填充符号存储类型

	//不记录结构体成员及匿名符号
	if((v & SC_STRUCT) || v < SC_ANOM)//如果(v & SC_STRUCT)不为0，则为结构体符号
	{
		//更新单词sym_struct 或 sym_identifier字段
		ts = (TKWord *)tktable.data[(v &~SC_STRUCT)];//(v &~SC_STRUCT)的值为v所表示的符号在单词表中的索引值
		if(v & SC_STRUCT)    //如果符号表示的是结构体，用单词表中的sym_struct字段来填充
			pps = &ts->sym_struct;
		else
			pps = &ts->sym_identifier;
		ps->prev_tok = *pps;   //放在同名符号队列中
	    *pps = ps;
	}
	return ps;//返回该符号队列
}

/**********************************************************
        将函数符号放入全局符号表中
***********************************************************/
Symbol *func_sym_push(int v, Type *type)
{
	Symbol *s, **ps;
	s = sym_direct_push(&global_sym_stack, v, type, 0);
	ps = &((TKWord *)tktable.data[v])->sym_identifier;
	//同名符号，函数符号放在最后
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
	if((r & SC_VALMASK) ==SC_LOCAL) //局部变量
		sym = sym_push(v, type, r, addr);
	else if(v && (r & SC_VALMASK) == SC_GLOBAL)  //全局变量
	{
		sym = sym_search(v);
		if(sym)
			error("%s重定义\n", ((TKWord *)tktable.data[v])->spelling);
		else
			sym = sym_push(v, type, r|SC_SYM, 0);
	}
	//else 字符串常量
	return sym;
}

/*************************************************************
             将节名称放入全局符号表
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
                  符号的删除
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
		//更新单词表中 sym_struct sym_identifier
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
                 查找结构定义
************************************************************/
Symbol *struct_search(int v)
{
	if(v>tktable.count)
		return NULL;
	else
		return ((TKWord *)tktable.data[v])->sym_struct;
}

/************************************************************
                 查找标识符定义
************************************************************/
Symbol *sym_search(int v)
{
	if(v>tktable.count)
		return NULL;
	else
		return ((TKWord *)tktable.data[v])->sym_identifier;
}