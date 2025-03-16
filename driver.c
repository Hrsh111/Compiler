#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "lexerDef.h"   // Contains tokenInfo, TwinBuffer, tokenStrings, etc.
#include "lexer.h"      // Contains initialise(), getNextToken(), etc.

// Extern declarations – ensure these match your definitions
extern TwinBuffer twinBuffer;
extern const char *tokenStrings[];

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
    
    // Continuously get tokens until END_OF_INPUT is reached.
    do {
        token = getNextToken(&twinBuffer);
        // If a token indicates an error, print an error message in the desired format.
        if (token.token == TK_ERROR) {
            printf("Line no. %-3d\t Lexeme: %-20s\t Error: Unknown symbol or pattern\n", 
                   token.lineNumber, token.lexeme);
        } else {
            printf("Line no. %-3d\t Lexeme: %-20s\t Token %s\n",
                   token.lineNumber, token.lexeme, tokenStrings[token.token]);
        }
    } while (token.token != END_OF_INPUT);
    
    clock_t endLexer = clock();
    double elapsedLexer = (double)(endLexer - startLexer) / CLOCKS_PER_SEC;
    printf("Lexer elapsed time: %.4f seconds\n", elapsedLexer);
    
    fclose(src);
    return EXIT_SUCCESS;
}
