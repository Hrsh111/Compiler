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
        printf("Token: %s, Lexeme: \"%s\", Line: %d\n",
               tokenStrings[token.token], token.lexeme, token.lineNumber);
    } while (token.token != END_OF_INPUT);


    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Elapsed time: %.4f seconds\n", elapsed);

    fclose(src);
    return EXIT_SUCCESS;
}
