#include "compiler.h"
#include <assert.h>
#include "helpers/vector.h"

struct vector* node_vector=NULL;
struct vector* node_vector_root=NULL;


void node_set_vector(struct vector* vec,struct vector* root_vec)
{
    node_vector=vec;
    node_vector_root=root_vec;
}

void node_push(struct node* node)
{
    vector_push(node_vector,&node);
}

struct node* node_peek_or_null()
{
    return vector_back_ptr_or_null(node_vector);
}

struct node* node_peek()
{
    return *(struct node**)(vector_back(node_vector));
}

struct node* node_pop()
{
    struct node* last_node = vector_back_ptr(node_vector);
    struct node* last_node_root = vector_empty(node_vector) ? NULL : vector_back_ptr_or_null(node_vector_root);

    vector_pop(node_vector);

    if(last_node==last_node_root)
    {
        vector_pop(node_vector_root);
    }
    
    return last_node;

}


struct node* node_create(struct node* _node)//注意命名！++复制构造函数 ++在类vec中构造node
{
    //warning have to  be binded
    struct node* node=malloc(sizeof(struct node));
    memcpy(node,_node,sizeof(struct node));
    node_push(node);
    return node;
}

bool node_is_expressionable(struct node* node)
{
    return node->type == NODE_TYPE_EXPRESSION || node->type == NODE_TYPE_EXPRESSION_PARENTHSES || node->type == NODE_TYPE_UNARY || node->type == NODE_TYPE_IDENIFIER || node->type == NODE_TYPE_NUMBER || node->type == NODE_TYPE_STRING ;                 
}



struct node* node_peek_expressionable_or_null()
{
     struct node* last_node = node_peek_or_null();
     return node_is_expressionable(last_node) ? last_node : NULL;

}

void make_bracket_node(struct node* node){
	node_create(&(struct node){.type = NODE_TYPE_BRACKET, .bracket.inner = node});
}


void make_exp_node(struct node* left_node,struct node* right_node,const char* op)
{
    assert(left_node);
    assert(right_node);
    struct node* tmp=node_create(&(struct node){.exp.left=left_node, .exp.right=right_node, .exp.op=op, .type=NODE_TYPE_EXPRESSION});

}

void make_body_node(struct vector* body_vec, size_t size, bool padded,struct node* largest_var_node)
{
    node_create(&(struct node){.type = NODE_TYPE_BODY,.body.statements = body_vec, .body.size = size, .body.padded = padded,.body.largest_var_node = largest_var_node});

}
