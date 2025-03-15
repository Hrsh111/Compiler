#include <stdio.h>
#include "lexerDef.h"
#include "lexer.h"
#include "parserDef.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "parser.h"
#define INIT_CAPACITY 8
#define NUM_TERMINALS (sizeof(tokenStrings)/sizeof(tokenStrings[0]))



extern const char *TERMINALS_STRINGS[];


int containsSymbol(char **arr, int count, const char *symbol) {
    for (int i = 0; i < count; i++) {
        if (strcmp(arr[i], symbol) == 0)
            return 1;
    }
    return 0;
}
void freeParseTree(ParseTreeNode *root) {
    if (root == NULL) return;
    if (root->children != NULL) {
        for (int i = 0; i < root->numChildren; i++) {
            if (((ParseTreeNode **)root->children)[i] != NULL) {
                freeParseTree(((ParseTreeNode **)root->children)[i]);
            }
        }
        free(root->children);
        root->children = NULL; // Prevent double free
    }
    free(root);
    root = NULL; // Avoid dangling pointer
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

int getNonTerminalIndex(const char *nonTerminal) {
    if (strcmp(nonTerminal, "program") == 0) return NT_PROGRAM;
    if (strcmp(nonTerminal, "mainFunction") == 0) return NT_MAINFUNCTION;
    if (strcmp(nonTerminal, "otherFunctions") == 0) return NT_OTHERFUNCTIONS;
    if (strcmp(nonTerminal, "function") == 0) return NT_FUNCTION;
    if (strcmp(nonTerminal, "input_par") == 0) return NT_INPUT_PAR;
    if (strcmp(nonTerminal, "output_par") == 0) return NT_OUTPUT_PAR;
    if (strcmp(nonTerminal, "opt_input_par") == 0) return NT_OPT_INPUT_PAR;
    if (strcmp(nonTerminal, "iterativeStmt") == 0) return NT_ITERATIVE_STMT;
    if (strcmp(nonTerminal, "parameter_list") == 0) return NT_PARAMETER_LIST;
    if (strcmp(nonTerminal, "dataType") == 0) return NT_DATA_TYPE;
    if (strcmp(nonTerminal, "primitiveDatatype") == 0) return NT_PRIMITIVE_DATATYPE;
    if (strcmp(nonTerminal, "constructedDatatype") == 0) return NT_CONSTRUCTED_DATATYPE;
    if (strcmp(nonTerminal, "A") == 0) return NT_A;
    if (strcmp(nonTerminal, "stmts") == 0) return NT_STMTS;
    if (strcmp(nonTerminal, "remaining_list") == 0) return NT_REMAINING_LIST;
    if (strcmp(nonTerminal, "typeDefinitions") == 0) return NT_TYPE_DEFINITIONS;
    if (strcmp(nonTerminal, "actualOrRedefined") == 0) return NT_ACTUAL_OR_REDEFINED;
    if (strcmp(nonTerminal, "typeDefinition") == 0) return NT_TYPE_DEFINITION;
    if (strcmp(nonTerminal, "fieldDefinitions") == 0) return NT_FIELD_DEFINITIONS;
    if (strcmp(nonTerminal, "fieldDefinition") == 0) return NT_FIELD_DEFINITION;
    if (strcmp(nonTerminal, "fieldType") == 0) return NT_FIELD_TYPE;
    if (strcmp(nonTerminal, "moreFields") == 0) return NT_MORE_FIELDS;
    if (strcmp(nonTerminal, "declarations") == 0) return NT_DECLARATIONS;
    if (strcmp(nonTerminal, "declaration") == 0) return NT_DECLARATION;
    if (strcmp(nonTerminal, "global_or_not") == 0) return NT_GLOBAL_OR_NOT;
    if (strcmp(nonTerminal, "otherStmts") == 0) return NT_OTHERSTMTS;
    if (strcmp(nonTerminal, "stmt") == 0) return NT_STMT;
    if (strcmp(nonTerminal, "assignmentStmt") == 0) return NT_ASSIGNMENT_STMT;
    if (strcmp(nonTerminal, "singleOrRecId") == 0) return NT_SINGLE_OR_REC_ID;
    if (strcmp(nonTerminal, "option_single_constructed") == 0) return NT_OPTION_SINGLE_CONSTRUCTED;
    if (strcmp(nonTerminal, "oneExpansion") == 0) return NT_ONE_EXPANSION;
    if (strcmp(nonTerminal, "moreExpansions") == 0) return NT_MORE_EXPANSIONS;
    if (strcmp(nonTerminal, "funCallStmt") == 0) return NT_FUN_CALL_STMT;
    if (strcmp(nonTerminal, "outputParameters") == 0) return NT_OUTPUT_PARAMETERS;
    if (strcmp(nonTerminal, "inputParameters") == 0) return NT_INPUT_PARAMETERS;
    if (strcmp(nonTerminal, "conditionalStmt") == 0) return NT_CONDITIONAL_STMT;
    if (strcmp(nonTerminal, "elsePart") == 0) return NT_ELSE_PART;
    if (strcmp(nonTerminal, "ioStmt") == 0) return NT_IO_STMT;
    if (strcmp(nonTerminal, "arithmeticExpression") == 0) return NT_ARITHMETIC_EXPRESSION;
    if (strcmp(nonTerminal, "expPrime") == 0) return NT_EXP_PRIME;
    if (strcmp(nonTerminal, "term") == 0) return NT_TERM;
    if (strcmp(nonTerminal, "termPrime") == 0) return NT_TERM_PRIME;
    if (strcmp(nonTerminal, "factor") == 0) return NT_FACTOR;
    if (strcmp(nonTerminal, "highPrecedenceOperators") == 0) return NT_HIGH_PRECEDENCE_OPERATORS;
    if (strcmp(nonTerminal, "lowPrecedenceOperators") == 0) return NT_LOW_PRECEDENCE_OPERATORS;
    if (strcmp(nonTerminal, "booleanExpression") == 0) return NT_BOOLEAN_EXPRESSION;
    if (strcmp(nonTerminal, "var") == 0) return NT_VAR;
    if (strcmp(nonTerminal, "logicalOp") == 0) return NT_LOGICAL_OP;
    if (strcmp(nonTerminal, "relationalOp") == 0) return NT_RELATIONAL_OP;
    if (strcmp(nonTerminal, "returnStmt") == 0) return NT_RETURN_STMT;
    if (strcmp(nonTerminal, "optionalReturn") == 0) return NT_OPTIONAL_RETURN;
    if (strcmp(nonTerminal, "idList") == 0) return NT_ID_LIST;
    if (strcmp(nonTerminal, "more_ids") == 0) return NT_MORE_IDS;
    if (strcmp(nonTerminal, "definetypestmt") == 0) return NT_DEFINETYPE_STMT;
    return -1;  // Return -1 if not found (error)
}


// Define RHS arrays explicitly
static char *program_rhs[] = {"otherFunctions", "mainFunction"};
static char *mainFunction_rhs[] = {"TK_FIELDID", "stmts", "TK_END"};
static char *otherFunctions_rhs1[] = {"function", "otherFunctions"};
static char *otherFunctions_rhs2[] = {"epsilon"};
static char *input_par_rhs1[] = {"epsilon"};
static char *input_par_rhs2[] = {"TK_INPUT", "TK_PARAMETER", "TK_LIST", "TK_SQL", "parameter_list", "TK_SQR"};
static char *function_rhs[] = {"TK_ID", "opt_input_par", "output_par", "stmts", "TK_END"};
static char *opt_input_par_rhs1[] = {"TK_INPUT", "TK_PARAMETER", "TK_LIST", "TK_SQL", "parameter_list", "TK_SQR"};
static char *opt_input_par_rhs2[] = {"epsilon"};
static char *parameter_list_rhs[] = {"dataType", "TK_ID", "remaining_list"};
static char *dataType_rhs1[] = {"primitiveDatatype"};
static char *dataType_rhs2[] = {"constructedDatatype"};
static char *primitiveDatatype_rhs1[] = {"TK_INT"};
static char *primitiveDatatype_rhs2[] = {"TK_REAL"};
static char *constructedDatatype_rhs1[] = {"TK_RECORD", "TK_RUID"};
static char *constructedDatatype_rhs2[] = {"TK_UNION", "TK_RUID"};
static char *constructedDatatype_rhs3[] = {"TK_RUID"};
static char *remaining_list_rhs1[] = {"TK_COMMA", "parameter_list"};
static char *remaining_list_rhs2[] = {"epsilon"};
static char *stmts_rhs[] = {"typeDefinitions", "declarations", "otherStmts", "returnStmt"};
static char *typeDefinitions_rhs1[] = {"actualOrRedefined", "typeDefinitions"};
static char *typeDefinitions_rhs2[] = {"epsilon"};
static char *actualOrRedefined_rhs1[] = {"typeDefinition"};
static char *actualOrRedefined_rhs2[] = {"definetypestmt"};
static char *typeDefinition_rhs1[] = {"TK_RECORD", "TK_RUID", "fieldDefinitions", "TK_ENDRECORD"};
static char *typeDefinition_rhs2[] = {"TK_UNION", "TK_RUID", "fieldDefinitions", "TK_ENDUNION"};
static char *fieldDefinitions_rhs[] = {"fieldDefinition", "fieldDefinition", "moreFields"};
static char *fieldDefinition_rhs[] = {"TK_TYPE", "fieldType", "TK_COLON", "TK_FIELDID", "TK_SEM"};
static char *fieldType_rhs1[] = {"primitiveDatatype"};
static char *fieldType_rhs2[] = {"constructedDatatype"};
static char *moreFields_rhs1[] = {"fieldDefinition", "moreFields"};
static char *moreFields_rhs2[] = {"epsilon"};
static char *iterativeStmt_rhs[] = {"TK_WHILE", "TK_OP", "booleanExpression", "TK_CL", "stmt", "otherStmts", "TK_ENDWHILE"};

// Grammar Rule Definitions
GrammarRule grammarRules[] = {
    {"program", program_rhs, 2},
    {"mainFunction", mainFunction_rhs, 3},
    {"otherFunctions", otherFunctions_rhs1, 2},
    {"otherFunctions", otherFunctions_rhs2, 1},
    {"input_par", input_par_rhs1, 1},
    {"input_par", input_par_rhs2, 6},
    {"function", function_rhs, 5},
    {"opt_input_par", opt_input_par_rhs1, 6},
    {"opt_input_par", opt_input_par_rhs2, 1},
    {"parameter_list", parameter_list_rhs, 3},
    {"dataType", dataType_rhs1, 1},
    {"dataType", dataType_rhs2, 1},
    {"primitiveDatatype", primitiveDatatype_rhs1, 1},
    {"primitiveDatatype", primitiveDatatype_rhs2, 1},
    {"constructedDatatype", constructedDatatype_rhs1, 2},
    {"constructedDatatype", constructedDatatype_rhs2, 2},
    {"constructedDatatype", constructedDatatype_rhs3, 1},
    {"remaining_list", remaining_list_rhs1, 2},
    {"remaining_list", remaining_list_rhs2, 1},
    {"stmts", stmts_rhs, 4},
    {"typeDefinitions", typeDefinitions_rhs1, 2},
    {"typeDefinitions", typeDefinitions_rhs2, 1},
    {"actualOrRedefined", actualOrRedefined_rhs1, 1},
    {"actualOrRedefined", actualOrRedefined_rhs2, 1},
    {"typeDefinition", typeDefinition_rhs1, 4},
    {"typeDefinition", typeDefinition_rhs2, 4},
    {"fieldDefinitions", fieldDefinitions_rhs, 3},
    {"fieldDefinition", fieldDefinition_rhs, 5},
    {"fieldType", fieldType_rhs1, 1},
    {"fieldType", fieldType_rhs2, 1},
    {"moreFields", moreFields_rhs1, 2},
    {"moreFields", moreFields_rhs2, 1},
    {"iterativeStmt", iterativeStmt_rhs, 7}
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

/*int getNonTerminalIndex(const char *nonTerminal) {
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
        "idList", "more_ids", "definetypestmt", "A", "var","opt_input_par", "iterativeStmt", "SingleOrRecId", NULL
    };
    for (int i = 0; nonTerminals[i] != NULL; i++) {
        if (strcmp(nonTerminal, nonTerminals[i]) == 0)
            return i;
    }
    printf("ERROR: Non-terminal '%s' not found\n", nonTerminal);
    return -1;
}*/
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
        if (!ff[i].first) { perror("malloc first"); exit(EXIT_FAILURE); }

        ff[i].followCount = 0;
        ff[i].followCapacity = INIT_CAPACITY;
        ff[i].follow = malloc(ff[i].followCapacity * sizeof(char *));
        if (!ff[i].follow) { perror("malloc follow"); exit(EXIT_FAILURE); }
    }

    // --- FIRST SET COMPUTATION ---
    int changed = 1;
    while (changed) {
        changed = 0;
        for (int i = 0; i < G->numRules; i++) {
            GrammarRule rule = G->rules[i];
            int A_index = getNonTerminalIndex(rule.lhs);
            if (A_index < 0 || A_index >= numNonTerminals) {  
                fprintf(stderr, "ERROR: Invalid non-terminal index for %s\n", rule.lhs);
                continue;  
            }

            char **temp = malloc(INIT_CAPACITY * sizeof(char *));
            if (!temp) {
                fprintf(stderr, "ERROR: Memory allocation failed for temp FIRST set\n");
                exit(EXIT_FAILURE);
            }
            int tempCount = 0, tempCapacity = INIT_CAPACITY;
            int allNullable = 1;

            for (int j = 0; j < rule.rhsSize; j++) {
                char *X = rule.rhs[j];
                if (!isNonTerminal(X)) {
                    addSymbol(&temp, &tempCount, &tempCapacity, X);
                    allNullable = 0;
                    break;
                } else {  
                    int X_index = getNonTerminalIndex(X);
                    if (X_index < 0 || X_index >= numNonTerminals) {  
                        fprintf(stderr, "ERROR: Invalid non-terminal index for %s\n", X);
                        continue;
                    }
                    for (int k = 0; k < ff[X_index].firstCount; k++) {
                        if (ff[X_index].first[k] != NULL && strcmp(ff[X_index].first[k], "epsilon") != 0)
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

            for (int t = 0; t < tempCount; t++) {
                if (temp[t] != NULL && !containsSymbol(ff[A_index].first, ff[A_index].firstCount, temp[t])) {
                    // 🚨 Check if FIRST set is full before adding
                    if (ff[A_index].firstCount >= ff[A_index].firstCapacity) {
                        ff[A_index].firstCapacity *= 2;  // Double the capacity
                        ff[A_index].first = realloc(ff[A_index].first, ff[A_index].firstCapacity * sizeof(char *));
                        if (!ff[A_index].first) {
                            fprintf(stderr, "ERROR: Memory allocation failed while expanding FIRST set\n");
                            exit(EXIT_FAILURE);
                        }
                    }
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
            if (A_index < 0 || A_index >= numNonTerminals) {
                fprintf(stderr, "ERROR: Invalid non-terminal index for %s\n", rule.lhs);
                continue;
            }
            
            // For each symbol X in the RHS
            for (int j = 0; j < rule.rhsSize; j++) {
                char *X = rule.rhs[j];
                if (isNonTerminal(X)) {
                    int X_index = getNonTerminalIndex(X);
                    if (X_index < 0 || X_index >= numNonTerminals) {
                        fprintf(stderr, "ERROR: Invalid RHS non-terminal index for %s\n", X);
                        continue;
                    }
                    
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

void createParseTable(FirstFollow *ffArr, int numNonTerminals, int table[TOTAL_NON_TERMINALS][NUM_TERMINALS]) {
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
        if (A_index == -1) {
            printf("❌ ERROR: Non-terminal '%s' not found in index!\n", rule.lhs);
            continue;
        }
        
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
                if (X_index == -1) {
                    printf("WARNING: RHS non-terminal '%s' not found in index!\n", X);
                    continue;
                }
                
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
                    printf(" Adding rule: %s -> %s at parseTable[%d][%d]\n", 
                        rule.lhs, rule.rhs[0], A_index, termIndex);
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
    if (node==NULL) {
        fprintf(stderr, "ERROR: Memory allocation failed for parse tree node\n");
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    if (symbol == NULL) {  
        fprintf(stderr, "ERROR: Attempting to create a parse node with NULL symbol\n");
        free(node);
        return NULL;
    }
    node->nodeSymbol = strdup(symbol);
    if (node->nodeSymbol == NULL) {
        fprintf(stderr, "ERROR: strdup failed\n");
        free(node);
        exit(EXIT_FAILURE);
    }
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

ParseTreeNode* parseInputSourceCode(char *testcaseFile, int parseTable[TOTAL_NON_TERMINALS][NUM_TERMINALS])
 {
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

// Ensure indices are valid before accessing the parse table
if (nonTerminalIndex < 0 || nonTerminalIndex >= (int)TOTAL_NON_TERMINALS ||
    tokenIndex < 0 || tokenIndex >= (int)NUM_TERMINALS) {
    fprintf(stderr, "Syntax error at line %d: invalid non-terminal %s or token %s\n",
            token.lineNumber, X, tokenStrings[token.token]);
    freeParseTree(root); // Free allocated memory before exiting
    exit(EXIT_FAILURE);
    return NULL;
}

// Initialize prod safely
int prod = -1;
prod = parseTable[nonTerminalIndex][tokenIndex];

if (prod == -1) {
    fprintf(stderr, "Syntax error at line %d: no rule for non-terminal %s with token %s\n",
            token.lineNumber, X, tokenStrings[token.token]);
    freeParseTree(root); // Free allocated memory before exiting
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

