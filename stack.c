#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stack.h"

// Create a new stack.
Stack *createStack() {
    Stack *stack = (Stack *)malloc(sizeof(Stack));
    if (!stack) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    stack->top = NULL;
    return stack;
}

// Create a new stack node with the given symbol and associated parse tree node.
StackNode *createStackNode(char *symbol, ParseTreeNode *node) {
    StackNode *newNode = (StackNode *)malloc(sizeof(StackNode));
    if (!newNode) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    newNode->symbol = strdup(symbol);
    if (!newNode->symbol) {
        perror("strdup");
        exit(EXIT_FAILURE);
    }
    newNode->node = node;
    newNode->next = NULL;
    return newNode;
}

// Push a node onto the stack.
void push(Stack *stack, StackNode *node) {
    if (stack == NULL) {
        fprintf(stderr, "Error: push() received a NULL stack pointer.\n");
        exit(EXIT_FAILURE);
    }
    node->next = stack->top;
    stack->top = node;
}

// Pop a node from the stack and return it.
StackNode *pop(Stack *stack) {
    if (isStackEmpty(stack))
        return NULL;
    StackNode *temp = stack->top;
    stack->top = stack->top->next;
    return temp;
}

// Peek at the top node of the stack without removing it.
StackNode *peek(Stack *stack) {
    if (isStackEmpty(stack))
        return NULL;
    return stack->top;
}

// Check if the stack is empty.
int isStackEmpty(Stack *stack) {
    return (stack->top == NULL);
}

// Free all nodes in the stack and the stack itself.
void freeStack(Stack *stack) {
    while (!isStackEmpty(stack)) {
        StackNode *node = pop(stack);
        if (node) {
            free(node->symbol);
            free(node);
        }
    }
    free(stack);
}
