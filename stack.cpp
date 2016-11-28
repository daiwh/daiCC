#include "daicc.h"

/**************************************************
             ��ʼ��ջ�洢����
**************************************************/
void stack_init(Stack *stack, int initsize)
{
	stack->base = (void **)malloc(sizeof(void **)*initsize);
	if(!stack->base)
		error("�ڴ����ʧ��");
	else
	{
		stack->top = stack->base;
		stack->stacksize = initsize;
	}
}

/***************************************************
             ����Ԫ��Ϊ�µ�ջ��Ԫ��
***************************************************/
void *stack_push(Stack *stack, void *element, int size)
{
	int newsize;
	if(stack->top = stack->base + stack->stacksize)
	{
		newsize = stack->stacksize*2;
		stack->base = (void **)realloc(stack->base, sizeof(void **)*newsize);
		if(!stack->base)
		{
			error("�ڴ����ʧ��");
			return NULL;
		}
		stack->top = stack->base + stack->stacksize;
		stack->stacksize = newsize;
	}
	*stack->top = (void **)malloc(size);
	memcpy(*stack->top, element, size);
}

/****************************************************
                ����ջ��Ԫ��
*****************************************************/
void stack_pop(Stack* stack)
{
	if(stack->top>stack->base)
		free(*(--stack->top));
}

/****************************************************
                  ���ջ��Ԫ��
****************************************************/
void *stack_get_top(Stack * stack)
{
	void **element;
	if(stack->top > stack->base)
	{
		element = stack->top-1;
		return *element;
	}
	else
		return NULL;
}

/****************************************************
                 �ж�ջ�Ƿ�Ϊ��
****************************************************/
bool stack_is_empty(Stack *stack)
{
	if(stack->base == stack->top)
		return true;
	else
		return false;
}

/****************************************************
                   ����ջ
****************************************************/
void stack_destory(Stack* stack)
{
	void **element;
	for(element=stack->base;element<stack->top;element++)
		free( *element);
	if(stack->base)
		free (stack->base);
	stack->base=NULL;
	stack->top=NULL;
	stack->stacksize=0;
}