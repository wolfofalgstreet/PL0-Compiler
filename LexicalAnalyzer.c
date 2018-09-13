//
// Created by Isaias Perez Vega
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_IDENT_LENGTH 11
#define MAX_NUM_LENGTH 5
#define MAX_TOKENS 1000

// Internal representation of PL/0 symbols //
typedef enum{
    nulsym = 1, identsym, numbersym, plussym, minussym, multsym, slashsym, oddsym, eqlsym, neqsym, lessym, leqsym, gtrsym, geqsym, lparentsym, rparentsym, commasym, semicolonsym, periodsym, becomessym, beginsym, endsym, ifsym, thensym , whilesym, dosym, callsym, constsym, varsym, procsym, writesym, readsym, elsesym
} tokenType;

/Users/Isaias/Desktop/FALL 2017/SYSTEMS SOFTWARE/HOMEWORK/PL0Compiler/CompilerFinal/compiler.c
// Lexeme table token //
typedef struct {
    int type;
    char ident[MAX_IDENT_LENGTH + 1];
} token;

// Symbol representation //
char* symbolRep[34] = {"", "nulsym", "identsym", "numbersym", "plussym", "minussym", "multsym", "slashsym", "oddsym", "eqlsym", "neqsym", "lessym", "leqsym", "gtrsym", "geqsym", "lparentsym", "rparentsym", "commasym", "semicolonsym", "periodsym", "becomessym", "beginsym", "endsym", "ifsym", "thensym" , "whilesym", "dosym", "callsym", "constsym", "varsym", "procsym", "writesym", "readsym", "elsesym"};


// -- FUNCTION SIGNATURES -- //
int skipComments(FILE* inputFile, char currentC);
void printLexemeList(token* lexemeList, int tokenCount);
void parseSpecialSymbol(FILE* inputFile, token* currentToken, char currentC);
void parseString(FILE* inputFile, char currentC, token* currentToken);
token* parseResWord( token* currentToken);
void parseNumberToken(FILE* inputFile, token* currenToken, char currentC);
void isInvalidVar(token* tokenTable, int start, int end);
void outLexemeTable(token* tokenTable, int tokenCount);
void outLexemeList(token* tokenTable, int tokenCount, FILE* traceFile, int l);
void outSourceCode(const char* fileName, FILE* traceFile);
void outSymRep(token* tokenTable, int tokenCount, FILE* traceFile, int l);
void lexicalScan(FILE* traceFile, int l);


///// END LEXICAL //////////

/////// LEXICAL SCANNER ///////////

// Parameter indicates output: console = 0, file = 1
void lexicalScan(FILE* traceFile, int l) {
    
    //UNCOMMENT THIS FOR SUBMISSION //
    //const char* fileName = "case5.txt";
    const char* fileName = "sourceCode.txt";
    
    // Open File, proceed only if process was successful
    FILE* lexFileOut = NULL;
    FILE* inputFile = NULL;
    inputFile = fopen(fileName, "r");
    lexFileOut = fopen("lexTable.txt", "w");
    if (inputFile != NULL || lexFileOut!= NULL) {
        int tokenCount = 0;
        char currentC;
        token* currToken = NULL;
        token* tokenTable = malloc(sizeof(token) * MAX_TOKENS);
        int start = -1; int end = -1;
        
        // Run through all source code
        while (!feof(inputFile)) {
            
            // Initialize new token for current char
            currToken = malloc(sizeof(token));
            currToken->type = 0;
            currentC = getc(inputFile);
            
            // Base case, end processing chars if at end of file
            if (feof(inputFile)) break;
            
            // Skip comments
            int foundComment = 0;
            if (currentC == '/') foundComment = skipComments(inputFile, currentC);
            
            // Ignore white space and comments
            if (isspace(currentC) || foundComment) continue;
            
            // Char is a letter, could be an identifier or reserved word
            if (isalpha(currentC) == 1) {
                parseString(inputFile, currentC,currToken);
                
            // Char is a digit, has to be a number
            } else if (isdigit(currentC)){
                parseNumberToken(inputFile, currToken, currentC);
                
                // Char is a special symbol, tokenize if part of the language
            } else {
                parseSpecialSymbol(inputFile, currToken, currentC);
            }
            
            // Add token to table
            tokenTable[tokenCount] = *currToken;
            
            // Flags to detect declaration of var that does not start with a letter
            if (currToken->type == varsym) start = tokenCount;
            if (currToken->type == semicolonsym && start != -1) end = tokenCount;
            
            // If no variable name violation has occured continue, else warning terminate
            if (start != -1 && end != -1) {
                isInvalidVar(tokenTable, start, end);
                start = -1;
            }
            
            tokenCount++;
        }
        
        // Output Source Code to TraceFile
        outSourceCode(fileName, traceFile);
        
        // Output Lexeme Table
        //outLexemeTable(tokenTable, tokenCount);
        
        // Output Lexeme List file for Parser
        outLexemeList(tokenTable, tokenCount, lexFileOut, 0);
        
        // Out lexime List for trace
        outLexemeList(tokenTable, tokenCount, traceFile, l);
        
        // Out Lexeme symbolic representation for trace
        outSymRep(tokenTable, tokenCount, traceFile, l);
        
        fclose(lexFileOut);
        
        // Fail safe
    } else {
        printf("Could not open file!, check name and path\n");
    }
}

// Print Lexeme DEBUG //
void printLexemeList(token* lexemeList, int tokenCount) {
    for (int x = 0; x < tokenCount; x++) {
        printf("index: %d   val: %s  type: %d\n", x, lexemeList[x].ident, lexemeList[x].type);
    }
}


// Output lexeme as table //
void outLexemeTable(token* tokenTable, int tokenCount) {
    printf("Lexeme Table:\nlexeme\t\t\ttoken type\n");
    for (int x = 0; x < tokenCount; x++) {
        if (strlen(tokenTable[x].ident) > 10) {
            printf("%s\t\t%d\n", tokenTable[x].ident, tokenTable[x].type);
        } else if (strlen(tokenTable[x].ident) > 7) {
            printf("%s\t\t%d\n", tokenTable[x].ident, tokenTable[x].type);
        } else {
            printf("%s\t\t\t%d\n", tokenTable[x].ident, tokenTable[x].type);
        }
    }
    printf("\n");
}

// Output symbolic representation //
void outSymRep(token* tokenTable, int tokenCount, FILE* traceFile, int l) {
    // Print to console
    if (l) {
        for (int x = 0; x < tokenCount; x++) {
            printf("%s ", symbolRep[tokenTable[x].type]);
            if (tokenTable[x].type == identsym) {
                printf("%s ", tokenTable[x].ident);
            } else if (tokenTable[x].type == numbersym) {
                printf("%d ", atoi(tokenTable[x].ident));
            }
        }
        printf("\n\n");
        
        
    }
    // Print to file
    fprintf(traceFile, "Internal Symbol Representation:\n\n");
    for (int x = 0; x < tokenCount; x++) {
        fprintf(traceFile,"%s ", symbolRep[tokenTable[x].type]);
        if (tokenTable[x].type == identsym) {
            fprintf(traceFile,"%s ", tokenTable[x].ident);
        } else if (tokenTable[x].type == numbersym) {
            fprintf(traceFile,"%d ", atoi(tokenTable[x].ident));
        }
    }
    fprintf(traceFile,"\n\n");
}

// Output lexeme as list //
void outLexemeList(token* tokenTable, int tokenCount, FILE* traceFile, int l) {
    // Print to console
    if (l) {
        //printf("Lexeme List:\n");
        for (int x = 0; x < tokenCount; x++) {
            printf("%d ", tokenTable[x].type);
            if (tokenTable[x].type == identsym || tokenTable[x].type == numbersym) printf("%s ", tokenTable[x].ident);
        }
        printf("\n\n");
        
        
    }
    // Print to file
    for (int x = 0; x < tokenCount; x++) {
        fprintf(traceFile,"%d ", tokenTable[x].type);
        if (tokenTable[x].type == identsym || tokenTable[x].type == numbersym) {
            fprintf(traceFile,"%s ", tokenTable[x].ident);
        }
    }
    fprintf(traceFile, "\n\n");
}

// Ooutput Source Code from file //
void outSourceCode(const char* fileName, FILE* traceFile) {
    FILE* inputFile = NULL;
    inputFile = fopen(fileName, "r");
    if (inputFile) {
        char current;
        fprintf(traceFile, "Source Program:\n\n");
        while (!feof(inputFile)) {
            current = getc(inputFile);
            if (current == EOF) break;
            fprintf(traceFile,"%c", current);
        }
        fprintf(traceFile, "\n\n");
        fprintf(traceFile, "Lexime List:\n\n");
    }
}


// If there is a declared variable that did not start with a letter, warning terminate //
void isInvalidVar(token* tokenTable, int start, int end) {
    // We will check each token in the table between start and end
    // Flags mark points var ..... ;
    // All tokens in between should be identifiers or commas
    if (start < end && (start != -1)) {
        for (int x = start + 1; x < end; x++) {
            if (tokenTable[x].type == identsym || tokenTable[x].type == commasym) {
                // we do nothing
            } else {
                fprintf(stderr, "WARNING!: All declared variables should start with a letter\n\t  ");
                for (int y = start; y <= end; y++) {
                    fprintf(stderr, "%s", tokenTable[y].ident);
                    if (tokenTable[y].type == commasym || tokenTable[y].type == varsym) fprintf(stderr, " ");
                }
                fprintf(stderr, "\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

// Skips comment and returns 1 to alert that a comment was skipped /*...*/
int skipComments(FILE* inputFile, char currentC) {
    
    // Discard until comment ends
    int comment = 1; char prev;
    if (currentC == '/') {
        
        // We will look ahead to determine if this is a comment and
        // if it is a comment, where does it end
        prev = currentC; currentC = getc(inputFile);
        if (currentC == '*') {
            while (comment) {
                prev = currentC;
                currentC = getc(inputFile);
                if (prev == '*' && currentC == '/') comment = 0;
            }
            return 1;
        } else {
            ungetc(currentC, inputFile);
        }
    }
    return 0;
}

// Parse token that consists of digitis //
void parseNumberToken(FILE* inputFile, token* currenToken, char currentC) {
    int numberTooLong = 0; int numLength = 0; // Control flags
    
    while (isdigit(currentC) && !feof(inputFile)) {
        
        // Only add if there is space reserved
        if (numLength < MAX_IDENT_LENGTH) currenToken->ident[numLength] = currentC;
        currentC = getc(inputFile);
        numLength++;
        
        // Flag violation of number length
        if (numLength > MAX_NUM_LENGTH) numberTooLong = 1;
    }
    
    // Handle warning termination
    if (numberTooLong) {
        fprintf(stderr, "WARNING!: Number %s is too long!\n", currenToken->ident);
        exit(EXIT_FAILURE);
    }
    
    // Finalize parsing token
    currenToken->ident[numLength + 1] = '\0';
    currenToken->type = numbersym;
    
    // Char not part of token
    ungetc(currentC, inputFile);
}

// Parse special character //
void parseSpecialSymbol(FILE* inputFile, token* currentToken, char currentC) {
    int index = 0; currentToken->ident[index] = currentC;
    
    // Parse tokens that consist of one symbol
    switch (currentC) {
        case '+':
            currentToken->ident[index + 1] = '\0';
            currentToken->type = plussym;
            break;
        case '-':
            currentToken->ident[index + 1] = '\0';
            currentToken->type = minussym;
            break;
        case '*':
            currentToken->ident[index + 1] = '\0';
            currentToken->type = multsym;
            break;
        case ',':
            currentToken->ident[index + 1] = '\0';
            currentToken->type = commasym;
            break;
        case '.':
            currentToken->ident[index + 1] = '\0';
            currentToken->type = periodsym;
            break;
        case ';':
            currentToken->ident[index + 1] = '\0';
            currentToken->type = semicolonsym;
            break;
        case '=':
            currentToken->ident[index + 1] = '\0';
            currentToken->type = eqlsym;
            break;
        case '(':
            currentToken->ident[index + 1] = '\0';
            currentToken->type = lparentsym;
            break;
        case ')':
            currentToken->ident[index +1] = '\0';
            currentToken->type = rparentsym;
            break;
        case '/':
            currentToken->ident[index + 1] = '\0';
            currentToken->type = slashsym;
            break;
    }
    
    // Parse tokens that consist on more than one symbol
    // we must look ahead to determine which token this is
    switch (currentC) {
            // Token can be '<' or '<> or '<='
        case '<':
            currentC = getc(inputFile);
            if (isspace(currentC) || isalpha(currentC) || isdigit(currentC)) {
                currentToken->type = lessym;
                currentToken->ident[index + 1] = '\0';
                ungetc(currentC, inputFile);
            } else if (currentC == '>') {
                currentToken->type = neqsym;
                currentToken->ident[index + 1] = currentC;
                currentToken->ident[index + 2] = '\0';
            } else if (currentC == '=') {
                currentToken->ident[index + 1] = currentC;
                currentToken->ident[index + 2] = '\0';
                currentToken->type = leqsym;
            } else {
                ungetc(currentC, inputFile);
            }
            break;
            // Token can be '>' or '>='
        case '>':
            currentC = getc(inputFile);
            if (isspace(currentC) || isalpha(currentC) || isdigit(currentC)) {
                currentToken->type = gtrsym;
                currentToken->ident[index + 1] = '\0';
                ungetc(currentC, inputFile);
            } else if (currentC == '=') {
                currentToken->type = geqsym;
                currentToken->ident[index + 1] = currentC;
                currentToken->ident[index + 2] = '\0';
            } else {
                ungetc(currentC, inputFile);
            }
            break;
            // Token must be ':='
        case ':':
            currentC = getc(inputFile);
            if (currentC != '=') {
                fprintf(stderr, "WARNING!: Expecting '=' after ':'\n");
                exit(EXIT_FAILURE);
            }
            currentToken->ident[index + 1] = currentC;
            currentToken->ident[index + 2] = '\0';
            currentToken->type = becomessym;
            break;
    }
    
    // Detect any symbol that is not part of the language
    if (currentToken->type == 0) {
        fprintf(stderr, "WARNING: Could not recognize symbol: %s\n", currentToken->ident);
        exit(EXIT_FAILURE);
    }
}

// Parse identifier or reserved word token //
void parseString(FILE* inputFile, char currentC, token* currentToken) {
    int tokenLength = 0; int identIsTooLong = 0;
    
    while ((isalpha(currentC) || isdigit(currentC)) && !feof(inputFile)) {
        // For safety, only add if there is reserved space
        if (tokenLength < MAX_IDENT_LENGTH) currentToken->ident[tokenLength] = currentC;
        
        // Flag violation of token length
        if (tokenLength > MAX_IDENT_LENGTH - 1) identIsTooLong = 1;
        
        // If a digit is encountered, this must be an identifier
        if (isdigit(currentC)) currentToken->type = identsym;
        
        currentC = getc(inputFile);
        tokenLength++;
    }
    currentToken->ident[tokenLength] = '\0';
    
    // Handle identifier too long, termination warning
    if (identIsTooLong) {
        fprintf(stderr, "WARNING!: Identifier is too long, could only read '%s'\n", currentToken->ident);
        exit(EXIT_FAILURE);
    }
    
    // Token type hasn't been found yet, check if its a reserved word
    if (currentToken->type == 0) {
        currentToken = parseResWord(currentToken);
        
        // Not a reserved word, has to be an identifier
        if (currentToken->type == 0) currentToken->type = identsym;
    }
    
    // Char not part of token
    ungetc(currentC, inputFile);
}

// Parse reserved word token if string matches //
token* parseResWord(token* currentToken) {
    if (strcmp(currentToken->ident, "const") == 0) currentToken->type = constsym;
    if (strcmp(currentToken->ident, "var") == 0) currentToken->type = varsym;
    if (strcmp(currentToken->ident, "procedure") == 0) currentToken->type = procsym;
    if (strcmp(currentToken->ident, "call") == 0) currentToken->type = callsym;
    if (strcmp(currentToken->ident, "begin") == 0) currentToken->type = beginsym;
    if (strcmp(currentToken->ident, "end") == 0) currentToken->type = endsym;
    if (strcmp(currentToken->ident, "if") == 0) currentToken->type = ifsym;
    if (strcmp(currentToken->ident, "then") == 0) currentToken->type = thensym;
    if (strcmp(currentToken->ident, "else") == 0) currentToken->type = elsesym;
    if (strcmp(currentToken->ident, "while") == 0) currentToken->type = whilesym;
    if (strcmp(currentToken->ident, "do") == 0) currentToken->type = dosym;
    if (strcmp(currentToken->ident, "read") == 0) currentToken->type = readsym;
    if (strcmp(currentToken->ident, "write") == 0) currentToken->type = writesym;
    if (strcmp(currentToken->ident, "null") == 0) currentToken->type = nulsym;
    if (strcmp(currentToken->ident, "odd") == 0) currentToken->type = oddsym;
    
    return currentToken;
}




