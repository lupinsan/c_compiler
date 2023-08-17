#include <stdlib.h>
#include "helpers/vector.h"
#include "compiler.h"



struct lex_process* lex_process_create(struct compile_process* compiler,struct lex_process_functions* functions,void* private)
{
    struct lex_process* process = calloc(1,sizeof(struct lex_process));
    process->compiler=compiler;
    process->function=functions;
    process->private=private;
    process->pos.col=1;
    process->pos.line=1;
    process->token_vec=vector_create(sizeof(struct token));
    return process;
}

void lex_process_free(struct lex_process* process)
{
    vector_free(process->token_vec);
    free(process);
}

void* lex_process_private(struct lex_process* process)
{
    return process->private;
}
struct vector* lex_process_tokens(struct lex_process* process)
{
    return process->token_vec;
}













