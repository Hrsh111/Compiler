#include <stdio.h>
#include "lexerDef.h"
#include "lexer.h"
#include "parserDef.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#define INIT_CAPACITY 8
#define NUM_TERMINALS (sizeof(tokenStrings)/sizeof(tokenStrings[0]))
#define NUM_NONTERMINALS (sizeof(nonTerminals)/sizeof(nonTerminals[0]))
extern const char *TERMINALS_STRINGS[];


int containsSymbol(char **arr, int count, const char *symbol) {
    for (int i = 0; i < count; i++) {
        if (strcmp(arr[i], symbol) == 0)
            return 1;
    }
    return 0;
}
void addSymbol(char ***arr, int *count, int *capacity, const char *symbol) {
    if (!containsSymbol(*arr, *count, symbol)) {
        if (*count == *capacity) {
            *capacity *= 2;
            *arr = realloc(*arr, (*capacity) * sizeof(char *));
            if (!*arr) {
                perror("realloc");
                exit(EXIT_FAILURE);
            }
        }
        (*arr)[*count] = strdup(symbol);
        if (!(*arr)[*count]) {
            perror("strdup");
            exit(EXIT_FAILURE);
        }
        (*count)++;
    }
}



GrammarRule grammarRules[] = {
    {"program", (char *[]){"otherFunctions", "mainFunction"}, 2},
    {"mainFunction", (char *[]){"TK_MAIN", "stmts", "TK_END"}, 3},
    {"otherFunctions", (char *[]){"function", "otherFunctions"}, 2},
    {"otherFunctions", (char *[]){"epsilon"}, 1},
    {"output_par", (char *[]){"TK_OUTPUT", "TK_PARAMETER", "TK_LIST", "TK_SQL", "parameter_list", "TK_SQR"}, 6},
    {"output_par", (char *[]){"epsilon"}, 1},
    {"parameter_list", (char *[]){"dataType", "TK_ID", "remaining_list"}, 3},
    {"dataType", (char *[]){"primitiveDatatype"}, 1},
    {"dataType", (char *[]){"constructedDatatype"}, 1},
    {"primitiveDatatype", (char *[]){"TK_INT"}, 1},
    {"primitiveDatatype", (char *[]){"TK_REAL"}, 1},
    {"constructedDatatype", (char *[]){"TK_RECORD", "TK_RUID"}, 2},
    {"constructedDatatype", (char *[]){"TK_UNION", "TK_RUID"}, 2},
    {"constructedDatatype", (char *[]){"TK_RUID"}, 1},
    {"remaining_list", (char *[]){"TK_COMMA", "parameter_list"}, 2},
    {"remaining_list", (char *[]){"epsilon"}, 1},
    {"stmts", (char *[]){"typeDefinitions", "declarations", "otherStmts", "returnStmt"}, 4},
    {"typeDefinitions", (char *[]){"actualOrRedefined", "typeDefinitions"}, 2},
    {"typeDefinitions", (char *[]){"epsilon"}, 1},
    {"actualOrRedefined", (char *[]){"typeDefinition"}, 1},
    {"actualOrRedefined", (char *[]){"definetypestmt"}, 1},
    {"typeDefinition", (char *[]){"TK_RECORD", "TK_RUID", "fieldDefinitions", "TK_ENDRECORD"}, 4},
    {"typeDefinition", (char *[]){"TK_UNION", "TK_RUID", "fieldDefinitions", "TK_ENDUNION"}, 4},
    {"fieldDefinitions", (char *[]){"fieldDefinition", "fieldDefinition", "moreFields"}, 3},
    {"fieldDefinition", (char *[]){"TK_TYPE", "fieldType", "TK_COLON", "TK_FIELDID", "TK_SEM"}, 5},
    {"fieldType", (char *[]){"primitiveDatatype"}, 1},
    {"fieldType", (char *[]){"constructedDatatype"}, 1},
    {"moreFields", (char *[]){"fieldDefinition", "moreFields"}, 2},
    {"moreFields", (char *[]){"epsilon"}, 1},
    {"declarations", (char *[]){"declaration", "declarations"}, 2},
    {"declarations", (char *[]){"epsilon"}, 1},
    {"declaration", (char *[]){"TK_TYPE", "dataType", "TK_COLON", "TK_ID", "TK_COLON", "global_or_not", "TK_SEM"}, 7},
    {"declaration", (char *[]){"TK_TYPE", "dataType", "TK_COLON", "TK_ID", "global_or_not", "TK_SEM"}, 6},
    {"global_or_not", (char *[]){"TK_COLON", "TK_GLOBAL"}, 2},
    {"global_or_not", (char *[]){"epsilon"}, 1},
    {"otherStmts", (char *[]){"stmt", "otherStmts"}, 2},
    {"otherStmts", (char *[]){"epsilon"}, 1},
    {"stmt", (char *[]){"assignmentStmt"}, 1},
    {"stmt", (char *[]){"iterativeStmt"}, 1},
    {"stmt", (char *[]){"conditionalStmt"}, 1},
    {"stmt", (char *[]){"ioStmt"}, 1},
    {"stmt", (char *[]){"funCallStmt"}, 1},
    {"assignmentStmt", (char *[]){"SingleOrRecId", "TK_ASSIGNOP", "arithmeticExpression", "TK_SEM"}, 4},
    {"singleOrRecId", (char *[]){"TK_ID", "option_single_constructed"}, 2},
    {"option_single_constructed", (char *[]){"epsilon"}, 1},
    {"option_single_constructed", (char *[]){"oneExpansion", "moreExpansions"}, 2},
    {"oneExpansion", (char *[]){"TK_DOT", "TK_FIELDID"}, 2},
    {"moreExpansions", (char *[]){"oneExpansion", "moreExpansions"}, 2},
    {"moreExpansions", (char *[]){"epsilon"}, 1},
    {"funCallStmt", (char *[]){"outputParameters", "TK_CALL", "TK_FUNID", "TK_WITH", "TK_PARAMETERS", "inputParameters", "TK_SEM"}, 7},
    {"outputParameters", (char *[]){"TK_SQL", "idList", "TK_SQR", "TK_ASSIGNOP"}, 4},
    {"outputParameters", (char *[]){"epsilon"}, 1},
    {"inputParameters", (char *[]){"TK_SQL", "idList", "TK_SQR"}, 3},
    {"conditionalStmt", (char *[]){"TK_IF", "TK_OP", "booleanExpression", "TK_CL", "TK_THEN", "stmt", "otherStmts", "elsePart"}, 8},
    {"elsePart", (char *[]){"TK_ELSE", "stmt", "otherStmts", "TK_ENDIF"}, 4},
    {"elsePart", (char *[]){"TK_ENDIF"}, 1},
    {"ioStmt", (char *[]){"TK_READ", "TK_OP", "var", "TK_CL", "TK_SEM"}, 5},
    {"ioStmt", (char *[]){"TK_WRITE", "TK_OP", "var", "TK_CL", "TK_SEM"}, 5},
    {"arithmeticExpression", (char *[]){"term", "expPrime"}, 2},
    {"expPrime", (char *[]){"lowPrecedenceOperators", "term", "expPrime"}, 3},
    {"expPrime", (char *[]){"epsilon"}, 1},
    {"term", (char *[]){"factor", "termPrime"}, 2},
    {"termPrime", (char *[]){"highPrecedenceOperators", "factor", "termPrime"}, 3},
    {"termPrime", (char *[]){"epsilon"}, 1},
    {"factor", (char *[]){"TK_OP", "arithmeticExpression", "TK_CL"}, 3},
    {"factor", (char *[]){"var"}, 1},
    {"highPrecedenceOperators", (char *[]){"TK_MUL"}, 1},
    {"highPrecedenceOperators", (char *[]){"TK_DIV"}, 1},
    {"lowPrecedenceOperators", (char *[]){"TK_PLUS"}, 1},
    {"lowPrecedenceOperators", (char *[]){"TK_MINUS"}, 1},
    {"booleanExpression", (char *[]){"TK_OP", "booleanExpression", "TK_CL", "logicalOp", "TK_OP", "booleanExpression", "TK_CL"}, 7},
    {"booleanExpression", (char *[]){"var", "relationalOp", "var"}, 3},
    {"booleanExpression", (char *[]){"TK_NOT", "TK_OP", "booleanExpression", "TK_CL"}, 4},
    {"var", (char *[]){"singleOrRecId"}, 1},
    {"var", (char *[]){"TK_NUM"}, 1},
    {"var", (char *[]){"TK_RNUM"}, 1},
    {"logicalOp", (char *[]){"TK_AND"}, 1},
    {"logicalOp", (char *[]){"TK_OR"}, 1},
    {"relationalOp", (char *[]){"TK_LT"}, 1},
    {"relationalOp", (char *[]){"TK_LE"}, 1},
    {"relationalOp", (char *[]){"TK_EQ"}, 1},
    {"relationalOp", (char *[]){"TK_GT"}, 1},
    {"relationalOp", (char *[]){"TK_GE"}, 1},
    {"relationalOp", (char *[]){"TK_NE"}, 1},
    {"returnStmt", (char *[]){"TK_RETURN", "optionalReturn", "TK_SEM"}, 3},
    {"optionalReturn", (char *[]){"TK_SQL", "idList", "TK_SQR"}, 3},
    {"optionalReturn", (char *[]){"epsilon"}, 1},
    {"idList", (char *[]){"TK_ID", "more_ids"}, 2},
    {"more_ids", (char *[]){"TK_COMMA", "idList"}, 2},
    {"more_ids", (char *[]){"epsilon"}, 1},
    {"definetypestmt", (char *[]){"TK_DEFINETYPE", "A", "TK_RUID", "TK_AS", "TK_RUID"}, 5},
    {"A", (char *[]){"TK_RECORD"}, 1},
    {"A", (char *[]){"TK_UNION"}, 1}

    
};


// Returns 1 if the symbol is a terminal, 0 otherwise.
int isTerminal(const char *symbol) {
    // For example, assume terminals start with "TK_"
    if (strncmp(symbol, "TK_", 3) == 0)
        return 1;
    return 0;
}

// Returns the index of the terminal in the tokenStrings array; -1 if not found.
int getTerminalIndex(const char *terminal) {
    for (int i = 0; i < (int)NUM_TERMINALS; i++) {
        if (strcmp(tokenStrings[i], terminal) == 0)
            return i;
    }
    return -1;
}
int numGrammarRules = sizeof(grammarRules) / sizeof(grammarRules[0]);
int isNonTerminal(const char *symbol) {
    // Treat "epsilon" as a special marker (not a non-terminal)
    if (strcmp(symbol, "epsilon") == 0)
        return 0;
    // If symbol starts with "TK_", then it is a terminal.
    if (strncmp(symbol, "TK_", 3) == 0)
        return 0;
    return 1;
}

int getNonTerminalIndex(const char *nonTerminal) {
    static const char *nonTerminals[] = {
        "program", "mainFunction", "otherFunctions", "function", "input_par",
        "output_par", "parameter_list", "dataType", "primitiveDatatype",
        "constructedDatatype", "remaining_list", "stmts", "typeDefinitions",
        "actualOrRedefined", "typeDefinition", "fieldDefinitions", "fieldDefinition",
        "fieldType", "moreFields", "declarations", "declaration", "global_or_not",
        "otherStmts", "stmt", "assignmentStmt", "singleOrRecId", "option_single_constructed",
        "oneExpansion", "moreExpansions", "funCallStmt", "outputParameters", "inputParameters",
        "conditionalStmt", "elsePart", "ioStmt", "arithmeticExpression", "expPrime",
        "term", "termPrime", "factor", "highPrecedenceOperators", "lowPrecedenceOperators",
        "booleanExpression", "logicalOp", "relationalOp", "returnStmt", "optionalReturn",
        "idList", "more_ids", "definetypestmt", "A", "var", NULL
    };
    for (int i = 0; nonTerminals[i] != NULL; i++) {
        if (strcmp(nonTerminal, nonTerminals[i]) == 0)
            return i;
    }
    return -1;
}
// Compute FIRST and FOLLOW sets for the given grammar.
// G->rules: array of GrammarRule; G->numRules: total productions; G->startSymbol: start symbol.
FirstFollow* ComputeFirstAndFollowSets(Grammar *G, int numNonTerminals) {
    // Allocate an array for each non-terminal.
    FirstFollow *ff = malloc(numNonTerminals * sizeof(FirstFollow));
    if (!ff) { perror("malloc"); exit(EXIT_FAILURE); }
    for (int i = 0; i < numNonTerminals; i++) {
        ff[i].firstCount = 0;
        ff[i].firstCapacity = INIT_CAPACITY;
        ff[i].first = malloc(ff[i].firstCapacity * sizeof(char *));
        ff[i].followCount = 0;
        ff[i].followCapacity = INIT_CAPACITY;
        ff[i].follow = malloc(ff[i].followCapacity * sizeof(char *));
    }

    // --- FIRST SET COMPUTATION ---
    int changed = 1;
    while (changed) {
        changed = 0;
        // For each production A -> X1 X2 ... Xn
        for (int i = 0; i < G->numRules; i++) {
            GrammarRule rule = G->rules[i];
            int A_index = getNonTerminalIndex(rule.lhs);
            // Create temporary FIRST(alpha) for this production.
            char **temp = malloc(INIT_CAPACITY * sizeof(char *));
            int tempCount = 0, tempCapacity = INIT_CAPACITY;
            int allNullable = 1;
            for (int j = 0; j < rule.rhsSize; j++) {
                char *X = rule.rhs[j];
                if (!isNonTerminal(X)) {  // X is a terminal
                    addSymbol(&temp, &tempCount, &tempCapacity, X);
                    allNullable = 0;
                    break;
                } else {  // X is a non-terminal
                    int X_index = getNonTerminalIndex(X);
                    // Add all symbols from FIRST(X) except "epsilon"
                    for (int k = 0; k < ff[X_index].firstCount; k++) {
                        if (strcmp(ff[X_index].first[k], "epsilon") != 0)
                            addSymbol(&temp, &tempCount, &tempCapacity, ff[X_index].first[k]);
                    }
                    if (!containsSymbol(ff[X_index].first, ff[X_index].firstCount, "epsilon")) {
                        allNullable = 0;
                        break;
                    }
                }
            }
            if (allNullable) {
                addSymbol(&temp, &tempCount, &tempCapacity, "epsilon");
            }
            // Merge temp into FIRST(A)
            for (int t = 0; t < tempCount; t++) {
                if (!containsSymbol(ff[A_index].first, ff[A_index].firstCount, temp[t])) {
                    addSymbol(&ff[A_index].first, &ff[A_index].firstCount, &ff[A_index].firstCapacity, temp[t]);
                    changed = 1;
                }
            }
            for (int t = 0; t < tempCount; t++)
                free(temp[t]);
            free(temp);
        }
    }
    
    // --- FOLLOW SET COMPUTATION ---
    // For the start symbol, add the end marker "$".
    int startIndex = getNonTerminalIndex(G->startSymbol);
    addSymbol(&ff[startIndex].follow, &ff[startIndex].followCount, &ff[startIndex].followCapacity, "$");
    
    changed = 1;
    while (changed) {
        changed = 0;
        // For each production A -> X1 X2 ... Xn
        for (int i = 0; i < G->numRules; i++) {
            GrammarRule rule = G->rules[i];
            int A_index = getNonTerminalIndex(rule.lhs);
            // For each symbol X in the RHS
            for (int j = 0; j < rule.rhsSize; j++) {
                char *X = rule.rhs[j];
                if (isNonTerminal(X)) {
                    int X_index = getNonTerminalIndex(X);
                    // Compute FIRST(beta) for beta = X_{j+1} ... X_n
                    char **firstBeta = malloc(INIT_CAPACITY * sizeof(char *));
                    int betaCount = 0, betaCapacity = INIT_CAPACITY;
                    int betaNullable = 1;
                    for (int k = j + 1; k < rule.rhsSize; k++) {
                        char *Y = rule.rhs[k];
                        if (!isNonTerminal(Y)) {
                            addSymbol(&firstBeta, &betaCount, &betaCapacity, Y);
                            betaNullable = 0;
                            break;
                        } else {
                            int Y_index = getNonTerminalIndex(Y);
                            for (int m = 0; m < ff[Y_index].firstCount; m++) {
                                if (strcmp(ff[Y_index].first[m], "epsilon") != 0)
                                    addSymbol(&firstBeta, &betaCount, &betaCapacity, ff[Y_index].first[m]);
                            }
                            if (!containsSymbol(ff[Y_index].first, ff[Y_index].firstCount, "epsilon")) {
                                betaNullable = 0;
                                break;
                            }
                        }
                    }
                    // Merge firstBeta into FOLLOW(X)
                    for (int t = 0; t < betaCount; t++) {
                        if (!containsSymbol(ff[X_index].follow, ff[X_index].followCount, firstBeta[t])) {
                            addSymbol(&ff[X_index].follow, &ff[X_index].followCount, &ff[X_index].followCapacity, firstBeta[t]);
                            changed = 1;
                        }
                    }
                    // Free firstBeta array.
                    for (int t = 0; t < betaCount; t++)
                        free(firstBeta[t]);
                    free(firstBeta);
                    
                    // If beta is empty or beta is nullable, add FOLLOW(A) to FOLLOW(X)
                    if (betaNullable || j == rule.rhsSize - 1) {
                        for (int t = 0; t < ff[A_index].followCount; t++) {
                            if (!containsSymbol(ff[X_index].follow, ff[X_index].followCount, ff[A_index].follow[t])) {
                                addSymbol(&ff[X_index].follow, &ff[X_index].followCount, &ff[X_index].followCapacity, ff[A_index].follow[t]);
                                changed = 1;
                            }
                        }
                    }
                }
            }
        }
    }
    
    return ff;
}

// --- createParseTable Function ---
// We assume that FIRST and FOLLOW sets have been computed for each non-terminal
// and stored in an array `ffArr` of length 'numNonTerminals'.
// The parse table is a 2D array: rows correspond to non-terminals (by index),
// columns correspond to terminals (by index). We set the cell to the production number
// that should be applied when the non-terminal (row) and terminal (column) are encountered.
// If no production applies, the cell remains -1.

void createParseTable(FirstFollow *ffArr, int numNonTerminals, int table[][NUM_TERMINALS]) {
    // Initialize parse table cells to -1.
    for (int i = 0; i < numNonTerminals; i++) {
        for (int j = 0; j < (int)NUM_TERMINALS; j++) {
            table[i][j] = -1;
        }
    }
    
    // Iterate over each production rule in your grammar.
    // Assume 'grammarRules' is a global array of GrammarRule and 'numGrammarRules' is defined.
     // For each production A -> α
    for (int prod = 0; prod < numGrammarRules; prod++) {
        GrammarRule rule = grammarRules[prod];
        // Get the index of the LHS non-terminal.
        int A_index = getNonTerminalIndex(rule.lhs);
        // Create a temporary dynamic array to hold FIRST(alpha) for the RHS (alpha).
        char **firstAlpha = malloc(INIT_CAPACITY * sizeof(char *));
        int firstAlphaCount = 0, firstAlphaCapacity = INIT_CAPACITY;
        int allNullable = 1;
        
        // For each symbol in the RHS, from left to right.
        for (int j = 0; j < rule.rhsSize; j++) {
            char *X = rule.rhs[j];
            if (!isNonTerminal(X)) {
                // X is a terminal. (It might be "epsilon", but normally epsilon appears only in nullable productions.)
                if (strcmp(X, "epsilon") != 0) {
                    addSymbol(&firstAlpha, &firstAlphaCount, &firstAlphaCapacity, X);
                }
                allNullable = 0;
                break;
            } else {
                // X is a non-terminal. Get its FIRST set from ffArr.
                int X_index = getNonTerminalIndex(X);
                for (int k = 0; k < ffArr[X_index].firstCount; k++) {
                    char *sym = ffArr[X_index].first[k];
                    if (strcmp(sym, "epsilon") != 0) {
                        addSymbol(&firstAlpha, &firstAlphaCount, &firstAlphaCapacity, sym);
                    }
                }
                // If X's FIRST set does not contain "epsilon", stop.
                if (!containsSymbol(ffArr[X_index].first, ffArr[X_index].firstCount, "epsilon")) {
                    allNullable = 0;
                    break;
                }
            }
        }
        // If all symbols in the RHS can derive epsilon, then add "epsilon".
        if (allNullable) {
            addSymbol(&firstAlpha, &firstAlphaCount, &firstAlphaCapacity, "epsilon");
        }
        
        // For each terminal in FIRST(alpha) (except epsilon), update the parse table.
        for (int i = 0; i < firstAlphaCount; i++) {
            if (strcmp(firstAlpha[i], "epsilon") != 0) {
                int termIndex = getTerminalIndex(firstAlpha[i]);
                if (termIndex != -1) {
                    // Set the production number for the cell (A_index, termIndex).
                    table[A_index][termIndex] = prod;
                }
            }
        }
        // If epsilon is in FIRST(alpha), then for each terminal in FOLLOW(A), update the table.
        if (containsSymbol(firstAlpha, firstAlphaCount, "epsilon")) {
            for (int i = 0; i < ffArr[A_index].followCount; i++) {
                int termIndex = getTerminalIndex(ffArr[A_index].follow[i]);
                if (termIndex != -1) {
                    table[A_index][termIndex] = prod;
                }
            }
        }
        
        // Free temporary FIRST(alpha) array.
        for (int i = 0; i < firstAlphaCount; i++) {
            free(firstAlpha[i]);
        }
        free(firstAlpha);
    }
}


#include "stack.h"         // Assumed to provide a stack for ParseTreeNode pointers (createStack, push, pop, peek, isStackEmpty)


// Helper function to create a new parse tree node.
// This function allocates memory for a ParseTreeNode, sets the nodeSymbol, and initializes other fields.
// Helper function to create a new parse tree node.
ParseTreeNode* createParseTreeNode(const char *symbol) {
    ParseTreeNode *node = (ParseTreeNode*)malloc(sizeof(ParseTreeNode));
    if (!node) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    node->nodeSymbol = strdup(symbol);
    node->lexeme = NULL;
    node->tokenName = NULL;
    node->lineno = 0;
    node->value = 0;
    node->parentNodeSymbol = NULL;
    node->isLeafNode = 0;
    node->children = NULL;
    node->numChildren = 0;
    return node;
}

ParseTreeNode* parseInputSourceCode(char *testcaseFile, int parseTable[][NUM_TERMINALS]) {
    // Initialize the lexer with the input source file.
    init_lexer(testcaseFile);

    // Create the parse tree root node with the start symbol "program".
    ParseTreeNode *root = createParseTreeNode("program");

    // Create and initialize the parser stack.
    Stack *stack = createStack();
    // Push the end marker "$" and the start symbol.
    push(stack, createStackNode("$", NULL));
    push(stack, createStackNode("program", root));

    // Fetch the first token from the lexer.
    tokenInfo token = getNextToken(&twinBuffer);
    do {
        token = getNextToken(&twinBuffer);
    } while (token.token == TK_COMMENT); 

    // Main parsing loop.
    while (!isStackEmpty(stack)) {
        StackNode *top = peek(stack);
        char *X = top->symbol;

        // If X is a terminal (or the end marker "$")
        if (isTerminal(X) || strcmp(X, "$") == 0) {
            if (strcmp(X, tokenStrings[token.token]) == 0) {
                // Terminal match: attach token info to the parse tree node.
                top->node->tokenName = strdup(tokenStrings[token.token]);
                top->node->lexeme = strdup(token.lexeme);
                top->node->lineno = token.lineNumber;
                top->node->isLeafNode = 1;
                pop(stack);
                token = getNextToken(&twinBuffer);
            } else {
                fprintf(stderr, "Syntax error at line %d: expected %s, found %s\n",
                        token.lineNumber, X, tokenStrings[token.token]);
                return NULL;
            }
        } else {
            // X is a non-terminal.
            int nonTerminalIndex = getNonTerminalIndex(X);
            int tokenIndex = getTerminalIndex(tokenStrings[token.token]);
            if (tokenIndex == -1) {
                fprintf(stderr, "Syntax error at line %d: invalid token %s\n",
                        token.lineNumber, tokenStrings[token.token]);
                return NULL;
            }
            int prod = parseTable[nonTerminalIndex][tokenIndex];
            if (prod == -1) {
                fprintf(stderr, "Syntax error at line %d: no rule for non-terminal %s with token %s\n",
                        token.lineNumber, X, tokenStrings[token.token]);
                return NULL;
            }
            GrammarRule rule = grammarRules[prod];
            ParseTreeNode *parentNode = top->node;
            pop(stack);

            if (rule.rhsSize == 1 && strcmp(rule.rhs[0], "epsilon") == 0) {
                ParseTreeNode *epsilonNode = createParseTreeNode("epsilon");
                epsilonNode->isLeafNode = 1;
                parentNode->children = (ParseTreeNode *)malloc(sizeof(ParseTreeNode *));
                ((ParseTreeNode **)parentNode->children)[0] = epsilonNode;
                parentNode->numChildren = 1;
            } else {
                int numSymbols = rule.rhsSize;
                ParseTreeNode **childNodes = (ParseTreeNode **)malloc(numSymbols * sizeof(ParseTreeNode *));
                for (int i = 0; i < numSymbols; i++) {
                    childNodes[i] = createParseTreeNode(rule.rhs[i]);
                }
                parentNode->children = (ParseTreeNode *)childNodes;
                parentNode->numChildren = numSymbols;
                for (int i = numSymbols - 1; i >= 0; i--) {
                    push(stack, createStackNode(rule.rhs[i], childNodes[i]));
                }
            }
        }
    }

    if (strcmp(tokenStrings[token.token], "$") != 0) {
         fprintf(stderr, "Syntax error: extra token %s at line %d\n", tokenStrings[token.token], token.lineNumber);
         return NULL;
    }

    printf("Input source code is syntactically correct...........\n");
    return root;
}

// Helper function to print a single node’s information.
// 'nodeCounter' is used to assign a unique order number to each visited node.
void printNodeInfo(ParseTreeNode *node, FILE *fp, int nodeCounter) {
    char lexemeStr[20];
    char tokenNameStr[20];
    char valueStr[20];
    char parentStr[20];
    char isLeafStr[5];
    char nodeSymbolStr[20];

    // Column 1: Lexeme (only if leaf)
    if (node->isLeafNode && node->lexeme != NULL)
        snprintf(lexemeStr, sizeof(lexemeStr), "%s", node->lexeme);
    else
        snprintf(lexemeStr, sizeof(lexemeStr), "----");

    // Column 2: Current node number (order of visit)
    int currNodeNum = nodeCounter;

    // Column 3: Line number
    int lineNo = node->lineno;

    // Column 4: Token name (only for leaf nodes)
    if (node->isLeafNode && node->tokenName != NULL)
        snprintf(tokenNameStr, sizeof(tokenNameStr), "%s", node->tokenName);
    else
        snprintf(tokenNameStr, sizeof(tokenNameStr), "----");

    // Column 5: Value if number (if token is TK_NUM or TK_RNUM)
    if (node->isLeafNode && node->tokenName != NULL && 
       (strcmp(node->tokenName, "TK_NUM") == 0 || strcmp(node->tokenName, "TK_RNUM") == 0))
    {
        snprintf(valueStr, sizeof(valueStr), "%.2lf", node->value);
    } else {
        snprintf(valueStr, sizeof(valueStr), "----");
    }

    // Column 6: Parent node symbol (if not available, print ROOT)
    if (node->parentNodeSymbol != NULL)
        snprintf(parentStr, sizeof(parentStr), "%s", node->parentNodeSymbol);
    else
        snprintf(parentStr, sizeof(parentStr), "ROOT");

    // Column 7: isLeafNode (yes/no)
    snprintf(isLeafStr, sizeof(isLeafStr), "%s", node->isLeafNode ? "yes" : "no");

    // Column 8: Node symbol (if non-leaf, print actual non-terminal symbol; else dummy)
    if (!node->isLeafNode && node->nodeSymbol != NULL)
        snprintf(nodeSymbolStr, sizeof(nodeSymbolStr), "%s", node->nodeSymbol);
    else
        snprintf(nodeSymbolStr, sizeof(nodeSymbolStr), "----");

    // Print the row with appropriate fixed widths.
    // Field widths: lexeme(15), current node (5), lineno (5), tokenName (15), value (15),
    // parent (15), isLeaf (5), nodeSymbol (15)
    fprintf(fp, "%-15s %-5d %-5d %-15s %-15s %-15s %-5s %-15s\n",
            lexemeStr, currNodeNum, lineNo, tokenNameStr, valueStr, parentStr, isLeafStr, nodeSymbolStr);
}

// Recursive helper function to perform inorder traversal on the parse tree.
// For an n-ary tree, we define inorder as:
//   - If there are children, traverse the first half, then print the current node,
//     then traverse the remaining children.
//   - If the node is a leaf, simply print it.
void printParseTreeHelper(ParseTreeNode *node, FILE *fp, int *counter) {
    if (node == NULL)
        return;

    // If the node has children, we perform a split for inorder.
    if (node->numChildren > 0) {
        int mid = node->numChildren / 2;
        ParseTreeNode **childArray = (ParseTreeNode **)node->children;
        // Traverse the first half of the children.
        for (int i = 0; i < mid; i++) {
            printParseTreeHelper(childArray[i], fp, counter);
        }
        // Print the current (non-leaf) node.
        (*counter)++;
        printNodeInfo(node, fp, *counter);
        // Traverse the remaining children.
        for (int i = mid; i < node->numChildren; i++) {
            printParseTreeHelper(childArray[i], fp, counter);
        }
    } else {
        // For leaf nodes, simply print the node.
        (*counter)++;
        printNodeInfo(node, fp, *counter);
    }
}

// Main function to print the parse tree to the file 'outfile' in inorder.
void printParseTree(ParseTree *PT, char *outfile) {
    if (PT == NULL || PT->root == NULL) {
        fprintf(stderr, "Parse Tree is empty.\n");
        return;
    }

    FILE *fp = fopen(outfile, "w");
    if (fp == NULL) {
        perror("Error opening output file");
        return;
    }

    // Print a header (optional).
    fprintf(fp, "%-15s %-5s %-5s %-15s %-15s %-15s %-5s %-15s\n",
            "Lexeme", "Node#", "Lineno", "TokenName", "Value", "Parent", "Leaf", "NodeSymbol");
    fprintf(fp, "------------------------------------------------------------------------------------------\n");

    int nodeCounter = 0;
    printParseTreeHelper(PT->root, fp, &nodeCounter);

    fclose(fp);
}

