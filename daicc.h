#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define   MAXKEY 1024   //用于存放单词地址的哈希表的长度
#define  ALIGN_SET 0X100
#define   print true
/**********************************************************/
/*                      结构定义                          */
/**********************************************************/

//动态数组
typedef struct DynArray  
{              
	int count;     //当前数组元素个数
	int capacity;  //缓冲区长度
	void **data;   //数据存放
}DynArray;

//动态字符串
typedef struct DynString 
{
	int count;      //当前长度
	int capacity;   //缓冲区长度
	char* data;     //字符数据
}DynString;

//单词定义
 typedef struct TKWord   
 {
	int tkcode;                     //单词编码
	struct TKWord *next;            //指向哈希冲突的同义词
	char *spelling;                 //单词字符串
	struct Symbol *sym_struct;      //指向单词所标示的结构定义
	struct Symbol *sym_identifier;  //指向单词所表示的标识符
} TKWord;

 //动态栈
 typedef struct Stack
 {
	 void **base; //栈底指针
	 void **top;  //栈顶指针
	 int stacksize; //占当前可使用的最大容量（元素的个数）
 }Stack;

 //数据类型结构
 typedef struct Type
 {
	 int t;                //数据类型
	 struct Symbol *ref;   //引用符号  
 }Type;

 //符号存储结构
 typedef struct Symbol
 {
	 int v;                     //符号的单词编码
	 int r;                     //符号关联的寄存器
	 int c;                     //符号关联的值
	 Type type;                 //符号数据类型
	 struct Symbol *nest;       //关联的其他符号
	 struct Symbol *prev_tok;   //指向前一定义的同名符号
 }Symbol;

/***************************************************************************/
/*                             全局变量                                    */
/***************************************************************************/
static int tkvalue;           //单词值
static char ch;               //在词法分析时记录待解析的字符
static int token;             //语法分析时记录当前的单词索引
static int line_num;          //记录当前分析到的行号
static FILE* fin;             //源文件输入流
static DynArray tktable;      //单词表，里边存放所有单词的记录，包括运算符，分隔符，常量，关键字，以及标识符
static DynString tkstr;       //单词字符串（词法分析时记录当前解析单词）
static DynString sourcestr;   //单词源码字符串，它和tkstr的区别在于它记录的是源码，而tkstr记录的是值
static TKWord *tk_hashtable[MAXKEY]; //单词表所对应的哈希表，是一个指针数组，里边存的单词的地址
static int syntax_state;      //语法状态
static int syntax_level;      //语法缩进级别，语法分析的时候为了格式化输出用到
static char* filename="source.cpp"; 
static Stack global_sym_stack; //全局符号栈
static Stack local_sym_stack;  //局部符号栈
static Type char_pointer_type; //字符串指针
static Type int_type;          //int类型
static Type default_func_type; //缺省函数类型

//语法缩进标记                                          
enum e_SynTaxTtate 
{
	SNTX_NUL,      //空状态，没有语法缩进动作
	SNTX_SP,       //空格
	SNTX_LF_HT,    //换行并缩进
	SNTX_DELAY     //延迟到取出下一个单词后确定输出状态
};

//编译阶段及连接阶段遇到的错误的级别
enum e_ErrorLevel 
{
	LEVEL_WARNING,   //warning
	LEVEL_ERROR      //error
};

//发生异常是的工作阶段
enum e_WorkStage 
{
	STAGE_COMPILE,
	STAGE_LINK
};

//数据类型编码
enum e_TypeCode
{
	T_INT   =0,       //整形
	T_CHAR  =1,       //字符型
	T_SHORT =2,       //短整型
	T_VOID  =3,       //空类型
	T_PTR   =4,       //指针
	T_FUNC  =5,       //函数
	T_STRUCT=6,       //结构体

	T_BTYPE =0x000f,  //基本类型掩码
	T_ARRAY =0x0010   //数组
};

//存储类型
enum e_StorageClass
{
	SC_GLOBAL  =0x00f0,      //包括整形常量，字符常量，字符串常量，全局变量，函数定义
	SC_LOCAL   =0x00f1,      //栈中变量
	SC_LLOCAL  =0x00f2,      //寄存器溢出存放栈中
	SC_CMP     =0x00f3,      //使用标志寄存器
	SC_VALMASK =0x00ff,      //存储类型掩码
	SC_LVAL    =0x0100,      //左值
	SC_SYM     =0x0200,      //符号

	SC_ANOM    =0x10000000,  //匿名符号
	SC_STRUCT  =0x20000000,  //结构体符号
	SC_MEMBER  =0x40000000,  //结构成员变量
	SC_PARAMS   =0x80000000,  //函数参数
};

enum TokenCode
{
	/*用算符及分割符*/
TK_PLUS,	TK_MINUS,	TK_STAR,	TK_DIVIDE,	TK_MOD,
TK_EQ,		TK_NEQ,		TK_LT,		TK_LEQ,		TK_GT,
TK_GEQ,		TK_ASSIGN,	TK_POINTSTO,TK_DOT,		TK_AND,
TK_OPENPA,	TK_CLOSEPA,	TK_OPENBR,	TK_CLOSEBR,	TK_BEGIN,
TK_END,		TK_SEMICOLON,TK_COMMA,	TK_ELLIPSIS,TK_EOF,

/*常量：*/
TK_CINT,	TK_CCHAR,	TK_CSTR,


/*关键字*/

KW_CHAR,	KW_SHORT,	KW_INT,	KW_VOID,	KW_STRUCT,
KW_IF,		KW_ELSE,	KW_FOR,	KW_CONTINUE,KW_BREAK,
KW_RETURN,	KW_SIZEOF,	KW_ALIGN,KW_CDECL,	KW_STDCALL,

/*标识符*/
TK_IDENT
};
/***********************************************************************************/
/*                      其他文件要引用的函数申明                                   */
/***********************************************************************************/
//dynarray动态数组(dynarray.cpp)
void dynarray_init(DynArray* parr,int initsize);           //初始化动态数组
void dynarray_add(DynArray *parr,void *data);              //追加动态数组元素
void dynarray_realloc(DynArray *parr,int new_size);        //重新分配动态数组容量
void dynarray_free(DynArray *parr);                        //释放动态数组占用的全部内存
int  dynarray_search(DynArray *parr,int key);               //在动态数组中查找元素，找到返回索引，查找不到返回-1

//dynstring动态字符串（dynstring.cpp)
void dynstring_init(DynString * pstr,int initsize);        //初始化动态字符串
void dynstring_free(DynString *pstr);                      //释放动态字符串使用的内存空间
void dynstring_reset(DynString *pstr);                     //先释放动态字符串，然后重新初始化
void dynstring_realloc(DynString *pstr,int new_size);      //重新分配动态字符串容量
void dynstring_chcat(DynString *pstr,char ch);             //追加字符到动态字符串尾部

//tkword单词(tkword.cpp)
TKWord * tkword_insert(char *p);                            //标识符插入单词表
TKWord *tkword_direct_insert(TKWord *tp);                   //运算符及分割符，关键字，常量标记放入单词表
TKWord * tkword_find(char *p,int keyno);                    //根据拼写查找单词

//动态栈(stack.cpp)
void stack_init(Stack *stack, int initsize);                //初始化栈存储容量
void *stack_push(Stack *stack, void *element, int size);    //插入元素为新的栈顶元素
void stack_pop(Stack* stack);                               //弹出栈顶元素
void *stack_get_top(Stack * stack);                         //获得栈顶元素
bool stack_is_empty(Stack *stack);                          //判断栈是否为空
void stack_destory(Stack* stack);                           //销毁栈

//符号(symbol.cpp)
Symbol *sym_direct_push(Stack *ss, int v, Type *type, int c);//将符号放入符号栈中
Symbol *sym_push(int v,Type *type, int r, int c);            //将符号放入符号栈中，动态判断是放入全局符号栈还是局部符号栈
Symbol *func_sym_push(int v, Type *type);                    //将函数符号放入全局符号表中
Symbol *var_sym_put(Type *type, int r, int v, int addr);     //
Symbol *sec_sym_put(char *sec, int c);                       //将节名称放入全局符号表中
void sym_pop(Stack *ptop, Symbol *b);                        //符号的删除
Symbol *struct_search(int v);                                //查找结构定义
Symbol *sym_search(int v);                                   //查找标识符定义

//编译链接过程中异常处理(error.cpp)
void warning(char *fmt, ...);                              //编译警告
void error(char *fmt, ...);                                //编译错误
void link_error(char *fmt, ...);                           //链接错误
void expect(char *msg);                                    //语法成分缺少
char *get_tkstr(int v);                                    //取单词所代表的源码字符串
void skip(int c);                                          //跳过指定单词并检测

//词法分析模块(lex.cpp)
void get_token();                                          //获得词法分析中解析出的单词
void init_lex();                                           //词法分析初始化


//语法分析模块(grammar.cpp)
//外部申明 
void translation_unit();                                   //语法分析入口函数
void external_declaration(int l);                          // 解析外部申明
int  type_specifier();                                     //类型区分符,解析成功返回1，不成功返回0
void struct_specifier();                                   // 结构区分符
void struct_declaration_list();                            //结构申明符表
void struct_declaration();                                 //结构申明
void function_calling_convention(int *fc);                 //用于函数申明上，用在数据申明上忽略掉
void struct_member_alignment();                            //结构成员对齐
void declarator();                                         //申明符
void direct_declarator();                                  //直接申明符
void direct_declarator_postfix();                          //直接申明后缀
void parameter_type_list(int func_call);                   //形参类型表
void funcbody();                                           // 函数体
void initializer();                                        // 初值符

// 语句
void statement(int *bsym, int *csym);                      //语句分析入口
void compound_statement();                                 //复合语句
int is_type_specifier(int v);                              // 判断是否为类型区分符
void expression_statement();                               //表达式语句
void if_statement();                                       //if-else选择语句
void for_statement();                                      //for循环语句
void continue_statement();                                 //continue跳转语句
void break_statement();                                    //break跳转语句
void return_statement();                                   //return跳转语句

// 表达式
void expression();                                         //表达式分析入口
void assignment_expression();                              //赋值表达式
void equality_expression();                                //相等类表达式
void relational_expression();                              //关系表达式
void additive_expression();                                //加减类表达式
void multiplicative_expression();                          //乘除类表达式
void unary_expression();                                   //一元表达式
void sizeof_expression();                                  //sizeof表达式
void postfix_expression();                                 // 后缀表达式
void primary_expression();                                 //初值表达式
void argument_expression_list();                           // 实参表达式.

//测试语法对齐
void syntax_indent();