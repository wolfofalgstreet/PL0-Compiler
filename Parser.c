//
// Created by Isaias Perez Vega
//

// ----- PARSER---- //
#define MAX_TOKEN_TABLE_LEN 1000
#define MAX_IDENT_LENGTH 11
#define MAX_INSTRUCTIONS 300
#define NUM_REGISTERS 8
#define MAX_LEXI_LEVELS 3

// INSTRUCTION SET ARCHITECTURE //
typedef enum {
    LIT = 1, RTN, LOD, STO, CAL, INC, JMP, JPC, SIO, NEG, ADD, SUB, MUL, DIV, ODD, MOD, EQL, NEQ, LSS, LEQ, GTR, GEQ
} opCodes;


// SYMBOL TOKEN REPRESENTATION //
typedef struct {
    int kin;                            // const = 1, var = 2, proc = 3
    char name[MAX_IDENT_LENGTH];        // name up to 11 chars
    int val;                            // number in ASCII
    int level;                          // L level
    int addr;                           // M address
} symbol;

// MACHINE CODE INSTRUCTION REPRESENTATION //
typedef struct {
    int opCode;
    int reg;
    int lLevel;
    int offset;
} mInstruction;

// DEFINES THE TYPE OF SYMBOL //
typedef enum{
    constant = 1, variable, method
} typeofSymbol;


// CAINTAINS ALL ERROR MESSAGES //
char* errors[26] = {
    "",                                                         // NULL
    "Use = instead of :=\n",                                    // 1
    "= must be followed by a number.\n",                        // 2
    "Identifier must be followed by =\n",                       // 3
    "const, var, procedure must be followed by identifier\n",   // 4
    "Semicolon or comma missing\n",                             // 5
    "Incorrect symbol after procedure declaration\n",           // 6
    "Statement expected\n",                                     // 7
    "Incorrect symbol after statement part in block\n",         // 8
    "Period expected\n",                                        // 9
    "Semicolon between statements missing\n",                   // 10
    "Undeclared identifier\n",                                  // 11
    "Assignment to constant or procedure is not allowed\n",     // 12
    "Assignment operator expected\n",                           // 13
    "call must be followed by an identifier\n",                 // 14
    "Call of a constant or variable is meaningless\n",          // 15
    "then   expected\n",                                        // 16
    "Semicolon or } expected\n",                                // 17
    "do   expected\n",                                          // 18
    "Incorrect symbol following statement\n",                   // 19
    "Relational operator expected\n",                           // 20
    "Expression must not contain a procedure identifier\n",     // 21
    "Right parenthesis missing\n",                              // 22
    "The preceeding factor cannot begin with this symbol\n",    // 23
    "An expression cannot begin with this symbol\n",            // 24
    "This number is too large\n"};                              // 25

// THIS ARRAY IS USED TO STORE THE NAME OF THE PROCEDURES //
char ARIdent[MAX_LEXI_LEVELS][MAX_IDENT_LENGTH + 1];


// FUNCTION SIGNATURES //
token* importTokens(char* filename, token* tokenTable);
void getNextToken(token* tokenTable);
void errorFound(int error);
void factor(token* tokenTable);
void term(token* tokenTable);
void expression(token* tokenTable);
void condition(token* tokenTable);
void statement(token* tokenTable);
void block(token* tokenTable);
void program(token* tokenTbale);
void instStack(int val);
int relOp(void);
symbol* lookupSym(char* identifier, token* tokenTable);
void genInstruction(int op, int r, int l, int m);
void addSymbol(int kind, char* name, int value, int addr, token* tokenTable);
void printmCode(FILE* outFile, int a);
void printList(token* table);
void parseCodeGenerator(FILE* traceFile, int a);
void procedure (int jmpadd, token* tokenTable);



// GLOBALS //
int tokenCount = 0;         // Count of tokens in lex list
int tokenIndex = 0;         // Index of token in lex list
int lexLevel = 0;           // Block Level
int symIndex = 0;           // Symbol table index
token* currentToken;
symbol* symbolTable[MAX_TOKEN_TABLE_LEN];
mInstruction* mCode [MAX_INSTRUCTIONS];
int mIndex = 0;             // Code Index
int registers[NUM_REGISTERS];
int regIndex = 0;           // Registers stack address
int lodDrive = 0;           // Helps manage registers when doing operations
int varCount = 0;


void parseCodeGenerator(FILE* traceFile, int a) {
    // Source File
    char* lexicalFileName = "lexTable.txt";
    //char* lexicalFileName = "/Users/Isaias/Desktop/FALL 2017/SYSTEMS SOFTWARE/HOMEWORK/Compiler/Compiler/lexTable.txt";
    
    // Machine code file for vm
    FILE* outFile = fopen("assemblyCode.txt", "w");
    
    // Read lexical table file generated from the scanner
    token* tokenTable = NULL;
    tokenTable = importTokens(lexicalFileName, tokenTable);
    
    // Run parser / code generation
    program(tokenTable);
    
    // No errors encounterd while parsing
    if (a) {
        printf("No errors, program is syntatically correct.\n\n");
    }
    fprintf(traceFile,"No errors, program is syntatically correct.\n\n");
    
    // Write mcode to file for VM
    printmCode(outFile, 0);
    fclose(outFile);
    
    // Write mcode to traceFile
    fprintf(traceFile, "\nMachine Code: \n\n");
    printmCode(traceFile, a);
    
    
    
}

// PRINT ASSEMBLY CODE //
void printmCode(FILE* outFile, int toFile) {
    if (toFile) {
        printf("Assembly:\n\n");
        for (int x = 0; x < mIndex; x++) {
            mInstruction* current = mCode[x];
            printf("%d %d %d %d\n", current->opCode, current->reg, current->lLevel, current->offset);
            
        }
    }
    for (int x = 0; x < mIndex; x++) {
        mInstruction* current = mCode[x];
        fprintf(outFile,"%d %d %d %d\n", current->opCode, current->reg, current->lLevel, current->offset);
        
    }
}


// READS LEXIME LIST FILE //
token* importTokens(char* fileName, token* tokenTable) {
    // If able to open file, read and load token into an array
    
    FILE* tokenFile = NULL;  tokenFile = fopen(fileName, "r");
    if (tokenFile != NULL) {
        
        token* currToken = NULL;
        tokenTable = (token*)malloc(sizeof(token) * MAX_TOKEN_TABLE_LEN);
        
        // Process all tokens in file
        while (!feof(tokenFile)) {
            
            currToken = (token*)malloc(sizeof(token));
            // Fail safe
            strcpy(currToken->ident, "");
            
            // Only read non white space, we get the type of token first
            fscanf(tokenFile, "%d", &currToken->type);
            
            // This will avoid adding an empty token to the end of the table
            if (feof(tokenFile)) break;
            
            // Read token value if its an identifier or number, then
            // the value associated with it must be read as well
            if (currToken->type == numbersym || currToken->type == identsym) {
                fscanf(tokenFile, "%s", currToken->ident);
            }
            
            
            // Store token in table
            tokenTable[tokenCount] = *currToken;
            tokenCount++;
            
            
            
        }
        
        fclose(tokenFile);
        
    } else {
        printf("Error while reading token file\n");
        exit(EXIT_FAILURE);
    }
    
    return tokenTable;
}


// DISPLAY CONTENTS OF TOKENTABLE CREATED FROM INPUT FILE //
void printList(token* table) {
    for (int x = 0; x < tokenCount; x++) {
        printf("Token[%d]  type: %d  val: %s\n", x, table[x].type, table[x].ident);
    }
}

// GENERATE MACHINE CODE AND ADD TO THE CODEFILE //
void genInstruction(int op, int r, int l, int m) {
    mInstruction* newCode = (mInstruction*)malloc(sizeof(mInstruction));
    newCode->opCode = op;
    newCode->reg = r;
    newCode->lLevel = l;
    newCode->offset = m;
    mCode[mIndex] = newCode;
    mIndex++;
}

// PARSE TOKENS AND GENERATE MACHINE CODE //
// THIS IS THE TOP MOST LEVEL (START) OF THE GRAMMAR //
void program(token* tokenTable) {
    // implements class
    // program :== block ".".
    
    getNextToken(tokenTable);
    
    // Do block syntatic class
    block(tokenTable);
    
    if (currentToken->type != periodsym) errorFound(9);     // Expecting period after block
    
    // Generate halt flag instruction
    genInstruction(SIO, 0, 0, 3);
    
}

// ADD SYMBOL TO TABLE //
void addSymbol(int kind, char* name, int value, int addr, token* tokenTable) {
    // No need to add if it already exists in table
    if (lookupSym(name, tokenTable)) return;
    
    // Create symbol
    symbol* newSymbol = (symbol*)malloc(sizeof(symbol));
    strcpy(newSymbol->name, name);
    newSymbol->kin = kind;
    newSymbol->val = value;
    newSymbol->addr = addr;
    newSymbol->level = lexLevel;
    
    symbolTable[symIndex] = newSymbol;
    symIndex++;
    
}

// RUN BLOCK SYNTATIC CLASS //
void block(token* tokenTable) {
    // implements class
    // block :== const-declaration var-declaration statement.
    
    int arOffset = 4;
    
    // Flag helps track what symbols to delete once we leave the procedure
    //lexLevel++;
    int prevSX = symIndex;
    
    
    // Need to save the current address in case of procedure
    int jmpAdd = mIndex;
    genInstruction(JMP, 0, 0, 0);
    
    // Do constant first
    // constdeclaration ::= [ “const” ident "=" number {"," ident "=" number} “;"] .
    if (currentToken->type == constsym) {
        do {
            getNextToken(tokenTable);
            // save identifier for constant
            token* identifier = currentToken;
            
            if (identifier->type != identsym) errorFound(4);    // Expecting identifier
            
            getNextToken(tokenTable);
            
            if (currentToken->type != eqlsym) errorFound(3);   // Expecting "=" after identifier
            
            getNextToken(tokenTable);
            
            if (currentToken->type != numbersym) errorFound(2); // Ecpecting number after "="
            
            // Insert constant to table
            addSymbol(constant, identifier->ident, atoi(currentToken->ident), 0, tokenTable);
            
            getNextToken(tokenTable);
            
        } while (currentToken->type == commasym);
        
        if (currentToken->type != semicolonsym) errorFound(10);  // Expecting semicolon
        
        getNextToken(tokenTable);
    }
        
    // Do variable second
    // var-declaration  ::= [ "var" ident {"," ident} “;"].
    if (currentToken->type == varsym) {
            
        do {
            getNextToken(tokenTable);
            if (currentToken->type != identsym) errorFound(4);  // Expecting identifier
                
                
            addSymbol(variable, currentToken->ident, currentToken->type, arOffset, tokenTable);
            getNextToken(tokenTable);
                
            arOffset++;
                
        } while (currentToken->type == commasym);
            
        if (currentToken->type != semicolonsym) errorFound(10);     // Expecting semicolon
            getNextToken(tokenTable);
    }
    
    // If procedure exists do
    // procedure-dec ::= {"procedure" ident ";" block ";"}
    while (currentToken->type == procsym) {
        procedure(jmpAdd, tokenTable);
    }
    
    // Since we scoped the procedure, we can now allocate necessary
    // space for its AR
    mCode[jmpAdd]->offset = mIndex;
    
    // Allocate AR space
    genInstruction(INC, 0, 0, arOffset);
    
    statement(tokenTable);
    
    symIndex = prevSX;
    //lexLevel--;
}

// RUN PROCEDURE SYNTATIC CLASS //
void procedure (int jmpadd, token* tokenTable) {
    // implement class
    // procedure :== {"procedure" ident ";" block ";"}
    
    getNextToken(tokenTable);
    
    // Entered procedure class, expecting ident
    if (currentToken->type != identsym) {
        errorFound(4);
    }
    
    // Save identifier for procedure
    strcpy(ARIdent[lexLevel], currentToken->ident);
    
    // Procedure needs to be tracked, so add to symbol table but do not account
    // for it to generate code, address starts after the position fo the jmp instruction
    addSymbol(method, currentToken->ident, -1, jmpadd + 1, tokenTable);
    
    // Have to update the the address of procedure lexical level
    symbolTable[symIndex - 1]->level = lexLevel;
    lexLevel++;
    
    // Expecting semicolon
    getNextToken(tokenTable);
    if (currentToken->type != semicolonsym) {
        errorFound(6);          // Incorrect symbol after procedure declaration
    }
    
    getNextToken(tokenTable);
    block(tokenTable);
    
    // Expecting semicolon after block class
    if (currentToken->type != semicolonsym) {
        errorFound(10);
    }
    
    // Finally gen instruction to return from subroutine
    genInstruction(RTN, 0, 0, 0);
    
    getNextToken(tokenTable);
    
    // Procedurte is done, restore level and symbol table index
    lexLevel--;
    
}


// RUN STATEMENT SYNTATIC CLASS //
void statement(token* tokenTable) {
    // implements class
    // statement :==[ ident ":=" expression | "call" ident
    //              | "begin" statement { ";" statement } "end"
    //              | "if" condition "then" statement [“else" statement]
    //              ​| "while" condition "do" statement
    //              | “read” ident | “write” ident | e ] .
    
    // Assignment //
    // Case: ident ":=" expression
    if (currentToken->type == identsym) {
        
        // Detect syntatic errors
        symbol* currSymbol = lookupSym(currentToken->ident, tokenTable);
        if (currSymbol == NULL)  {
            errorFound(11);                          // Undeclared var
        } else if (currSymbol->kin != variable) {
            errorFound(12);                         // Assignment to const or proc
        }
        
        getNextToken(tokenTable);
        
        // Expecting an assignment operator
        if (currentToken->type != becomessym) {
            if (currentToken->type == eqlsym) {
                errorFound(1);                      // using "=" instead of ":="
            }
            errorFound(19);                         // Incorrecty symbol after statemnt
        }
        
        getNextToken(tokenTable);
        expression(tokenTable);
        
        regIndex--;
        genInstruction(STO, regIndex, lexLevel - currSymbol->level, currSymbol->addr);
    }
    
    // Call //
    // Case: "call" ident
    else if (currentToken->type == callsym) {
        
        // Need to check if this procedure has been declared
        getNextToken(tokenTable);
        symbol* tempSym = lookupSym(currentToken->ident, tokenTable);
        
        // Expecting identifier and that it is already declared
        if (currentToken->type != identsym) {
            errorFound(14);                 // Call must be followed by an identifier
        }
        if (tempSym == NULL) {
            errorFound(11);                 // Undeclared identifier
        }
        
        // Identifier found, check if this is actually a procedure
        if (tempSym->kin != method) {
            errorFound(15);                 // Calling a constant or variable
        }
        
        // Instruction to jump to the procedure
        genInstruction(CAL, 0, lexLevel - tempSym->level, tempSym->addr);
        
        getNextToken(tokenTable);
    }

    
    // Begin //
    // Case: "begin" statement { ";" statement } "end"
    else if (currentToken->type == beginsym) {
        getNextToken(tokenTable);
        statement(tokenTable);
        
        // Expecting a semicolon or end symbol
        if (currentToken->type != semicolonsym) {
            errorFound(10);                            // Not a semicolon
        }
        
        // There are more statements
        while (currentToken->type == semicolonsym) {
            getNextToken(tokenTable);
            statement(tokenTable);
            
            if (currentToken->type != semicolonsym && currentToken->type != endsym) {
                errorFound(10);
            }
        }
        
        // Expecting end symbol
        if (currentToken->type != endsym) errorFound(8); // Expecting end symbol
        
        getNextToken(tokenTable);
        
    }
    
    // If, then, else//
    // Case: "if" condition "then" statement ["else" statement] .
    else if (currentToken->type == ifsym) {
        getNextToken(tokenTable);
        
        condition(tokenTable);
        
        // Exptecting "then"
        if (currentToken->type != thensym)  errorFound(16); // Expecting "then" after "if"
        
        // Save current address to modify the JPC address later
        int currPC = mIndex;
        regIndex--;
        genInstruction(JPC, regIndex, 0, 0);
        //regIndex++;
        
        // Continue to statemnt class
        getNextToken(tokenTable);
        statement(tokenTable);
        
        // We need to save the current address again in case
        // of "else"
        int elseIndex = mIndex;
        
        genInstruction(JMP, 0, 0, 0);
        
        mCode[currPC]->offset = mIndex;
        
        // Check if we have an "else" class
        if (currentToken->type == elsesym) {
            getNextToken(tokenTable);
            statement(tokenTable);
        }
        
        // Need to modify the JMP address to where we are at now
        mCode[elseIndex]->offset = mIndex;
    }
    
    // While //
    // Case: "while" condition "do" statement .
    if (currentToken->type == whilesym) {
        
        // Saving current instruction location
        int startIndex = mIndex;
        
        getNextToken(tokenTable);
        condition(tokenTable);
        
        // Current instruction index
        int endIndex = mIndex;
        
        // May not enter loop in the first place
        regIndex--;
        genInstruction(JPC, regIndex, 0, 0);
        
        if (currentToken->type != dosym) errorFound(18);    // Expecting "do" after "while"
        
        getNextToken(tokenTable);
        statement(tokenTable);
        
        // May restart loop
        genInstruction(JMP, 0, 0, startIndex);
        
        // Condition failed, must fix offset for instruction
        mCode[endIndex]->offset = mIndex;
    }
    
    // Read //
    // Case: “read” ident
    else if (currentToken->type == readsym) {
        getNextToken(tokenTable);
        
        if (currentToken->type != identsym) errorFound(26); // Expecting identifier
        
        // Identifier should've been declared
        symbol* currSym = lookupSym(currentToken->ident, tokenTable);
        if (currSym == NULL) errorFound(11);                // Identifier not found
        
        // Read input
        genInstruction(SIO, regIndex, 0, 2);
        regIndex++;
        
        if (currSym->kin != variable) errorFound(12);       // Expecting a variable
        
        // Store val
        regIndex--;
        genInstruction(STO, regIndex, lexLevel - currSym->level, currSym->addr);
        
        getNextToken(tokenTable);
    }
    
    // Write //
    // Case: “write” ident .
    else if (currentToken->type == writesym) {
        getNextToken(tokenTable);
        
        if (currentToken->type != identsym) errorFound(26); // Expecting identifier
        
        symbol* currSym = lookupSym(currentToken->ident, tokenTable);
        if (currSym == NULL) errorFound(11);                // Identifier not found
        
        // Var, load a variable from mem
        if (currSym->kin == variable) {
            genInstruction(LOD, regIndex, lexLevel - currSym->level, currSym->addr);
            regIndex++;
        }
        
        // Constant, push to register
        if (currSym->kin == constant) {
            genInstruction(LIT, regIndex, 0, currSym->val);
        }
        
        // Write value to screen
        regIndex--;
        genInstruction(SIO, regIndex, 0, 1);
        
        
        getNextToken(tokenTable);
        
    }
}


// RUN CONDITION SYNTATIC CLASS //
void condition(token* tokenTable) {
    // implements class
    // condition :== "odd" expression | expression relOp expression .
    
    // Case: "odd" expression
    if (currentToken->type == oddsym) {
        genInstruction(ODD, regIndex, 0, 0);
        
        getNextToken(tokenTable);
        expression(tokenTable);
        
        // Case: expression relOp expression
    } else  {
        expression(tokenTable);
        
        // Did not follow grammar, expecting relOp
        int relOpType = relOp();
        if (relOpType == 0) {
            errorFound(20);
        }
        getNextToken(tokenTable);
        expression(tokenTable);
        
        regIndex = regIndex - 1;
        genInstruction(relOpType, regIndex - 1, regIndex - 1, regIndex);
    }
}

// RETURN ZERO IF NOT A RELOP, ELSE RETURN RELOP TYPE //
int relOp(void) {
    
    int type = 0;
    switch (currentToken->type) {
        case eqlsym:
            type = 17;
            break;
        case neqsym:
            type = 18;
            break;
        case lessym:
            type = 19;
            break;
        case leqsym:
            type = 20;
            break;
        case geqsym:
            type = 22;
            break;
        case gtrsym:
            type = 21;
            break;
        default:
            break;
    }
    return type;
}

// RUN EXPRESSION SYNTATIC CLASS //
void expression(token* tokenTable) {
    // implements class
    // expression :== ["+" | "-"] term { ("+" | "-") term} .
    
    int operation;
    // Check for a case where this is a negation
    if (currentToken->type == plussym || currentToken->type == minussym) {
        operation = currentToken->type;
        getNextToken(tokenTable);
        term(tokenTable);
        
        
        // This is a negation
        if (operation == minussym) {
            genInstruction(NEG, regIndex, 0, 0);    // check parameters of generated instruction
            
        }
        
        // Regular expression
    } else {
        term(tokenTable);
    }
    
    while (currentToken->type == plussym || currentToken->type == minussym) {
        operation = currentToken->type;
        getNextToken(tokenTable);
        term(tokenTable);
        
        if (operation == plussym) {
            regIndex = regIndex - 1;
            genInstruction(ADD, regIndex - 1, regIndex - 1, regIndex);
            
        } else {
            regIndex = regIndex - 1;
            genInstruction(SUB, regIndex - 1, regIndex - 1, regIndex);
            
        }
    }
}


// RUN TERM SYNTATIC CLASS //
void term(token* tokenTable) {
    // implements class
    // term :== factor {("*" | "/") factor} .
    
    // Do factor syntatic class
    factor(tokenTable);
    
    // {"*" | "/") factor }
    while (currentToken->type == multsym || currentToken->type == slashsym) {
        int operation = currentToken->type;
        getNextToken(tokenTable);
        
        // Do factor syntatic class
        factor(tokenTable);
        
        // Generate instruction
        if (operation == multsym) {
            regIndex = regIndex - 1;
            genInstruction(MUL, regIndex - 1, regIndex - 1, regIndex);
            
        } else {
            genInstruction(DIV, regIndex - 1, regIndex - 1, regIndex);
            
        }
    }
}

// SEARCH SYMBOL IN TABLE, RETURN IT IF FOUND //
symbol* lookupSym(char* identifier, token* tokenTable) {
    // Look top to bottom for identifier
    for (int x = symIndex - 1; x >= 0; x--) {
        
        // Check scope of identifiers
        if (symbolTable[x]->level <= lexLevel) {
            if (strcmp(identifier, symbolTable[x]->name) == 0) {
                return symbolTable[x];
            }
        }
    }
    return NULL;
}

// RUN FACTOR SYNTATIC CLASS //
void factor(token* tokenTable) {
    // implements class
    // factor :== ident | number | "(" expression ")" .
    
    // Check if an identifier was declared in valid scope
    if (currentToken->type == identsym ) {
        symbol* currentSym = lookupSym(currentToken->ident, tokenTable);
        // Not found in record
        if (currentSym == NULL) {
            errorFound(11);
        }
        
        getNextToken(tokenTable);
        
        // If this is a variable, retrieve from memory
        if (currentSym->kin == variable) {
            
            genInstruction(LOD, regIndex, lexLevel - currentSym->level, currentSym->addr);
            //lodDrive = regIndex;
            regIndex++;
        }
        
        // If this is a constant
         else if (currentSym->kin == constant) {
            genInstruction(LIT, regIndex, lexLevel - currentSym->level, currentSym->val);
            regIndex++;
        }
    }
    
    // Number
    else if (currentToken->type == numbersym) {
        genInstruction(LIT, regIndex, 0, atoi(currentToken->ident));
        regIndex++;
        getNextToken(tokenTable);
        
        
        // "(" expression ")"
    } else if (currentToken->type == lparentsym) {
        getNextToken(tokenTable);
        
        // Do expression syntatic class
        expression(tokenTable);
        
        // Throw an error if the token isn't ")"
        if (currentToken->type != rparentsym) errorFound(22);
        getNextToken(tokenTable);
    } else {
        
        // Didnt follow factor production
        errorFound(24);
    }
    
}


// OUTPUTS SPECIFIC ERROR MSG AND TERMINATES PROGRAM //
void errorFound(int error) {
    printf("Error Code: %s\n", errors[error]);
    exit(EXIT_FAILURE);
}


// HELPS RETRIEVE TOKENS FROM TABLE //
void getNextToken(token* tokenTable) {
    currentToken = &tokenTable[tokenIndex];
    tokenIndex++;
}

