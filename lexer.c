#include <stdlib.h>
#include "compiler.h"
#include "helpers/vector.h"
#include "helpers/buffer.h"
#include <string.h>
#include<assert.h>
#include <ctype.h>

static struct lex_process* lex_process;
static struct token tmp_token;
#define LEX_GETC_IF(buffer, c, exp)     \
    for(c=peekc();exp;c=peekc())        \
    {                                   \
        buffer_write(buffer,c);         \
        nextc();                        \
    }                                   \


bool lex_is_in_expression();
struct token* read_token();



static char peekc()
{
    return lex_process->function->peek_char(lex_process);
}

static char nextc()
{
    char c=lex_process->function->next_char(lex_process);
    if(lex_is_in_expression())
    {
        buffer_write(lex_process->parenteses_buffer,c);
    }


    lex_process->pos.col+=1;
    if(c=='\n')
    {
        lex_process->pos.col=1;
        lex_process->pos.line+=1;

    }
    return c;
}

static void pushc(char c)
{
    return lex_process->function->push_char(lex_process,c);
}


struct pos lex_file_position()
{
    return lex_process->pos;
}

static struct token* lexer_last_token()
{
    return vector_back_or_null(lex_process->token_vec);
}



static struct token* handle_whitespace()
{
    struct token* last_token=lexer_last_token();
    if(last_token)
    {
        last_token->whitespace=true;
    }

    nextc();
    return read_token();
}




struct token* token_create(struct token* _token)//????
{
    memcpy(&tmp_token,_token,sizeof(struct token));
    tmp_token.pos=lex_file_position();
    if(lex_is_in_expression())
    {
        tmp_token.bwteen_brackwets=buffer_ptr(lex_process->parenteses_buffer);
    }

    return &tmp_token;
} 




const char* read_number_str()
{
    const char* num=NULL;
    char c=peekc();
    struct buffer* buffer=buffer_create();
    LEX_GETC_IF( buffer, c, (c>='0'&&c<='9'));
    buffer_write(buffer,0x00);
    return buffer_ptr(buffer);
} 


unsigned long long read_number()
{
    const char* s=read_number_str();
    return(atoll(s));
}

int lexer_number_type(char c)
{
    int res=NUMBER_TYPE_NOMAL;
    if(c=='L')
    {
        res=NUMBER_TYPE_LONG;
    }
    else if(c=='f')
    {
        res=NUMBER_TYPE_FLOAT;
    }


    return res;


}



struct token* token_make_number_for_value(unsigned long number)
{
    int number_type=lexer_number_type(peekc());

    if(number_type!=NUMBER_TYPE_NOMAL)
    {
        nextc();//why don not skip in the lexer_number_type func?
    }

    return token_create(&(struct token){.type=TOKEN_TYPE_NUMBER,.llnum=number});
}





struct token* token_make_number()
{
    return token_make_number_for_value(read_number());
}



struct token* token_make_string(char start_delim,char end_delim)
{
    struct buffer* buf=buffer_create(sizeof(struct token));
    assert(nextc()==start_delim);
    char c=nextc();
    for(;c!=EOF && c!=end_delim;c=nextc())
    {
        if(c=='\\')
        {
            continue;
        }
        buffer_write(buf,c);
    }

    return token_create(&(struct token){.type=TOKEN_TYPE_STRING , .sval= buffer_ptr(buf)});//初始化匿名结构体
}

static bool op_treated_as_one(char op)
{
    return op == '{' ||op == '(' ||op == ',' ||op == '.' ||op == '*' ||op == '?'; 
}

static bool is_single_operator(char op)
{
    return op == '+'||
            op == '-'||
            op == '/'||
            op == '*'||
            op == '='||
            op == '>'||
            op == '<'||
            op == '|'||
            op == '&'||
            op == '^'||
            op == '%'||
            op == '!'||
            op == '('||
            op == '['||
            op == ','||
            op == '.'||
            op == '~'||
            op == '?';
}

bool op_valid(const char* op)
{
    return
    S_EQ(op,"+")||
    S_EQ(op,"-")||
    S_EQ(op,"*")||
    S_EQ(op,"/")||
    S_EQ(op,"!")||
    S_EQ(op,"^")||
    S_EQ(op,"+=")||
    S_EQ(op,"-=")||
    S_EQ(op,"*=")||
    S_EQ(op,"/=")||
    S_EQ(op,">>")||
    S_EQ(op,"<<")||
    S_EQ(op,">=")||
    S_EQ(op,"<=")||
    S_EQ(op,">")||
    S_EQ(op,"<")||
    S_EQ(op,"||")||
    S_EQ(op,"&&")||
    S_EQ(op,"|")||
    S_EQ(op,"&")||
    S_EQ(op,"++")||
    S_EQ(op,"--")||
    S_EQ(op,"=")||
    S_EQ(op,"!=")||
    S_EQ(op,"==")||
    S_EQ(op,"->")||
    S_EQ(op,"(")||
    S_EQ(op,"[")||
    S_EQ(op,",")||
    S_EQ(op,".")||
    S_EQ(op,"...")||
    S_EQ(op,"~")||
    S_EQ(op,"?")||
    S_EQ(op,"%");
}



void read_op_flush_back_keep_first(struct buffer* buffer)
{
    int len=buffer->len;
    const char* ptr=buffer_ptr(buffer);
    int i;
    for( i=len-1;i>=1;--i)
    {
        if(ptr[i]==0x00)
        {
            continue;
        }
        pushc(ptr[i]);       
    }


}

const char * read_op()
{
    bool single_operator=true;
    char op=nextc();
    struct buffer* buffer= buffer_create();
    buffer_write(buffer,op);
    if(!op_treated_as_one(op))
    {
        op=peekc();
        if(is_single_operator(op))
        {
            nextc();
            buffer_write(buffer,op);
            single_operator=false;
        }
    }
    

    buffer_write(buffer,0x00);
    char* ptr=buffer_ptr(buffer);

    if(!single_operator)
    {
        if(!op_valid(ptr))
        {
            read_op_flush_back_keep_first(buffer);
            ptr[1]=0x00;
        }

    }
    else if(!op_valid(ptr))
    {
        compile_error(lex_process->compiler,"the operator %s is not valid ",ptr);
    }

    return ptr;
}

static void lex_new_expression()
{
    lex_process->current_expression_count++;
    if(lex_process->current_expression_count==1)
    {
        lex_process->parenteses_buffer = buffer_create();
    }
}

bool lex_is_in_expression()
{
    return lex_process->current_expression_count >0;
}

bool keyword_is_datatype(const char* str)
{
    return S_EQ(str,"unsigned") ||
            S_EQ(str,"signed") ||
            S_EQ(str,"char") ||
            S_EQ(str,"short") ||
            S_EQ(str,"int") ||
            S_EQ(str,"long") ||
            S_EQ(str,"float") ||
            S_EQ(str,"double") ||
            S_EQ(str,"void") ||
            S_EQ(str,"struct") ||
            S_EQ(str,"union") ;

}






bool is_keyword(const char* str)
{
    return S_EQ(str,"unsigned") ||
            S_EQ(str,"signed") ||
            S_EQ(str,"char") ||
            S_EQ(str,"short") ||
            S_EQ(str,"int") ||
            S_EQ(str,"long") ||
            S_EQ(str,"float") ||
            S_EQ(str,"double") ||
            S_EQ(str,"void") ||
            S_EQ(str,"struct") ||
            S_EQ(str,"union") ||
            S_EQ(str,"static") ||
            S_EQ(str,"__ignore_typecheck") ||
            S_EQ(str,"return") ||
            S_EQ(str,"include") ||
            S_EQ(str,"sizeof") ||
            S_EQ(str,"if") ||
            S_EQ(str,"else") ||
            S_EQ(str,"while") ||
            S_EQ(str,"for") ||
            S_EQ(str,"do") ||
            S_EQ(str,"break") ||
            S_EQ(str,"continue") ||
            S_EQ(str,"switch") ||
            S_EQ(str,"case") ||
            S_EQ(str,"default") ||
            S_EQ(str,"goto") ||
            S_EQ(str,"typedef") ||
            S_EQ(str,"const") ||
            S_EQ(str,"extern") ||
            S_EQ(str,"restrict") ;
}


struct token* token_make_line_comment()
{
    struct buffer* buffer=buffer_create();
    char c=0;
    LEX_GETC_IF(buffer , c , c != '\n'&& c!=EOF);
    
    
    //add
    //buffer_write(buffer,0x00);
    
    
    
    return token_create(&(struct token){.type=TOKEN_TYPE_COMMENT,.sval=buffer_ptr(buffer)});
}


struct token* token_make_multiline_comment()
{
    struct buffer* buffer=buffer_create();
    char c=0;
    while(1)
    {
        LEX_GETC_IF(buffer, c, c!='*'&& c!=EOF);
        if(c==EOF)
        {
            compile_error(lex_process->compiler,"you did not close this multiline comment\n");

        }
        if(c=='*')
        {
            nextc();
            if(peekc()=='/')
            {
                nextc();
                break;
            }
        }
    }
    return token_create(&(struct token){.type=TOKEN_TYPE_COMMENT, .sval=buffer_ptr(buffer)});
}








static void lex_finish_expression()
{
    lex_process->current_expression_count--;
    if(lex_process->current_expression_count<0)
    {
        compile_error(lex_process->compiler,"you closed an expression that you never opened\n");
    }
}


struct token* token_make_operator_string()
{
    
    char op=peekc();
    if(op=='<')
    {
        if(token_is_keyword(lexer_last_token(),"include"))
        {
            return token_make_string('<','>');
        }
    }
    if(op=='(')
    {
        lex_new_expression();

    }

    



    struct token* token=token_create(&(struct token){.type=TOKEN_TYPE_OPERATOR, .sval=read_op()});
    return token;

}

struct token* token_make_symbol()
{
    char c=nextc();
    if(c==')')
    {
        lex_finish_expression();
    }
    struct token* token=token_create(&(struct token){.type=TOKEN_TYPE_SYMBOL,.cval=c});
    return token;
}


static struct token* token_make_identifier_or_keyword()
{
    struct buffer* buffer=buffer_create();
    char c = 0;
    LEX_GETC_IF(buffer ,c ,((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||(c=='_')));
    
    buffer_write(buffer,0x00);



    if(is_keyword(buffer_ptr(buffer)))
    {
        return token_create(&(struct token){.type=TOKEN_TYPE_KEYWORD,.sval=buffer_ptr(buffer)});
    }



    return token_create(&(struct token){.type=TOKEN_TYPE_IDENTIFIER, .sval=buffer_ptr(buffer)});

} 

struct token* read_special_token()
{
    char c=peekc();
    if(isalpha(c)||c=='_')
    {
        return token_make_identifier_or_keyword();
    }

    return NULL;
}


struct token* token_make_newline()
{
    nextc();
    //nextc();//deal with \r
    return token_create(&(struct token){.type=TOKEN_TYPE_NEWLINE});
}


struct token* handle_comment()
{
    char c=peekc();
    if(c=='/')
    {
        nextc();
        if(peekc()=='/')
        {
            nextc();
            return token_make_line_comment();
        }
        else if(peekc()=='*')
        {
            nextc();
            return token_make_multiline_comment();
        }
        
        pushc('/');
        return token_make_operator_string();

    }
    return NULL;
}

bool is_hex_char(char c)
{
    c=tolower(c);
    return (c>='0'&&c<='9')||(c>='a'&&c<='f'); 
}



const char* read_hex_number_str()
{
    char c=peekc();
    struct buffer* buffer=buffer_create();
    LEX_GETC_IF(buffer,c,is_hex_char(c));
    buffer_write(buffer,0x00);
    return buffer_ptr(buffer);

}



struct token* token_make_special_number_hexadecimal()
{
    nextc();
    
    unsigned long number=0;
    
    const char* number_str=read_hex_number_str();

    number=strtol(number_str,0,16);
    return token_make_number_for_value(number);
}

void lexer_validate_binary_string(const char* str)
{
    size_t len=strlen(str);
    int i;
    for(i=0;i<len-1;++i)
    {
        if(str[i]!='0' && str[i]!='1')
        {
            compile_error(lex_process->compiler,"this is not a valid binary number\n");
        }
    }
    return;
}



struct token* token_make_special_number_binary()
{
    nextc();
    
    unsigned long number=0;
    
    const char* number_str=read_number_str();

    lexer_validate_binary_string(number_str);
    number=strtol(number_str,0,2);
    return token_make_number_for_value(number);
}









void lexer_pop_token()
{
    vector_pop(lex_process->token_vec);
}




struct token* token_make_special_number()
{
    struct token* token=NULL;
    struct token* last_token=lexer_last_token();
    if(!last_token||!(last_token->type==TOKEN_TYPE_NUMBER&&last_token->llnum==0))
    {
        return token_make_identifier_or_keyword();
    }

    lexer_pop_token();

    char c=peekc();
    if(c=='x')
    {
        token=token_make_special_number_hexadecimal();
    }
    else if (c=='b')
    {
        token=token_make_special_number_binary();
    }

    return token;


}






struct token* read_token()
{
    struct token* token=NULL;
    char c=peekc();
	
		
    token=handle_comment();//优先解决注释
    if(token)
    {
        return token;
    }
    
	switch(c){
        NUMERIC_CASE:
		
            token=token_make_number();
            break;
        
        OPERATOR_CASE_EXCLUDING_DIVISION:
            
            token=token_make_operator_string();
            break;
        SYMBOL_CASE:
            token=token_make_symbol();
            break;
        case 'b':
            token=token_make_special_number();
        case 'x':
            token=token_make_special_number();
        case ' ':
        case '\t':
            token=handle_whitespace();
            break;

        case '\r'://for the '\r''\n'
            nextc();
            token=token_make_newline();
            break;
        case '\n':
            token=token_make_newline();
            break;

        case '"':
            token=token_make_string('"','"');
            break;
        
        case EOF:
            break;

        
        default:
            token=read_special_token();
            if(!token)
            {
                compile_error(lex_process->compiler,"Unexpected token!\n");
            }
    }

    return token;
}



int lex(struct lex_process* process)
{
	
    process->current_expression_count=0;
    process->parenteses_buffer=NULL;
    process->pos.filename=process->compiler->cfile.abs_path;
    lex_process=process;
	
	
    struct token* token=read_token();
	
    while(token)
    {
        vector_push(lex_process->token_vec , token);
        token=read_token();
    }

    return LEXICAL_ANALYSIS_ALL_OK;
}

char lexer_string_buffer_next_char(struct lex_process* process)
{
    struct buffer* buf=lex_process_private(process);
    return buffer_read(buf);
}

char lexer_string_buffer_peek_char(struct lex_process* process)
{
    struct buffer* buf=lex_process_private(process);
    return buffer_peek(buf);
}

void lexer_string_buffer_push_char(struct lex_process* process,char c)
{
    struct buffer* buf=lex_process_private(process);
    return buffer_write(buf,c);
}




struct lex_process_functions lexer_string_buffer_function={
    .next_char=lexer_string_buffer_next_char,
    .peek_char=lexer_string_buffer_peek_char,
    .push_char=lexer_string_buffer_push_char
};



struct lex_process* tokens_build_for_string(struct compile_process* compiler,const char* str)
{
    struct buffer* buffer=buffer_create();
    buffer_printf(buffer,str);
    
    struct lex_process* lex_process = lex_process_create(compiler , &lexer_string_buffer_function, buffer);

    if(!lex_process)
    {
        return NULL;
    }

    if(lex(lex_process)!=LEXICAL_ANALYSIS_ALL_OK)
    {
        return NULL;
    }

    return lex_process;
}