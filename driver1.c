

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
        fprintf(stderr, "Usage: %s <option> <input source file> [<output file>]\n", argv[0]);
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "  1: Lexical Analysis only\n");
        fprintf(stderr, "  2: Parsing only (requires output file)\n");
        fprintf(stderr, "  3: Print FIRST and FOLLOW sets\n");
        fprintf(stderr, "  4: Print Parse Table\n");
        fprintf(stderr, "  5: Full process (Lexer and Parser, requires output file)\n");
        return EXIT_FAILURE;
    }
    
    int option = atoi(argv[1]);
    char *inputFile = argv[2];
    char *outputFile = (argc >= 4) ? argv[3] : NULL;
    
    switch(option) {
        case 1: {

            printf("========== Running Lexer (Option 1) ==========\n");
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
            break;
        }
        case 2: {

            if (outputFile == NULL) {
                fprintf(stderr, "Output file is required for Parsing option.\n");
                return EXIT_FAILURE;
            }
            printf("========== Running Parser (Option 2) ==========\n");
            

            init_lexer(inputFile);
            

            Grammar *G = loadGrammar("grammar.txt");
            if (G == NULL) {
                fprintf(stderr, "Error loading grammar from grammar.txt\n");
                return EXIT_FAILURE;
            }
            

            FirstFollow *ffArr = ComputeFirstAndFollowSets(G, TOTAL_NON_TERMINALS);
            if (ffArr == NULL) {
                fprintf(stderr, "Error computing FIRST and FOLLOW sets.\n");
                return EXIT_FAILURE;
            }
            

            int parseTable[TOTAL_NON_TERMINALS][NUM_TERMINALS];
            memset(parseTable, -1, sizeof(parseTable));
            createParseTable(ffArr, TOTAL_NON_TERMINALS, G, parseTable);
            printf("Parse table created successfully.\n");
            

            clock_t startParser = clock();
            ParseTreeNode *parseTreeRoot = parseInputSourceCode(inputFile, parseTable, G);
            clock_t endParser = clock();
            double elapsedParser = (double)(endParser - startParser) / CLOCKS_PER_SEC;
            
            if (parseTreeRoot == NULL) {
                fprintf(stderr, "Parsing failed.\n");
                return EXIT_FAILURE;
            }
            
            printf("Parsing successful. Time taken: %.4f seconds.\n", elapsedParser);
            printf("Input source code is syntactically correct.\n");
            
            ParseTree PT;
            PT.root = parseTreeRoot;
            printParseTree(&PT, outputFile);
            printf("Parse tree written to %s\n", outputFile);
            break;
        }
        case 3: {

            printf("========== Computing FIRST and FOLLOW Sets (Option 3) ==========\n");
            Grammar *G = loadGrammar("grammar.txt");
            if (G == NULL) {
                fprintf(stderr, "Error loading grammar from grammar.txt\n");
                return EXIT_FAILURE;
            }
            FirstFollow *ffArr = ComputeFirstAndFollowSets(G, TOTAL_NON_TERMINALS);
            if (ffArr == NULL) {
                fprintf(stderr, "Error computing FIRST and FOLLOW sets.\n");
                return EXIT_FAILURE;
            }

            printf("FIRST and FOLLOW sets computed successfully.\n");
            break;
        }
        case 4: {

            printf("========== Creating and Printing Parse Table (Option 4) ==========\n");
            Grammar *G = loadGrammar("grammar.txt");
            if (G == NULL) {
                fprintf(stderr, "Error loading grammar from grammar.txt\n");
                return EXIT_FAILURE;
            }
            FirstFollow *ffArr = ComputeFirstAndFollowSets(G, TOTAL_NON_TERMINALS);
            if (ffArr == NULL) {
                fprintf(stderr, "Error computing FIRST and FOLLOW sets.\n");
                return EXIT_FAILURE;
            }
            int parseTable[TOTAL_NON_TERMINALS][NUM_TERMINALS];
            memset(parseTable, -1, sizeof(parseTable));
            createParseTable(ffArr, TOTAL_NON_TERMINALS, G, parseTable);
            printf("Parse table created successfully.\n");
            


            printf("\nParse Table:\n");
            printf("%-15s", "NT/T");
            for (size_t j = 0; j < NUM_TERMINALS; j++) {
                printf("%-15s", tokenStrings[j]);
            }
            printf("\n");
            for (int i = 0; i < TOTAL_NON_TERMINALS; i++) {

                printf("%-15s", "NT");
                for (size_t j = 0; j < NUM_TERMINALS; j++) {
                    if (parseTable[i][j] == -1)
                        printf("%-15s", "");
                    else
                        printf("%-15d", parseTable[i][j]);
                }
                printf("\n");
            }
            break;
        }
        case 5: {
            // Option 5: Full Process (Lexer + Parser).
            if (outputFile == NULL) {
                fprintf(stderr, "Output file is required for Full Process option.\n");
                return EXIT_FAILURE;
            }
            printf("========== Running Lexer (Option 5) ==========\n");
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
            
            printf("\n========== Running Parser (Option 5) ==========\n");
            init_lexer(inputFile);
            Grammar *G = loadGrammar("grammar.txt");
            if (G == NULL) {
                fprintf(stderr, "Error loading grammar from grammar.txt\n");
                return EXIT_FAILURE;
            }
            FirstFollow *ffArr = ComputeFirstAndFollowSets(G, TOTAL_NON_TERMINALS);
            if (ffArr == NULL) {
                fprintf(stderr, "Error computing FIRST and FOLLOW sets.\n");
                return EXIT_FAILURE;
            }
            int parseTable[TOTAL_NON_TERMINALS][NUM_TERMINALS];
            memset(parseTable, -1, sizeof(parseTable));
            createParseTable(ffArr, TOTAL_NON_TERMINALS, G, parseTable);
            printf("Parse table created successfully.\n");
            clock_t startParser = clock();
            ParseTreeNode *parseTreeRoot = parseInputSourceCode(inputFile, parseTable, G);
            clock_t endParser = clock();
            double elapsedParser = (double)(endParser - startParser) / CLOCKS_PER_SEC;
            if (parseTreeRoot == NULL) {
                fprintf(stderr, "Parsing failed.\n");
                return EXIT_FAILURE;
            }
            printf("Parsing successful. Time taken: %.4f seconds.\n", elapsedParser);
            printf("Input source code is syntactically correct.\n");
            ParseTree PT;
            PT.root = parseTreeRoot;
            printParseTree(&PT, outputFile);
            printf("Parse tree written to %s\n", outputFile);
            break;
        }
        default: {
            fprintf(stderr, "Invalid option. Please choose between 1 and 5.\n");
            return EXIT_FAILURE;
        }
    }
    
    return EXIT_SUCCESS;
}
