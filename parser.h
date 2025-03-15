#ifndef PARSER_H
#define PARSER_H

#include "parserDef.h"
#define NUM_TERMINALS (sizeof(tokenStrings)/sizeof(tokenStrings[0]))
#define NUM_NONTERMINALS (sizeof(nonTerminals)/sizeof(nonTerminals[0]))

// Global grammar rule array and count (defined in parser.c)
extern GrammarRule grammarRules[];
extern int numGrammarRules;

// Main parser interface:
// Computes FIRST and FOLLOW sets for the given grammar.
FirstFollow* ComputeFirstAndFollowSets(Grammar *G, int numNonTerminals);

// Creates the predictive parse table from the FIRST/FOLLOW sets.
void createParseTable(FirstFollow *ffArr, int numNonTerminals, int table[][NUM_TERMINALS]);

// Parses the input source code (given by testcaseFile) using the predictive parse table.
// Returns the root of the constructed parse tree.
ParseTreeNode* parseInputSourceCode(char *testcaseFile, int parseTable[][NUM_TERMINALS]);

// Prints the parse tree in inorder to the given output file.
void printParseTree(ParseTree *PT, char *outfile);

// Helper functions for grammar symbol handling:
// Returns 1 if the symbol is a non-terminal; 0 otherwise.
int isNonTerminal(const char *symbol);

// Returns the index of a given non-terminal in the nonTerminals array.
int getNonTerminalIndex(const char *nonTerminal);
int containsSymbol(char **arr, int count, const char *symbol);


#endif // PARSER_H
