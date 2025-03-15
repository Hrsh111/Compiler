#include <stdio.h>
#include "lexerDef.h"
#include "lexer.h"
#include "parserDef.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#define INIT_CAPACITY 8



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

