#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    VR, NM, IT, SC, PT, IF, PV, AT, AP, FP, AC, FC, OP, CP, EF, UK
} TokenType;

const char* tokenNames[] = {
    "VR", "NM", "IT", "SC", "PT", "IF", "PV", "AT", "AP", "FP", "AC", "FC", "OP", "CP", "EF", "UK"
};

TokenType getTokenTypeFromName(const char* name) {
    int numTokenTypes = sizeof(tokenNames) / sizeof(tokenNames[0]);
    for (int i = 0; i < numTokenTypes; i++) {
        if (strcmp(name, tokenNames[i]) == 0) {
            return (TokenType)i;
        }
    }
    return UK;
}

TokenType updateLine(FILE* source) {
    char token_str[2];
    char ch;

    while ((ch = fgetc(source)) != '\n');
    ungetc(ch, source);
    fprintf(source, "  ok");
    fgetc(source);
    token_str[0] = fgetc(source);
    token_str[1] = fgetc(source);
    return getTokenTypeFromName(token_str);
}

int checkPV(FILE* source) {
    TokenType nextToken;

    nextToken = updateLine(source);
    if (nextToken == PV)
        return 1;

    return 0;
}

int checkExpression(FILE* source) {
    TokenType nextToken;

    nextToken = updateLine(source);
    if (nextToken != NM && nextToken != VR)
        return 0;

    nextToken = updateLine(source);
    if (nextToken != OP)
        return 0;

    nextToken = updateLine(source);
    if (nextToken != NM && nextToken != VR)
        return 0;
    
    return 1;
}


int intCheck(FILE* source) {
    TokenType nextToken;

    nextToken = updateLine(source);
    if (nextToken != VR)
        return 0;

    return checkPV(source);
}

int varCheck(FILE* source) {
    TokenType nextToken;

    nextToken = updateLine(source);
    if (nextToken != AT)
        return 0;

    checkExpression(source);

    return checkPV(source);
}

int scanfCheck(FILE* source) {
    TokenType nextToken;

    nextToken = updateLine(source);
    if (nextToken != AP)
        return 0;

    nextToken = updateLine(source);
    if (nextToken != VR)
        return 0;

    nextToken = updateLine(source);
    if (nextToken != FP)
        return 0;
    
    return checkPV(source);
}

int printfCheck(FILE* source) {
    TokenType nextToken;

    nextToken = updateLine(source);
    if (nextToken != AP)
        return 0;

    checkExpression(source);

    nextToken = updateLine(source);
    if (nextToken != FP)
        return 0;
    
    return checkPV(source);
}

int ifCheck(FILE* source) {
    TokenType nextToken;

    nextToken = updateLine(source);
    if (nextToken != AP)
        return 0;
    
    checkExpression(source);

    nextToken = updateLine(source);
    if (nextToken != CP)
        return 0;

    checkExpression(source);

    nextToken = updateLine(source);
    if (nextToken != FP)
        return 0;
    
    varCheck(source);
}

int instrucionCheck(TokenType token, FILE* source) {
    switch (token)
    {
    case IT: intCheck(source);
    case VR: varCheck(source);
    case SC: scanfCheck(source);
    case PT: printfCheck(source);
    case IF: ifCheck(source);
    default: return 0;
    }
}


int main() {
    FILE* source = fopen("tokens.txt", "r+");
    
    TokenType token;
    char token_str[2];
    char ch;

    token_str[0] = fgetc(source);
    token_str[1] = fgetc(source);
    token = getTokenTypeFromName(token_str);
    printf("TOKEN antes do while: %s", token);

    while (token != EF) {
        instrucionCheck(token, source);

        token = updateLine(source);
    }

    printf("a");
    while ((ch = fgetc(source)) != '\n');
    printf("b");
    ungetc(ch, source);
    fprintf(source, "  OK");

    fclose(source);
    return 0;
}
