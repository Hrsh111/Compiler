#ifndef STACK_H
#define STACK_H

#include "parserDef.h"  // Ensure this header defines the ParseTreeNode structure

// Define a stack node structure.
typedef struct StackNode {
    char *symbol;              // The grammar symbol (terminal or non-terminal)
    ParseTreeNode *node;       // Pointer to the associated parse tree node
    struct StackNode *next;    // Pointer to the next stack node
} StackNode;

// Define a stack structure.
typedef struct Stack {
    StackNode *top;            // Top of the stack
} Stack;

// Function prototypes
Stack *createStack();
void push(Stack *stack, StackNode *node);
StackNode *pop(Stack *stack);
StackNode *peek(Stack *stack);
int isStackEmpty(Stack *stack);
void freeStack(Stack *stack);

StackNode *createStackNode(char *symbol, ParseTreeNode *node);

#endif  // STACK_H
