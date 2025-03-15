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
FILE *srcFile=NULL;
bool exhaustedInput;
bool ldfirstBuff;
bool ldsecondBuff;
TwinBuffer twinBuffer;

Symboltable *table = NULL;
keyword *kwEntries[KC] = {NULL};

FILE *initialise(char *inputFile) {
    srcFile = fopen(inputFile, "r");  // Open file for reading
    if (srcFile == NULL) {
        printf("ERROR! Could not open file: %s\n", inputFile);
        return NULL;
    }

    // Read into buffer
    int size = fread(twinBuffer.buffer1, sizeof(char), BUFFER_SIZE, srcFile);
    if (size == 0 && ferror(srcFile)) {
        printf("ERROR: Failed to read from file: %s\n", inputFile);
        fclose(srcFile);
        return NULL;
    }

    // Handle end-of-file scenarios
    if (size < BUFFER_SIZE) {
        if(size>=0)twinBuffer.buffer1[size] = EOF;  // Mark EOF in buffer
        exhaustedInput = true;
    } else {
        exhaustedInput = false;
    }

    // Initialize twin buffer pointers
    activeBuffer = 1;
    ldfirstBuff = true;
    ldsecondBuff = false;
    lexemebegin = twinBuffer.buffer1;
    forward = twinBuffer.buffer1;

    // Initialize Symbol Table (Ensure this function is correctly defined)
    initializeSymbolTable(&table);

    printf("File %s opened and initialized successfully.\n", inputFile);
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
        {"write", TK_WRITE},
        
    };
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

    while ((c = *forward) != '\n' && c != '\0' && c != EOF)
    {
        incrementForward(B);
    }

}

void setBeginToForward(TwinBuffer *B)
{
    (void)B;
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
    else if (isalpha(*forward) || *forward == '_') {
        int i = 0;

        while (isalpha(*forward) || *forward == '_') {
            token.lexeme[i++] = *forward;
            forward++;
        }
        token.lexeme[i] = '\0';
        token.lineNumber = lineCount;
        

        if (isdigit(*forward)) {
            token.token = TK_FIELDID;  
            return token;
        }
        

        while (isalnum(*forward) || *forward == '_') {
            token.lexeme[i++] = *forward;
            forward++;
        }
        token.lexeme[i] = '\0';
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
    if (strcmp(lex, "endunion") == 0)
        return 1;
    if (strcmp(lex, "definetype") == 0)
        return 1;
    if (strcmp(lex, "global") == 0)
        return 1;
    if (strcmp(lex, "list") == 0)
        return 1;
    if (strcmp(lex, "if") == 0)
        return 1;
    if (strcmp(lex, "then") == 0)
        return 1;
    if (strcmp(lex, "endif") == 0)
        return 1;
    if (strcmp(lex, "read") == 0)
        return 1;
    if (strcmp(lex, "write") == 0)
        return 1;
    if (strcmp(lex, "return") == 0)
        return 1;
    if (strcmp(lex, "record") == 0)
        return 1;
    if (strcmp(lex, "endrecord") == 0)
        return 1;
    if (strcmp(lex, "else") == 0)
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
    if (strcmp(lex, "endunion") == 0)
        return TK_ENDUNION;
    if (strcmp(lex, "global") == 0)
        return TK_GLOBAL;
    if (strcmp(lex, "if") == 0)
        return TK_IF;
    if (strcmp(lex, "list") == 0)
        return TK_LIST;
    if (strcmp(lex, "then") == 0)
        return TK_THEN;
    if (strcmp(lex, "endif") == 0)
        return TK_ENDIF;
    if (strcmp(lex, "read") == 0)
        return TK_READ;
    if (strcmp(lex, "write") == 0)
        return TK_WRITE;
    if (strcmp(lex, "return") == 0)
        return TK_RETURN;
    if (strcmp(lex, "record") == 0)
        return TK_RECORD;
    if (strcmp(lex, "endrecord") == 0)
        return TK_ENDRECORD;
    if (strcmp(lex, "else") == 0)
        return TK_ELSE;
    if (strcmp(lex, "call") == 0)
        return TK_CALL;

    return TK_ID;
}
void incrementForward(TwinBuffer *B)
{
    (void)B;
    forward++;

    if (activeBuffer == 1)
    {
        if (forward >= twinBuffer.buffer1 + BUFFER_SIZE)
        {
            getStream(srcFile);
        }
    }
    else
    {
        if (forward >= twinBuffer.buffer2 + BUFFER_SIZE)
        {
            getStream(srcFile);
        }
    }
}

void retractForward(TwinBuffer *B)
{
    (void)B;
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


#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "lexerDef.h"  

typedef enum {
    STATE_ERROR      = -1,
    STATE_INITIAL    = 0,
    STATE_PERCENT,      
    STATE_RSQ,          
    STATE_LSQ,          
    STATE_COMMA,        
    STATE_SEMICOLON,    
    STATE_COLON,        
    STATE_DOT,          
    STATE_OPAREN,       
    STATE_CPAREN,       
    STATE_PLUS,         
    STATE_MINUS,        
    STATE_MUL,          
    STATE_DIV,          
    STATE_NOT,          
    STATE_EXCLAMATION,  
    STATE_EXCL_EQ,      
    STATE_AND_START,    
    STATE_AND_MIDDLE,   
    STATE_AND,          
    STATE_AT_START,
    STATE_AT_MIDDLE,
    STATE_AT,
    STATE_WHITESPACE,
    STATE_GT,
    STATE_GE,
    STATE_LT,
    STATE_LE,
    STATE_ASSIGNOP,
    STATE_NUM,          
    STATE_AFTER_DOT,    
    STATE_RNUM,         
    STATE_ID_START, 
    STATE_RUID,    
    STATE_FINAL         
} State;

tokenInfo getNextToken(TwinBuffer *B) {
    tokenInfo t;
    t.lexeme[0] = '\0';
    t.lineNumber = lineCount;  

    char lexeme[100];
    memset(lexeme, '\0', sizeof(lexeme));
    int counter = 0;
    char c = '\0';
    State state = STATE_INITIAL;

    while (1) {
        switch (state) {

            case STATE_ERROR: {
                fprintf(stderr, "DFA error at line %d. Current lexeme: \"%s\", failing character: '%c'\n",
                        lineCount, lexeme, c);
                createToken(&t, TK_ERROR, lineCount, lexeme);
                //setBeginToForward(B);
                incrementForward(B);
                return t;
            }


            case STATE_INITIAL: {
                if (*forward == '#') {
                    lexeme[counter++] = *forward;
                    incrementForward(B);
                    state = STATE_RUID;
                } 
                c = *forward;
                lexeme[counter++] = c;
                if (c == '%') {
                    state = STATE_PERCENT;
                    incrementForward(B);
                }
                else if (c == ']') {
                    state = STATE_RSQ;
                    incrementForward(B);
                }
                else if (c == '[') {
                    state = STATE_LSQ;
                    incrementForward(B);
                }
                else if (c == ',') {
                    state = STATE_COMMA;
                    incrementForward(B);
                }
                else if (c == ';') {
                    state = STATE_SEMICOLON;
                    incrementForward(B);
                }
                else if (c == ':') {
                    state = STATE_COLON;
                    incrementForward(B);
                }
                else if (c == '.') {
                    state = STATE_DOT;
                    incrementForward(B);
                }
                else if (c == '(') {
                    state = STATE_OPAREN;
                    incrementForward(B);
                }
                else if (c == ')') {
                    state = STATE_CPAREN;
                    incrementForward(B);
                }
                else if (c == '+') {
                    state = STATE_PLUS;
                    incrementForward(B);
                }
                else if (c == '-') {
                    state = STATE_MINUS;
                    incrementForward(B);
                }
                else if (c == '*') {
                    state = STATE_MUL;
                    incrementForward(B);
                }
                else if (c == '/') {
                    state = STATE_DIV;
                    incrementForward(B);
                }
                else if (c == '~') {
                    state = STATE_NOT;
                    incrementForward(B);
                }
                else if (c == '!') {
                    state = STATE_EXCLAMATION;
                    incrementForward(B);
                }
                else if (c == '&') {
                    state = STATE_AND_START;
                    incrementForward(B);
                }
                else if (c == '@') {
                    state = STATE_AT_START;
                    incrementForward(B);
                }
                else if (c == ' ' || c == '\t' || c == '\n' || c == '\0' || c == EOF) {
                    state = STATE_WHITESPACE;
                }
                else if (c == '>') {
                    state = STATE_GT;
                    incrementForward(B);
                }
                else if (c == '<') {
                    state = STATE_LT;
                    incrementForward(B);
                }
                else if (c == '=') {
                    state = STATE_ASSIGNOP;
                    incrementForward(B);
                }
                else if (isdigit(c)) {
                    state = STATE_NUM;
                }
                else if (islower(c) || c == '_') {
                    state = STATE_ID_START;
                    incrementForward(B);
                }
                else {
                    state = STATE_ERROR;
                    incrementForward(B);
                }
                break;
            }
            case STATE_RUID: {
                c = *forward;
            
                // Accept only lowercase letters
                if (islower(c)) {
                    lexeme[counter++] = c;
                    incrementForward(B);
                } else {
                    lexeme[counter] = '\0';  // Terminate lexeme string
                    createToken(&t, TK_RUID, lineCount, lexeme);
                    setBeginToForward(B);
                    return t;
                }
                break;
            }

            case STATE_PERCENT: {

                createToken(&t, TK_COMMENT, lineCount, lexeme);
                ignoreComment(B);
                setBeginToForward(B);
                return t;
            }

            case STATE_RSQ: {
                createToken(&t, TK_SQR, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            }

            case STATE_LSQ: {
                createToken(&t, TK_SQL, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            }

            case STATE_COMMA: {
                createToken(&t, TK_COMMA, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            }

            case STATE_SEMICOLON: {
                createToken(&t, TK_SEM, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            }

            case STATE_COLON: {
                createToken(&t, TK_COLON, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            }

            case STATE_DOT: {
                createToken(&t, TK_DOT, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            }

            case STATE_OPAREN: {
                createToken(&t, TK_OP, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            }

            case STATE_CPAREN: {
                createToken(&t, TK_CL, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            }

            case STATE_PLUS: {
                createToken(&t, TK_PLUS, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            }

            case STATE_MINUS: {
                createToken(&t, TK_MINUS, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            }

            case STATE_MUL: {
                createToken(&t, TK_MUL, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            }

            case STATE_DIV: {
                createToken(&t, TK_DIV, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            }

            case STATE_NOT: {
                createToken(&t, TK_NOT, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            }

            case STATE_EXCLAMATION: {
                c = *forward;
                if (c == '=') {
                    lexeme[counter++] = c;
                    state = STATE_EXCL_EQ;
                    incrementForward(B);
                }
                else {
                    state = STATE_ERROR;
                }
                break;
            }

            case STATE_EXCL_EQ: {
                createToken(&t, TK_NE, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            }

            case STATE_AND_START: {
                c = *forward;
                lexeme[counter++] = c;
                if (c == '&') {
                    state = STATE_AND_MIDDLE;
                }
                else {
                    state = STATE_ERROR;
                }
                incrementForward(B);
                break;
            }

            case STATE_AND_MIDDLE: {
                c = *forward;
                lexeme[counter++] = c;
                if (c == '&')
                    state = STATE_AND;
                else
                    state = STATE_ERROR;
                incrementForward(B);
                break;
            }

            case STATE_AND: {
                createToken(&t, TK_AND, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            }

            case STATE_AT_START: {
                c = *forward;
                lexeme[counter++] = c;
                if (c == '@') {
                    state = STATE_AT_MIDDLE;
                }
                else {
                    state = STATE_ERROR;
                }
                incrementForward(B);
                break;
            }

            case STATE_AT_MIDDLE: {
                c = *forward;
                lexeme[counter++] = c;
                if (c == '@')
                    state = STATE_AT;
                else
                    state = STATE_ERROR;
                incrementForward(B);
                break;
            }

            case STATE_AT: {

                createToken(&t, TK_AT, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            }

            case STATE_WHITESPACE: {
                while (*forward == ' ' || *forward == '\t' || *forward == '\n') {
                    if (*forward == '\n')
                        lineCount++;
                    incrementForward(B);
                }
                if (*forward == '\0' || *forward == EOF) {
                    createToken(&t, END_OF_INPUT, lineCount, lexeme);
                    return t;
                }
                memset(lexeme, 0, sizeof(lexeme));
                counter = 0;
                setBeginToForward(B);
                state = STATE_INITIAL;
                break;
            }

            case STATE_GT: {
                c = *forward;
                lexeme[counter++] = c;
                if (c == '=') {
                    state = STATE_GE;
                    incrementForward(B);
                }
                else {
                    state = STATE_FINAL;
                break;
                }
            }

            case STATE_GE: {
                createToken(&t, TK_GE, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            }

            case STATE_FINAL: {

                lexeme[counter - 1] = '\0';

                createToken(&t, TK_GT, lineCount, lexeme);
                retractForward(B);
                setBeginToForward(B);
                return t;
            }

            case STATE_LT: {
                c = *forward;
                if (c == '=') {
                    lexeme[counter++] = c;
                    incrementForward(B);
                    lexeme[counter] = '\0';
                    createToken(&t, TK_LE, lineCount, lexeme);
                } else {

                    lexeme[counter] = '\0';
                    createToken(&t, TK_LT, lineCount, lexeme);
                }
                setBeginToForward(B);
                return t;
            }
            

            case STATE_LE: {
                createToken(&t, TK_LE, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            }

            case STATE_ASSIGNOP: {
                c = *forward;
                lexeme[counter++] = c;
                if (c == '=') {
                    createToken(&t, TK_EQ, lineCount, lexeme);
                    setBeginToForward(B);
                    return t;
                }
                else {
                    state = STATE_ERROR;
                }
                break;
            }

            case STATE_NUM: {

                char next = *forward;
                if (isdigit(next)) {
                    lexeme[counter++] = next;
                    incrementForward(B);
                    state = STATE_NUM;
                }
                else if (next == '.') {
                    lexeme[counter++] = next;
                    incrementForward(B);
                    state = STATE_AFTER_DOT;
                }
                else {
                    createToken(&t, TK_NUM, lineCount, lexeme);
                    setBeginToForward(B);
                    return t;
                }
                break;
            }

            case STATE_AFTER_DOT: {
                c = *forward;
                lexeme[counter++] = c;
                if (isdigit(c))
                    state = STATE_RNUM;
                else
                    state = STATE_ERROR;
                incrementForward(B);
                break;
            }

            case STATE_RNUM: {
                c = *forward;
                lexeme[counter++] = c;
                if (isdigit(c))
                    state = STATE_RNUM;
                else {
                    createToken(&t, TK_RNUM, lineCount, lexeme);
                    setBeginToForward(B);
                    return t;
                }
                incrementForward(B);
                break;
            }

            case STATE_ID_START: {
                // Read an identifier (or keyword) starting with a lowercase letter or underscore.
                while (isalnum(*forward) || *forward == '_') {
                    lexeme[counter++] = *forward;
                    incrementForward(B);
                }
                lexeme[counter] = '\0';
                if (isKeyword(lexeme))
                    createToken(&t, getKeywordToken(lexeme), lineCount, lexeme);
                else
                    createToken(&t, TK_ID, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            }
            

            default: {
                state = STATE_ERROR;
                break;
            }
        }
    }
    }

