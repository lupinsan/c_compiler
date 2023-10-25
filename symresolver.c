#include "compiler.h"

static void symresolver_push_symbol(struct compile_process* process, struct symbol* sym){
    vector_push(process->symbols.tables, &sym);
}


void symresolver_initialize(struct compile_process* process){
    process->symbols.tables = vector_create(sizeof(struct vector*));

}

void symresolver_new_table(struct compile_process* process){
    vector_push(process->symbols.tables,&process->symbols.table);

    process->symbols.table = vector_create(sizeof(struct symbol*));
}

void symresolver_end_table(struct compile_process* process){
    struct vector* last_table = vector_back_ptr(process->symbols.tables);
    process->symbols.table = last_table;
    vector_pop(process->symbols.tables);
}


struct symbol* symresolver_get_symbol(struct compile_process* process, const char* name){
    vector_set_peek_pointer(process->symbols.table,0);
    struct symbol* symbol = vector_peek_ptr(process->symbols.table);
    while(symbol){
        if(S_EQ(symbol->name,name)){
            break;
        }

        symbol = vector_peek_ptr(process->symbols.table);
    }

    return symbol;
}


struct symbol* symresolver_get_symbol_for_native_function(struct compile_process* process, const char* name){
    struct symbol * sym = symresolver_get_symbol(process, name);

    if(!sym){
        return NULL;
    }

    if(sym->type != SYMBOL_TYPE_NATIVE_FUNCTION){
        return NULL;
    }
    return sym;
}

struct symbol* symresolver_register_symbol(struct compile_process* process, const char* name, int type, void* data){
    if(symresolver_get_symbol(process, name)){
        return NULL;
    }

    struct symbol* sym = calloc(1,sizeof(struct symbol));
    sym->name = name;
    sym->type = type;
    sym->data = data;
    symresolver_push_symbol(process, sym);
    return sym;
}

struct node* symresolver_node(struct symbol* sym){
    if(sym->type != SYMBOL_TYPE_NODE){
        return NULL;
    }

    return sym->data;
}


void symresolver_build_for_variable_node(struct compile_process* process, struct node* node){
    compile_error(process,"var not sup\n");   
}

void symresolver_build_for_function_node(struct compile_process* process, struct node* node){
    compile_error(process,"func not sup\n");   
}

void symresolver_build_for_structure_node(struct compile_process* process, struct node* node){
    if(node->flags & NODE_FLAG_IS_FORWARD_DECLARATION)
    {
        return;
    }
    symresolver_register_symbol(process, node->_struct.name , SYMBOL_TYPE_NODE, node);

}

void symresolver_build_for_union_node(struct compile_process* process, struct node* node){
    compile_error(process,"unions not sup\n");   
}

void symresolver_build_for_node(struct compile_process* process, struct node* node)
{
    switch (node->type)
    {
    case NODE_TYPE_VARIABLE:
        symresolver_build_for_variable_node(process,node);
        break;

    case NODE_TYPE_FUNCTION:
        symresolver_build_for_function_node(process,node);
        break;

    case NODE_TYPE_STRUCT:
        symresolver_build_for_structure_node(process,node);
        break;
    case NODE_TYPE_UNION:
        symresolver_build_for_union_node(process,node);
        break;
    // default:
    //     compile_error(process,"error in symresolver\n");
    }
}