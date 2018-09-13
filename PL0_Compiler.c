//
// Created by Isaias Perez Vega
//

#include <stdio.h>
#include <stdlib.h>

// Compiler Modules
#include "LexicalScan.c"
#include "Parser.c"
#include "VirtualMachine.c"


// PL0 Compiler will use command line parameters to determine if
// the lexical list, machine code, and or virtual machine trace should be printed.  
int main(int argc, const char * argv[]) {
    
    FILE* traceFile = fopen("traceFile.txt", "w");
    
    int l = 0, a = 0, v = 0;
    
    // Run and print nothing to console except in and out
    if (argc == 0) {
        // Do nothing
        
        // Print different types of output
    } else if (argc > 0) {
        for (int i = 0; i < argc; i++) {
            
            // Print Lexime List
            if (strcmp(argv[i], "-l") == 0) {
                l = 1;
            }
            
            // Print Machine Code
            if (strcmp(argv[i], "-a") == 0) {
                a = 1;
            }
            
            // Print VM Trace
            if (strcmp(argv[i], "-v") == 0) {
                v = 1;
            }
        }
    }
    
    // Execute compiler process
    lexicalScan(traceFile, l);
    parseCodeGenerator(traceFile, a);
    runVirtualMachine(traceFile, v);
    fclose(traceFile);
    
    
    return 0;
}