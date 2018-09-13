//
// Created by Isaias Perez vega
//
// ----- VM ------//

// MACHINE CONSTANTS //
#define MAX_STACK_HEIGHT 2000
#define MAX_CODE_LENGTH 500
//#define NUM_REGISTERS 8

// USED FOR INSTRUCTION INFORMATION //
typedef struct instruction{
    int op;
    int r;
    int l;
    int m;
    int line;
} instruction;

// FUNCTION PROTOTYPES //
struct instruction* processInput(FILE* inputfile,const char* fileName);
void outInstructions(struct instruction* codeFile, int toFile, FILE* traceFile);
int locBase(int l, int base, int* stack);
void printRegFile(int* registerFile, FILE* traceFile, int v);
void printStackTrace(int* stack, int* registerFile, struct instruction IR, FILE* traceFile, int v);
void runVirtualMachine(FILE* traceFile, int v);


/////// GLOBAL VARS //////////
int codeCount, instruct, print, run;
int* AR;
char* opCodess[22] = {"lit", "rtn", "lod", "sto", "cal", "inc", "jmp", "jpc", "sio", "ned", "add", "sub", "mul", "div", "odd", "mod", "eql", "neq", "lss", "leq", "gtr", "geq"};

// CPU REGISTERS //
int SP, BP, PC;


// VIRTUAL MACHINE //
void runVirtualMachine(FILE* traceFile, int v) {
    // Handle input file //
    FILE* inputFile = NULL;
    const char* fileName = "assemblyCode.txt";
    //const char* fileName = "/Users/Isaias/Desktop/Compiler/assemblyCode.txt";
    
    instruction* codeFile = NULL;
    codeFile = processInput(inputFile, fileName);
    fclose(inputFile);
    //outInstructions(codeFile, toFile, traceFile);
    
    // CPU registers init //
    SP = 0; BP = 1; PC = 0;
    instruction IR = *(instruction*)malloc(sizeof(instruction));
    
    // Register file init //
    int* registerFile = malloc(sizeof(int) * NUM_REGISTERS);
    for (int x = 0; x < NUM_REGISTERS; x++) registerFile[x] = 0;
    
    // Stack init //
    int* stack = malloc(sizeof(int) * MAX_STACK_HEIGHT);
    AR = malloc(sizeof(int) * MAX_STACK_HEIGHT);
    for (int x = 0; x < MAX_STACK_HEIGHT; x++) {
        stack[x] = 0;
        AR[x] = 0;
    }
    stack[1] = 1;
    
    // Output
    if (v) {
        printf("\n\nProgram Stack Trace:\n");
        printf("Inital Values\t\t\t\tpc\tpb\tsp\n");
    }
    
    // Write to traceFile
    fprintf(traceFile,"\n\nProgram Stack Trace:\n");
    fprintf(traceFile,"Inital Values\t\t\tpc\tpb\tsp\n");
    
    run = 1;
    while(run) {
        
        // Fetch Instruction //
        IR = codeFile[PC];
        PC = PC + 1;
        print = 0;
        
        // Execute Instruction //
        switch (IR.op) {
            case 1: // LIT
                registerFile[IR.r] = IR.m;
                break;
            case 2: // RTN
                SP = BP - 1;                                // SP pointing to top of prev AR
                BP = stack[SP + 2];                         // Restore DL
                PC = stack[SP + 3];                         // Return control to caller
                break;
            case 3: //LOD
                registerFile[IR.r] = stack[locBase(IR.l, BP, stack) + IR.m - 1];
                break;
            case 4: // STO
                stack[locBase(IR.l, BP, stack) + IR.m -1] = registerFile[IR.r];
                break;
            case 5: // CAL
                AR[SP] = 1;
                stack[SP] = 0;                              // Return Val
                stack[SP + 1] = locBase(IR.l, BP, stack);   // Static Link
                stack[SP + 2] = BP;                         // Dynamic Link
                stack[SP + 3] = PC;                         // Return Address
                BP = SP + 1;                                // Base of Current AR
                PC = IR.m;                                  // PC at code/data location
                break;
            case 6: // INC
                SP = SP + IR.m;
                break;
            case 7: // JMP
                PC = IR.m;
                break;
            case 8: // JPC
                if (registerFile[IR.r] == 0) PC = IR.m;
                break;
            case 9: // SIO
                switch (IR.m) {
                    case 1: // PRINT
                        print = 1;
                        break;
                    case 2: // READ
                        printf("Enter value: ");
                        scanf("%d", &registerFile[0]);
                        break;
                    case 3: // HALT
                        run = 0; PC = 0; BP = 1; SP = 0;
                        break;
                    default:
                        break;
                }
                break;
            case 10: // NEG
                registerFile[IR.r] = registerFile[IR.l] * -1;
                break;
            case 11: // ADD
                registerFile[IR.r] = registerFile[IR.l] + registerFile[IR.m];
                break;
            case 12: // SUB
                registerFile[IR.r] = registerFile[IR.l] - registerFile[IR.m];
                break;
            case 13: // MUL
                registerFile[IR.r] = registerFile[IR.l] * registerFile[IR.m];
                break;
            case 14: // DIV
                registerFile[IR.r] = registerFile[IR.l] / registerFile[IR.m];
                break;
            case 15: // ODD
                registerFile[IR.r] = registerFile[IR.r] % 2;
                break;
            case 16: // MOD
                registerFile[IR.r] = registerFile[IR.l] % registerFile[IR.m];
                break;
            case 17: // EQL
                if (registerFile[IR.l] == registerFile[IR.m]) {
                    registerFile[IR.r] = 1;
                } else {
                    registerFile[IR.r] = 0;
                }
                break;
            case 18: // NEQ
                if (registerFile[IR.l] != registerFile[IR.m]) {
                    registerFile[IR.r] = 1;
                } else {
                    registerFile[IR.r] = 0;
                }
                break;
            case 19: // LSS
                if (registerFile[IR.l] < registerFile[IR.m]) {
                    registerFile[IR.r] = 1;
                } else {
                    registerFile[IR.r] = 0;
                }
                break;
            case 20: // LEQ
                if (registerFile[IR.l] <= registerFile[IR.m]) {
                    registerFile[IR.r] = 1;
                } else {
                    registerFile[IR.r] = 0;
                }
                break;
            case 21: // GTR
                if (registerFile[IR.l] > registerFile[IR.m]) {
                    registerFile[IR.r] = 1;
                } else {
                    registerFile[IR.r] = 0;
                }
                break;
            case 22: // GEQ
                if (registerFile[IR.l] >= registerFile[IR.m]) {
                    registerFile[IR.r] = 1;
                } else {
                    registerFile[IR.r] = 0;
                }
                break;
            default:
                break;
        }
        
        // Write to terminal and/or traceFile
        printStackTrace(stack, registerFile, IR, traceFile, v);
    }
    
    // Write to terminal
    if (v) {
        printf("\nProgram Output:\n%d\n", registerFile[0]);
    }
    
    // Write to traceFile
    fprintf(traceFile, "\nProgram Output:\n%d\n", registerFile[0]);
    
    
    
}


// READS INPUT FROM FIE, AND GENERATE CODEFILE //
struct instruction* processInput(FILE* inputFile, const char* fileName) {
    //char* filePath = "/Users/Isaias/Desktop/PMachine/PMachine/inputFile.txt";
    inputFile = fopen(fileName, "r");
    instruction* codeFile = (instruction*)malloc(sizeof(instruction) * MAX_CODE_LENGTH);
    if (inputFile != NULL) {
        codeCount = 0;
        while (!feof(inputFile)) {
            fscanf(inputFile, "%d%d%d%d", &codeFile[codeCount].op, &codeFile[codeCount].r, &codeFile[codeCount].l, &codeFile[codeCount].m);
            codeFile[codeCount].line = codeCount;
            codeCount++;
        }
        // Failsafe //
    } else {
        printf("Error reading file, check name and path!\n");
        exit(-1);
    }
    return codeFile;
}

// OUTPUT FORMATTED INSTRUCTIONS //
void outInstructions(struct instruction* codeFile, int toFile, FILE* traceFile) {
    if (toFile == 0) {
        printf("Program Op Printout:\n");
        for (int line = 0; line < codeCount; line++) {
            printf("%d %s %d %d %d\n", line, opCodess[codeFile[line].op - 1], codeFile[line].r, codeFile[line].l, codeFile[line].m);
        }
        printf("\n\n");
    } else {
        fprintf(traceFile,"Program Op Printout:\n");
        for (int line = 0; line < codeCount; line++) {
            fprintf(traceFile, "%d %s %d %d %d\n", line, opCodess[codeFile[line].op - 1], codeFile[line].r, codeFile[line].l, codeFile[line].m);
        }
        fprintf(traceFile, "\n\n");
    }
}

// FIND VARIABLE IN ACTIVATION RECORD L LEVELS DOWN //
int locBase(int level, int base, int* stack) {
    int var = base;
    while (level > 0) {
        var = stack[var];
        level--;
    }
    return var;
}

// SHOW REGISTER FILE STATUS //
void printRegFile(int* registerFile, FILE* traceFile, int v) {
    if (v == 1) {
        printf("RF: ");
        for (int x = 0; x < NUM_REGISTERS; x++) printf("%d ", registerFile[x]);
        printf("\n");
    }
    
    // Write to TraceFile
    fprintf(traceFile, "RF: ");
    for (int x = 0; x < NUM_REGISTERS; x++) fprintf(traceFile,"%d ", registerFile[x]);
    fprintf(traceFile, "\n");
}

// OUTPUT STACK TRACE FROM INSTRUCTION //
void printStackTrace(int* stack, int* registerFile, struct instruction IR, FILE* traceFile, int v) {
    if (v) {
        printf("%d\t%s\t%d\t%d\t%d\t %d\t%d\t%d\t ", IR.line, opCodess[IR.op - 1], IR.r, IR.l, IR.m, PC, BP, SP);
        for (int x = 0; x < SP; x++) {
            if (AR[x]) printf("| ");
            printf("%d ", stack[x]);
        }
        // Print Stack[0] if halt flag //
        if (run == 0) printf("%d", stack[0]);
        printf("\n");
        //printRegFile(registerFile, traceFile, v);
        printf("RF: ");
        for (int x = 0; x < NUM_REGISTERS; x++) printf("%d ", registerFile[x]);
        printf("\n");
        
        // Print value in register //
        if (print) printf("Print: %d\n", registerFile[IR.r ]);
        
    }
    
    // Write to traceFile
    fprintf(traceFile, "%d\t%s\t%d\t%d\t%d\t %d\t%d\t%d\t ", IR.line, opCodess[IR.op - 1], IR.r, IR.l, IR.m, PC, BP, SP);
    for (int x = 0; x < SP; x++) {
        if (AR[x]) fprintf(traceFile,"| ");
        fprintf(traceFile, "%d ", stack[x]);
    }
    // Print Stack[0] if halt flag //
    if (run == 0) fprintf(traceFile, "%d", stack[0]);
    fprintf(traceFile, "\n");
    //printRegFile(registerFile, traceFile, v);
    // Write to TraceFile
    fprintf(traceFile, "RF: ");
    for (int x = 0; x < NUM_REGISTERS; x++) fprintf(traceFile,"%d ", registerFile[x]);
    fprintf(traceFile, "\n");
    
    // Print value in register //
    if (print) fprintf(traceFile, "Print:  %d\n", registerFile[IR.r]);
}

