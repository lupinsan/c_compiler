#include "compiler.h"



struct expressionable_op_precedence_group op_precedence[TOTAL_OPERATOR_GROUPS]={
    {.operators={"++", "--", "()", "[]", "(", "[", ".", "->" , NULL},.associtivity=ASSOSIATIVITY_LEFT_TO_RIGHT},
    {.operators={"*", "/", "%" , NULL},.associtivity=ASSOSIATIVITY_LEFT_TO_RIGHT},
    {.operators={"+", "-", NULL},.associtivity=ASSOSIATIVITY_LEFT_TO_RIGHT},
    {.operators={">>", "<<" , NULL},.associtivity=ASSOSIATIVITY_LEFT_TO_RIGHT},
    {.operators={"<", "<=", ">", ">=", NULL},.associtivity=ASSOSIATIVITY_LEFT_TO_RIGHT},
    {.operators={"==", "!=",  NULL},.associtivity=ASSOSIATIVITY_LEFT_TO_RIGHT},
    {.operators={"&", NULL},.associtivity=ASSOSIATIVITY_LEFT_TO_RIGHT},
    {.operators={"^", NULL},.associtivity=ASSOSIATIVITY_LEFT_TO_RIGHT},
    {.operators={"|", NULL},.associtivity=ASSOSIATIVITY_LEFT_TO_RIGHT},
    {.operators={"&&", NULL},.associtivity=ASSOSIATIVITY_LEFT_TO_RIGHT},
    {.operators={"||", NULL},.associtivity=ASSOSIATIVITY_LEFT_TO_RIGHT},
    {.operators={"?", ":", NULL},.associtivity=ASSOSIATIVITY_RIGHT_TO_LEFT},
    {.operators={"=", "+=", "-=", "*=", "/=", "%=", "<<=", ">>=" , "&=" ,  "^=", "|=" , NULL},.associtivity=ASSOSIATIVITY_RIGHT_TO_LEFT},
    {.operators={",", NULL},.associtivity=ASSOSIATIVITY_LEFT_TO_RIGHT}

};