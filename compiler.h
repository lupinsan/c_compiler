#ifndef     PEACHCOMPILER_H
#define     PEACHCOMPILER_H


#include <stdio.h>
#include<stdbool.h>
#include"helpers/vector.h"
#include"helpers/buffer.h"
#include <string.h>

#define S_EQ(str1,str2) \
        (str1&&str2&&(strcmp(str1,str2)==0))






enum
{
    COMPILER_FILE_COMPILED_OK,
    COMPILER_FAILED_WITH_ERRORS
};


//struct pos;
struct pos
{
    int line;
    int col;
    const char* filename;
};

struct scope
{
    int flags;

    struct vector* entities;
    //堆栈对齐16字节
    size_t size;


    struct scope* parent;
};

enum{
    SYMBOL_TYPE_NODE,
    SYMBOL_TYPE_NATIVE_FUNCTION,
    SYMBOL_TYPE_UNKNOWN
};

struct symbol{
    const char* name;
    int type;
    void* data;
};






struct  compile_process
{
    int flags;
    struct pos pos;
    struct compile_process_input_file
    {
        FILE* fp;
        const char* abs_path;
    }cfile;
    

    struct vector* token_vec;
    struct vector* node_vec;
    struct vector* node_tree_vec;

    FILE* ofile;

    struct {
        struct scope* root;
        struct scope* current; 
    }scope;

    struct 
    {
        struct vector* table;

        struct vector* tables;
    }symbols;
    



    
};






enum
{
    PARSE_ALL_OK,
    PARSE_GENERAL_ERROR
};



enum
{
    NODE_TYPE_EXPRESSION,
    NODE_TYPE_EXPRESSION_PARENTHSES,
    NODE_TYPE_NUMBER,
    NODE_TYPE_IDENIFIER,
    NODE_TYPE_STRING,
    NODE_TYPE_VARIABLE,
    NODE_TYPE_VARIABLE_LIST,
    NODE_TYPE_FUNCTION,
    NODE_TYPE_BODY,
    NODE_TYPE_STATEMENT_RETURN,
    NODE_TYPE_STATEMENT_IF,
    NODE_TYPE_STATEMENT_ELSE,
    NODE_TYPE_STATEMENT_WHILE,
    NODE_TYPE_STATEMENT_DO_WHILE,
    NODE_TYPE_STATEMENT_BREAK,
    NODE_TYPE_STATEMENT_FOR,
    NODE_TYPE_STATEMENT_CONTINUE,
    NODE_TYPE_STATEMENT_SWITCH,
    NODE_TYPE_STATEMENT_CASE,
    NODE_TYPE_STATEMENT_DEFAULT,
    NODE_TYPE_GOTO,


    NODE_TYPE_UNARY,
    NODE_TYPE_TENARY,
    NODE_TYPE_LABEL,
    NODE_TYPE_STRUCT,
    NODE_TYPE_UNION,
    NODE_TYPE_BRACKET,
    NODE_TYPE_CAST,
    NODE_TYPE_BLANK
};

struct array_brackets{
    struct vector* n_brackets;
};




enum
{
    NODE_FLAG_INSIDE_EXPRESSION = 0b00000001,
    NODE_FLAG_IS_FORWARD_DECLARATION = 0b00000010,
    NODE_FLAG_HAS_VARIABLE_COMBINED = 0b00000100

};

struct node;

struct datatype
{
    int flags;

    int type;

    struct datatype* secondary;

    const char* type_str;
    size_t size; 
    int pointer_depth;
    union
    {
        struct node* struct_node;
        struct node* union_node;
    };
    
    struct array{
    	struct array_brackets* brackets;        
        size_t size;//size*n_brackets
	}array;
};

struct node
{
    int type;
    int flags;

    struct pos pos;

    struct node_binded
    {
        struct node* owner;

        struct node* function;

    }binded;

    union 
    {
        struct exp
        {
            struct node* left;
            struct node* right;
            const char* op;
        }exp;

        struct var
        {
            struct datatype type;
            const char* name;
            struct node* val;
            int padding;
            int aoffset;
        } var;
        
        struct var_list
        {
            struct vector* list;
        }var_list;

        struct bracket{
        	struct node* inner;
        }bracket;
        
        struct _struct{
        const char* name;
        struct node* body_n;

        struct node* var;//struct创建时带的实例
        }_struct;

        struct body{
            struct vector* statements;

            size_t size;

            bool padded;

            struct node* largest_var_node;
        }body;

        struct function
        {
            int flags;
            //return type
            struct datatype rtype;

            //name
            const char* name;

            struct function_arguments
            {   
                struct vector* vector;
                size_t stack_addition;

            } args;

            //指向bodynode的指针
            struct node* body_n;
            //函数栈
            size_t stack_size;

        } func;
    };
    

    union 
    {
        char cval;
        const char* sval;
        unsigned int inum;
        unsigned long lnum;
        unsigned long long llnum;
    };
    
    

};

enum
{
    DATATYPE_FLAG_IS_SIGNED = 0b00000001,
    DATATYPE_FLAG_IS_STATIC = 0b00000010,
    DATATYPE_FLAG_IS_CONST = 0b00000100,
    DATATYPE_FLAG_IS_POINTER = 0b00001000,
    DATATYPE_FLAG_IS_ARRAY = 0b00010000,
    DATATYPE_FLAG_IS_EXTERN = 0b00100000,
    DATATYPE_FLAG_IS_RESTRICT = 0b01000000,
    DATATYPE_FLAG_IS_IGNORE_TYPE_CHECKING = 0b10000000,
    DATATYPE_FLAG_IS_SECONDARY = 0b100000000,
    DATATYPE_FLAG_STRUCT_UNION_NO_NAME = 0b1000000000,
    DATATYPE_FLAG_IS_LITERAL = 0b10000000000
};


enum
{
    DATA_TYPE_VOID,
    DATA_TYPE_CHAR,
    DATA_TYPE_SHORT,
    DATA_TYPE_INTEGER,
    DATA_TYPE_LONG,
    DATA_TYPE_FLOAT,
    DATA_TYPE_DOUBLE,
    DATA_TYPE_STRUCT,
    DATA_TYPE_UNION,
    DATA_TYPE_UNKNOW
};









enum 
{
    DATA_TYPE_EXPRCT_PRIMITIVE,
    DATA_TYPE_EXPECT_UNION,
    DATA_TYPE_EXPECT_STRUCT
};


enum 
{
    DATA_SIZE_ZERO = 0,
    DATA_SIZE_BYTE = 1,
    DATA_SIZE_WORD = 2,
    DATA_SIZE_DWORD = 4,
    DATA_SIZE_DDWORD = 8
};

enum
{
    FUNCTION_NODE_IS_NATIVE = 0b00000001
};



int compile_file(const char* file_name, const char*out_filename, int flags);
struct  compile_process* compile_process_create(const char* filename,   const char* filename_out,   int flags);



enum
{
    TOKEN_TYPE_IDENTIFIER,
    TOKEN_TYPE_KEYWORD,
    TOKEN_TYPE_OPERATOR,
    TOKEN_TYPE_SYMBOL,
    TOKEN_TYPE_NUMBER,
    TOKEN_TYPE_STRING,
    TOKEN_TYPE_COMMENT,
    TOKEN_TYPE_NEWLINE
};

enum
{
    NUMBER_TYPE_NOMAL,
    NUMBER_TYPE_LONG,
    NUMBER_TYPE_FLOAT,
    NUMBER_TYPE_DOUBLIE
};




struct token
{
    int type;
    int flags;
    struct pos pos;
    union 
    {
        char cval;
        const char* sval;
        unsigned int inum;
        unsigned long lnum;
        unsigned long long llnum;
        void* any;
    };

    struct token_number
    {
        int type;
    } num;


    bool whitespace;
    const char* bwteen_brackwets;
    
};


struct lex_process
{
    struct pos pos;
    struct compile_process* compiler;
    struct vector* token_vec;

    int current_expression_count;
    struct buffer* parenteses_buffer;
    struct lex_process_functions* function;


    void * private;
};

typedef char (*LEX_PROCESS_NEXT_CHAR)(struct lex_process* process);
typedef char (*LEX_PROCESS_PEEK_CHAR)(struct lex_process* process);
typedef void (*LEX_PROCESS_PUSH_CHAR)(struct lex_process* process,char c);


struct lex_process_functions
{
    LEX_PROCESS_NEXT_CHAR next_char;
    LEX_PROCESS_PEEK_CHAR peek_char;
    LEX_PROCESS_PUSH_CHAR push_char;

};



char compile_process_next_char(struct lex_process* lex_process);
char compile_process_peek_char(struct lex_process* lex_process);
void compile_process_push_char(struct lex_process* lex_process,char c);




struct lex_process* lex_process_create(struct compile_process* compiler,struct lex_process_functions* functions,void* private);
void lex_process_free(struct lex_process* process);
void* lex_process_private(struct lex_process* process);
struct vector* lex_process_tokens(struct lex_process* process);

int lex(struct lex_process* process);

struct lex_process* tokens_build_for_string(struct compile_process* compiler,const char* str);


enum
{
    LEXICAL_ANALYSIS_ALL_OK,
    LEXICAL_ANALYSIS_INPUT_ERROR
};

void compile_error(struct compile_process* compiler,const char* msg, ... );
void compile_warning(struct compile_process* compiler,const char* msg, ... );


#define NUMERIC_CASE \
    case '0':       \
    case '1':       \
    case '2':       \
    case '3':       \
    case '4':       \
    case '5':       \
    case '6':       \
    case '7':       \
    case '8':       \
    case '9'       

#define OPERATOR_CASE_EXCLUDING_DIVISION    \
    case '+':                               \
    case '-':                               \
    case '*':                               \
    case '>':                               \
    case '<':                               \
    case '^':                               \
    case '%':                               \
    case '!':                               \
    case '=':                               \
    case '~':                               \
    case '|':                               \
    case '&':                               \
    case '(':                               \
    case '[':                               \
    case ',':                               \
    case '.':                               \
    case '?'                                     



#define SYMBOL_CASE \
    case '{':       \
    case '}':       \
    case ':':       \
    case ';':       \
    case '#':       \
    case '\\':       \
    case ')':       \
    case ']'       


bool token_is_keyword(struct token* token, const char* value);
bool token_is_identifier(struct token* token);
bool keyword_is_datatype(const char* str);




int parser(struct compile_process* process);


bool token_is_nl_or_comment_or_newline_seperator(struct token* token);
bool token_is_symbol(struct token* token,char c);
bool token_is_primitive_keyword(struct token* token);
bool token_is_operator(struct token* token,const char* op);

struct array_brackets* array_brackets_new();
void array_brackets_free(struct array_brackets* b);
void array_brackets_add(struct array_brackets* brackets , struct node* bracket_node);
struct vector* array_brackets_node_vector(struct array_brackets* brackets);
size_t array_brackets_calculate_size_from_index(struct datatype* dtype, struct array_brackets* brackets, int index);
size_t array_brackets_calculate_size(struct datatype* dtype, struct array_brackets* brackets);
int array_total_indexes(struct datatype* dtype);

struct scope* scope_alloc();
void scope_dealloc(struct scope* scope);
struct scope* scope_create_root(struct compile_process* process);
void scope_free_root(struct compile_process* process);
struct scope* scope_new(struct compile_process* process, int flags);
void scope_iteratoration_start(struct scope* scope);
void scope_iteration_end(struct scope* scope);
void* scope_iterator_back(struct scope* sccope);
void* scope_last_entity_at_scope(struct scope* scope);
void* scope_last_entity_from_sccope_stop_at(struct scope* scope, struct scope* stop_scope);
void* scope_last_entity_stop_at(struct compile_process* process, struct scope* stop_scope);
void* scope_last_entity(struct compile_process* process);
void scope_push(struct compile_process* process,void* ptr, size_t elem_size);
void scope_finish(struct compile_process* process);
struct scope* scope_current(struct compile_process* process);






struct node* node_create(struct node* _node);//注意命名！++复制构造函数 ++在类vec中构造node
struct node* node_pop();
struct node* node_peek();
struct node* node_peek_or_null();
void node_push(struct node* node);
void node_set_vector(struct vector* vec,struct vector* root_vec);

bool node_is_expressionable(struct node* node);
bool node_is_struct_or_union_variable(struct node* node);
bool variable_node_is_primitive(struct node* node);
struct node* variable_node_or_list(struct node* node);
struct node* node_from_sym(struct symbol* sym);
struct node* node_from_symbol(struct compile_process* current_process, const char* name);
struct node* struct_node_from_for_name(struct compile_process* current_process, const char* name);


struct node* node_peek_expressionable_or_null();
void make_exp_node(struct node* left_node,struct node* right_node,const char* op);
void make_bracket_node(struct node* node);
void make_body_node(struct vector* body_vec, size_t size, bool padded,struct node* largest_var_node);
void make_struct_node(const char* name, struct node* body_node);
struct node*  make_function_node(struct datatype* ret_type, const char* name, struct vector* arguments, struct node* body_node);




#define TOTAL_OPERATOR_GROUPS 14
#define MAX_OPERATOR_IN_GROUP 12

enum
{
    ASSOSIATIVITY_LEFT_TO_RIGHT,
    ASSOSIATIVITY_RIGHT_TO_LEFT
};


struct expressionable_op_precedence_group
{
    char* operators[MAX_OPERATOR_IN_GROUP];
    int associtivity;
};


 bool datatype_is_struct_or_union_for_name(const char* name);
 bool datatype_is_struct_or_union(struct datatype* dtype);
bool datatype_is_primitive(struct datatype* dtype);

size_t datatype_size_for_array_access(struct datatype* dtype);
size_t datatype_element_size(struct datatype* dtype);
size_t datatype_size_no_ptr(struct datatype* dtype);
size_t datatype_size(struct datatype* dtype);

//从variablenode中提取variablesize
size_t variable_size(struct node* var_node);
size_t variable_size_for_list(struct node*var_list_node);


struct node* variable_node(struct node* node);
int padding(int val, int to);
int align_value(int val, int to);
int align_value_treat_positive(int val, int to);//处理栈时
int compute_sum_padding(struct vector* vec);
struct node* variable_struct_or_union_body_node(struct node* node);

struct symbol* symresolver_get_symbol_for_native_function(struct compile_process* process, const char* name);
void symresolver_build_for_node(struct compile_process* process, struct node* node);
struct symbol* symresolver_get_symbol(struct compile_process* process, const char* name);
void symresolver_initialize(struct compile_process* process);
void symresolver_new_table(struct compile_process* process);
void symresolver_end_table(struct compile_process* process);


#endif
