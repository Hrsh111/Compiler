#ifndef PARSERDEF_H
#define PARSERDEF_H

#include <stdio.h>
#include "lexerDef.h"

#define LEXEME_SIZE 256


typedef struct {
    char **first;
    char **follow;
} FirstAndFollow;


typedef struct GrammarRule {
    char *lhs;      
    char **rhs;     
    int rhsSize;    
} GrammarRule;


typedef struct Grammar {
    GrammarRule *rules;
    int numRules;
    char *startSymbol;   
} Grammar;


typedef struct ParseTableEntry {
    char *lhs;     
    char **rhs;    
} ParseTableEntry;


typedef struct ParseTable {
    ParseTableEntry **entries; 
    int rows;                 
    int cols;                 
} ParseTable;


typedef struct ParseTreeNode {
    char *lexeme;           
    int lineno;             
    char *tokenName;        
    double value;           
    char *parentNodeSymbol; 
    int isLeafNode;         
    char *nodeSymbol;       
    struct ParseTreeNode *children; 
    int numChildren;        
} ParseTreeNode;


typedef struct ParseTree {
    ParseTreeNode *root;   
} ParseTree;
#define INIT_CAPACITY 8

typedef struct FirstFollow {
    char **first;      
    int firstCount;     
    int firstCapacity; 

    char **follow;      
    int followCount;   
    int followCapacity; 
} FirstFollow;


#endif 
