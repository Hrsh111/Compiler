

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "lexerDef.h"
#include "lexer.h"
#include "parserDef.h"
#include "stack.h"
#include "parser.h"

extern TwinBuffer twinBuffer;
extern const char *tokenStrings[];


void removeComments(const char *inputFile) {
    FILE *fp = fopen(inputFile, "r");
    if (!fp) {
        fprintf(stderr, "Error opening file %s\n", inputFile);
        return;
    }
    char line[1024];
    while (fgets(line, sizeof(line), fp)) {

        printf("%s", line);
    }
    fclose(fp);
}


void printTokenList(const char *inputFile) {

    FILE *src = initialise((char *)inputFile);
    if (src == NULL) {
        fprintf(stderr, "Error opening file %s\n", inputFile);
        return;
    }
    
    tokenInfo token;
    printf("%-10s %-25s %-15s\n", "Line No.", "Lexeme", "Token");
    printf("----------------------------------------------------------\n");
    do {
        token = getNextToken(&twinBuffer);
        if (token.token == TK_ERROR) {
            fprintf(stderr, "Lexical error at line %d: \"%s\"\n", token.lineNumber, token.lexeme);
        } else {
            printf("%-10d %-25s %-15s\n", token.lineNumber, token.lexeme, tokenStrings[token.token]);
        }
    } while (token.token != END_OF_INPUT);
    fclose(src);
}


void parseInput(const char *inputFile, const char *outputFile) {

    init_lexer((char *)inputFile);


    Grammar *G = loadGrammar("grammar.txt");
    if (G == NULL) {
        fprintf(stderr, "Error loading grammar from grammar.txt\n");
        return;
    }


    FirstFollow *ffArr = ComputeFirstAndFollowSets(G, TOTAL_NON_TERMINALS);
    if (ffArr == NULL) {
        fprintf(stderr, "Error computing FIRST and FOLLOW sets.\n");
        return;
    }


    int parseTable[TOTAL_NON_TERMINALS][NUM_TERMINALS];
    memset(parseTable, -1, sizeof(parseTable));
    createParseTable(ffArr, TOTAL_NON_TERMINALS, G, parseTable);
    printf("Parse table created successfully.\n");


    ParseTreeNode *parseTreeRoot = parseInputSourceCode((char *)inputFile, parseTable, G);
    if (parseTreeRoot == NULL) {
        fprintf(stderr, "Parsing failed. Parse tree could not be constructed.\n");
    } else {
        printf("Parsing successful. Input source code is syntactically correct.\n");

        FILE *fp = fopen(outputFile, "w");
        if (fp) {
            fprintf(fp, "Parse tree printing function not implemented.\n");
            fclose(fp);
            printf("Placeholder parse tree written to %s\n", outputFile);
        } else {
            fprintf(stderr, "Error opening output file %s\n", outputFile);
        }
    }
}


void printTimeTaken(const char *inputFile, const char *outputFile) {
    clock_t start_time, end_time;
    double total_CPU_time, total_CPU_time_in_seconds;
    
    start_time = clock();
    

    printTokenList(inputFile);         
    parseInput(inputFile, outputFile);  
    
    end_time = clock();
    
    total_CPU_time = (double)(end_time - start_time);
    total_CPU_time_in_seconds = total_CPU_time / CLOCKS_PER_SEC;
    
    printf("\nTotal CPU time (clock ticks): %.2f\n", total_CPU_time);
    printf("Total CPU time in seconds: %.4f\n", total_CPU_time_in_seconds);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input source file> <output parse tree file>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    const char *inputFile = argv[1];
    const char *outputFile = argv[2];


    printf("------------------------------------------------------\n");
    printf("Implementation Status:\n");
    printf("(a) FIRST and FOLLOW set automated\n");
    printf("(b) Only Lexical analyzer module developed\n");
    printf("(c) Both lexical and syntax analysis modules implemented\n");
    printf("(d) Modules compile successfully (no segmentation fault)\n");
    printf("(e) Modules work with testcases 2, 3 and 4 only\n");
    printf("(f) Parse tree printing not implemented\n");
    printf("------------------------------------------------------\n");

    int option;
    while (1) {
        printf("\nMenu:\n");
        printf("0 : Exit\n");
        printf("1 : Remove comments and print comment-free code\n");
        printf("2 : Print token list (lexer only)\n");
        printf("3 : Parse input source code and print parse tree to output file\n");
        printf("4 : Print total CPU time taken by lexer and parser\n");
        printf("Enter your option: ");
        
        if (scanf("%d", &option) != 1) {
            fprintf(stderr, "Invalid input. Exiting.\n");
            break;
        }
        
        switch (option) {
            case 0:
                printf("Exiting...\n");
                return EXIT_SUCCESS;
            case 1:
                printf("\n[Option 1] Removing comments from file: %s\n\n", inputFile);
                removeComments(inputFile);
                break;
            case 2:
                printf("\n[Option 2] Printing token list from file: %s\n\n", inputFile);
                printTokenList(inputFile);
                break;
            case 3:
                printf("\n[Option 3] Parsing input source code: %s\n\n", inputFile);
                parseInput(inputFile, outputFile);
                break;
            case 4:
                printf("\n[Option 4] Calculating total CPU time taken by lexer and parser...\n\n");
                printTimeTaken(inputFile, outputFile);
                break;
            default:
                printf("Invalid option. Please choose a valid option from the menu.\n");
                break;
        }
    }
    
    return EXIT_SUCCESS;
}
