#ifndef LEXER_H
#define LEXER_H

#include "lexerDef.h"
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>

//---------------------------------------
// File Handling & Buffer Management
FILE *initialise(char *inputFile);
FILE *getStream(FILE *fp);
void stripComments(char *testcaseFile, char *cleanFile);
terminals checkKeyword(const char *lexeme);
void initializeKeywords(void);

//---------------------------------------
// Symbol Table Management
void initializeSymbolTable(Symboltable **table);
void insert(const char *lexeme, terminals token);
void addToSymbolTable(const char *lexeme, terminals token);
bool searchSymbolTable(const char *lexeme);
int CalHash(const char *lexeme);

//---------------------------------------
// Tokenization & Helper Functions
tokenInfo generateToken(void);
tokenInfo getNextToken(TwinBuffer *B);
void incrementForward(TwinBuffer *B);
void retractForward(TwinBuffer *B);
void retract(void);
void setBeginToForward(TwinBuffer *B);
void ignoreComment(TwinBuffer *B);
void flush(char *str);
void createToken(tokenInfo *tk, terminals tokenType, int line, const char *lexeme);
char fetchNextChar(void);

//---------------------------------------
// Keyword Handling
int isKeyword(const char *lex);
terminals getKeywordToken(const char *lex);

//---------------------------------------
// Tokenization & Error Handling
// (Other prototypes can go here if needed)

#endif
