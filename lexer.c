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

/* Function Prototypes (if not in header) */
// You might have prototypes in your headers; if not, add them here.

FILE *initialise(char *inputFile) {
    srcFile = fopen(inputFile, "r");  
    if (srcFile == NULL) {
        printf("ERROR! Could not open file: %s\n", inputFile);
        return NULL;
    }

    // Read into first buffer
    int size = (int)fread(twinBuffer.buffer1, sizeof(char), BUFFER_SIZE, srcFile);
    if (size == 0 && ferror(srcFile)) {
        printf("ERROR: Failed to read from file: %s\n", inputFile);
        fclose(srcFile);
        return NULL;
    }

    // Mark the end of the buffer with a special character.
    if (size < BUFFER_SIZE) {
        if (size >= 0)
            twinBuffer.buffer1[size] = '\0';  // using '\0' as end marker
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

/* Keyword and Symbol Table Functions */
void initializeKeywords() {
    /* Note: The reservedList has 29 entries. Ensure KC is updated accordingly (e.g., KC = 29)
       if you intend to use all of them. */
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

    /* Insert keywords into the symbol table */
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

/* Pointer management functions using fetchNextChar exclusively */
char fetchNextChar(void) {
    /* Check if we are at the end of the current buffer and need a switch */
    if (activeBuffer == 1 && forward >= twinBuffer.buffer1 + BUFFER_SIZE) {
        getStream(srcFile);
    } else if (activeBuffer == 2 && forward >= twinBuffer.buffer2 + BUFFER_SIZE) {
        getStream(srcFile);
    }
    char c = *forward;
    if (c == '\0')  // if end-of-buffer marker reached
        return EOF;
    forward++;  // advance pointer
    return c;
}

void retract(void) {
    if (forward > lexemebegin) {
        forward--;
    }
}

void setBeginToForward(TwinBuffer *B) {
    (void)B; // unused parameter
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

/* Modified ignoreComment using fetchNextChar */
void ignoreComment(TwinBuffer *B) {
    (void)B; // unused in this implementation
    char c;
    // Consume characters until a newline or end-of-buffer is reached.
    while ((c = *forward) != '\n' && c != '\0') {
        fetchNextChar();  // advances pointer
    }
    // If newline is encountered, consume it and update line count.
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
    return TK_ERROR;
}

terminals getKeywordToken(const char *lexeme) {
    if (strcmp(lexeme, "with") == 0)
        return TK_WITH;
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
    return TK_ERROR;
}

/* getNextToken: DFA implementation using fetchNextChar exclusively.
   Note: This is a direct adaptation of your earlier state-machine.
   All calls to incrementForward(B) have been removed since fetchNextChar()
   now handles pointer advancement and buffer switching. */
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
                // Error state: report error, skip the offending character, and return error token.
                createToken(&t, TK_ERROR, lineCount, lexeme);
                // Advance one character to avoid reprocessing the same error character.
                forward++;
                setBeginToForward(B);
                return t;

            case 0:
                c = fetchNextChar();
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

            // Comment: create TK_COMMENT and skip the rest of the line.
            case 1:
                createToken(&t, TK_COMMENT, lineCount, lexeme);
                ignoreComment(B);
                setBeginToForward(B);
                return t;

            // Single character tokens...
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

            // Two-character token: "!="
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

            // Logical AND: &&&
            case 17:
                c = fetchNextChar();
                lexeme[counter++] = c;
                if (c == '&')
                    state = 18;
                else
                    state = -1;
                break;
            case 18:
                c = fetchNextChar();
                lexeme[counter++] = c;
                if (c == '&')
                    state = 19;
                else
                    state = -1;
                break;
            case 19:
                createToken(&t, TK_AND, lineCount, lexeme);
                setBeginToForward(B);
                return t;

            // Logical OR: @@@
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

            // Whitespace/EOF handling
            case 23:
                if (lexeme[0] == '\0' || c == EOF) {
                    createToken(&t, END_OF_INPUT, lineCount, "EOF");
                    return t;
                } else if (c == '\n') {
                    lineCount++;
                }
                setBeginToForward(B);
                counter = 0;
                memset(lexeme, 0, sizeof(lexeme));
                state = 0;
                break;

            // Greater-than tokens (">" or ">=")
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

            // Less-than tokens ("<", "<=", "<-")
            case 27:
            c = fetchNextChar();
            if (c == '=')
            {
                lexeme[counter++] = c;
                state = 28;
            }
            else if (c == '-')
            {
                lexeme[counter++] = c;
                state = 29;
            }
            else
            {
                // c is not '=' or '-', so retract this character
                retract();  
                state = 32;
            }
            break;
        
            case 28:
                createToken(&t, TK_LE, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            case 29:
                c = fetchNextChar();
                lexeme[counter++] = c;
                if (c == '-')
                    state = 30;
                else
                    state = 61;
                break;
            case 30:
                c = fetchNextChar();
                lexeme[counter++] = c;
                if (c == '-')
                    state = 31;
                else
                    state = -1;
                break;
            case 31:
                createToken(&t, TK_ASSIGNOP, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            case 61:
                lexeme[counter - 1] = '\0';
                lexeme[counter - 2] = '\0';
                createToken(&t, TK_LT, lineCount, lexeme);
                retract();
                retract();
                setBeginToForward(B);
                return t;
                case 32:
                lexeme[counter] = '\0';
                createToken(&t, TK_LT, lineCount, lexeme);
                // Do NOT retract here, because we've already retracted in state 27.
                setBeginToForward(B);
                return t;
            

            // Reserved word/identifier starting with '#' (RUID)
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

            // Equal sign (==)
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

            // Numeric literals (integers and reals)
  // State 38: Reading the integer part.
case 38: {
    c = fetchNextChar();
    if (isdigit(c)) {
         lexeme[counter++] = c;
         state = 38;
    } else if (c == '.') {
         lexeme[counter++] = c;
         state = 39;  // Transition to reading fractional digits.
    } else {
         // End of integer literal.
         retract();  // Put back the delimiter.
         lexeme[counter] = '\0';
         createToken(&t, TK_NUM, lineCount, lexeme);
         setBeginToForward(B);
         return t;
    }
    break;
}

// State 39: We have read the dot; now expect the first fractional digit.
case 39: {
    c = fetchNextChar();
    if (isdigit(c)) {
         lexeme[counter++] = c;
         state = 40;  // Now expect the second fractional digit.
    } else {
         // No digit immediately after dot → error.
         retract();
         lexeme[counter] = '\0';
         createToken(&t, TK_ERROR, lineCount, lexeme);
         setBeginToForward(B);
         return t;
    }
    break;
}
            
            case 60:
                lexeme[counter - 1] = '\0';
                lexeme[counter - 2] = '\0';
                createToken(&t, TK_NUM, lineCount, lexeme);
                retract();
                retract();
                setBeginToForward(B);
                return t;
    // State 40: Expect exactly one more digit for the fractional part.
case 40: {
    c = fetchNextChar();
    if (isdigit(c)) {
         lexeme[counter++] = c;
         // Now we have [integer part] '.' [2 fractional digits].
         // Check if the next character is an exponent marker.
         char look = fetchNextChar();
         if (look == 'E' || look == 'e') {
              lexeme[counter++] = look;  // Append the exponent marker.
              state = 42;  // Transition to exponent handling.
         } else {
              // No exponent: retract the lookahead and finalize TK_RNUM.
              retract();
              lexeme[counter] = '\0';
              createToken(&t, TK_RNUM, lineCount, lexeme);
              setBeginToForward(B);
              return t;
         }
    } else {
         // No second fractional digit → error.
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

            
// State 42: After reading 'E' (or 'e') in the number.
case 42: {
    c = fetchNextChar();
    if (c == '+' || c == '-') {
         lexeme[counter++] = c;
         state = 43;  // Now expect the first exponent digit.
    } else if (isdigit(c)) {
         // No sign present; treat this digit as the first exponent digit.
         lexeme[counter++] = c;
         state = 44;
    } else {
         state = -1;  // Error.
    }
    break;
}
case 43: {
    c = fetchNextChar();
    if (isdigit(c)) {
         lexeme[counter++] = c;
         state = 44;  // Now expect the second exponent digit.
    } else {
         state = -1;  // Error.
    }
    break;
}
// State 44: Expect the second exponent digit.
case 44: {
    c = fetchNextChar();
    if (isdigit(c)) {
         lexeme[counter++] = c;
         // Now the exponent part is complete (exactly two digits).
         // Lookahead: fetch the next character (if any) and then retract it.
         char look = fetchNextChar();
         retract();  
         lexeme[counter] = '\0';
         createToken(&t, TK_RNUM, lineCount, lexeme);
         setBeginToForward(B);
         return t;
    } else {
         state = -1;  // Error.
    }
    break;
}


/*// State 45: Finalize the real number token.
case 45: {
    // We already have exactly two exponent digits.
    // Retract the character that is not part of the exponent (if any).
    retract();
    lexeme[counter] = '\0';
    createToken(&t, TK_RNUM, lineCount, lexeme);
    setBeginToForward(B);
    return t;
}*/

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

            // Identifiers/reserved words starting with b, c, or d
            case 48:
                c = fetchNextChar();
                lexeme[counter++] = c;
                if (islower(c))
                    state = 49;
                else if (isdigit(c) && c != '0' && c != '1' && c != '8' && c != '9')
                    state = 51;
                else
                    state = 50;
                break;
            case 49:
                c = fetchNextChar();
                lexeme[counter++] = c;
                if (islower(c))
                    state = 49;
                else
                    state = 50;
                break;
            case 50:
                lexeme[counter - 1] = '\0';
                if (checkKeyword(lexeme) != TK_ERROR)
                    createToken(&t, checkKeyword(lexeme), lineCount, lexeme);
                else
                    createToken(&t, TK_FIELDID, lineCount, lexeme);
                retract();
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

            // Identifiers/reserved words starting with a lowercase letter (not b, c, or d)
            case 54: {
                c = fetchNextChar();
                if (c >= 'a' && c <= 'z') {  // Only allow a-z
                     lexeme[counter++] = c;
                     state = 54;
                }
                else {
                     state = 55;  // Encountered something that doesn't belong in FIELDID
                }
                break;
            }
            
        
            case 55: {
                lexeme[counter] = '\0';  // Terminate the lexeme.
                retract();  // Put back the character that did not match.
                terminals maybeKeyword = checkKeyword(lexeme);
                if (maybeKeyword != TK_ERROR)
                     createToken(&t, maybeKeyword, lineCount, lexeme);
                else
                     createToken(&t, TK_FIELDID, lineCount, lexeme);
                setBeginToForward(B);
                return t;
            }
            

            // Function identifiers: start with an underscore
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
        } // end switch
    } // end while
}