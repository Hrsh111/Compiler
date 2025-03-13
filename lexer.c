#include <stdio.h>
#include "lexerDef.h"
#include "lexer.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
int lineCount = 1;
char *forward;
char *lexemebegin;
int activeBuffer;
FILE *srcFile;
bool exhaustedInput;
bool ldfirstBuff;
bool ldsecondBuff;
TwinBuffer twinBuffer;

Symboltable *table = NULL;      
keyword *kwEntries[KC] = {NULL}; 

FILE *initialise(char *inputFile)
{
    srcFile = fopen(inputFile, "r");
    if (srcFile == NULL)
    {
        printf("ERROR! File not opened.\n");
        return NULL;
    }


    int size = fread(twinBuffer.buffer1, sizeof(char), BUFFER_SIZE, srcFile);
    if (size < BUFFER_SIZE)
    {
        twinBuffer.buffer1[size] = EOF;
        exhaustedInput = true;
    }
    else
    {
        exhaustedInput = false;
    }


    activeBuffer = 1;
    ldfirstBuff = true;
    ldsecondBuff = false;
    lexemebegin = twinBuffer.buffer1;
    forward = twinBuffer.buffer1;

    initializeSymbolTable(&table);

    return srcFile;
}
FILE *getStream(FILE *fp)
{
    if (exhaustedInput)
        return NULL; 

    if (activeBuffer == 1)
    {
       
        size_t bytesRead = fread(twinBuffer.buffer2, sizeof(char), BUFFER_SIZE, fp);
        if (bytesRead < BUFFER_SIZE)
        {
            twinBuffer.buffer2[bytesRead] = EOF;
            exhaustedInput = true;
        }
        forward = twinBuffer.buffer2; 
        activeBuffer = 2;
        ldsecondBuff = true;
    }
    else
    {

        size_t bytesRead = fread(twinBuffer.buffer1, sizeof(char), BUFFER_SIZE, fp);
        if (bytesRead < BUFFER_SIZE)
        {
            twinBuffer.buffer1[bytesRead] = EOF;
            exhaustedInput = true;
        }
        forward = twinBuffer.buffer1;
        activeBuffer = 1;
        ldfirstBuff = true;
    }

    return fp;
}

void initializeKeywords()
{
    static keyword reservedList[] = {
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
                                     {"write", TK_WRITE}};
    for (int i = 0; i < KC; i++)
    {
        kwEntries[i] = (keyword *)malloc(sizeof(keyword));

        if (kwEntries[i] == NULL)
        {
            fprintf(stderr, "error allocating mem for %d\n", i);
            exit(EXIT_FAILURE);
        }

        kwEntries[i]->keyword = strdup(reservedList[i].keyword);
        kwEntries[i]->token = reservedList[i].token;
    }
}
// initializing the symbol table use chain hashing
void initializeSymbolTable(Symboltable **table)
{
    *table = (Symboltable *)calloc(1, sizeof(Symboltable));
    if (*table == NULL)
    {
        fprintf(stderr, "failed memory allocation for symbol table.\n");
        exit(EXIT_FAILURE);
    }
    initializeKeywords();

    for (int i = 0; i < KC; i++)
    {
        insert(kwEntries[i]->keyword, kwEntries[i]->token);
    }
}
// djb2 algorithm
int CalHash(const char *lexeme)
{
    unsigned long h = 5381;
    int c;
    while ((c = *lexeme++))
        // left shit by 5 bits(2^5=32)
        // h=h*(32+1)+c
        // h*33+c
        h = ((h << 5) + h) + c;
    return h % HASH_TABLE_SIZE;
}

bool lookup(const char *lexeme)
{
    int index = CalHash(lexeme);
    SymbolItem *current = table->data[index];

    while (current != NULL)
    {
        if (strcmp(current->lexeme, lexeme) == 0)
        {
            return true;
        }
        current = current->next;
    }
    return false;
}

void insert(const char *lexeme, terminals token)
{
    int index = CalHash(lexeme);
    if (!lookup(lexeme))
    {
        SymbolItem *newItem = (SymbolItem *)calloc(1, sizeof(SymbolItem));
        if (newItem == NULL)
        {
            fprintf(stderr, "Memory allocation failed for SymbolItem.\n");
            exit(EXIT_FAILURE);
        }
        newItem->lexeme = strdup(lexeme);
        if (newItem->lexeme == NULL)
        {
            fprintf(stderr, "Memory allocation failed for lexeme.\n");
            exit(EXIT_FAILURE);
        }
        newItem->token = token;
        if (table->data[index] == NULL)
        {
            table->data[index] = newItem;
        }
        else
        {
            SymbolItem *current = table->data[index];
            while (current->next != NULL)
            {
                current = current->next;
            }
            current->next = newItem;
        }
        table->currentsize++;
    }
    else
    {
        printf("Lexeme %s already exists in the symbol table.\n", lexeme);
    }
}
char fetchNextChar(void)
{
    char c = *forward;
    if (c == '\0')
        return EOF;
    forward++;
    return c;
}

void retract()
{
    if (forward > lexemebegin)
    {
        forward--;
    }
}
void stripComments(char *testcaseFile, char *cleanFile)
{
    FILE *inFile = fopen(testcaseFile, "r");
    if (inFile == NULL)
    {
        fprintf(stderr, "Error: Could not open input file %s\n", testcaseFile);
        exit(EXIT_FAILURE);
    }

    FILE *outFile = fopen(cleanFile, "w");
    if (outFile == NULL)
    {
        fprintf(stderr, "Error: Could not open output file %s\n", cleanFile);
        fclose(inFile);
        exit(EXIT_FAILURE);
    }

    int c;
    bool inComment = false;

    while ((c = fgetc(inFile)) != EOF)
    {
        if (c == '%')
        {
            inComment = true;
        }

        if (c == '\n')
        {
            fputc('\n', outFile);
            inComment = false;
        }
        else if (!inComment)
        {
            fputc(c, outFile);
        }
    }

    fclose(inFile);
    fclose(outFile);
}
void ignoreComment(TwinBuffer *B)
{
    char c;
    printf("Entering ignoreComment at line %d. Starting char: '%c'\n", lineCount, *forward);
    while ((c = *forward) != '\n' && c != '\0' && c != EOF)
    {
        incrementForward(B);
    }
    printf("Exiting ignoreComment. Current char: '%c'\n", *forward);
}

void setBeginToForward(TwinBuffer *B)
{
    lexemebegin = forward;
}
void createToken(tokenInfo *tk, terminals tokenType, int line, const char *lexeme)
{
    tk->token = tokenType;

    strncpy(tk->lexeme, lexeme, sizeof(tk->lexeme) - 1);
    tk->lexeme[sizeof(tk->lexeme) - 1] = '\0';
    tk->lineNumber = line;
}

terminals checkKeyword(const char *lexeme)
{
    if (strcmp(lexeme, "main") == 0)
        return TK_MAIN;
    return TK_ID;
}

tokenInfo generateToken()
{
    tokenInfo token;
    int i = 0;

    memset(token.lexeme, 0, sizeof(token.lexeme));

    while (*forward == ' ' || *forward == '\t' || *forward == '\n')
    {
        if (*forward == '\n')
        {
            lineCount++;
        }
        forward++;
    }

    lexemebegin = forward;

    if (*forward == '\0')
    {
        token.token = END_OF_INPUT;
        strcpy(token.lexeme, "EOF");
        token.lineNumber = lineCount;
        return token;
    }

    if (isdigit(*forward))
    {

        while (isdigit(*forward))
        {
            token.lexeme[i++] = *forward;
            forward++;
        }

        if (*forward == '.')
        {
            token.lexeme[i++] = *forward;
            forward++;
            while (isdigit(*forward))
            {
                token.lexeme[i++] = *forward;
                forward++;
            }
            token.token = TK_RNUM;
        }
        else
        {
            token.token = TK_NUM;
        }
        token.lexeme[i] = '\0';
        token.lineNumber = lineCount;
        return token;
    }

    else if (isalpha(*forward))
    {
        while (isalnum(*forward) || *forward == '_')
        {
            token.lexeme[i++] = *forward;
            forward++;
        }
        token.lexeme[i] = '\0';
        token.lineNumber = lineCount;
        token.token = checkKeyword(token.lexeme);
        return token;
    }
    
    else
    {

        token.lexeme[0] = *forward;
        token.lexeme[1] = '\0';
        token.lineNumber = lineCount;

        switch (*forward)
        {
        case '+':
            token.token = TK_PLUS;
            break;
        case '-':
            token.token = TK_MINUS;
            break;
        case '*':
            token.token = TK_MUL;
            break;
        case '/':
            token.token = TK_DIV;
            break;
        case '=':
            token.token = TK_ASSIGNOP;
            break;
        case '>':
        {
            forward++;

            if (*forward == '=')
            {
                token.lexeme[1] = '=';
                token.lexeme[2] = '\0';
                token.token = TK_GE;
            }
            else
            {
                token.token = TK_GT;

                forward--;
            }
            break;
        }
        case '<':
        {
            forward++;

            if (*forward == '=')
            {
                token.lexeme[1] = '=';
                token.lexeme[2] = '\0';
                token.token = TK_LE;
            }
            else
            {
                token.token = TK_LT;
                forward--;
            }
            break;
        }

        default:
            token.token = TK_ERROR;
            break;
        }

        forward++;
        return token;
    }
}


int isKeyword(const char *lex)
{

    if (strcmp(lex, "with") == 0)
        return 1;
    if (strcmp(lex, "parameters") == 0)
        return 1;
    if (strcmp(lex, "end") == 0)
        return 1;
    if (strcmp(lex, "while") == 0)
        return 1;
    if (strcmp(lex, "union") == 0)
        return 1;
    if (strcmp(lex, "definetype") == 0)
        return 1;
    if (strcmp(lex, "as") == 0)
        return 1;
    if (strcmp(lex, "type") == 0)
        return 1;
    if (strcmp(lex, "_main") == 0)
        return 1;
   
    return 0;
}


terminals getKeywordToken(const char *lex)
{
    if (strcmp(lex, "with") == 0)
        return TK_WITH;
    if (strcmp(lex, "parameters") == 0)
        return TK_PARAMETERS;
    if (strcmp(lex, "end") == 0)
        return TK_END;
    if (strcmp(lex, "while") == 0)
        return TK_WHILE;
    if (strcmp(lex, "union") == 0)
        return TK_UNION;
    if (strcmp(lex, "definetype") == 0)
        return TK_DEFINETYPE;
    if (strcmp(lex, "as") == 0)
        return TK_AS;
    if (strcmp(lex, "type") == 0)
        return TK_TYPE;
    if (strcmp(lex, "_main") == 0)
        return TK_MAIN;
   
    return TK_ID; 
}
void incrementForward(TwinBuffer *B) {
    forward++; 
  
    if (activeBuffer == 1) {
        if (forward >= twinBuffer.buffer1 + BUFFER_SIZE) {
            getStream(srcFile);
        }
    } else {
        if (forward >= twinBuffer.buffer2 + BUFFER_SIZE) {
            getStream(srcFile);
        }
    }
}


void retractForward(TwinBuffer *B)
{
    if (forward > lexemebegin)
    {
        forward--;
    }
}
void flush(char *str)
{
    if (str)
        str[0] = '\0';
}

tokenInfo getNextToken(TwinBuffer *B)
{
    tokenInfo t;
    t.lexeme[0] = '\0';
    t.lineNumber = lineCount;


    char lexeme[100];
    memset(lexeme, '\0', sizeof(lexeme));
    int counter = 0;
    char c = '\0';
    terminals l;    
    short state = 0;

    while (1)
    {
        switch (state)
        {
        case -1:
            fprintf(stderr, "DFA error at line %d. Current lexeme: \"%s\", failing character: '%c'\n",
                    lineCount, lexeme, c);
            exit(1);
            break;
            case 0:
            {
                c = *forward;
                lexeme[counter++] = c;
                
                if (c == '%') {
                    state = 1;
                    incrementForward(B);
                } 
                else if (c == ']') {
                    state = 2;
                    incrementForward(B);
                }
                else if (c == '[') {
                    state = 3;
                    incrementForward(B);
                }
                else if (c == ',') {
                    state = 4;
                    incrementForward(B);
                }
                else if (c == ';') {
                    state = 5;
                    incrementForward(B);
                }
                else if (c == ':') {
                    state = 6;
                    incrementForward(B);
                }
                else if (c == '.') {
                    state = 7;
                    incrementForward(B);
                }
                else if (c == '(') {
                    state = 8;
                    incrementForward(B);
                }
                else if (c == ')') {
                    state = 9;
                    incrementForward(B);
                }
                else if (c == '+') {
                    state = 10;
                    incrementForward(B);
                }
                else if (c == '-') {
                    state = 11;
                    incrementForward(B);
                }
                else if (c == '*') {
                    state = 12;
                    incrementForward(B);
                }
                else if (c == '/') {
                    state = 13;
                    incrementForward(B);
                }
                else if (c == '~') {
                    state = 14;
                    incrementForward(B);
                }
                else if (c == '!') {
                    state = 15;
                    incrementForward(B);
                }
                else if (c == '&') {
                    state = 17;
                    incrementForward(B);
                }
                else if (c == '@') {
                    state = 20;
                    incrementForward(B);
                }
                else if (c == ' ' || c == '\t' || c == '\n' || c == '\0' || c == EOF) {
                    state = 23;

                }
                else if (c == '>') {
                    state = 24;
                    incrementForward(B);
                }
                else if (c == '<') {
                    state = 27;
                    incrementForward(B);
                }
                else if (c == '#') {
                    state = 33;
                    incrementForward(B);
                }
                else if (c == '=') {
                    state = 36;
                    incrementForward(B);
                }
                else if (isdigit(c)) {
                    state = 38;

                }
                else if (c == 'b' || c == 'c' || c == 'd') {
                    state = 48;
                   
                }
                else if (islower(c)) {
                    state = 54;
                    incrementForward(B);
                }
                else if (c == '_') {
                   
                    lexeme[counter++] = c;
                    incrementForward(B);
                    state = 56;
                }
                else {
                    state = -1;
                    incrementForward(B);
                }
                break;
            }
                

        case 1:
            // TK_COMMENT: skip the rest of the line
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
            c = *forward;
            if (c == '=')
            {
                lexeme[counter++] = c;
                createToken(&t, TK_NE, lineCount, lexeme);
                setBeginToForward(B);
            }
            else
            {
                state = -1;
            }
            return t;

        case 17:
            c = *forward;
            lexeme[counter++] = c;
            if (c == '&')
                state = 18;
            else
                state = -1;
            incrementForward(B);
            break;

        case 18:
            c = *forward;
            lexeme[counter++] = c;
            if (c == '&')
                state = 19;
            else
                state = -1;
            incrementForward(B);
            break;

        case 19:
            createToken(&t, TK_AND, lineCount, lexeme);
            setBeginToForward(B);
            return t;

        case 20:
            c = *forward;
            lexeme[counter++] = c;
            if (c == '@')
                state = 21;
            else
                state = -1;
            incrementForward(B);
            break;

        case 21:
            c = *forward;
            lexeme[counter++] = c;
            if (c == '@')
                state = 22;
            else
                state = -1;
            incrementForward(B);
            break;
            case 23:

            while (*forward == ' ' || *forward == '\t' || *forward == '\n') {
                printf("State 23: Skipping whitespace, current char: '%c'\n", *forward);
                if (*forward == '\n') {
                    lineCount++;
                }
                incrementForward(B);
            }

            printf("State 23: Finished skipping, current char: '%c' (line %d)\n", *forward, lineCount);
            

            if (*forward == '\0' || *forward == EOF) {
                createToken(&t, END_OF_INPUT, lineCount, lexeme);
                return t;
            }
            

            memset(lexeme, 0, sizeof(lexeme));
            counter = 0;
            setBeginToForward(B);
            state = 0;
            break;
        

        
        

        case 24:
            c = *forward;
            lexeme[counter++] = c;
            if (c == '=')
                state = 25;
            else
                state = 26;
            incrementForward(B);
            break;

        case 25:
            createToken(&t, TK_GE, lineCount, lexeme);
            setBeginToForward(B);
            return t;

        case 26:
            lexeme[counter - 1] = '\0';
            createToken(&t, TK_GT, lineCount, lexeme);
            retractForward(B);
            setBeginToForward(B);
            return t;

            case 27:
            c = *forward;
            lexeme[counter++] = c;
            printf("State 27: read '%c', lexeme so far: \"%s\"\n", c, lexeme);
            if (c == '=')
                state = 28;
            else if (c == '-')
                state = 29;
            else
                state = 32;
            incrementForward(B);
            break;
        

            case 28:

            lexeme[counter] = '\0';
            printf("State 28: Final lexeme before token creation: \"%s\"\n", lexeme);
            createToken(&t, TK_LE, lineCount, lexeme);
            setBeginToForward(B);
            return t;
        

        case 29:
            c = *forward;
            lexeme[counter++] = c;
            if (c == '-')
                state = 30;
            else
                state = 61;
            incrementForward(B);
            break;

        case 30:
            c = *forward;
            lexeme[counter++] = c;
            if (c == '-')
                state = 31;
            else
                state = -1;
            incrementForward(B);
            break;

        case 31:
            createToken(&t, TK_ASSIGNOP, lineCount, lexeme);
            setBeginToForward(B);
            return t;

        case 61:
            lexeme[counter - 1] = '\0';
            lexeme[counter - 2] = '\0';
            createToken(&t, TK_LT, lineCount, lexeme);
            retractForward(B);
            retractForward(B);
            setBeginToForward(B);
            return t;

        case 32:
            createToken(&t, TK_LT, lineCount, lexeme);
            retractForward(B);
            setBeginToForward(B);
            return t;

        case 33:
            c = *forward;
            lexeme[counter++] = c;
            if (islower(c))
                state = 34;
            else
                state = -1;
            incrementForward(B);
            break;

        case 34:
            c = *forward;
            lexeme[counter++] = c;
            if (isalpha(c))
                state = 34;
            else
                state = 35;
            incrementForward(B);
            break;

        case 35:
            lexeme[counter - 1] = '\0';

            if (isKeyword(lexeme))
                createToken(&t, getKeywordToken(lexeme), lineCount, lexeme);
            else
                createToken(&t, TK_RUID, lineCount, lexeme);
            retractForward(B);
            setBeginToForward(B);
            return t;

        case 36:
            c = *forward;
            lexeme[counter++] = c;
            if (c == '=')
                state = 37;
            else
                state = -1;
            incrementForward(B);
            break;

        case 37:
            createToken(&t, TK_EQ, lineCount, lexeme);
            setBeginToForward(B);
            return t;

        case 38:
            c = *forward;
            lexeme[counter++] = c;
            if (isdigit(c))
                state = 38;
            else if (c == '.')
                state = 39;
            else
                state = 41;
            incrementForward(B);
            break;

        case 39:
            c = *forward;
            lexeme[counter++] = c;
            if (isdigit(c))
                state = 40;
            else
                state = 60;
            incrementForward(B);
            break;

        case 60:
            lexeme[counter - 1] = '\0';
            lexeme[counter - 2] = '\0';
            createToken(&t, TK_NUM, lineCount, lexeme);
            retractForward(B);
            retractForward(B);
            setBeginToForward(B);
            return t;

        case 40:
            c = *forward;
            lexeme[counter++] = c;
            if (isdigit(c))
                state = 42;
            else
                state = -1;
            incrementForward(B);
            break;

        case 41:
            lexeme[counter - 1] = '\0';
            createToken(&t, TK_NUM, lineCount, lexeme);
            retractForward(B);
            setBeginToForward(B);
            return t;

        case 42:
            c = *forward;
            lexeme[counter++] = c;
            if (c == 'E')
                state = 43;
            else
                state = 47;
            incrementForward(B);
            break;

        case 43:
            c = *forward;
            lexeme[counter++] = c;
            if (c == '+' || c == '-')
                state = 44;
            else if (isdigit(c))
                state = 45;
            else
                state = -1;
            incrementForward(B);
            break;

        case 44:
            c = *forward;
            lexeme[counter++] = c;
            if (isdigit(c))
                state = 45;
            else
                state = -1;
            incrementForward(B);
            break;

        case 45:
            c = *forward;
            lexeme[counter++] = c;
            if (isdigit(c))
                state = 46;
            else
                state = -1;
            incrementForward(B);
            break;

        case 46:
            createToken(&t, TK_RNUM, lineCount, lexeme);
            setBeginToForward(B);
            return t;

        case 47:
            lexeme[counter - 1] = '\0';
            createToken(&t, TK_RNUM, lineCount, lexeme);
            retractForward(B);
            setBeginToForward(B);
            return t;

        case 48:
            c = *forward;
            lexeme[counter++] = c;
            if (islower(c))
                state = 49;
            else if (isdigit(c) && c != '0' && c != '1' && c != '8' && c != '9')
                state = 51;
            else
                state = 50;
            incrementForward(B);
            break;

        case 49:
            c = *forward;
            lexeme[counter++] = c;
            if (islower(c))
                state = 49;
            else
                state = 50;
            incrementForward(B);
            break;

        case 50:
            lexeme[counter - 1] = '\0';
            if (isKeyword(lexeme))
                createToken(&t, getKeywordToken(lexeme), lineCount, lexeme);
            else
                createToken(&t, TK_FIELDID, lineCount, lexeme);
            retractForward(B);
            setBeginToForward(B);
            return t;

        case 51:
            c = *forward;
            lexeme[counter++] = c;
            if (c == 'b' || c == 'c' || c == 'd')
                state = 51;
            else if (isdigit(c) && c != '0' && c != '1' && c != '8' && c != '9')
                state = 53;
            else
                state = 52;
            incrementForward(B);
            break;

        case 52:
            lexeme[counter - 1] = '\0';
            createToken(&t, TK_ID, lineCount, lexeme);
            retractForward(B);
            setBeginToForward(B);
            return t;

        case 53:
            c = *forward;
            lexeme[counter++] = c;
            if (isdigit(c) && c != '0' && c != '1' && c != '8' && c != '9')
                state = 53;
            else
                state = 52;
            incrementForward(B);
            break;

        case 54:
            c = *forward;
            lexeme[counter++] = c;
            if (islower(c))
                state = 54;
            else
                state = 55;
            incrementForward(B);
            break;

        case 55:
            retractForward(B);
            lexeme[counter - 1] = '\0';
            setBeginToForward(B);
            if (isKeyword(lexeme))
                createToken(&t, getKeywordToken(lexeme), lineCount, lexeme);
            else
                createToken(&t, TK_FIELDID, lineCount, lexeme);
            return t;


       case 56:
       c = *forward;
       printf("State 56: after '_' read: '%c'\n", c);
       if (isalpha(c)) {
            lexeme[counter++] = c;
            state = 57;
            incrementForward(B);
       } else {
            state = -1;
       }
       break;
   
   case 57:
       c = *forward;
       printf("State 57: reading identifier, current char: '%c'\n", c);
       if (c == '\0' || c == EOF) {
            state = 59;
       } else {
            lexeme[counter++] = c;
            if (isalpha(c))
                state = 57;
            else if (isdigit(c))
                state = 58;
            else
                state = 59;
            incrementForward(B);
       }
       break;
   
   case 58:
       c = *forward;
       printf("State 58: reading digits in identifier, current char: '%c'\n", c);
       if (c == '\0' || c == EOF) {
            state = 59;
       } else {
            lexeme[counter++] = c;
            if (isdigit(c))
                state = 58;
            else
                state = 59;
            incrementForward(B);
       }
       break;
   

case 59:
lexeme[counter] = '\0'; 
createToken(&t, TK_FUNID, lineCount, lexeme);
setBeginToForward(B);    
return t;
        } 
    }
}
