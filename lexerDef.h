#ifndef LEXERDEF_H
#define LEXERDEF_H

#include <stdbool.h>
#include <stdio.h>


#define BUFFER_SIZE 4096
#define N_TC 54
#define TC 59
#define KC 29
#define HASH_TABLE_SIZE (1<<20)

typedef enum {
    TK_GT, TK_LT, TK_ASSIGNOP, TK_COMMENT, TK_EQ, TK_PLUS, TK_NE, TK_LE, TK_GE,
    TK_SQR, TK_SQL, TK_OR, TK_NOT, TK_AND, TK_ID, TK_FIELDID, TK_DIV, TK_MUL,
    TK_MINUS, TK_FUNID, TK_DOT, TK_CL, TK_OP, TK_COLON, TK_SEM, TK_RUID,
    TK_COMMA, TK_NUM, TK_RNUM, TK_MAIN, TK_AS, TK_CALL, TK_DEFINETYPE,
    TK_ELSE, TK_END, TK_ENDIF, TK_ENDWHILE, TK_ENDRECORD, TK_ENDUNION,
    TK_GLOBAL, TK_IF, TK_INPUT, TK_OUTPUT, TK_INT, TK_REAL, TK_LIST,
    TK_PARAMETERS, TK_PARAMETER, TK_READ, TK_WRITE, TK_RECORD, TK_UNION,
    TK_RETURN, TK_THEN, TK_TYPE, TK_WHILE, TK_WITH, TK_AT, END_OF_INPUT, TK_ERROR,TK_EOF,TK_EPS,
} terminals;

static const char* tokenStrings[] = {
    "TK_GT", "TK_LT", "TK_ASSIGNOP", "TK_COMMENT", "TK_EQ", "TK_PLUS", 
    "TK_NE", "TK_LE", "TK_GE", "TK_SQR", "TK_SQL", "TK_OR", "TK_NOT", 
    "TK_AND", "TK_ID", "TK_FIELDID", "TK_DIV", "TK_MUL", "TK_MINUS", 
    "TK_FUNID", "TK_DOT", "TK_CL", "TK_OP", "TK_COLON", "TK_SEM", "TK_RUID", 
    "TK_COMMA", "TK_NUM", "TK_RNUM", "TK_MAIN", "TK_AS", "TK_CALL", 
    "TK_DEFINETYPE", "TK_ELSE", "TK_END", "TK_ENDIF", "TK_ENDWHILE", 
    "TK_ENDRECORD", "TK_ENDUNION", "TK_GLOBAL", "TK_IF", "TK_INPUT", 
    "TK_OUTPUT", "TK_INT", "TK_REAL", "TK_LIST", "TK_PARAMETERS", 
    "TK_PARAMETER", "TK_READ", "TK_WRITE", "TK_RECORD", "TK_UNION", 
    "TK_RETURN", "TK_THEN", "TK_TYPE", "TK_WHILE", "TK_WITH", "TK_AT", 
    "EPS", "END_OF_INPUT", "TK_ERROR"  
};

static const char* nonTerminals[] = {
    "program", "mainFunction", "otherFunctions", "function", "input_par", 
    "output_par", "opt_input_par", "iterativeStmt", "parameter_list", 
    "dataType", "primitiveDatatype", "constructedDatatype", "A", "stmts", 
    "remaining_list", "typeDefinitions", "operator", "actualOrRedefined", 
    "typeDefinition", "fieldDefinitions", "fieldDefinition", "fieldType", 
    "moreFields", "declarations", "declaration", "global_or_not", "otherStmts", 
    "stmt", "assignmentStmt", "singleOrRecId", "option_single_constructed", 
    "oneExpansion", "moreExpansions", "funCallStmt", "outputParameters", 
    "inputParameters", "conditionalStmt", "elsePart", "ioStmt", 
    "arithmeticExpression", "expPrime", "term", "termPrime", "factor", 
    "highPrecedenceOperators", "lowPrecedenceOperators", "booleanExpression", 
    "var", "logicalOp", "relationalOp", "returnStmt", "optionalReturn", 
    "idList", "more_ids", "definetypestmt"
};



typedef struct TwinBuffer {
    char buffer1[BUFFER_SIZE];
    char buffer2[BUFFER_SIZE];
} TwinBuffer;

// Structure for field 
typedef struct Field {
    char *fieldname;
    char *fieldtype;
    struct Field *next;
} Field;


typedef struct SymbolTableitem {
    char *lexeme;
    
    int token;  
    char *type;
    union { 
        int intVal;
        float floatVal;
    } value;
    int lineCount;
    int scopeLevel;
    int startPos;
    int endPos;
    char **parameter;
    char *returnType;
    Field *fields;
    struct SymbolTableitem *next;
} SymbolItem;




typedef struct {
    SymbolItem *data[HASH_TABLE_SIZE];
    int currentsize;
} Symboltable;

typedef struct tokenInfo {
    terminals token;      
    char lexeme[256];     
    int lineNumber;       
} tokenInfo;


typedef struct {
    const char *keyword;
    int token;
} keyword;



extern int lineCount;
extern char *forward;
extern char *lexemebegin;
extern int activeBuffer;     
extern FILE *srcFile;
extern bool exhaustedInput;
extern bool ldfirstBuff;
extern bool ldsecondBuff;
extern TwinBuffer twinBuffer;
extern Symboltable *table;
extern keyword* kwEntries[KC];

void initializeKeywords();

#endif
