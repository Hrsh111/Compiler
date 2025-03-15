#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "lexerDef.h"    // Contains tokenInfo, TwinBuffer, tokenStrings, etc.
#include "lexer.h"       // Contains init_lexer(), getNextToken(), initialise(), etc.
#include "parserDef.h"   // Contains Grammar, GrammarRule, ParseTree, ParseTreeNode, etc.
#include "stack.h"       // Contains stack functions used by the parser
#include "parser.h"
#include "lexer.h"
// Extern declarations (adjust as per your project structure)
extern TwinBuffer twinBuffer;
extern GrammarRule grammarRules[];
extern int numGrammarRules; // computed as sizeof(grammarRules)/sizeof(grammarRules[0])
extern int getNonTerminalIndex(const char *nonTerminal);
extern int getTerminalIndex(const char *terminal);
extern int isTerminal(const char *symbol);




// Main driver integrating both lexer and parser.
int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input source file> <output parse tree file>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    char *inputFile = argv[1];
    char *outputFile = argv[2];
    
    // ================================
    // Part 1: Lexical Analysis (Lexer)
    // ================================
    printf("========== Running Lexer ==========\n");
    
    // Open the source file using your initialise() function.
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
    
    // =======================================
    // Part 2: Syntax Analysis (Parser)
    // =======================================
    printf("\n========== Running Parser ==========\n");
    
    // Reinitialize the lexer so that parsing starts fresh.
    // This assumes init_lexer() resets necessary global state.
    init_lexer(inputFile);
    
    // Set up the grammar.
    Grammar G;
    G.rules = grammarRules;
    G.numRules = numGrammarRules;
    G.startSymbol = "program";
    
    // Compute FIRST and FOLLOW sets for the grammar.
    FirstFollow *ffArr = ComputeFirstAndFollowSets(&G, NUM_NONTERMINALS);
    
    // Create the predictive parse table.
    int parseTable[NUM_NONTERMINALS][NUM_TERMINALS];
    createParseTable(ffArr, NUM_NONTERMINALS, parseTable);
    
    // Start the parser and measure time.
    clock_t startParser = clock();
    ParseTreeNode *parseTreeRoot = parseInputSourceCode(inputFile, parseTable);
    clock_t endParser = clock();
    double elapsedParser = (double)(endParser - startParser) / CLOCKS_PER_SEC;
    
    if (parseTreeRoot == NULL) {
        fprintf(stderr, "Parsing failed.\n");
        return EXIT_FAILURE;
    }
    
    printf("Parsing successful. Time taken: %.4f seconds.\n", elapsedParser);
    printf("Input source code is syntactically correct...........\n");
    
    // Build the ParseTree structure and print it.
    ParseTree PT;
    PT.root = parseTreeRoot;
    printParseTree(&PT, outputFile);
    printf("Parse tree written to %s\n", outputFile);
    
    // Clean up (free allocated memory) if applicable.
    
    return EXIT_SUCCESS;
}
