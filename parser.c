#include "compiler.h"
#include "helpers/vector.h"
#include <assert.h>
static struct compile_process* current_process; 
static struct token* parser_last_token;
extern struct expressionable_op_precedence_group op_precedence[TOTAL_OPERATOR_GROUPS];

extern struct node* parser_current_body;
extern struct node* parser_current_function;



enum
{
    PARSER_SCOPE_ENTITY_ON_STACK = 0b00000001,
    PARSER_SCOPE_ENTITY_STRUCTURE_SCOPE = 0b00000010
};

struct parser_scope_entity
{   
    int flags;

    int stack_offest;

    struct node* node;//变量declaration
};

struct parser_scope_entity* parser_new_scope_entity(struct node* node, int stack_offset,int flags)
{
    struct parser_scope_entity* entity = calloc(1,sizeof(struct parser_scope_entity));
    entity->node = node;
    entity->flags = flags;
    entity->stack_offest = stack_offset;
    return entity;
}

struct parser_scope_entity* parser_scope_last_entity_stop_global_scope()
{
    return scope_last_entity_stop_at(current_process, current_process->scope.root);
}

struct history
{
    int flags;
};

enum
{
    HISTORY_FLAG_INSIDE_UNION = 0b00000001,
    HISTORY_FLAG_IS_UPWARD_STACK = 0b00000010,
    HISTORY_FLAG_IS_GLOBAL_SCOPE = 0b00000100,
    HISTORY_FLAG_INSIDE_STRUCTURE = 0b00001000,
    HISTORY_FLAG_INSIDE_FUNCTION_BODY = 0b00010000
};




int parse_expressionable_single(struct history* history);




void parse_expressionable(struct history* history);

struct history* history_begin(int flags)
{
    struct history* history = calloc(1,sizeof(struct history));
    history->flags=flags;
    return history;
}


struct history* history_down(struct history* history, int flags)
{
    struct history* new_history = calloc(1,sizeof(struct history));
    memcpy(new_history,history,sizeof(struct history));
    new_history->flags=flags;
    return new_history;
}

void parser_scope_new(){
    scope_new(current_process, 0);
}

void parser_scope_finish(){
    scope_finish(current_process);
}

struct parser_scope_entity* parser_scope_last_entity()
{
    return scope_last_entity(current_process);
}

void parser_scope_push(struct parser_scope_entity* entity, size_t size)
{
    scope_push(current_process, entity, size);
}



static void parser_ignore_nl_or_comment(struct token* token)
{
    while(token && token_is_nl_or_comment_or_newline_seperator(token))
    {
        vector_peek(current_process->token_vec);
        token=vector_peek_no_increment(current_process->token_vec);
    }
}




struct token* token_next()
{
    struct token* next_token = vector_peek_no_increment(current_process->token_vec);
    parser_ignore_nl_or_comment(next_token);
    if(next_token)
        current_process->pos=next_token->pos;//若为null时？
    parser_last_token=next_token;
    return vector_peek(current_process->token_vec);

}


struct token* token_peek_next()
{
    struct token* next_token=vector_peek_no_increment(current_process->token_vec);
    parser_ignore_nl_or_comment(next_token);
    return vector_peek_no_increment(current_process->token_vec);
}


static bool token_next_is_operator(char * op)
{
    struct token* token = token_peek_next();
    return token_is_operator(token,op);
}

static bool token_next_is_symbol(char c){
    struct token* token = token_peek_next();
    return token_is_symbol(token,c);
}


static void expect_op(const char* op){
    struct token* next_token = token_next();
    if(!next_token||next_token->type!= TOKEN_TYPE_OPERATOR|| !S_EQ(next_token->sval,op)){
    	compile_error(current_process,"expecting op %s but not",op);    
    }
}

static void expect_sym(const char sym){
    struct token* next_token = token_next();
    if(!next_token||next_token->type!= TOKEN_TYPE_SYMBOL|| sym!=next_token->cval){
    	compile_error(current_process, "expecting sym %c but not",sym);    
    }
	    
}


void parse_single_token_to_node()
{
    struct token* token= token_next();
    struct node* node = NULL;
    switch(token->type)
    {
        case TOKEN_TYPE_NUMBER:
            node=node_create(&(struct node){.type=NODE_TYPE_NUMBER,.llnum=token->llnum});
            break;
        case TOKEN_TYPE_IDENTIFIER:
            node=node_create(&(struct node){.type=NODE_TYPE_IDENIFIER,.sval=token->sval});
            break;
        
        case TOKEN_TYPE_STRING:
            node=node_create(&(struct node){.type=NODE_TYPE_STRING,.sval=token->sval});
            break;

        default:
            compile_error(current_process,"this is not a single token that can be converted to a node");
    }

}

void parse_expressionble_for_op(struct history* history,const char* op)
{
    parse_expressionable(history);


}

static int parser_get_precedence_for_operator(const char* op, struct expressionable_op_precedence_group** group_out )
{
    *group_out = NULL;
    int i,j;
    for(i=0;i<TOTAL_OPERATOR_GROUPS;++i)
    {
        for(j=0;op_precedence[i].operators[j];++j)
        {
            const char* _op = op_precedence[i].operators[j];
            if(S_EQ(op,_op))
            {
                *group_out = &op_precedence[i];
                return i;
            }
        }
    }

    return -1;
}

static bool parser_left_op_has_priority(const char* op_left, const char* op_right)
{
    struct expressionable_op_precedence_group * group_left = NULL;
    struct expressionable_op_precedence_group * group_right = NULL;
    if(S_EQ(op_left,op_right))
    {
        return false;//???
    }
    
    int precedence_left = parser_get_precedence_for_operator(op_left,&group_left);
    int precedence_right = parser_get_precedence_for_operator(op_right,&group_right);

    if(group_left->associtivity==ASSOSIATIVITY_RIGHT_TO_LEFT)
    {
        return false;
    }

    return precedence_left<=precedence_left;

}



void parser_node_shift_children_left(struct node* node)
{
    //检验assert
    assert(node->type==NODE_TYPE_EXPRESSION);
    assert(node->exp.right->type==NODE_TYPE_EXPRESSION);
    
    
    //
    const char* right_op=node->exp.right->exp.op;
    struct node* new_exp_left_node = node->exp.left;
    struct node* new_exp_right_node = node->exp.right->exp.left;
    make_exp_node(new_exp_left_node, new_exp_right_node , node->exp.op);

    struct node* new_left_operand = node_pop();

    struct node* new_right_operand = node->exp.right->exp.right;
    
    node->exp.left = new_left_operand;
    node->exp.right = new_right_operand;
    node->exp.op = right_op;



}





void parser_reorder_expression(struct node** node_out)//分类讨论？
{
    struct node*node=* node_out;

    if(node->type!=NODE_TYPE_EXPRESSION)
    {
        return;
    }

    if(node->exp.left->type!=NODE_TYPE_EXPRESSION 
        && node->exp.right && node->exp.right->type!=NODE_TYPE_EXPRESSION)
    {
        return;
    }

    if(node->exp.left->type!=NODE_TYPE_EXPRESSION 
        &&node->exp.right && node->exp.right->type == NODE_TYPE_EXPRESSION)
    {
        const char* right_op = node->exp.right->exp.op;
        if(parser_left_op_has_priority(node->exp.op, right_op))
        {
            parser_node_shift_children_left(node);

            parser_reorder_expression(&node->exp.left);
            parser_reorder_expression(&node->exp.right);//迭代(有必要吗)
        }
    }



}





void parse_exp_normal(struct history* history)
{
    struct token* token=token_peek_next();
    char* op=token->sval;

    struct node* node_left =node_peek_expressionable_or_null();

    //pop off the token operator
    token_next();


    node_pop();

    node_left->flags|=NODE_FLAG_INSIDE_EXPRESSION;

    parse_expressionble_for_op(history_down(history,history->flags),op);

    struct node* node_right=node_pop();
    node_right->flags|=NODE_FLAG_INSIDE_EXPRESSION;

    make_exp_node(node_left,node_right,op);
    struct node* exp_node=node_pop();

    //to do reorder

    parser_reorder_expression(&exp_node);


    node_push(exp_node);
}



int parse_exp(struct history* history)
{
    parse_exp_normal(history);
    return 0;

}

void parse_identifier(struct history * history)
{
    struct token* token = token_next();
    assert(token->type == TOKEN_TYPE_IDENTIFIER);
    parse_single_token_to_node();

}

static bool is_keyword_variable_modifier(const char * val)
{
    return S_EQ(val,"unsigned") ||
            S_EQ(val,"signed") ||
            S_EQ(val,"static") ||
            S_EQ(val,"const") ||
            S_EQ(val,"extern") ||
            S_EQ(val,"__ignore_typecheck__");
}
void parse_datatype_modifiers(struct datatype* dtype)
{
    struct token* token = token_peek_next();
    while(token && token->type == TOKEN_TYPE_KEYWORD)
    {
        if(!is_keyword_variable_modifier(token->sval))
        {
            break;
        }
        else if(S_EQ(token->sval,"signed"))
        {
            dtype->flags |= DATATYPE_FLAG_IS_SIGNED;
        }
        else if(S_EQ(token->sval,"unsigned"))
        {
            dtype->flags &= ~DATATYPE_FLAG_IS_SIGNED;
            
        }
        else if(S_EQ(token->sval,"static"))
        {
            dtype->flags |= DATATYPE_FLAG_IS_STATIC;

        }
        else if(S_EQ(token->sval,"const"))
        {
            dtype->flags |= DATATYPE_FLAG_IS_CONST;

        }
        else if(S_EQ(token->sval,"extern"))
        {
            dtype->flags |= DATATYPE_FLAG_IS_EXTERN;
        }
        else if(S_EQ(token->sval,"__ignore_typecheck__"))
        {
            dtype->flags |= DATATYPE_FLAG_IS_IGNORE_TYPE_CHECKING;
        }

        token_next();
        token=token_peek_next();

    }

}


void parser_get_datatype_tokens(struct token** datatype_token, struct token** datatype_secondary_token )
{
    *datatype_token=token_next();
    struct token* next_token = token_peek_next();
    if(token_is_primitive_keyword(next_token))
    {
        *datatype_secondary_token = next_token;  
        token_next();
    }
}

int parser_datatype_expected_for_type_string(const char* str)
{
    if(S_EQ(str,"union"))
    {
        return DATA_TYPE_EXPECT_UNION;
    }
    else if(S_EQ(str,"struct"))
    {
        return DATA_TYPE_EXPECT_STRUCT;
    }
    
    return DATA_TYPE_EXPRCT_PRIMITIVE;
}

int parser_get_random_index()
{
    static int x = 0;
    x++;
    return x;
}



struct token* parser_build_random_type_name()
{
    char tmp_name[25];
    printf(tmp_name,"customtypename_%i",parser_get_random_index());
    char* sval = malloc(sizeof(tmp_name));
    strncpy(sval,tmp_name,sizeof(tmp_name));
    struct token* token = malloc(sizeof(struct token));
    token->type = TOKEN_TYPE_IDENTIFIER;
    token->sval = sval;
    return token;
}

int parser_get_pointer_depth()
{
    int depth = 0;
    struct token* token=token_peek_next();
    while (token_next_is_operator("*"))
    {
        depth++;
        token_next();
    }
    return depth;
}


//break down questions
bool parser_datatype_is_secondery_allowed(int expected_type)
{
    return expected_type == DATA_TYPE_EXPRCT_PRIMITIVE;
}

bool parser_datatype_is_secondary_allowed_for_type(const char* type)
{
    return S_EQ(type,"long") || S_EQ(type,"short") || S_EQ(type,"double") || S_EQ(type,"float");
}

void parser_datatype_init_type_and_size_for_primitive(struct token* datatype_token, struct token* datatype_secondary_token ,struct datatype* datatype_out);

void parser_datatype_adjust_for_sencondary(struct datatype* datatype, struct token* datatype_secondary_token)
{
    if(!datatype_secondary_token)
    {
        return;
    }
    struct datatype secondary_datatype ;
    parser_datatype_init_type_and_size_for_primitive(datatype_secondary_token,NULL,&secondary_datatype);
    datatype->size+= secondary_datatype.size;
    datatype->secondary = datatype_secondary_token;
    datatype->flags |= DATATYPE_FLAG_IS_SECONDARY;

}


void parser_datatype_init_type_and_size_for_primitive(struct token* datatype_token, struct token* datatype_secondary_token ,struct datatype* datatype_out)
{
    if(!parser_datatype_is_secondary_allowed_for_type(datatype_token-> sval) && datatype_secondary_token)//终止递归
    {
        compile_error(current_process,"you provied an invalid secondary datatype for the given type\n");
    }

    if(S_EQ(datatype_token->sval,"void"))
    {
        datatype_out->type = DATA_TYPE_VOID;
        datatype_out->size = DATA_SIZE_ZERO;

    }
    else if(S_EQ(datatype_token->sval,"char"))
    {
         datatype_out->type = DATA_TYPE_CHAR;
         datatype_out->size = DATA_SIZE_BYTE;
    }
    else if(S_EQ(datatype_token->sval,"short"))
    {
         datatype_out->type = DATA_TYPE_SHORT;
         datatype_out->size = DATA_SIZE_WORD;
    }
    else if(S_EQ(datatype_token->sval,"int"))
    {
         datatype_out->type = DATA_TYPE_INTEGER;
         datatype_out->size = DATA_SIZE_DWORD;
    }
    else if(S_EQ(datatype_token->sval,"long"))
    {
         datatype_out->type = DATA_TYPE_LONG;
         datatype_out->size = DATA_SIZE_DWORD;
    }
    else if(S_EQ(datatype_token->sval,"float"))
    {
         datatype_out->type = DATA_TYPE_FLOAT;
         datatype_out->size = DATA_SIZE_DWORD;
    }
    else if(S_EQ(datatype_token->sval,"double"))
    {
         datatype_out->type = DATA_TYPE_CHAR;
         datatype_out->size = DATA_SIZE_DWORD;
    }
    else
    {
        compile_error(current_process,"BUG: Invalid primitive variable\n");
    }


    parser_datatype_adjust_for_sencondary(datatype_out, datatype_secondary_token);
}

size_t size_of_struct(const char* struct_name)
{
    struct symbol* sym = symresolver_get_symbol(struct_name);
    if(!sym)
    {
        return 0;
    }

    assert(sym->type == SYMBOL_TYPE_NODE);
    struct node* node =sym->data;
    assert(node->type==NODE_TYPE_STRUCT);
    return node->_struct.body_n->body.size;
}







void parser_datatype_init_type_and_size(struct token* datatype_token, struct token* datatype_secondary_token ,struct datatype* datatype_out, int pointer_depth, int expected_type)
{
    if(!parser_datatype_is_secondery_allowed(expected_type) && datatype_secondary_token)
    {
        compile_error(current_process,"you provied an invalid secondary datatype\n");
    }

    switch(expected_type)
    {
        case DATA_TYPE_EXPRCT_PRIMITIVE:
            parser_datatype_init_type_and_size_for_primitive(datatype_token,datatype_secondary_token,datatype_out);
            
            break;
        case DATA_TYPE_EXPECT_STRUCT:
            datatype_out->type = DATA_TYPE_STRUCT;
            datatype_out->size = size_of_struct(datatype_token->sval);
            datatype_out->struct_node = struct_node_from_for_name(current_process, datatype_token->sval);
            break;
        case DATA_TYPE_EXPECT_UNION:
            compile_error(current_process," union xxxxx\n");
            break;
        default:
            compile_error(current_process,"BUG: Unsupported datatype expectation\n");
    }
}
void parser_datatype_init(struct token* datatype_token, struct token* datatype_secondary_token, struct datatype* datatype_out, int pointer_depth, int expected_type)
{
    parser_datatype_init_type_and_size( datatype_token,  datatype_secondary_token , datatype_out,  pointer_depth,  expected_type);
    
    datatype_out->type_str = datatype_token ->sval;//??
    if(S_EQ(datatype_token->sval,"long") && datatype_secondary_token &&S_EQ(datatype_secondary_token->sval,"long"))
    {
        compile_warning(current_process,"no 64x long long\n");
        datatype_out ->size = DATA_SIZE_DWORD;
    }
}


void parse_datatype_type(struct datatype *dtype)
{
    struct token* datatype_token = NULL;
    struct token* datatype_secondary_token = NULL;
    parser_get_datatype_tokens(&datatype_token, &datatype_secondary_token);
    int expected_type = parser_datatype_expected_for_type_string(datatype_token->sval);
    if(datatype_is_struct_or_union_for_name(datatype_token->sval))
    {
        if(token_peek_next()->type == TOKEN_TYPE_IDENTIFIER)
        {
            datatype_token = token_next();
        }
	 else
    	{
        datatype_token = parser_build_random_type_name();
        datatype_token->flags |= DATATYPE_FLAG_STRUCT_UNION_NO_NAME;
    	}
    }
    

    int pointer_depth = parser_get_pointer_depth();

    parser_datatype_init( datatype_token,  datatype_secondary_token , dtype,  pointer_depth,  expected_type);//如果struct等类型在符号表中，则直接导出size以及_struct_node

}









void parse_datatype( struct datatype* dtype)
{
    memset(dtype,0,sizeof(struct datatype));
    dtype->flags!=DATATYPE_FLAG_IS_SIGNED ;
    parse_datatype_modifiers(dtype);
    parse_datatype_type(dtype);
    parse_datatype_modifiers(dtype);

}
bool parser_is_int_valid_after_datatype(struct datatype* dtype){
    return dtype->type == DATA_TYPE_LONG || dtype->type == DATA_TYPE_FLOAT || dtype == DATA_TYPE_DOUBLE;
}



void parser_ignore_int(struct datatype* dtype){
    if(!token_is_keyword(token_peek_next(),"int")){
        return;
    }

    if(!parser_is_int_valid_after_datatype(dtype)){
        compile_error(current_process, "you provide a secondary int type is not supported\n");
    }

    token_next();
}

void parse_expressionable_root(struct history* history){//分析等号后的表达式
    parse_expressionable(history);
    struct node* result_node = node_pop();
    node_push(result_node);
}
void make_variable_node(struct datatype* dtype, struct token* name_token, struct node* value_node){
    const char* name_str = NULL;
    if(name_token){
        name_str = name_token->sval;
    }

    node_create(&(struct node){.type = NODE_TYPE_VARIABLE, .var.name = name_str,.var.type = *dtype,.var.val = value_node});

}

void parser_scope_offset_for_stack(struct node* node, struct history* history)//新的栈点位--实体可能是变量
{
    struct parser_scope_entity* last_entity = parser_scope_last_entity_stop_global_scope();
    bool upward_stack = history->flags & HISTORY_FLAG_IS_UPWARD_STACK;//参数向上，变量向下
    int offset = -variable_size(node);
    if(upward_stack)
    {
        compile_error(current_process, "NOT yet implement\n");
    }

    if(last_entity)
    {
        offset += variable_node(last_entity->node)->var.aoffset;
        if(variable_node_is_primitive(node))
        {
            variable_node(node)->var.padding = padding(upward_stack ? offset : -offset, node->var.type.size);
        }
    }

}

void parser_scope_offset_for_global(struct node* node ,struct history* history)
{

}

void parser_scope_offset_for_structure(struct node* node , struct history* history)
{
    int offset = 0;
    struct parser_scope_entity* last_entity = parser_scope_last_entity();
    if(last_entity)
    {
        offset += last_entity->stack_offest +last_entity->node->var.type.size;
        if(variable_node_is_primitive(node))
        {
            node->var.padding = padding(offset, node-> var.type.size);
        }

        node->var.aoffset = offset + node->var.padding;
    }



}

void parser_scope_offset(struct node* node, struct history* history)
{
    if(history->flags & HISTORY_FLAG_IS_GLOBAL_SCOPE)
    {
        parser_scope_offset_for_global(node, history);
        return;
    }

    if(history->flags & HISTORY_FLAG_INSIDE_STRUCTURE)
    {
        parser_scope_offset_for_structure(node, history);
        return;
    }


    parser_scope_offset_for_stack(node, history);
}


void make_variable_node_and_register(struct history* history, struct datatype* dtype, struct token* name_token,struct node* value_node){

    make_variable_node(dtype, name_token, value_node);
    struct node* var_node = node_pop();
    //remember cal scope offset

    parser_scope_offset(var_node, history);

    struct parser_scope_entity* tmp = parser_new_scope_entity(var_node, var_node->var.aoffset,0);
    parser_scope_push(tmp,var_node->var.type.size);


    node_push(var_node);
}

struct array_brackets* parse_array_brackets(struct history* history){
    struct array_brackets* brackets = array_brackets_new();
    while(token_next_is_operator("[")){
    	expect_op("[");
        if(token_is_symbol(token_peek_next(),']')){
            expect_sym(']');//向前做一个nexttoken动作
            break;    
        }
        
        parse_expressionable_root(history);
        expect_sym(']');
        
        struct node* exp_node = node_pop();
        make_bracket_node(exp_node);
        
        struct node* bracket_node = node_pop();
        array_brackets_add(brackets, bracket_node);
    }
    
    return brackets;
}



void parse_variable(struct datatype* dtype, struct token* name_token, struct history* history){
    
    struct node* value_node = NULL;
    //数组b[30]情况
    //to do
    //
    struct array_brackets* brackets;
    if(token_next_is_operator("[")){
    	brackets = parse_array_brackets(history);
        dtype->array.brackets = brackets;
        dtype->array.size = array_brackets_calculate_size(dtype, brackets);
        dtype->flags |= DATATYPE_FLAG_IS_ARRAY;
    }
    
    
    

    if(token_next_is_operator("=")){
        token_next();
        parse_expressionable_root(history);
        value_node = node_pop();
    }

    make_variable_node_and_register(history,dtype,name_token, value_node);
}

void parse_body(size_t* variable_size, struct history* history);

void parse_function_body(struct history* history)
{
    parse_body(NULL,history_down(history,history->flags | HISTORY_FLAG_INSIDE_FUNCTION_BODY));
}


void parse_function(struct datatype* ret_type, struct token* name_token, struct history* history)
{
    struct vector* arguments_vector = NULL;
    parser_scope_new();
    make_function_node(ret_type, name_token->sval, NULL,NULL);
    struct node* function_node = node_peek();
    parser_current_function = function_node;

    if(datatype_is_struct_or_union(ret_type))//main函数提供struct指针，func直接在指针上构造或者修改
    {
        function_node->func.args.stack_addition+=DATA_SIZE_DWORD;
    }
    
    expect_op("(");
    #warning "need to parse args"

    expect_sym(')');

    function_node->func.args.vector = arguments_vector;
    if(symresolver_get_symbol_for_native_function(current_process,name_token->sval))
    {
        function_node->func.flags|= FUNCTION_NODE_IS_NATIVE;
    }
    if(token_next_is_symbol('('))
    {
        parse_function_body(history_begin(0));
        struct node* body_node = node_pop;
        function_node->func.body_n = body_node;
    }
    else
    {
        expect_sym(';');
    }


    parser_current_function = NULL;//c中不可嵌套，若为c++需要使用此变量 
    parser_scope_finish();



}


void parse_symbol()
{
    if(token_next_is_symbol('{'))
    {
        size_t variable_size = 0;
        struct history* history = history_begin(HISTORY_FLAG_IS_GLOBAL_SCOPE);
        parse_body(&variable_size, history);

        struct node* body_node = node_pop();

        node_push(body_node);
    }
}


void parse_statement(struct history* history)
{
    if(token_peek_next()->type == TOKEN_TYPE_KEYWORD)
    {
        parse_keyword(history);
        return;
    }
    parse_expressionable_root(history);
    if(token_peek_next()->type == TOKEN_TYPE_SYMBOL && !token_is_symbol(token_peek_next(),';'))
    {
        parse_symbol();
        return;
    }

    //所有语句结束于分号
    expect_sym(';');

}
void parser_append_size_for_node(struct history* history, size_t* _variable_size, struct node* node)
{
     *_variable_size += variable_size(node);//int x ; 返回x中var.type.size
     if (node->var.type.flags & DATATYPE_FLAG_IS_POINTER)
     {
        return;
     }
    

    struct node* largest_var_node = variable_struct_or_union_body_node(node)? variable_struct_or_union_body_node(node)->body.largest_var_node: NULL;

    if(largest_var_node)//仅仅对struct内的最大节点align
    {
        *_variable_size+=align_value(*_variable_size, largest_var_node->var.type.size);
    }

}   

void parser_append_size_for_variable_list(struct history* history, size_t* variable_size, struct vector* vec)
{
    vector_set_peek_pointer(vec,0);
    struct node* node = vector_peek_ptr(vec);
    while(node)
    {
        parser_append_size_for_node(history, variable_size, node);
        node = vector_peek_ptr(vec);
    }
}



void parser_append_size_for_node_struct_union(struct history* history, size_t* _variable_size, struct node* node)
{
    if(!node)
    {
        return;
    }
    if(node->type == NODE_TYPE_VARIABLE)
    {
        if(node_is_struct_or_union_variable(node))
        {
            parser_append_size_for_node_struct_union(history, _variable_size, node);
            return;
        }

        *_variable_size += variable_size(node);
    }
    else if(node->type == NODE_TYPE_VARIABLE_LIST)
    {
        parser_append_size_for_variable_list(history, _variable_size, node->var_list.list);
    }

}

void parser_finalize_body(struct history* history, struct node* body_node, struct vector* body_vec,size_t* _variable_size, struct node* largest_align_eligible_var_node, struct node* largest_possible_var_node)
{
    if(history->flags & HISTORY_FLAG_INSIDE_UNION)//对于union进行修改varisize
    {
        if(largest_possible_var_node)
        {
            *_variable_size = align_value(_variable_size, largest_align_eligible_var_node->var.type.size);
        }
    }

    int padding = compute_sum_padding(body_vec);
    *_variable_size+=padding;

    if(largest_align_eligible_var_node)//原始数据类型
    {
        *_variable_size = align_value(*_variable_size, largest_align_eligible_var_node->var.type.size);
    }

    bool padded = padding!=0;

    //将vec以及padding全加到bodynode中

    body_node->body.largest_var_node = largest_align_eligible_var_node;
    body_node->body.padded = padded;
    body_node->body.size = *_variable_size;
    body_node->body.statements = body_vec;
}



void parse_body_single_statement(size_t* variable_size, struct vector* body_vec, struct history* history)
{
    make_body_node(NULL,0,false,NULL);
    struct node* body_node = node_pop();

    body_node->binded.owner = parser_current_body;
    parser_current_body = body_node;

    struct node* stmt_node = NULL;
    parse_statement(history_down(history,history->flags));//
    stmt_node = node_pop();
    vector_push(body_vec, &stmt_node);


    parser_append_size_for_node(history, variable_size, stmt_node);

    struct node* largest_var_node = NULL;
    if(stmt_node->type ==NODE_TYPE_VARIABLE){
        largest_var_node = stmt_node;
    }

    parser_finalize_body(history, body_node, body_vec, variable_size, largest_var_node, largest_var_node);

    parser_current_body = body_node->binded.owner;
    node_push(body_node);
}

void parser_body_mutiple_statements(size_t* variable_size, struct vector* body_vec , struct history* history)
{
    make_body_node(NULL, 0, false, NULL);
    struct node* body_node = node_pop();
    body_node->binded.owner = parser_current_body;
    parser_current_body = body_node;

    struct node* stmt_node = NULL;
    struct node* largest_possiable_var_node = NULL;
    struct node* largest_align_eligible_var_node = NULL;

    expect_sym('{');

    while(!token_next_is_symbol('}'))
    {
        parse_statement(history_down(history, history->flags));
        stmt_node = node_pop();
        if(stmt_node->type == NODE_TYPE_VARIABLE)
        {
            if(!largest_possiable_var_node ||
                largest_possiable_var_node->var.type.size <= stmt_node ->var.type.size)
            {
                largest_possiable_var_node = stmt_node;
            }
            if(variable_node_is_primitive(stmt_node))
            {
                if(!largest_align_eligible_var_node ||
                    largest_align_eligible_var_node->var.type.size <= stmt_node ->var.type.size)
                {
                    largest_align_eligible_var_node = stmt_node;
                }
            }
        }
        //推进bodyvec中
        vector_push(body_vec, &stmt_node);

        // change variable size
        parser_append_size_for_node(history, variable_size, variable_node_or_list(stmt_node));//对于struct等先进行对于最大body节点的align

        parser_finalize_body(history, body_node, body_vec, variable_size, largest_align_eligible_var_node,largest_possiable_var_node);

    }

    expect_sym('}');

    parser_finalize_body(history, body_node, body_vec, variable_size, largest_align_eligible_var_node,largest_possiable_var_node);


    parser_current_body = body_node->binded.owner;
    node_push(body_node);
}


void parse_body(size_t* variable_size, struct history* history)
{
    parser_scope_new();
    size_t tmp_size = 0x00;
    if(!variable_size){
        variable_size = &tmp_size;
    }

    struct vector* body_vec = vector_create(sizeof(struct node*));
    if(!token_next_is_symbol('{'))
    {
        parse_body_single_statement(variable_size, body_vec, history);
        parser_scope_finish();
        return;
    }

    //some sta between {}
    parser_body_mutiple_statements(variable_size, body_vec, history);
    parser_scope_finish();

    #warning "need to adjust the function stack size"

}


void parse_struct_no_new_scope(struct datatype* dtype, bool is_forward_declaration)
{
    struct node* body_node = NULL;
    size_t body_variable_size = 0;

    if(!is_forward_declaration)
    {
        parse_body(&body_variable_size, history_begin(HISTORY_FLAG_INSIDE_STRUCTURE));
        body_node = node_pop();
    }

    make_struct_node(dtype->type_str, body_node);
    struct node* struct_node = node_pop();
    if(body_node)
    {
        dtype->size = body_node->body.size;
    }
    dtype->struct_node = struct_node;

    if(token_is_identifier(token_peek_next()))
    {
        struct token* var_name = token_next();
        struct_node->flags |= NODE_FLAG_HAS_VARIABLE_COMBINED;
        if(dtype->flags & DATATYPE_FLAG_STRUCT_UNION_NO_NAME)
        {
            dtype->type_str = var_name->sval;
            dtype->flags &= ~DATATYPE_FLAG_STRUCT_UNION_NO_NAME;
            struct_node->_struct.name = var_name->sval;
        }

        make_variable_node_and_register(history_begin(0), dtype, var_name, NULL);
    }

    expect_sym(';');
    node_push(struct_node);

}


void parse_struct(struct datatype* dtype)
{
    bool is_forward_declaration = !token_is_symbol(token_peek_next(),'{');
    if(!is_forward_declaration){
        parser_scope_new();
    }

    parse_struct_no_new_scope(dtype, is_forward_declaration);

    if(!is_forward_declaration){
        parser_scope_finish();
    }



    
}



void parse_struct_or_union(struct datatype* dtype){
    switch (dtype->type)
    {
    case DATA_TYPE_STRUCT:
        parse_struct(dtype);
        break;
    
    case DATA_TYPE_UNION:
        break;

    default:
        compile_error(current_process,"datatype is not a struct or union\n");
        break;
    }
}


void parse_variable_function_or_struct_union(struct history* history)//分析keyword总起，先分析datatype
{
	
    struct datatype dtype;
    parse_datatype(&dtype);//如果struct等类型在符号表中，则直接导出size以及_struct_node

    //至此已经分析完datatype
    //struct and union

    if(datatype_is_struct_or_union(&dtype) && token_next_is_symbol('{')){
        parse_struct_or_union(&dtype);
    //struct 已经分析完成包括分号以后的变量
    
        struct node* su_node = node_pop();
        symresolver_build_for_node(current_process,su_node);//传入数据为struct类型的node
        node_push(su_node);
        return;
    }




    //eg. long int
    parser_ignore_int(&dtype);

    // int abc
    struct token* name_token = token_next();
    if(name_token->type != TOKEN_TYPE_IDENTIFIER){
        compile_error(current_process, "Expecting a vaild name for the given var\n");
    }
    
    if(token_next_is_operator("("))
    {
        parse_function(&dtype,name_token,history);
        return;
    }

    


    parse_variable(&dtype, name_token, history);
    

}









void parse_keyword(struct history* history)
{
    struct token* token= token_peek_next();
    if(is_keyword_variable_modifier(token->sval) || keyword_is_datatype(token->sval))
    {
        parse_variable_function_or_struct_union(history);
        return;
    }
    


}




int parse_expressionable_single(struct history* history)
{
    struct token* token= token_peek_next();
    if(!token)
    {
        return -1;
    }

    history->flags!=NODE_FLAG_INSIDE_EXPRESSION;

    int res=-1;

    switch(token->type)
    {
        case TOKEN_TYPE_NUMBER:
            parse_single_token_to_node();
            res = 0;
            break;
        case TOKEN_TYPE_IDENTIFIER:
            parse_identifier(history);
            res = 0;
            break;
        
        case TOKEN_TYPE_OPERATOR:
            parse_exp(history);
            res=0;
            break;
        case TOKEN_TYPE_KEYWORD:
            parse_keyword(history);
            break;
    }
    return res;


}





void parse_expressionable(struct history* history)
{
    while(parse_expressionable_single(history)==0)
    {


    }



}


void  parse_keyword_for_global(){
    parse_keyword(history_begin(0));
    struct node* node = node_pop();

    node_push(node);

    return;
}





parse_next()
{
    struct token* token=token_peek_next();
    if(!token)
    {
        return -1;
    }

    int res=0;

    switch(token->type)
    {
        case TOKEN_TYPE_NUMBER:
        case TOKEN_TYPE_IDENTIFIER:
        case TOKEN_TYPE_STRING:
            //parse_single_token_to_node();
            parse_expressionable(history_begin(0));
            break;
        
        case TOKEN_TYPE_KEYWORD:
            parse_keyword_for_global();
            break;
        case TOKEN_TYPE_SYMBOL:
            parse_symbol();
            break;
        return 0;
    }



    return 0;
}





int parser(struct compile_process* process)
{
    scope_create_root(process);
    current_process=process;

    vector_set_peek_pointer(process->token_vec,0);

    node_set_vector(current_process->node_vec,current_process->node_tree_vec);

    struct node* node=NULL;
    
    while(parse_next()==0)//从token_vec 中提取出一个token载入node_vec中---------------没有关于token的函数（操作）
    {
        node=node_peek();
        vector_push(process->node_tree_vec,&node);//将node_vec中的node载入node_tree_vec中

    }

    return PARSE_ALL_OK;


}
