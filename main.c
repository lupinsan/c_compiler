#include <stdio.h>
#include "compiler.h"
#include "helpers/vector.h"


int main(){

    int res = compile_file("./test.c","./test",0);
    if(res==COMPILER_FILE_COMPILED_OK){
        printf("everything is ok!\n");
    }
    else if(res==COMPILER_FAILED_WITH_ERRORS){
        printf("compile failed!\n");
    }
    else{
        printf("unknow error!\n");
    }
    return 0;
}