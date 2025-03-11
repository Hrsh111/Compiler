#ifndef LEXER_H
#define LEXER_H
#include "lexerDef.h"
#include <stdbool.h>
#include <stdio.h>

// File Handling & Buffer Management
FILE *initializeLexer(char *inputFile);
 FILE* getStream(FILE * fp);
void stripComments(char* sourceFile, char* outputFile);
void initializeKeywords(Symboltable*table);
char fetchNextChar(FILE *fp);

// Symbol Table Management
void setupSymbolTable();
int computeHash(char *lexeme);
void addToSymbolTable(char *lexeme, terminals token);
bool searchSymbolTable(char *lexeme);

// Keyword Handling
void setupKeywordTable();
terminals lookupKeyword(char *lexeme);

// Tokenization & Error Handling
SymbolItem generateError(int errorCode, char *lexeme, int lineNo);
char *extractLexeme();
SymbolItem constructToken(char *lexeme, terminals tokenType, int lineNo);
SymbolItem retrieveToken(FILE *fp);

// Token Linked List Management
// TokenNode* createToken(terminals tokenType, char* lexeme, int lineNo);
// void appendToken(TokenNode** head, terminals tokenType, char* lexeme, int lineNo);
// void displayTokens(TokenNode* head);
// void releaseTokenList(TokenNode* head);

#endif
