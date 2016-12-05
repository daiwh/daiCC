#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define   MAXKEY 1024   //���ڴ�ŵ��ʵ�ַ�Ĺ�ϣ��ĳ���
#define  ALIGN_SET 0X100
#define   print true
/**********************************************************/
/*                      �ṹ����                          */
/**********************************************************/

//��̬����
typedef struct DynArray  
{              
	int count;     //��ǰ����Ԫ�ظ���
	int capacity;  //����������
	void **data;   //���ݴ��
}DynArray;

//��̬�ַ���
typedef struct DynString 
{
	int count;      //��ǰ����
	int capacity;   //����������
	char* data;     //�ַ�����
}DynString;

//���ʶ���
 typedef struct TKWord   
 {
	int tkcode;                     //���ʱ���
	struct TKWord *next;            //ָ���ϣ��ͻ��ͬ���
	char *spelling;                 //�����ַ���
	struct Symbol *sym_struct;      //ָ�򵥴�����ʾ�Ľṹ����
	struct Symbol *sym_identifier;  //ָ�򵥴�����ʾ�ı�ʶ��
} TKWord;

 //��̬ջ
 typedef struct Stack
 {
	 void **base; //ջ��ָ��
	 void **top;  //ջ��ָ��
	 int stacksize; //ռ��ǰ��ʹ�õ����������Ԫ�صĸ�����
 }Stack;

 //�������ͽṹ
 typedef struct Type
 {
	 int t;                //��������
	 struct Symbol *ref;   //���÷���  
 }Type;

 //���Ŵ洢�ṹ
 typedef struct Symbol
 {
	 int v;                     //���ŵĵ��ʱ���
	 int r;                     //���Ź����ļĴ���
	 int c;                     //���Ź�����ֵ
	 Type type;                 //������������
	 struct Symbol *nest;       //��������������
	 struct Symbol *prev_tok;   //ָ��ǰһ�����ͬ������
 }Symbol;

/***************************************************************************/
/*                             ȫ�ֱ���                                    */
/***************************************************************************/
static int tkvalue;           //����ֵ
static char ch;               //�ڴʷ�����ʱ��¼���������ַ�
static int token;             //�﷨����ʱ��¼��ǰ�ĵ�������
static int line_num;          //��¼��ǰ���������к�
static FILE* fin;             //Դ�ļ�������
static DynArray tktable;      //���ʱ���ߴ�����е��ʵļ�¼��������������ָ������������ؼ��֣��Լ���ʶ��
static DynString tkstr;       //�����ַ������ʷ�����ʱ��¼��ǰ�������ʣ�
static DynString sourcestr;   //����Դ���ַ���������tkstr��������������¼����Դ�룬��tkstr��¼����ֵ
static TKWord *tk_hashtable[MAXKEY]; //���ʱ�����Ӧ�Ĺ�ϣ����һ��ָ�����飬��ߴ�ĵ��ʵĵ�ַ
static int syntax_state;      //�﷨״̬
static int syntax_level;      //�﷨���������﷨������ʱ��Ϊ�˸�ʽ������õ�
static char* filename="source.cpp"; 
static Stack global_sym_stack; //ȫ�ַ���ջ
static Stack local_sym_stack;  //�ֲ�����ջ
static Type char_pointer_type; //�ַ���ָ��
static Type int_type;          //int����
static Type default_func_type; //ȱʡ��������

//�﷨�������                                          
enum e_SynTaxTtate 
{
	SNTX_NUL,      //��״̬��û���﷨��������
	SNTX_SP,       //�ո�
	SNTX_LF_HT,    //���в�����
	SNTX_DELAY     //�ӳٵ�ȡ����һ�����ʺ�ȷ�����״̬
};

//����׶μ����ӽ׶������Ĵ���ļ���
enum e_ErrorLevel 
{
	LEVEL_WARNING,   //warning
	LEVEL_ERROR      //error
};

//�����쳣�ǵĹ����׶�
enum e_WorkStage 
{
	STAGE_COMPILE,
	STAGE_LINK
};

//�������ͱ���
enum e_TypeCode
{
	T_INT   =0,       //����
	T_CHAR  =1,       //�ַ���
	T_SHORT =2,       //������
	T_VOID  =3,       //������
	T_PTR   =4,       //ָ��
	T_FUNC  =5,       //����
	T_STRUCT=6,       //�ṹ��

	T_BTYPE =0x000f,  //������������
	T_ARRAY =0x0010   //����
};

//�洢����
enum e_StorageClass
{
	SC_GLOBAL  =0x00f0,      //�������γ������ַ��������ַ���������ȫ�ֱ�������������
	SC_LOCAL   =0x00f1,      //ջ�б���
	SC_LLOCAL  =0x00f2,      //�Ĵ���������ջ��
	SC_CMP     =0x00f3,      //ʹ�ñ�־�Ĵ���
	SC_VALMASK =0x00ff,      //�洢��������
	SC_LVAL    =0x0100,      //��ֵ
	SC_SYM     =0x0200,      //����

	SC_ANOM    =0x10000000,  //��������
	SC_STRUCT  =0x20000000,  //�ṹ�����
	SC_MEMBER  =0x40000000,  //�ṹ��Ա����
	SC_PARAMS   =0x80000000,  //��������
};

enum TokenCode
{
	/*��������ָ��*/
TK_PLUS,	TK_MINUS,	TK_STAR,	TK_DIVIDE,	TK_MOD,
TK_EQ,		TK_NEQ,		TK_LT,		TK_LEQ,		TK_GT,
TK_GEQ,		TK_ASSIGN,	TK_POINTSTO,TK_DOT,		TK_AND,
TK_OPENPA,	TK_CLOSEPA,	TK_OPENBR,	TK_CLOSEBR,	TK_BEGIN,
TK_END,		TK_SEMICOLON,TK_COMMA,	TK_ELLIPSIS,TK_EOF,

/*������*/
TK_CINT,	TK_CCHAR,	TK_CSTR,


/*�ؼ���*/

KW_CHAR,	KW_SHORT,	KW_INT,	KW_VOID,	KW_STRUCT,
KW_IF,		KW_ELSE,	KW_FOR,	KW_CONTINUE,KW_BREAK,
KW_RETURN,	KW_SIZEOF,	KW_ALIGN,KW_CDECL,	KW_STDCALL,

/*��ʶ��*/
TK_IDENT
};
/***********************************************************************************/
/*                      �����ļ�Ҫ���õĺ�������                                   */
/***********************************************************************************/
//dynarray��̬����(dynarray.cpp)
void dynarray_init(DynArray* parr,int initsize);           //��ʼ����̬����
void dynarray_add(DynArray *parr,void *data);              //׷�Ӷ�̬����Ԫ��
void dynarray_realloc(DynArray *parr,int new_size);        //���·��䶯̬��������
void dynarray_free(DynArray *parr);                        //�ͷŶ�̬����ռ�õ�ȫ���ڴ�
int  dynarray_search(DynArray *parr,int key);               //�ڶ�̬�����в���Ԫ�أ��ҵ��������������Ҳ�������-1

//dynstring��̬�ַ�����dynstring.cpp)
void dynstring_init(DynString * pstr,int initsize);        //��ʼ����̬�ַ���
void dynstring_free(DynString *pstr);                      //�ͷŶ�̬�ַ���ʹ�õ��ڴ�ռ�
void dynstring_reset(DynString *pstr);                     //���ͷŶ�̬�ַ�����Ȼ�����³�ʼ��
void dynstring_realloc(DynString *pstr,int new_size);      //���·��䶯̬�ַ�������
void dynstring_chcat(DynString *pstr,char ch);             //׷���ַ�����̬�ַ���β��

//tkword����(tkword.cpp)
TKWord * tkword_insert(char *p);                            //��ʶ�����뵥�ʱ�
TKWord *tkword_direct_insert(TKWord *tp);                   //��������ָ�����ؼ��֣�������Ƿ��뵥�ʱ�
TKWord * tkword_find(char *p,int keyno);                    //����ƴд���ҵ���

//��̬ջ(stack.cpp)
void stack_init(Stack *stack, int initsize);                //��ʼ��ջ�洢����
void *stack_push(Stack *stack, void *element, int size);    //����Ԫ��Ϊ�µ�ջ��Ԫ��
void stack_pop(Stack* stack);                               //����ջ��Ԫ��
void *stack_get_top(Stack * stack);                         //���ջ��Ԫ��
bool stack_is_empty(Stack *stack);                          //�ж�ջ�Ƿ�Ϊ��
void stack_destory(Stack* stack);                           //����ջ

//����(symbol.cpp)
Symbol *sym_direct_push(Stack *ss, int v, Type *type, int c);//�����ŷ������ջ��
Symbol *sym_push(int v,Type *type, int r, int c);            //�����ŷ������ջ�У���̬�ж��Ƿ���ȫ�ַ���ջ���Ǿֲ�����ջ
Symbol *func_sym_push(int v, Type *type);                    //���������ŷ���ȫ�ַ��ű���
Symbol *var_sym_put(Type *type, int r, int v, int addr);     //
Symbol *sec_sym_put(char *sec, int c);                       //�������Ʒ���ȫ�ַ��ű���
void sym_pop(Stack *ptop, Symbol *b);                        //���ŵ�ɾ��
Symbol *struct_search(int v);                                //���ҽṹ����
Symbol *sym_search(int v);                                   //���ұ�ʶ������

//�������ӹ������쳣����(error.cpp)
void warning(char *fmt, ...);                              //���뾯��
void error(char *fmt, ...);                                //�������
void link_error(char *fmt, ...);                           //���Ӵ���
void expect(char *msg);                                    //�﷨�ɷ�ȱ��
char *get_tkstr(int v);                                    //ȡ�����������Դ���ַ���
void skip(int c);                                          //����ָ�����ʲ����

//�ʷ�����ģ��(lex.cpp)
void get_token();                                          //��ôʷ������н������ĵ���
void init_lex();                                           //�ʷ�������ʼ��


//�﷨����ģ��(grammar.cpp)
//�ⲿ���� 
void translation_unit();                                   //�﷨������ں���
void external_declaration(int l);                          // �����ⲿ����
int  type_specifier();                                     //�������ַ�,�����ɹ�����1�����ɹ�����0
void struct_specifier();                                   // �ṹ���ַ�
void struct_declaration_list();                            //�ṹ��������
void struct_declaration();                                 //�ṹ����
void function_calling_convention(int *fc);                 //���ں��������ϣ��������������Ϻ��Ե�
void struct_member_alignment();                            //�ṹ��Ա����
void declarator();                                         //������
void direct_declarator();                                  //ֱ��������
void direct_declarator_postfix();                          //ֱ��������׺
void parameter_type_list(int func_call);                   //�β����ͱ�
void funcbody();                                           // ������
void initializer();                                        // ��ֵ��

// ���
void statement(int *bsym, int *csym);                      //���������
void compound_statement();                                 //�������
int is_type_specifier(int v);                              // �ж��Ƿ�Ϊ�������ַ�
void expression_statement();                               //���ʽ���
void if_statement();                                       //if-elseѡ�����
void for_statement();                                      //forѭ�����
void continue_statement();                                 //continue��ת���
void break_statement();                                    //break��ת���
void return_statement();                                   //return��ת���

// ���ʽ
void expression();                                         //���ʽ�������
void assignment_expression();                              //��ֵ���ʽ
void equality_expression();                                //�������ʽ
void relational_expression();                              //��ϵ���ʽ
void additive_expression();                                //�Ӽ�����ʽ
void multiplicative_expression();                          //�˳�����ʽ
void unary_expression();                                   //һԪ���ʽ
void sizeof_expression();                                  //sizeof���ʽ
void postfix_expression();                                 // ��׺���ʽ
void primary_expression();                                 //��ֵ���ʽ
void argument_expression_list();                           // ʵ�α��ʽ.

//�����﷨����
void syntax_indent();