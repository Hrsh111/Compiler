#ifndef PARSER_H
#define PARSER_H

#include "parserDef.h"
#define NUM_TERMINALS (sizeof(tokenStrings)/sizeof(tokenStrings[0]))
void freeParseTree(ParseTreeNode *root);
int isTerminal(const char *symbol);

typedef enum {
    NT_OPERATOR,
    NT_PROGRAM,
    NT_MAINFUNCTION,
    NT_OTHERFUNCTIONS,
    NT_FUNCTION,
    NT_INPUT_PAR,
    NT_OUTPUT_PAR,
    NT_OPT_INPUT_PAR,
    NT_ITERATIVE_STMT,
    NT_PARAMETER_LIST,
    NT_DATA_TYPE,
    NT_PRIMITIVE_DATATYPE,
    NT_CONSTRUCTED_DATATYPE,
    NT_A,
    NT_STMTS,
    NT_REMAINING_LIST,
    NT_TYPE_DEFINITIONS,
    NT_ACTUAL_OR_REDEFINED,
    NT_TYPE_DEFINITION,
    NT_FIELD_DEFINITIONS,
    NT_FIELD_DEFINITION,
    NT_FIELD_TYPE,
    NT_MORE_FIELDS,
    NT_DECLARATIONS,
    NT_DECLARATION,
    NT_GLOBAL_OR_NOT,
    NT_OTHERSTMTS,
    NT_STMT,
    NT_ASSIGNMENT_STMT,
    NT_SINGLE_OR_REC_ID,
    NT_OPTION_SINGLE_CONSTRUCTED,
    NT_ONE_EXPANSION,
    NT_MORE_EXPANSIONS,
    NT_FUN_CALL_STMT,
    NT_OUTPUT_PARAMETERS,
    NT_INPUT_PARAMETERS,
    NT_CONDITIONAL_STMT,
    NT_ELSE_PART,
    NT_IO_STMT,
    NT_ARITHMETIC_EXPRESSION,
    NT_EXP_PRIME,
    NT_TERM,
    NT_TERM_PRIME,
    NT_FACTOR,
    NT_HIGH_PRECEDENCE_OPERATORS,
    NT_LOW_PRECEDENCE_OPERATORS,
    NT_BOOLEAN_EXPRESSION,
    NT_VAR,
    NT_LOGICAL_OP,
    NT_RELATIONAL_OP,
    NT_RETURN_STMT,
    NT_OPTIONAL_RETURN,
    NT_ID_LIST,
    NT_MORE_IDS,
    NT_DEFINETYPE_STMT,
    TOTAL_NON_TERMINALS 
} NonTerminal;



extern GrammarRule grammarRules[];
extern int numGrammarRules;

Grammar* loadGrammar(const char *filename);


FirstFollow* ComputeFirstAndFollowSets(Grammar *G, int numNonTerminals);


void createParseTable(FirstFollow *ffArr, int numNonTerminals, Grammar *G, int table[TOTAL_NON_TERMINALS][NUM_TERMINALS]);
ParseTreeNode* parseInputSourceCode(char *testcaseFile, int parseTable[TOTAL_NON_TERMINALS][NUM_TERMINALS], Grammar *G);

void printParseTree(ParseTreeNode *PT, char *outfile);
void printNodeInfo(ParseTreeNode *node, FILE *fp, int nodeCounter) ;


void printParseTree(ParseTree *PT, char *outfile);
// In parser.h, add these lines (for example, after the declaration of getNonTerminalIndex):
int getTerminalIndex(const char *terminal);
int isTerminal(const char *symbol);



int isNonTerminal(const char *symbol);


int getNonTerminalIndex(const char *nonTerminal);
int containsSymbol(char **arr, int count, const char *symbol);
void freeParseTree(ParseTreeNode *root);



#endif 
