#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "lexerDef.h"
#include "lexer.h"
#include "parserDef.h"
#include "stack.h"
#include "parser.h"

extern TwinBuffer twinBuffer;
extern const char *tokenStrings[];

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input source file> <output parse tree file>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    char *inputFile = argv[1];
    char *outputFile = argv[2];
    
    printf("========== Running Lexer ==========\n");
    
    FILE *src = initialise(inputFile);
    if (src == NULL) {
        fprintf(stderr, "Error opening file %s\n", inputFile);
        return EXIT_FAILURE;
    }
    
    clock_t startLexer = clock();
    tokenInfo token;
    
    // Run lexer and print tokens
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
    
    printf("\n========== Running Parser ==========\n");
    
    // Reinitialize the lexer for parsing
    init_lexer(inputFile);
    
    // Load the grammar dynamically from "grammar.txt"
    Grammar *G = loadGrammar("grammar.txt");
    
    // Compute FIRST and FOLLOW sets using the loaded grammar.
    FirstFollow *ffArr = ComputeFirstAndFollowSets(G, TOTAL_NON_TERMINALS);
    if (ffArr == NULL) {
        fprintf(stderr, "Error computing FIRST and FOLLOW sets.\n");
        return EXIT_FAILURE;
    }
    
    // Create the parse table.
    int parseTable[TOTAL_NON_TERMINALS][NUM_TERMINALS];
    memset(parseTable, -1, sizeof(parseTable));
    createParseTable(ffArr, TOTAL_NON_TERMINALS, G, parseTable);
    printf("Parse table created successfully.\n");
    
    // Parse the input source code.
    clock_t startParser = clock();
    ParseTreeNode *parseTreeRoot = parseInputSourceCode(inputFile, parseTable, G);
    clock_t endParser = clock();
    double elapsedParser = (double)(endParser - startParser) / CLOCKS_PER_SEC;
    
    if (parseTreeRoot == NULL) {
        fprintf(stderr, "Parsing failed.\n");
        return EXIT_FAILURE;
    }
    
    printf("Parsing successful. Time taken: %.4f seconds.\n", elapsedParser);
    printf("Input source code is syntactically correct...........\n");
    
    ParseTree PT;
    PT.root = parseTreeRoot;
    printParseTree(&PT, outputFile);
    printf("Parse tree written to %s\n", outputFile);
    
    return EXIT_SUCCESS;
}
