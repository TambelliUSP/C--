#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    VR, NM, IT, SC, PT, IF, PV, AT, AP, FP, OP, CP, EF, UK
} TokenType;

const char* tokenNames[] = {
    "VR", "NM", "IT", "SC", "PT", "IF", "PV", "AT", "AP", "FP", "OP", "CP", "EF", "UK"
};

int freeRegister = 0;

typedef enum {
    R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12
} Registers;

const char* registerNames[] = {
    "R1", "R2", "R3", "R4", "R5", "R6", "R7", "R8", "R9", "R10", "R11", "R12"
};

typedef struct {
    Registers reg;
    char name[100];
} Variable;

Variable variableTable[12];

TokenType getTokenTypeFromName(const char* name) {
    int numTokenTypes = sizeof(tokenNames) / sizeof(tokenNames[0]);
    for (int i = 0; i < numTokenTypes; i++) {
        if (strcmp(name, tokenNames[i]) == 0) {
            return (TokenType)i;
        }
    }
    return UK;
}

TokenType updateToken(FILE* source) {
    char token_str[3];
    token_str[2] = '\0';

    while (fgetc(source) != '\n' && !feof(source));
    token_str[0] = fgetc(source);
    token_str[1] = fgetc(source);
    return getTokenTypeFromName(token_str);
}


void rollbackToken(FILE* source, TokenType token) {
    ungetc(tokenNames[token][1], source);
    ungetc(tokenNames[token][0], source);
    ungetc('\n', source);
}

int checkExpression(FILE* source) {
    TokenType nextToken;

    nextToken = updateToken(source);
    if (nextToken != NM && nextToken != VR)
        return 0;

    nextToken = updateToken(source);
    if (nextToken == OP) {
        nextToken = updateToken(source);
        if (nextToken != NM && nextToken != VR)
            return 0;
    }
    else {
        rollbackToken(source, nextToken);
    }
    
    return 1;
}


int variableDef(FILE* source) {
    updateToken(source);

    char ch;
    Variable variable;
    int index = 0;

    while ((ch = fgetc(source)) != '\n') {
        if (isspace(ch)) {
            continue;
        } else if (isalpha(ch) || ch == '_') {
            variable.name[index++] = ch;
            while (isalnum(ch = fgetc(source)) || ch == '_') {
                variable.name[index++] = ch;
            }
            ungetc(ch, source);
            variable.name[index] = '\0';
        }
    }

    variable.reg = freeRegister;
    freeRegister ++;

    // colocar variable na variable table
    updateToken(source);
}

int varCheck(FILE* source) {
    TokenType nextToken;

    nextToken = updateToken(source);
    if (nextToken != AT)
        return 0;

    checkExpression(source);

    return checkPV(source);
}

int scanfCheck(FILE* source) {
    TokenType nextToken;

    nextToken = updateToken(source);
    if (nextToken != AP)
        return 0;

    nextToken = updateToken(source);
    if (nextToken != VR)
        return 0;

    nextToken = updateToken(source);
    if (nextToken != FP)
        return 0;
    
    return checkPV(source);
}

int printfCheck(FILE* source) {
    TokenType nextToken;

    nextToken = updateToken(source);
    if (nextToken != AP)
        return 0;

    checkExpression(source);

    nextToken = updateToken(source);
    if (nextToken != FP)
        return 0;
    
    return checkPV(source);
}

int ifCheck(FILE* source) {
    TokenType nextToken;

    nextToken = updateToken(source);
    if (nextToken != AP)
        return 0;
    
    checkExpression(source);

    nextToken = updateToken(source);
    if (nextToken != CP)
        return 0;

    checkExpression(source);

    nextToken = updateToken(source);
    if (nextToken != FP)
        return 0;
    
    nextToken = updateToken(source);
    if (nextToken != VR)
        return 0;

    return varCheck(source);
}

int instrucionCheck(TokenType token, FILE* source) {
    switch (token)
    {
    case IT: return variableDef(source);
    case VR: return varCheck(source);
    case SC: return scanfCheck(source);
    case PT: return printfCheck(source);
    case IF: return ifCheck(source);
    default: return 0;
    }
}


int main() {
    FILE* source = fopen("tokens.txt", "r+");
    
    TokenType token;
    char token_str[3];
    token_str[2] = '\0';
    int success = 1;

    token_str[0] = fgetc(source);
    token_str[1] = fgetc(source);
    token = getTokenTypeFromName(token_str);

    while (token != EF && success == 1) {
        success = instrucionCheck(token, source);

        token = updateToken(source);
    }

    while (fgetc(source) != '\n' && !feof(source));
    ungetc('\n', source);
    fprintf(source, "  OK\n");

    printf("Success: %d\n", success);

    fclose(source);
    return 0;
}
