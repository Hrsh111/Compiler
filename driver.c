#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "lexerDef.h"   // Contains tokenInfo, TwinBuffer, tokenStrings, etc.
#include "lexer.h"      // Contains initialise(), getNextToken(), etc.

// Extern declarations (adjust as needed)
extern TwinBuffer twinBuffer;
//extern char *tokenStrings[];

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input source file>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    char *inputFile = argv[1];
    printf("========== Running Lexer ==========\n");
    
    // Open and initialize the source file
    FILE *src = initialise(inputFile);
    if (src == NULL) {
        fprintf(stderr, "Error opening file %s\n", inputFile);
        return EXIT_FAILURE;
    }
    
    clock_t startLexer = clock();
    tokenInfo token;
    
    do {
        token = getNextToken(&twinBuffer);
        if (token.token == TK_ERROR) {
            fprintf(stderr, "Lexical error at line %d: \"%s\"\n", token.lineNumber, token.lexeme);
        } else {
            printf("Line no. %d\tLexeme: \"%s\"\tToken: %s\n",
                   token.lineNumber, token.lexeme, tokenStrings[token.token]);
        }
    } while (token.token != END_OF_INPUT);
    
    clock_t endLexer = clock();
    double elapsedLexer = (double)(endLexer - startLexer) / CLOCKS_PER_SEC;
    printf("Lexer elapsed time: %.4f seconds\n", elapsedLexer);
    
    fclose(src);
    return EXIT_SUCCESS;
}
