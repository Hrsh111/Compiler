#include <stdio.h>
#include "lexerDef.h"
#include "lexer.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
char *forward;
char *lexemebegin;
int activeBuffer;
FILE *srcFile;
bool exhaustedInput;
bool ldfirstBuff;
bool ldsecondBuff;
TwinBuffer twinBuffer; // Global TwinBuffer instance
keyword *kwEntries[KC];
Symboltable *table;
// Initialising the lexer
FILE *initialise(char *inputFile)
{
    srcFile = fopen(inputFile, "r");
    if (srcFile == NULL)
    {
        printf("ERROR! File not opened.\n");
        return NULL;
    }

    // Read data into first buffer
    int size = fread(twinBuffer.buffer1, sizeof(char), BUFFER_SIZE, srcFile);
    if (size < BUFFER_SIZE)
    {
        twinBuffer.buffer1[size] = EOF; // Mark end of input
        exhaustedInput = true;          // Input exhausted if size < buff_size
    }
    else
    {
        exhaustedInput = false;
    }

    // Initialize global variables
    activeBuffer = 1; // Start with buffer1
    ldfirstBuff = true;
    ldsecondBuff = false;
    lexemebegin = twinBuffer.buffer1;
    forward = twinBuffer.buffer1;

    // initializeSymbolTable(); // Initialize symbol table

    return srcFile;
}
FILE *getStream(FILE *fp)
{
    if (exhaustedInput)
        return NULL; // No more input to read

    if (activeBuffer == 1)
    {
        // Load buffer2
        size_t bytesRead = fread(twinBuffer.buffer2, sizeof(char), BUFFER_SIZE, fp);
        if (bytesRead < BUFFER_SIZE)
        {
            twinBuffer.buffer2[bytesRead] = EOF; // Mark EOF
            exhaustedInput = true;
        }
        forward = twinBuffer.buffer2; // Move `forward` pointer to start of buffer2
        activeBuffer = 2;
        ldsecondBuff = true;
    }
    else
    {
        // Load buffer1
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

/// @brief 
void initializeKeywords()
{
    static keyword reservedList[] = {// 'static' ensures it persists after function exits
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
        kwEntries[i] = (keyword *)malloc(sizeof(keyword)); // Allocate memory for each keyword entry

        if (kwEntries[i] == NULL)
        {
            fprintf(stderr, "error allocating mem for %d\n", i);
            exit(EXIT_FAILURE);
        }

        kwEntries[i]->keyword = strdup(reservedList[i].keyword); // Copy the keyword string
        kwEntries[i]->token = reservedList[i].token;             // Copy the token
    }
}
// initializing the symbol table use chain hashing
void initializeSymbolTable(Symboltable *table)
{
    // Allocate and 0 initialize the symbol table
    table = (Symboltable *)calloc(1, sizeof(Symboltable));
    if (table == NULL)
    {
        fprint(stderr, "failed memory allocation for symbol table.\n");
        exit(EXIT_FAILURE);
    }
    initializeKeywords();

    for (int i = 0; i < KC; i++)
    {
        insert(kwEntries[i]->keyword, kwEntries[i]->token);
    }
}
// hash calculation using djb2 algorithm
int CalHash(char *lexeme)
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


// Function to check if a lexeme exists in the symbol table
bool lookup(char *lexeme) {
    int index = CalHash(lexeme);   // Compute the hash index for the lexeme
    SymbolItem *current = table->data[index]; // Start at the head of the linked list at that bucket
    
    // Traverse the linked list in this bucket
    while (current != NULL) {
        if (strcmp(current->lexeme, lexeme) == 0) { // Compare stored lexeme with the given lexeme
            return true;  // Lexeme found
        }
        current = current->next;
    }
    return false;  
}



void insert(char *lexeme, terminals token) {
    int index = hash(lexeme);
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

