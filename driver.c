#include "lexerDef.h"
#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input file>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    FILE *src = initialise(argv[1]);
    if (src == NULL) {
        fprintf(stderr, "Error opening file %s\n", argv[1]);
        return EXIT_FAILURE;
    }
    
    clock_t start = clock();
    tokenInfo token;
    
    do {
        token = getNextToken(&twinBuffer);
        if (token.token == TK_ERROR) {
            // Print the error but continue scanning
            fprintf(stderr, "Lexical error at line %d: \"%s\"\n", token.lineNumber, token.lexeme);
        } else {
            printf("Line no. %d\t Lexeme: \"%s\"\t Token: %s\n",
                   token.lineNumber, token.lexeme, tokenStrings[token.token]);
        }
    } while (token.token != END_OF_INPUT);
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Elapsed time: %.4f seconds\n", elapsed);
    
    fclose(src);
    return EXIT_SUCCESS;
}
