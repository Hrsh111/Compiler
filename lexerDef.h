#ifndef LEXERDEF_H
#define LEXERDEF_H

#include <stdbool.h>
#include <stdio.h>

// Macros
#define BUFFER_SIZE 4096
#define N_TC 54
#define TC 59
#define KC 28
#define HASH_TABLE_SIZE (1<<20)
// Token type enumeration
typedef enum {
    TK_GT, TK_LT, TK_ASSIGNOP, TK_COMMENT, TK_EQ, TK_PLUS, TK_NE, TK_LE, TK_GE,
    TK_SQR, TK_SQL, TK_OR, TK_NOT, TK_AND, TK_ID, TK_FIELDID, TK_DIV, TK_MUL,
    TK_MINUS, TK_FUNID, TK_DOT, TK_CL, TK_OP, TK_COLON, TK_SEM, TK_RUID,
    TK_COMMA, TK_NUM, TK_RNUM, TK_MAIN, TK_AS, TK_CALL, TK_DEFINETYPE,
    TK_ELSE, TK_END, TK_ENDIF, TK_ENDWHILE, TK_ENDRECORD, TK_ENDUNION,
    TK_GLOBAL, TK_IF, TK_INPUT, TK_OUTPUT, TK_INT, TK_REAL, TK_LIST,
    TK_PARAMETERS, TK_PARAMETER, TK_READ, TK_WRITE, TK_RECORD, TK_UNION,
    TK_RETURN, TK_THEN, TK_TYPE, TK_WHILE, TK_WITH, EPS, END_OF_INPUT, TK_ERROR
} terminals;

static const char* tokenStrings[] = {
    "TK_GT", "TK_LT", "TK_ASSIGNOP", "TK_COMMENT", "TK_EQ", "TK_PLUS", "TK_NE", "TK_LE",
    "TK_GE", "TK_SQR", "TK_SQL", "TK_OR", "TK_NOT", "TK_AND", "TK_ID", "TK_FIELDID",
    "TK_DIV", "TK_MUL", "TK_MINUS", "TK_FUNID", "TK_DOT", "TK_CL", "TK_OP", "TK_COLON",
    "TK_SEM", "TK_RUID", "TK_COMMA", "TK_NUM", "TK_RNUM", "TK_MAIN", "TK_AS", "TK_CALL",
    "TK_DEFINETYPE", "TK_ELSE", "TK_END", "TK_ENDIF", "TK_ENDWHILE", "TK_ENDRECORD",
    "TK_ENDUNION", "TK_GLOBAL", "TK_IF", "TK_INPUT", "TK_OUTPUT", "TK_INT", "TK_REAL",
    "TK_LIST", "TK_PARAMETERS", "TK_PARAMETER", "TK_READ", "TK_WRITE", "TK_RECORD",
    "TK_UNION", "TK_RETURN", "TK_THEN", "TK_TYPE", "TK_WHILE", "TK_WITH", "EPS", "END_OF_INPUT"
};

static const char* nonTerminals[] = {
    "program", "mainFunction", "otherFunctions", "function", "input_par", "output_par",
    "parameter_list", "dataType", "primitiveDatatype", "constructedDatatype", "A",
    "stmts", "remaining_list", "typeDefinitions", "typeDefinition", "fieldDefinitions",
    "fieldDefinition", "fieldtype", "moreFields", "declarations", "declaration",
    "global_or_not", "otherStmts", "stmt", "assignmentStmt", "singleOrRecId",
    "option_single_constructed", "oneExpansion", "moreExpansions", "funCallStmt",
    "outputParameters", "inputParameters", "iterativeStmt", "conditionalStmt",
    "elsePart", "ioStmt", "arithmeticExpression", "expPrime", "term", "termPrime",
    "factor", "highPrecedenceOperators", "lowPrecedenceOperators", "booleanExpression",
    "var", "logicalOp", "relationalOp", "returnStmt", "optionalReturn", "idList",
    "more_ids", "definetypestmt"
};


//-----------------------------------------------------
// Type Definitions

// Structure for Twin Buffer 
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

// Struct for symbol table item 
typedef struct SymbolTableitem {
    char *lexeme;
    // Use the terminals enum type (or int) for token type.
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

// Symbol table struct 
typedef struct {
    SymbolItem *data[HASH_TABLE_SIZE];
    int currentsize;
} Symboltable;

typedef struct tokenInfo {
    terminals token;      // Token type from the enum
    char lexeme[256];     // Buffer to store the lexeme text
    int lineNumber;       // The line where the token appears
} tokenInfo;

// Keyword struct 
typedef struct {
    const char *keyword;
    int token;
} keyword;


//-----------------------------------------------------
// Extern Variable Declarations (no initializers here)
extern int lineCount;
extern char *forward;
extern char *lexemebegin;
extern int activeBuffer;       // 1 for buffer1, 2 for buffer2
extern FILE *srcFile;
extern bool exhaustedInput;
extern bool ldfirstBuff;
extern bool ldsecondBuff;
extern TwinBuffer twinBuffer;
extern Symboltable *table;
extern keyword* kwEntries[KC];

void initializeKeywords();

#endif
