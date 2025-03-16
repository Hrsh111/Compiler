#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "lexerDef.h"
#include "lexer.h"
#include "parserDef.h"
#include "parser.h"
#include "stack.h"
#define DEBUG 1 

#define INIT_CAPACITY 8
#define NUM_TERMINALS (sizeof(tokenStrings)/sizeof(tokenStrings[0]))
#define MAX_LINE_LENGTH 1024
#define INIT_RULES_CAPACITY 10

extern const char *TERMINALS_STRINGS[];

/*--------------------------------------------------
  Grammar Loader (Dynamic Grammar)
--------------------------------------------------*/
// Helper function: trim leading and trailing whitespace.
char *trim(char *str) {
    char *end;
    while (*str == ' ' || *str == '\t')
        str++;
    if (*str == 0)
        return str;
    end = str + strlen(str) - 1;
    while (end > str && (*end==' ' || *end=='\t' || *end=='\n' || *end=='\r'))
        end--;
    *(end+1) = '\0';
    return str;
}

// Loader function: Reads a grammar file and returns a Grammar pointer.
Grammar* loadGrammar(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Error opening grammar file");
        exit(EXIT_FAILURE);
    }
    
    int capacity = INIT_RULES_CAPACITY;
    int count = 0;
    GrammarRule **rules = malloc(capacity * sizeof(GrammarRule *));
    if (!rules) {
        perror("malloc rules");
        exit(EXIT_FAILURE);
    }
    
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = '\0'; // Remove newline.
        char *trimmed = trim(line);
        if (strlen(trimmed) == 0 || trimmed[0] == '#')
            continue; // Skip empty lines or comments.
        
        GrammarRule *rule = malloc(sizeof(GrammarRule));
        if (!rule) {
            perror("malloc rule");
            exit(EXIT_FAILURE);
        }
        // The first token is the LHS.
        char *token = strtok(trimmed, " \t");
        if (!token)
            continue;
        rule->lhs = strdup(token);
        rule->rhsCount = 0;
        int rhsCapacity = 10;
        rule->rhs = malloc(rhsCapacity * sizeof(char *));
        if (!rule->rhs) {
            perror("malloc rhs");
            exit(EXIT_FAILURE);
        }
        // The rest are RHS symbols.
        while ((token = strtok(NULL, " \t")) != NULL) {
            // Treat "ε" or "epsilon" as an epsilon production (empty RHS)
            if (strcmp(token, "ε") == 0 || strcmp(token, "epsilon") == 0) {
                continue; // Skip adding any symbol for epsilon
            }
            if (rule->rhsCount == rhsCapacity) {
                rhsCapacity *= 2;
                rule->rhs = realloc(rule->rhs, rhsCapacity * sizeof(char *));
                if (!rule->rhs) {
                    perror("realloc rhs");
                    exit(EXIT_FAILURE);
                }
            }
            rule->rhs[rule->rhsCount++] = strdup(token);
        }
        

        if (count == capacity) {
            capacity *= 2;
            rules = realloc(rules, capacity * sizeof(GrammarRule *));
            if (!rules) {
                perror("realloc rules");
                exit(EXIT_FAILURE);
            }
        }
        rules[count++] = rule;
    }
    fclose(fp);
    
    Grammar *G = malloc(sizeof(Grammar));
    if (!G) {
        perror("malloc Grammar");
        exit(EXIT_FAILURE);
    }
    G->rules = rules;
    G->numRules = count;
    // Set the start symbol (adjust if necessary).
    G->startSymbol = strdup("program");
    return G;
}

/*--------------------------------------------------
  Utility Functions for FIRST/FOLLOW and Parse Table
--------------------------------------------------*/
int containsSymbol(char **arr, int count, const char *symbol) {
    for (int i = 0; i < count; i++) {
        if (strcmp(arr[i], symbol) == 0)
            return 1;
    }
    return 0;
}

bool addSymbol(char ***arr, int *count, int *capacity, const char *symbol) {
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
        return true;
    }
    return false;
}


int getNonTerminalIndex(const char *nonTerminal) {
    if (strcmp(nonTerminal, "operator") == 0) return NT_OPERATOR;
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
    return -1;
}

int getTerminalIndex(const char *terminal) {
    for (size_t i = 0; i < NUM_TERMINALS; i++) {
        if (strcmp(tokenStrings[i], terminal) == 0)
            return (int)i;
    }
    return -1;
}
int isTerminal(const char *symbol) {
    if (strcmp(symbol, "TK_EPS") == 0) return 0; // Treat TK_EPS as epsilon
    return (strncmp(symbol, "TK_", 3) == 0);
}

int isNonTerminal(const char *symbol) {
    if (strcmp(symbol, "TK_EPS") == 0) return 0;
    return (!isTerminal(symbol));
}

/*-----------------------------------------
   FIRST/FOLLOW and Parse Table Construction
-----------------------------------------*/
FirstFollow* ComputeFirstAndFollowSets(Grammar *G, int numNonTerminals) {
    if (DEBUG) printf("\nComputing FIRST & FOLLOW sets for %d non-terminals...\n", numNonTerminals);

    FirstFollow *ff = malloc(numNonTerminals * sizeof(FirstFollow));
    if (!ff) { perror("malloc failed"); exit(EXIT_FAILURE); }

    for (int i = 0; i < numNonTerminals; i++) {
        ff[i].firstCount = 0;
        ff[i].firstCapacity = INIT_CAPACITY;
        ff[i].first = malloc(ff[i].firstCapacity * sizeof(char *));
        if (!ff[i].first) { perror("malloc first failed"); exit(EXIT_FAILURE); }

        ff[i].followCount = 0;
        ff[i].followCapacity = INIT_CAPACITY;
        ff[i].follow = malloc(ff[i].followCapacity * sizeof(char *));
        if (!ff[i].follow) { perror("malloc follow failed"); exit(EXIT_FAILURE); }
    }

    // Compute FIRST sets
    int changed = 1;
    while (changed) {
        changed = 0;
        if (DEBUG) printf("\n[Iterating FIRST Set Computation...]\n");

        for (int i = 0; i < G->numRules; i++) {
            GrammarRule *rule = G->rules[i];
            int A_index = getNonTerminalIndex(rule->lhs);
            if (A_index < 0 || A_index >= numNonTerminals) {
                fprintf(stderr, "ERROR: Invalid non-terminal index for %s\n", rule->lhs);
                continue;
            }

            // If the production has an empty RHS, add TK_EPS to FIRST(A)
            if (rule->rhsCount == 0) {
                if (addSymbol(&ff[A_index].first, &ff[A_index].firstCount, 
                              &ff[A_index].firstCapacity, "TK_EPS"))
                    changed = 1;
                continue; // Move on to the next rule.
            }

            int allNullable = 1;  // Assume entire RHS can derive epsilon
            for (int j = 0; j < rule->rhsCount; j++) {
                char *X = rule->rhs[j];

                if (!isNonTerminal(X)) {
                    // X is a terminal. Add it to FIRST(A).
                    if (addSymbol(&ff[A_index].first, &ff[A_index].firstCount, 
                                  &ff[A_index].firstCapacity, X))
                        changed = 1;
                    allNullable = 0;
                    break;  // Stop at the first terminal.
                } else {
                    int X_index = getNonTerminalIndex(X);
                    if (X_index < 0 || X_index >= numNonTerminals) {
                        fprintf(stderr, "ERROR: Invalid non-terminal index for %s\n", X);
                        continue;
                    }

                    // Add all non-epsilon symbols from FIRST(X) to FIRST(A)
                    for (int k = 0; k < ff[X_index].firstCount; k++) {
                        if (strcmp(ff[X_index].first[k], "TK_EPS") != 0) {
                            if (addSymbol(&ff[A_index].first, &ff[A_index].firstCount, 
                                          &ff[A_index].firstCapacity, ff[X_index].first[k]))
                                changed = 1;
                        }
                    }

                    // If FIRST(X) does not contain epsilon, stop.
                    if (!containsSymbol(ff[X_index].first, ff[X_index].firstCount, "TK_EPS")) {
                        allNullable = 0;
                        break;
                    }
                }
            }

            // If every symbol in the RHS can derive epsilon, then add TK_EPS.
            if (allNullable) {
                if (addSymbol(&ff[A_index].first, &ff[A_index].firstCount, 
                              &ff[A_index].firstCapacity, "TK_EPS"))
                    changed = 1;
            }
        }
    }

    // Compute FOLLOW sets
    if (DEBUG) printf("\n[Computing FOLLOW Sets...]\n");

    int startIndex = getNonTerminalIndex(G->startSymbol);
    addSymbol(&ff[startIndex].follow, &ff[startIndex].followCount, 
              &ff[startIndex].followCapacity, "$");

    changed = 1;
    while (changed) {
        changed = 0;
        for (int i = 0; i < G->numRules; i++) {
            GrammarRule *rule = G->rules[i];
            int A_index = getNonTerminalIndex(rule->lhs);
            if (A_index < 0 || A_index >= numNonTerminals) {
                fprintf(stderr, "ERROR: Invalid non-terminal index for %s\n", rule->lhs);
                continue;
            }

            for (int j = 0; j < rule->rhsCount; j++) {
                char *X = rule->rhs[j];
                if (isNonTerminal(X)) {
                    int X_index = getNonTerminalIndex(X);
                    if (X_index < 0 || X_index >= numNonTerminals) {
                        fprintf(stderr, "ERROR: Invalid non-terminal index for %s\n", X);
                        continue;
                    }

                    int betaNullable = 1;
                    for (int k = j + 1; k < rule->rhsCount; k++) {
                        char *Y = rule->rhs[k];

                        if (!isNonTerminal(Y)) {
                            if (addSymbol(&ff[X_index].follow, &ff[X_index].followCount, 
                                          &ff[X_index].followCapacity, Y))
                                changed = 1;
                            betaNullable = 0;
                            break;
                        } else {
                            int Y_index = getNonTerminalIndex(Y);
                            for (int m = 0; m < ff[Y_index].firstCount; m++) {
                                if (strcmp(ff[Y_index].first[m], "TK_EPS") != 0) {
                                    if (addSymbol(&ff[X_index].follow, &ff[X_index].followCount, 
                                                  &ff[X_index].followCapacity, ff[Y_index].first[m]))
                                        changed = 1;
                                }
                            }

                            if (!containsSymbol(ff[Y_index].first, ff[Y_index].firstCount, "TK_EPS")) {
                                betaNullable = 0;
                                break;
                            }
                        }
                    }

                    if (betaNullable || j == rule->rhsCount - 1) {
                        for (int t = 0; t < ff[A_index].followCount; t++) {
                            if (addSymbol(&ff[X_index].follow, &ff[X_index].followCount, 
                                          &ff[X_index].followCapacity, ff[A_index].follow[t]))
                                changed = 1;
                        }
                    }
                }
            }
        }
    }

    if (DEBUG) {
        printf("\n[FINAL FIRST & FOLLOW SETS]\n");
        for (int i = 0; i < numNonTerminals; i++) {
            printf("FIRST(%s) = { ", G->rules[i]->lhs);
            for (int j = 0; j < ff[i].firstCount; j++) {
                printf("%s ", ff[i].first[j]);
            }
            printf("}\n");

            printf("FOLLOW(%s) = { ", G->rules[i]->lhs);
            for (int j = 0; j < ff[i].followCount; j++) {
                printf("%s ", ff[i].follow[j]);
            }
            printf("}\n");
        }
    }
    return ff;
}


void createParseTable(FirstFollow *ffArr, int numNonTerminals, Grammar *G, int table[TOTAL_NON_TERMINALS][NUM_TERMINALS]) {
    for (int i = 0; i < numNonTerminals; i++) {
        for (size_t j = 0; j < NUM_TERMINALS; j++) {
            table[i][j] = -1;
        }
    }
    for (int prod = 0; prod < G->numRules; prod++) {
        GrammarRule *rule = G->rules[prod];
        int A_index = getNonTerminalIndex(rule->lhs);
        if (A_index == -1) {
            printf(" ERROR: Non-terminal '%s' not found in index!\n", rule->lhs);
            continue;
        }
        char **firstAlpha = malloc(INIT_CAPACITY * sizeof(char *));
        int firstAlphaCount = 0, firstAlphaCapacity = INIT_CAPACITY;
        int allNullable = 1;
        for (int j = 0; j < rule->rhsCount; j++) {
            char *X = rule->rhs[j];
            if (!isNonTerminal(X)) {
                if (strcmp(X, "epsilon") != 0)
                    addSymbol(&firstAlpha, &firstAlphaCount, &firstAlphaCapacity, X);
                allNullable = 0;
                break;
            } else {
                int X_index = getNonTerminalIndex(X);
                if (X_index == -1) {
                    printf("WARNING: RHS non-terminal '%s' not found in index!\n", X);
                    continue;
                }
                for (int k = 0; k < ffArr[X_index].firstCount; k++) {
                    char *sym = ffArr[X_index].first[k];
                    if (strcmp(sym, "TK_EPS") != 0)
                        addSymbol(&firstAlpha, &firstAlphaCount, &firstAlphaCapacity, sym);
                }
                if (!containsSymbol(ffArr[X_index].first, ffArr[X_index].firstCount, "TK_EPS"))
                    allNullable = 0;
            }
        }
        if (allNullable)
            addSymbol(&firstAlpha, &firstAlphaCount, &firstAlphaCapacity, "TK_EPS");
        for (int i = 0; i < firstAlphaCount; i++) {
            if (strcmp(firstAlpha[i], "TK_EPS") != 0) {
                int termIndex = getTerminalIndex(firstAlpha[i]);
                if (termIndex != -1) {
                    printf(" Adding rule: %s -> %s at parseTable[%d][%d]\n",
                           rule->lhs, rule->rhs[0], A_index, termIndex);
                    table[A_index][termIndex] = prod;
                }
            }
        }
        if (containsSymbol(firstAlpha, firstAlphaCount, "TK_EPS")) {
            for (int i = 0; i < ffArr[A_index].followCount; i++) {
                int termIndex = getTerminalIndex(ffArr[A_index].follow[i]);
                if (termIndex != -1)
                    table[A_index][termIndex] = prod;
            }
        }
        for (int i = 0; i < firstAlphaCount; i++)
            free(firstAlpha[i]);
        free(firstAlpha);
    }
}

/*-----------------------------------------
   Parse Tree and Parser Functions
-----------------------------------------*/
ParseTreeNode* createParseTreeNode(const char *symbol) {
    ParseTreeNode *node = malloc(sizeof(ParseTreeNode));
    if (node == NULL) {
        fprintf(stderr, "ERROR: Memory allocation failed for parse tree node\n");
        exit(EXIT_FAILURE);
    }
    if (symbol == NULL) {
        fprintf(stderr, "ERROR: Attempting to create a parse node with NULL symbol\n");
        free(node);
        return NULL;
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


void freeParseTree(ParseTreeNode *root) {
    if (root == NULL)
        return;
    if (root->children != NULL) {
        for (int i = 0; i < root->numChildren; i++) {
            if (((ParseTreeNode **)root->children)[i] != NULL)
                freeParseTree(((ParseTreeNode **)root->children)[i]);
        }
        free(root->children);
        root->children = NULL;
    }
    free(root);
}
ParseTreeNode* parseInputSourceCode(char *testcaseFile, int parseTable[TOTAL_NON_TERMINALS][NUM_TERMINALS], Grammar *G) {
    init_lexer(testcaseFile);
    ParseTreeNode *root = createParseTreeNode("program");
    Stack *stack = createStack();
    push(stack, createStackNode("$", NULL));
    push(stack, createStackNode("program", root));
    
    tokenInfo token = getNextToken(&twinBuffer);
    // Skip any initial comment tokens.
    while (token.token == TK_COMMENT)
        token = getNextToken(&twinBuffer);
    
    while (!isStackEmpty(stack)) {

        while (token.token == TK_COMMENT)
            token = getNextToken(&twinBuffer);
        
        StackNode *top = peek(stack);
        char *X = top->symbol;
        if (isTerminal(X) || strcmp(X, "$") == 0) {
            if (strcmp(X, tokenStrings[token.token]) == 0) {
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
           
            if (strcmp(X, "option_single_constructed") == 0 && token.token != TK_DOT) {
                pop(stack);
                continue;
            }
            
            int nonTerminalIndex = getNonTerminalIndex(X);
            int tokenIndex = getTerminalIndex(tokenStrings[token.token]);
            if (nonTerminalIndex < 0 || tokenIndex < 0) {
                fprintf(stderr, "Syntax error at line %d: invalid non-terminal %s or token %s\n",
                        token.lineNumber, X, tokenStrings[token.token]);
                freeParseTree(root);
                exit(EXIT_FAILURE);
                return NULL;
            }
            int prod = parseTable[nonTerminalIndex][tokenIndex];
            if (prod == -1) {
                fprintf(stderr, "Syntax error at line %d: no rule for non-terminal %s with token %s\n",
                        token.lineNumber, X, tokenStrings[token.token]);
                freeParseTree(root);
                return NULL;
            }
            GrammarRule *rule = G->rules[prod];
            ParseTreeNode *parentNode = top->node;
            pop(stack);
            // If the production is an epsilon production, set children to NULL.
            if (rule->rhsCount == 1 && strcmp(rule->rhs[0], "TK_EPS") == 0) {
                parentNode->children = NULL;
                parentNode->numChildren = 0;
                continue;
            } else {
                int numSymbols = rule->rhsCount;
                ParseTreeNode **childNodes = malloc(numSymbols * sizeof(ParseTreeNode *));
                for (int i = 0; i < numSymbols; i++) {
                    childNodes[i] = createParseTreeNode(rule->rhs[i]);
                }
                parentNode->children = (ParseTreeNode *)childNodes;
                parentNode->numChildren = numSymbols;
                // Push children in reverse order onto the stack.
                for (int i = numSymbols - 1; i >= 0; i--) {
                    push(stack, createStackNode(rule->rhs[i], childNodes[i]));
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



void printNodeInfo(ParseTreeNode *node, FILE *fp, int nodeCounter) {
    char lexemeStr[20], tokenNameStr[20], valueStr[20], parentStr[20], isLeafStr[5], nodeSymbolStr[20];
    if (node->isLeafNode && node->lexeme != NULL)
        snprintf(lexemeStr, sizeof(lexemeStr), "%s", node->lexeme);
    else
        snprintf(lexemeStr, sizeof(lexemeStr), "----");
    int currNodeNum = nodeCounter;
    int lineNo = node->lineno;
    if (node->isLeafNode && node->tokenName != NULL)
        snprintf(tokenNameStr, sizeof(tokenNameStr), "%s", node->tokenName);
    else
        snprintf(tokenNameStr, sizeof(tokenNameStr), "----");
    if (node->isLeafNode && node->tokenName != NULL &&
       (strcmp(node->tokenName, "TK_NUM") == 0 || strcmp(node->tokenName, "TK_RNUM") == 0))
        snprintf(valueStr, sizeof(valueStr), "%.2lf", node->value);
    else
        snprintf(valueStr, sizeof(valueStr), "----");
    if (node->parentNodeSymbol != NULL)
        snprintf(parentStr, sizeof(parentStr), "%s", node->parentNodeSymbol);
    else
        snprintf(parentStr, sizeof(parentStr), "ROOT");
    snprintf(isLeafStr, sizeof(isLeafStr), "%s", node->isLeafNode ? "yes" : "no");
    if (!node->isLeafNode && node->nodeSymbol != NULL)
        snprintf(nodeSymbolStr, sizeof(nodeSymbolStr), "%s", node->nodeSymbol);
    else
        snprintf(nodeSymbolStr, sizeof(nodeSymbolStr), "----");
    fprintf(fp, "%-15s %-5d %-5d %-15s %-15s %-15s %-5s %-15s\n",
            lexemeStr, currNodeNum, lineNo, tokenNameStr, valueStr, parentStr, isLeafStr, nodeSymbolStr);
}

void printParseTreeHelper(ParseTreeNode *node, FILE *fp, int *counter) {
    if (node == NULL)
        return;
    if (node->numChildren > 0) {
        int mid = node->numChildren / 2;
        ParseTreeNode **childArray = (ParseTreeNode **)node->children;
        for (int i = 0; i < mid; i++) {
            printParseTreeHelper(childArray[i], fp, counter);
        }
        (*counter)++;
        printNodeInfo(node, fp, *counter);
        for (int i = mid; i < node->numChildren; i++) {
            printParseTreeHelper(childArray[i], fp, counter);
        }
    } else {
        (*counter)++;
        printNodeInfo(node, fp, *counter);
    }
}

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
    fprintf(fp, "%-15s %-5s %-5s %-15s %-15s %-15s %-5s %-15s\n",
            "Lexeme", "Node#", "Lineno", "TokenName", "Value", "Parent", "Leaf", "NodeSymbol");
    fprintf(fp, "-----------------------------------\n");
    int nodeCounter = 0;
    printParseTreeHelper(PT->root, fp, &nodeCounter);
    fclose(fp);
}
