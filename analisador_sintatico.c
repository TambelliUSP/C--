#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    VR, NM, IT, SC, PT, IF, PV, AT, AP, FP, AC, FC, OP, CP, EF, UNKNOWN
} TokenType;

const char* tokenNames[] = {
    "VR", "NM", "IT", "SC", "PT", "IF", "PV", "AT", "AP", "FP", "AC", "FC", "OP", "CP", "EF", "UNKNOWN"
};

typedef struct {
    TokenType type;
    char word[100];
} Token;

int main() {
    FILE* source = fopen("input.c", "r");
    Token token;

    do {
        token = getNextToken(source);
        printToken(token);
    } while (token.type != EF);

    fclose(source);
    return EXIT_SUCCESS;
}
