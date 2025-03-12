/**
 * Function to compute FIRST and FOLLOW sets for the grammar
 * Based on the compiler construction assignment requirements
 * 
 * @param G The grammar for which FIRST and FOLLOW sets are to be computed
 * @return FirstAndFollow structure containing the computed sets
 */
FirstAndFollow ComputeFirstAndFollowSets(Grammar G) {
    FirstAndFollow FF;
    
    // Initialize data structures for FIRST and FOLLOW sets
    FF.first = (Set*)malloc(G.numNonTerminals * sizeof(Set));
    FF.follow = (Set*)malloc(G.numNonTerminals * sizeof(Set));
    
    for(int i = 0; i < G.numNonTerminals; i++) {
        FF.first[i] = createEmptySet();
        FF.follow[i] = createEmptySet();
    }
    
    // FIRST set computation
    computeFirstSets(G, &FF);
    
    // FOLLOW set computation
    computeFollowSets(G, &FF);
    
    return FF;
}

/**
 * Helper function to compute FIRST sets for all non-terminals
 */
void computeFirstSets(Grammar G, FirstAndFollow *FF) {
    int changed;
    
    // Add terminals directly to FIRST sets
    for(int i = 0; i < G.numRules; i++) {
        Rule rule = G.rules[i];
        int lhsIndex = getNonTerminalIndex(G, rule.lhs);
        
        // If first symbol on RHS is terminal, add to FIRST
        if(rule.rhs[0].isTerminal) {
            addToSet(&FF->first[lhsIndex], rule.rhs[0]);
        }
        
        // If ε is on RHS, add to FIRST
        if(rule.numRhs == 1 && isEpsilon(rule.rhs[0])) {
            addToSet(&FF->first[lhsIndex], createEpsilonSymbol());
        }
    }
    
    // Iteratively compute FIRST sets until no more changes
    do {
        changed = 0;
        
        for(int i = 0; i < G.numRules; i++) {
            Rule rule = G.rules[i];
            int lhsIndex = getNonTerminalIndex(G, rule.lhs);
            
            // Skip rules with terminals or ε as first symbol (already handled)
            if(rule.numRhs > 0 && (rule.rhs[0].isTerminal || isEpsilon(rule.rhs[0]))) {
                continue;
            }
            
            // Handle non-terminal at the beginning of RHS
            int k = 0;
            int canDeriveEpsilon = 1;
            
            while(k < rule.numRhs && canDeriveEpsilon) {
                canDeriveEpsilon = 0;
                
                if(!rule.rhs[k].isTerminal) {
                    int rhsIndex = getNonTerminalIndex(G, rule.rhs[k]);
                    
                    // Add FIRST(RHSk) - {ε} to FIRST(LHS)
                    for(int j = 0; j < FF->first[rhsIndex].count; j++) {
                        Symbol sym = FF->first[rhsIndex].symbols[j];
                        if(!isEpsilon(sym)) {
                            if(addToSet(&FF->first[lhsIndex], sym)) {
                                changed = 1;
                            }
                        }
                    }
                    
                    // If FIRST(RHSk) contains ε, we need to consider next symbol
                    if(containsEpsilon(FF->first[rhsIndex])) {
                        canDeriveEpsilon = 1;
                    }
                } else {
                    // If terminal, add to FIRST and stop
                    if(addToSet(&FF->first[lhsIndex], rule.rhs[k])) {
                        changed = 1;
                    }
                    break;
                }
                
                k++;
            }
            
            // If all symbols on RHS can derive ε, add ε to FIRST(LHS)
            if(canDeriveEpsilon && k == rule.numRhs) {
                if(addToSet(&FF->first[lhsIndex], createEpsilonSymbol())) {
                    changed = 1;
                }
            }
        }
    } while(changed);
}

/**
 * Helper function to compute FOLLOW sets for all non-terminals
 */
void computeFollowSets(Grammar G, FirstAndFollow *FF) {
    int changed;
    
    // Add end marker $ to FOLLOW of start symbol
    int startIndex = getNonTerminalIndex(G, G.startSymbol);
    addToSet(&FF->follow[startIndex], createEndMarkerSymbol());
    
    // Iteratively compute FOLLOW sets until no more changes
    do {
        changed = 0;
        
        for(int i = 0; i < G.numRules; i++) {
            Rule rule = G.rules[i];
            int lhsIndex = getNonTerminalIndex(G, rule.lhs);
            
            for(int j = 0; j < rule.numRhs; j++) {
                // Only interested in non-terminals on RHS
                if(rule.rhs[j].isTerminal) {
                    continue;
                }
                
                int rhsIndex = getNonTerminalIndex(G, rule.rhs[j]);
                
                // Rule: If A -> αBβ, add FIRST(β) - {ε} to FOLLOW(B)
                if(j < rule.numRhs - 1) {
                    int betaCanDeriveEpsilon = 1;
                    int k = j + 1;
                    
                    while(k < rule.numRhs && betaCanDeriveEpsilon) {
                        if(rule.rhs[k].isTerminal) {
                            // If terminal, add to FOLLOW and break
                            if(addToSet(&FF->follow[rhsIndex], rule.rhs[k])) {
                                changed = 1;
                            }
                            betaCanDeriveEpsilon = 0;
                        } else {
                            // If non-terminal, add FIRST - {ε} to FOLLOW
                            int nextIndex = getNonTerminalIndex(G, rule.rhs[k]);
                            
                            for(int m = 0; m < FF->first[nextIndex].count; m++) {
                                Symbol sym = FF->first[nextIndex].symbols[m];
                                if(!isEpsilon(sym)) {
                                    if(addToSet(&FF->follow[rhsIndex], sym)) {
                                        changed = 1;
                                    }
                                }
                            }
                            
                            // Check if it can derive ε
                            if(!containsEpsilon(FF->first[nextIndex])) {
                                betaCanDeriveEpsilon = 0;
                            }
                        }
                        
                        k++;
                    }
                    
                    // Rule: If A -> αBβ and β can derive ε, add FOLLOW(A) to FOLLOW(B)
                    if(betaCanDeriveEpsilon) {
                        if(unionSets(&FF->follow[rhsIndex], FF->follow[lhsIndex])) {
                            changed = 1;
                        }
                    }
                } 
                // Rule: If A -> αB, add FOLLOW(A) to FOLLOW(B)
                else if(j == rule.numRhs - 1) {
                    if(unionSets(&FF->follow[rhsIndex], FF->follow[lhsIndex])) {
                        changed = 1;
                    }
                }
            }
        }
    } while(changed);
}

/**
 * Helper function to create an empty set
 */
Set createEmptySet() {
    Set set;
    set.count = 0;
    set.capacity = 10;
    set.symbols = (Symbol*)malloc(set.capacity * sizeof(Symbol));
    return set;
}

/**
 * Helper function to add a symbol to a set
 * Returns 1 if added, 0 if already in set
 */
int addToSet(Set *set, Symbol symbol) {
    // Check if symbol already in set
    for(int i = 0; i < set->count; i++) {
        if(symbolsEqual(set->symbols[i], symbol)) {
            return 0;
        }
    }
    
    // Expand capacity if needed
    if(set->count == set->capacity) {
        set->capacity *= 2;
        set->symbols = (Symbol*)realloc(set->symbols, set->capacity * sizeof(Symbol));
    }
    
    // Add symbol to set
    set->symbols[set->count++] = symbol;
    return 1;
}

/**
 * Helper function to union two sets
 * Returns 1 if destination set was modified
 */
int unionSets(Set *dest, Set src) {
    int modified = 0;
    
    for(int i = 0; i < src.count; i++) {
        if(addToSet(dest, src.symbols[i])) {
            modified = 1;
        }
    }
    
    return modified;
}

/**
 * Helper function to check if a set contains epsilon
 */
int containsEpsilon(Set set) {
    for(int i = 0; i < set.count; i++) {
        if(isEpsilon(set.symbols[i])) {
            return 1;
        }
    }
    return 0;
}

/**
 * Helper function to get index of a non-terminal in the grammar
 */
int getNonTerminalIndex(Grammar G, Symbol nonTerminal) {
    for(int i = 0; i < G.numNonTerminals; i++) {
        if(symbolsEqual(G.nonTerminals[i], nonTerminal)) {
            return i;
        }
    }
    return -1; // Error: non-terminal not found
}

/**
 * Helper function to check if two symbols are equal
 */
int symbolsEqual(Symbol s1, Symbol s2) {
    // This implementation depends on how symbols are represented in your compiler
    // For example, if symbols have a unique ID:
    return s1.id == s2.id;
}

/**
 * Helper function to check if a symbol is epsilon
 */
int isEpsilon(Symbol s) {
    // This depends on how epsilon is represented in your compiler
    return s.isEpsilon;
}

/**
 * Helper function to create an epsilon symbol
 */
Symbol createEpsilonSymbol() {
    Symbol s;
    s.isTerminal = 0;
    s.isEpsilon = 1;
    s.id = EPSILON_ID; // Assuming EPSILON_ID is defined
    return s;
}

/**
 * Helper function to create an end marker symbol ($)
 */
Symbol createEndMarkerSymbol() {
    Symbol s;
    s.isTerminal = 1;
    s.isEpsilon = 0;
    s.id = END_MARKER_ID; // Assuming END_MARKER_ID is defined
    return s;
}
