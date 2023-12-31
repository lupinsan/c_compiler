#include"compiler.h"
#include <stdlib.h>
#include <stdarg.h>


struct lex_process_functions compiler_lex_functions = {
    .next_char=compile_process_next_char,
    .peek_char=compile_process_peek_char,
    .push_char=compile_process_push_char
};

void compile_error(struct compile_process* compiler,const char* msg, ... )
{
    va_list args;
    va_start(args,msg);
    vfprintf(stderr,msg,args);
    va_end(args);
    fprintf(stderr,"on line %i, col %i in file %s\n",compiler->pos.line,compiler->pos.col,compiler->cfile.abs_path);
    
    exit(-1);//错误函数强制退出
}

void compile_warning(struct compile_process* compiler,const char* msg, ... )
{
    va_list args;
    va_start(args,msg);
    vfprintf(stderr,msg,args);
    va_end(args);
    fprintf(stderr,"on line %i, col %i in file %s\n",compiler->pos.line,compiler->pos.col,compiler->cfile.abs_path);
    
    return;
}









int compile_file(const char* file_name, const char*out_filename,    int flags)
{
    struct compile_process* process= compile_process_create(file_name,out_filename,flags);
    
    if(!process){
        return COMPILER_FAILED_WITH_ERRORS;
    }

    //lexical
	
	
    struct lex_process* lex_process= lex_process_create(process,&compiler_lex_functions,NULL);


    if(!lex_process)
    {
        return COMPILER_FAILED_WITH_ERRORS;
    }

    if(lex(lex_process)!= LEXICAL_ANALYSIS_ALL_OK)
    {
        return COMPILER_FAILED_WITH_ERRORS;
    }

    process->token_vec=lex_process->token_vec;


    //parsing
    if(parser(process)!=PARSE_ALL_OK)
    {
        return COMPILER_FAILED_WITH_ERRORS;
    }

    //perform code generation


    return COMPILER_FILE_COMPILED_OK;


}
