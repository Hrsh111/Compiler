#ifndef STACK_H
#define STACK_H

#include "parserDef.h"


typedef struct StackNode {
    char *symbol;             
    ParseTreeNode *node;      
    struct StackNode *next;   
} StackNode;


typedef struct Stack {
    StackNode *top;            
} Stack;


Stack *createStack();
void push(Stack *stack, StackNode *node);
StackNode *pop(Stack *stack);
StackNode *peek(Stack *stack);
int isStackEmpty(Stack *stack);
void freeStack(Stack *stack);

StackNode *createStackNode(char *symbol, ParseTreeNode *node);

#endif
