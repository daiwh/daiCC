#include "daicc.h"

/**************************************************
             ³õÊ¼»¯Õ»´æ´¢ÈÝÁ¿
**************************************************/
void stack_init(Stack *stack, int initsize)
{
	stack->base = (void **)malloc(sizeof(void **)*initsize);
	if(!stack->base)
		error("ÄÚ´æ·ÖÅäÊ§°Ü");
	else
	{
		stack->top = stack->base;
		stack->stacksize = initsize;
	}
}

/***************************************************
             ²åÈëÔªËØÎªÐÂµÄÕ»¶¥ÔªËØ
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
			error("ÄÚ´æ·ÖÅäÊ§°Ü");
			return NULL;
		}
		stack->top = stack->base + stack->stacksize;
		stack->stacksize = newsize;
	}
	*stack->top = (void **)malloc(size);
	memcpy(*stack->top, element, size);
}

/****************************************************
                µ¯³öÕ»¶¥ÔªËØ
*****************************************************/
void stack_pop(Stack* stack)
{
	if(stack->top>stack->base)
		free(*(--stack->top));
}

/****************************************************
                  »ñµÃÕ»¶¥ÔªËØ
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
                 ÅÐ¶ÏÕ»ÊÇ·ñÎª¿Õ
****************************************************/
bool stack_is_empty(Stack *stack)
{
	if(stack->base == stack->top)
		return true;
	else
		return false;
}

/****************************************************
                   Ïú»ÙÕ»
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