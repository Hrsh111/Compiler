#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "lexerDef.h"
#include "lexer.h"

/* Global variables */
int lineCount = 1;
char *forward;
char *lexemebegin;
int activeBuffer;
FILE *srcFile = NULL;
bool exhaustedInput;
bool ldfirstBuff;
bool ldsecondBuff;
TwinBuffer twinBuffer;

Symboltable *table = NULL;
keyword *kwEntries[KC] = {NULL};



FILE *initialise(char *inputFile) {
    srcFile = fopen(inputFile, "r");  
    if (srcFile == NULL) {
        printf("ERROR! Could not open file: %s\n", inputFile);
        return NULL;
    }


    int size = (int)fread(twinBuffer.buffer1, sizeof(char), BUFFER_SIZE, srcFile);
    if (size == 0 && ferror(srcFile)) {
        printf("ERROR: Failed to read from file: %s\n", inputFile);
        fclose(srcFile);
        return NULL;
    }


    if (size < BUFFER_SIZE) {
        if (size >= 0)
            twinBuffer.buffer1[size] = '\0'; 
        exhaustedInput = true;
    } else {
        exhaustedInput = false;
    }

    activeBuffer = 1;
    ldfirstBuff = true;
    ldsecondBuff = false;
    lexemebegin = twinBuffer.buffer1;
    forward = twinBuffer.buffer1;

    initializeSymbolTable(&table);

    printf("File %s opened and initialized successfully.\n", inputFile);
    return srcFile;
}

FILE *getStream(FILE *fp) {
    if (exhaustedInput)
        return NULL;

    if (activeBuffer == 1) {
        size_t bytesRead = fread(twinBuffer.buffer2, sizeof(char), BUFFER_SIZE, fp);
        if (bytesRead < BUFFER_SIZE) {
            twinBuffer.buffer2[bytesRead] = '\0';
            exhaustedInput = true;
        } else {
            twinBuffer.buffer2[BUFFER_SIZE - 1] = '\0';
        }
        forward = twinBuffer.buffer2;
        activeBuffer = 2;
        ldsecondBuff = true;
    } else {
        size_t bytesRead = fread(twinBuffer.buffer1, sizeof(char), BUFFER_SIZE, fp);
        if (bytesRead < BUFFER_SIZE) {
            twinBuffer.buffer1[bytesRead] = '\0';
            exhaustedInput = true;
        } else {
            twinBuffer.buffer1[BUFFER_SIZE - 1] = '\0';
        }
        forward = twinBuffer.buffer1;
        activeBuffer = 1;
        ldfirstBuff = true;
    }
    return fp;
}


void initializeKeywords() {
  
    static keyword reservedList[] = {
        {"identifier", TK_ID},
        {"_main", TK_MAIN},
        {"as", TK_AS},
        {"call", TK_CALL},
        {"definetype", TK_DEFINETYPE},
        {"else", TK_ELSE},
        {"end", TK_END},
        {"endif", TK_ENDIF},
        {"endrecord", TK_ENDRECORD},
        {"endunion", TK_ENDUNION},
        {"endwhile", TK_ENDWHILE},
        {"global", TK_GLOBAL},
        {"if", TK_IF},
        {"input", TK_INPUT},
        {"int", TK_INT},
        {"list", TK_LIST},
        {"output", TK_OUTPUT},
        {"parameters", TK_PARAMETERS},
        {"parameter", TK_PARAMETER},
        {"read", TK_READ},
        {"real", TK_REAL},
        {"record", TK_RECORD},
        {"return", TK_RETURN},
        {"then", TK_THEN},
        {"type", TK_TYPE},
        {"union", TK_UNION},
        {"while", TK_WHILE},
        {"with", TK_WITH},
        {"write", TK_WRITE}
    };
    int numKeywords = sizeof(reservedList) / sizeof(keyword);
    for (int i = 0; i < numKeywords; i++) {
        kwEntries[i] = (keyword *)malloc(sizeof(keyword));
        if (kwEntries[i] == NULL) {
            fprintf(stderr, "error allocating mem for keyword %d\n", i);
            exit(EXIT_FAILURE);
        }
        kwEntries[i]->keyword = strdup(reservedList[i].keyword);
        kwEntries[i]->token = reservedList[i].token;
    }
}

void initializeSymbolTable(Symboltable **table) {
    *table = (Symboltable *)calloc(1, sizeof(Symboltable));
    if (*table == NULL) {
        fprintf(stderr, "failed memory allocation for symbol table.\n");
        exit(EXIT_FAILURE);
    }
    initializeKeywords();


    int numKeywords = sizeof(kwEntries) / sizeof(kwEntries[0]);
    for (int i = 0; i < numKeywords; i++) {
        if (kwEntries[i] != NULL) {
            insert(kwEntries[i]->keyword, kwEntries[i]->token);
        }
    }
}

int CalHash(const char *lexeme) {
    unsigned long h = 5381;
    int c;
    while ((c = *lexeme++))
        h = ((h << 5) + h) + c;
    return h % HASH_TABLE_SIZE;
}

bool lookup(const char *lexeme) {
    int index = CalHash(lexeme);
    SymbolItem *current = table->data[index];
    while (current != NULL) {
        if (strcmp(current->lexeme, lexeme) == 0) {
            return true;
        }
        current = current->next;
    }
    return false;
}

void insert(const char *lexeme, terminals token) {
    int index = CalHash(lexeme);
    if (!lookup(lexeme)) {
        SymbolItem *newItem = (SymbolItem *)calloc(1, sizeof(SymbolItem));
        if (newItem == NULL) {
            fprintf(stderr, "Memory allocation failed for SymbolItem.\n");
            exit(EXIT_FAILURE);
        }
        newItem->lexeme = strdup(lexeme);
        if (newItem->lexeme == NULL) {
            fprintf(stderr, "Memory allocation failed for lexeme.\n");
            exit(EXIT_FAILURE);
        }
        newItem->token = token;
        newItem->next = NULL;
        if (table->data[index] == NULL) {
            table->data[index] = newItem;
        } else {
            SymbolItem *current = table->data[index];
            while (current->next != NULL) {
                current = current->next;
            }
            current->next = newItem;
        }
        table->currentsize++;
    } else {
        printf("Lexeme %s already exists in the symbol table.\n", lexeme);
    }
}


char fetchNextChar(void) {

    if (activeBuffer == 1 && forward >= twinBuffer.buffer1 + BUFFER_SIZE) {
        getStream(srcFile);
    } else if (activeBuffer == 2 && forward >= twinBuffer.buffer2 + BUFFER_SIZE) {
        getStream(srcFile);
    }
    char c = *forward;
    if (c == '\0') 
        return EOF;
    forward++; 
    return c;
}

void retract(void) {
    if (forward > lexemebegin) {
        forward--;
    }
}

void setBeginToForward(TwinBuffer *B) {
    (void)B; 
    lexemebegin = forward;
}

void stripComments(char *testcaseFile, char *cleanFile) {
    FILE *inFile = fopen(testcaseFile, "r");
    if (inFile == NULL) {
        fprintf(stderr, "Error: Could not open input file %s\n", testcaseFile);
        exit(EXIT_FAILURE);
    }
    FILE *outFile = fopen(cleanFile, "w");
    if (outFile == NULL) {
        fprintf(stderr, "Error: Could not open output file %s\n", cleanFile);
        fclose(inFile);
        exit(EXIT_FAILURE);
    }
    int c;
    bool inComment = false;
    while ((c = fgetc(inFile)) != EOF) {
        if (c == '%') {
            inComment = true;
        }
        if (c == '\n') {
            fputc('\n', outFile);
            inComment = false;
        } else if (!inComment) {
            fputc(c, outFile);
        }
    }
    fclose(inFile);
    fclose(outFile);
}


void ignoreComment(TwinBuffer *B) {
    (void)B; 
    char c;

    while ((c = *forward) != '\n' && c != '\0') {
        fetchNextChar(); 
    }

    if (*forward == '\n') {
        fetchNextChar();
        lineCount++;
    }
}

void createToken(tokenInfo *tk, terminals tokenType, int line, const char *lexeme) {
    tk->token = tokenType;
    strncpy(tk->lexeme, lexeme, sizeof(tk->lexeme) - 1);
    tk->lexeme[sizeof(tk->lexeme) - 1] = '\0';
    tk->lineNumber = line;
}

terminals checkKeyword(const char *lexeme) {
    if (strcmp(lexeme, "with") == 0)
        return TK_WITH;
        if(strcmp(lexeme, "endwhile")==0)
        return TK_ENDWHILE;
    if (strcmp(lexeme, "parameter") == 0)
        return TK_PARAMETER;
    if (strcmp(lexeme, "parameters") == 0)
        return TK_PARAMETERS;
    if (strcmp(lexeme, "end") == 0)
        return TK_END;
    if (strcmp(lexeme, "while") == 0)
        return TK_WHILE;
    if (strcmp(lexeme, "union") == 0)
        return TK_UNION;
    if (strcmp(lexeme, "definetype") == 0)
        return TK_DEFINETYPE;
    if (strcmp(lexeme, "as") == 0)
        return TK_AS;
    if (strcmp(lexeme, "type") == 0)
        return TK_TYPE;
    if (strcmp(lexeme, "_main") == 0)
        return TK_MAIN;
    if (strcmp(lexeme, "endunion") == 0)
        return TK_ENDUNION;
    if (strcmp(lexeme, "global") == 0)
        return TK_GLOBAL;
    if (strcmp(lexeme, "if") == 0)
        return TK_IF;
    if (strcmp(lexeme, "list") == 0)
        return TK_LIST;
    if (strcmp(lexeme, "then") == 0)
        return TK_THEN;
    if (strcmp(lexeme, "endif") == 0)
        return TK_ENDIF;
    if (strcmp(lexeme, "read") == 0)
        return TK_READ;
    if (strcmp(lexeme, "write") == 0)
        return TK_WRITE;
    if (strcmp(lexeme, "return") == 0)
        return TK_RETURN;
    if (strcmp(lexeme, "record") == 0)
        return TK_RECORD;
    if (strcmp(lexeme, "endrecord") == 0)
        return TK_ENDRECORD;
    if (strcmp(lexeme, "else") == 0)
        return TK_ELSE;
    if (strcmp(lexeme, "call") == 0)
        return TK_CALL;
    if (strcmp(lexeme, "real") == 0)  
        return TK_REAL;        
        if (strcmp(lexeme, "input") == 0)
        return TK_INPUT;  
        if(strcmp(lexeme, "int")==0)
        return TK_INT;
    if (strcmp(lexeme, "output") == 0)
        return TK_OUTPUT;     
    return TK_ERROR;
}




   tokenInfo getNextToken(TwinBuffer *B) {
    tokenInfo t;
    char lexeme[256];
    memset(lexeme, 0, sizeof(lexeme));
    int counter = 0;
    int state = 0;
    char c = '\0';

    while (1) {
        switch (state) {
            case -1:

                createToken(&t, TK_ERROR, lineCount, lexeme);
                setBeginToForward(B);
                return t;

            case 0:
                c = fetchNextChar();
                if (c == EOF) {
                    createToken(&t, END_OF_INPUT, lineCount, "EOF");
                    setBeginToForward(B);
                    return t;
                }
                lexeme[counter++] = c;
                if (c == '%')
                    state = 1;
                else if (c == ']')
                    state = 2;
                else if (c == '[')
                    state = 3;
                else if (c == ',')
                    state = 4;
                else if (c == ';')
                    state = 5;
                else if (c == ':')
                    state = 6;
                else if (c == '.')
                    state = 7;
                else if (c == '(')
                    state = 8;
                else if (c == ')')
                    state = 9;
                else if (c == '+')
                    state = 10;
                else if (c == '-')
                    state = 11;
                else if (c == '*')
                    state = 12;
                else if (c == '/')
                    state = 13;
                else if (c == '~')
                    state = 14;
                else if (c == '!')
                    state = 15;
                else if (c == '&')
                    state = 17;
                else if (c == '@')
                    state = 20;
                else if (c == ' ' || c == '\t' || c == '\n' || c == '\0' || c == EOF)
                    state = 23;
                else if (c == '>')
                    state = 24;
                else if (c == '<')
                    state = 27; 
                else if (c == '#')
                    state = 33;
                else if (c == '=')
                    state = 36;
                else if (isdigit(c))
                    state = 38;
                else if (c == 'b' || c == 'c' || c == 'd')
                    state = 48;
                else if (c >= 'a' && c <= 'z')
                    state = 54;
                else if (c == '_')
                    state = 56;
                else
                    state = -1;
                break;


            case 1:
                createToken(&t, TK_COMMENT, lineCount, lexeme);
                ignoreComment(B);
                setBeginToForward(B);
                return t;


            case 2:
                createToken(&t, TK_SQR, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            case 3:
                createToken(&t, TK_SQL, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            case 4:
                createToken(&t, TK_COMMA, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            case 5:
                createToken(&t, TK_SEM, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            case 6:
                createToken(&t, TK_COLON, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            case 7:
                createToken(&t, TK_DOT, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            case 8:
                createToken(&t, TK_OP, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            case 9:
                createToken(&t, TK_CL, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            case 10:
                createToken(&t, TK_PLUS, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            case 11:
                createToken(&t, TK_MINUS, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            case 12:
                createToken(&t, TK_MUL, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            case 13:
                createToken(&t, TK_DIV, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            case 14:
                createToken(&t, TK_NOT, lineCount, lexeme);
                setBeginToForward(B);
                return t;

            case 15:
                c = fetchNextChar();
                lexeme[counter++] = c;
                if (c == '=') {
                    createToken(&t, TK_NE, lineCount, lexeme);
                    setBeginToForward(B);
                } else {
                    createToken(&t, TK_ERROR, lineCount, lexeme);
                }
                return t;


            case 17: {
                c = fetchNextChar();
                if (c == '&') {
                     lexeme[counter++] = c;
                     state = 18;
                } else {
                     retract();
                     lexeme[counter] = '\0';
                     createToken(&t, TK_ERROR, lineCount, lexeme);
                     setBeginToForward(B);
                     return t;
                }
                break;
            }
            case 18: {
                c = fetchNextChar();
                if (c == '&') {
                     lexeme[counter++] = c;
                     lexeme[counter] = '\0';
                     createToken(&t, TK_AND, lineCount, lexeme);
                     setBeginToForward(B);
                     return t;
                } else {
                     retract();
                     lexeme[counter] = '\0';
                     createToken(&t, TK_ERROR, lineCount, lexeme);
                     setBeginToForward(B);
                     return t;
                }
                break;
            }


            case 20:
                c = fetchNextChar();
                lexeme[counter++] = c;
                if (c == '@')
                    state = 21;
                else
                    state = -1;
                break;
            case 21:
                c = fetchNextChar();
                lexeme[counter++] = c;
                if (c == '@')
                    state = 22;
                else
                    state = -1;
                break;
            case 22:
                createToken(&t, TK_OR, lineCount, lexeme);
                setBeginToForward(B);
                return t;

            case 23:
                if (c == '\n') {
                    lineCount++;
                }
                setBeginToForward(B);
                counter = 0;
                memset(lexeme, 0, sizeof(lexeme));
                state = 0;
                break;


            case 24:
                c = fetchNextChar();
                lexeme[counter++] = c;
                if (c == '=')
                    state = 25;
                else
                    state = 26;
                break;
            case 25:
                createToken(&t, TK_GE, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            case 26:
                lexeme[counter - 1] = '\0';
                createToken(&t, TK_GT, lineCount, lexeme);
                retract();
                setBeginToForward(B);
                return t;


            case 27:
                c = fetchNextChar();
                if (c == '=') {
                    lexeme[counter++] = c;
                    createToken(&t, TK_LE, lineCount, lexeme);
                    setBeginToForward(B);
                    return t;
                } else if (c == '-') {
                    lexeme[counter++] = c;

                    c = fetchNextChar();
                    if (c == '-') {
                        lexeme[counter++] = c;
                        c = fetchNextChar();
                        if (c == '-') {
                            lexeme[counter++] = c;
                            createToken(&t, TK_ASSIGNOP, lineCount, lexeme);
                            setBeginToForward(B);
                            return t;
                        } else {

                            retract();
                            state = -1;
                        }
                    } else {

                        retract();
                        state = -1;
                    }
                } else {
                    retract();
                    state = 32;
                }
                break;
            case 32:
                lexeme[counter] = '\0';
                createToken(&t, TK_LT, lineCount, lexeme);
                setBeginToForward(B);
                return t;


            case 33:
                c = fetchNextChar();
                lexeme[counter++] = c;
                if (islower(c))
                    state = 34;
                else
                    state = -1;
                break;
            case 34:
                c = fetchNextChar();
                lexeme[counter++] = c;
                if (isalpha(c))
                    state = 34;
                else
                    state = 35;
                break;
            case 35:
                lexeme[counter - 1] = '\0';
                createToken(&t, TK_RUID, lineCount, lexeme);
                retract();
                setBeginToForward(B);
                return t;


            case 36:
                c = fetchNextChar();
                lexeme[counter++] = c;
                if (c == '=')
                    state = 37;
                else
                    state = -1;
                break;
            case 37:
                createToken(&t, TK_EQ, lineCount, lexeme);
                setBeginToForward(B);
                return t;


            case 38: {
                c = fetchNextChar();
                if (isdigit(c)) {
                     lexeme[counter++] = c;
                     state = 38;
                } else if (c == '.') {
                     lexeme[counter++] = c;
                     state = 39;
                } else {
                     retract();
                     lexeme[counter] = '\0';
                     createToken(&t, TK_NUM, lineCount, lexeme);
                     setBeginToForward(B);
                     return t;
                }
                break;
            }
            case 39: {
                c = fetchNextChar();
                if (isdigit(c)) {
                     lexeme[counter++] = c;
                     state = 40;
                } else {
                     retract();
                     lexeme[counter] = '\0';
                     createToken(&t, TK_ERROR, lineCount, lexeme);
                     setBeginToForward(B);
                     return t;
                }
                break;
            }
            case 40: {
                c = fetchNextChar();
                if (isdigit(c)) {
                     lexeme[counter++] = c;
                     char look = fetchNextChar();
                     if (look == 'E' || look == 'e') {
                          lexeme[counter++] = look;
                          state = 42;
                     } else {
                          retract();
                          lexeme[counter] = '\0';
                          createToken(&t, TK_RNUM, lineCount, lexeme);
                          setBeginToForward(B);
                          return t;
                     }
                } else {
                     retract();
                     lexeme[counter] = '\0';
                     createToken(&t, TK_ERROR, lineCount, lexeme);
                     setBeginToForward(B);
                     return t;
                }
                break;
            }
            case 41:
                lexeme[counter - 1] = '\0';
                createToken(&t, TK_NUM, lineCount, lexeme);
                retract();
                setBeginToForward(B);
                return t;
            case 42: {
                c = fetchNextChar();
                if (c == '+' || c == '-') {
                     lexeme[counter++] = c;
                     state = 43;
                } else if (isdigit(c)) {
                     lexeme[counter++] = c;
                     state = 44;
                } else {
                     state = -1;
                }
                break;
            }
            case 43: {
                c = fetchNextChar();
                if (isdigit(c)) {
                     lexeme[counter++] = c;
                     state = 44;
                } else {
                     state = -1;
                }
                break;
            }
            case 44: {
                c = fetchNextChar();
                if (isdigit(c)) {
                     lexeme[counter++] = c;
                     (void)fetchNextChar(); 
                     retract();  
                     lexeme[counter] = '\0';
                     createToken(&t, TK_RNUM, lineCount, lexeme);
                     setBeginToForward(B);
                     return t;
                } else {
                     state = -1;
                }
                break;
            }
            case 46:
                createToken(&t, TK_RNUM, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            case 47:
                lexeme[counter - 1] = '\0';
                createToken(&t, TK_RNUM, lineCount, lexeme);
                retract();
                setBeginToForward(B);
                return t;
            

            case 48:
                c = fetchNextChar();
                if (islower(c)) {
                    lexeme[counter++] = c;
                    state = 49;
                } else if (isdigit(c) && c != '0' && c != '1' && c != '8' && c != '9') {
                    lexeme[counter++] = c;
                    state = 51;
                } else {
                    retract();
                    state = 50;
                }
                break;
            case 49:
                c = fetchNextChar();
                if (islower(c)) {
                    lexeme[counter++] = c;
                    state = 49;
                } else {
                    retract();
                    state = 50;
                }
                break;
            case 50:
                lexeme[counter] = '\0';
                if (strlen(lexeme) > 20) {
                    createToken(&t, TK_ERROR, lineCount,
                        "Error: Variable Identifier too long.");
                    setBeginToForward(B);
                    return t;
                }
                if (checkKeyword(lexeme) != TK_ERROR)
                    createToken(&t, checkKeyword(lexeme), lineCount, lexeme);
                else
                    createToken(&t, TK_ID, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            case 51:
                c = fetchNextChar();
                lexeme[counter++] = c;
                if (c == 'b' || c == 'c' || c == 'd')
                    state = 51;
                else if (isdigit(c) && c != '0' && c != '1' && c != '8' && c != '9')
                    state = 53;
                else
                    state = 52;
                break;
            case 52:
                lexeme[counter - 1] = '\0';
                if (strlen(lexeme) > 20) {
                     createToken(&t, TK_ERROR, lineCount,
                        "Error: Variable Identifier is longer than the prescribed length of 20 characters.");
                     setBeginToForward(B);
                     return t;
                }
                createToken(&t, TK_ID, lineCount, lexeme);
                retract();
                setBeginToForward(B);
                return t;
            case 53:
                c = fetchNextChar();
                lexeme[counter++] = c;
                if (isdigit(c) && c != '0' && c != '1' && c != '8' && c != '9')
                    state = 53;
                else
                    state = 52;
                break;
            case 54: {
                c = fetchNextChar();
                if (c >= 'a' && c <= 'z') {
                     lexeme[counter++] = c;
                     state = 54;
                } else {
                     retract();
                     state = 55;
                }
                break;
            }
            case 55: {
                lexeme[counter] = '\0';
                while (counter > 0 && isspace((unsigned char)lexeme[counter - 1])) {
                    counter--;
                    lexeme[counter] = '\0';
                }
                if (counter > 0 && lexeme[counter - 1] == ')') {
                    counter--;
                    lexeme[counter] = '\0';
                    retract();
                }
                if (strlen(lexeme) > 20) {
                     createToken(&t, TK_ERROR, lineCount,
                        "Error: Variable Identifier is longer than the prescribed length of 20 characters.");
                     setBeginToForward(B);
                     return t;
                }
                terminals maybeKeyword = checkKeyword(lexeme);
                if (maybeKeyword != TK_ERROR)
                     createToken(&t, maybeKeyword, lineCount, lexeme);
                else
                     createToken(&t, TK_ID, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            }

            case 56:
                c = fetchNextChar();
                lexeme[counter++] = c;
                if (isalpha(c))
                    state = 57;
                else
                    state = -1;
                break;
            case 57:
                c = fetchNextChar();
                lexeme[counter++] = c;
                if (isalpha(c))
                    state = 57;
                else if (isdigit(c))
                    state = 58;
                else
                    state = 59;
                break;
            case 58:
                c = fetchNextChar();
                lexeme[counter++] = c;
                if (isdigit(c))
                    state = 58;
                else
                    state = 59;
                break;
            case 59:
                retract();
                lexeme[counter - 1] = '\0';
                setBeginToForward(B);
                if (checkKeyword(lexeme) != TK_ERROR)
                    createToken(&t, checkKeyword(lexeme), lineCount, lexeme);
                else
                    createToken(&t, TK_FUNID, lineCount, lexeme);
                return t;

            default:
                state = -1;
                break;
        } 
    } 
}