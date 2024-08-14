#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum
{
    VR,
    NM,
    IT,
    SC,
    PT,
    IF,
    PV,
    AT,
    AP,
    FP,
    OP,
    CP,
    EF,
    UK
} TokenType;

const char *tokenNames[] = {
    "VR", "NM", "IT", "SC", "PT", "IF", "PV", "AT", "AP", "FP", "OP", "CP", "EF", "UK"};

TokenType getTokenTypeFromName(const char *name)
{
    int numTokenTypes = sizeof(tokenNames) / sizeof(tokenNames[0]);
    for (int i = 0; i < numTokenTypes; i++)
    {
        if (strcmp(name, tokenNames[i]) == 0)
        {
            return (TokenType)i;
        }
    }
    return UK;
}

TokenType updateToken(FILE *source)
{
    char token_str[3];
    token_str[2] = '\0';

    while (fgetc(source) != '\n' && !feof(source))
        ;
    token_str[0] = fgetc(source);
    token_str[1] = fgetc(source);
    return getTokenTypeFromName(token_str);
}

void rollbackToken(FILE *source, TokenType token)
{
    ungetc(tokenNames[token][1], source);
    ungetc(tokenNames[token][0], source);
    ungetc('\n', source);
}

int checkPV(FILE *source)
{
    TokenType nextToken;

    nextToken = updateToken(source);
    if (nextToken == PV)
        return 1;

    printf("ERRO: Sentenças devem ser finalizadas com \";\".\n");
    return 0;
}

int checkExpression(FILE *source)
{
    TokenType nextToken;

    nextToken = updateToken(source);
    if (nextToken != NM && nextToken != VR) {
        printf("ERRO: Toda expressão deve começar ou com um número, ou com uma variável.\n");
        return 0;
    }

    nextToken = updateToken(source);
    if (nextToken == OP)
    {
        nextToken = updateToken(source);
        if (nextToken != NM && nextToken != VR){
            printf("ERRO: Após um operador, sempre deve ter um número ou uma variável.\n");
            return 0;
        }
    }
    else
    {
        rollbackToken(source, nextToken);
    }

    return 1;
}

int intCheck(FILE *source)
{
    TokenType nextToken;

    nextToken = updateToken(source);
    if (nextToken != VR) {
        printf("ERRO: Em uma declaração, após o \"int\", sempre deve vir uma variável.\n");
        return 0;
    }

    return checkPV(source);
}

int varCheck(FILE *source)
{
    TokenType nextToken;

    nextToken = updateToken(source);
    if (nextToken != AT) {
        printf("ERRO: Após uma variável, é necessário haver uma atribuição (\"=\").\n");
        return 0;
    }

    int success = checkExpression(source);

    if (success == 0) {
        printf("ERRO: A expressão após a atribuição de variável está incorreta.\n");
        return 0;
    }

    return checkPV(source);
}

int scanfCheck(FILE *source)
{
    TokenType nextToken;

    nextToken = updateToken(source);
    if (nextToken != AP) {
        printf("ERRO: Após um \"scanf\", sempre deve vir um \"(\".\n");
        return 0;
    }

    nextToken = updateToken(source);
    if (nextToken != VR) {
        printf("ERRO: Após um \"scanf(\", sempre deve vir uma variável.\n");
        return 0;
    }

    nextToken = updateToken(source);
    if (nextToken != FP) {
        printf("ERRO: Um scanf deve ser sempre finalizado com um \")\".\n");
        return 0;
    }

    return checkPV(source);
}

int printfCheck(FILE *source)
{
    TokenType nextToken;

    nextToken = updateToken(source);
    if (nextToken != AP) {
        printf("ERRO: Após um \"printf\", sempre deve vir um \"(\".\n");
        return 0;
    }

    int success = checkExpression(source);

    if (success == 0) {
        printf("ERRO: A expressão dentro do printf está incorreta.\n");
        return 0;
    }

    nextToken = updateToken(source);
    if (nextToken != FP) {
        printf("ERRO: Um printf deve ser sempre finalizado com um \")\".\n");
        return 0;
    }

    return checkPV(source);
}

int ifCheck(FILE *source)
{
    TokenType nextToken;
    int success;

    nextToken = updateToken(source);
    if (nextToken != AP) {
        printf("ERRO: Após um \"if\", sempre deve vir um \"(\".\n");
        return 0;
    }

    success = checkExpression(source);

    if (success == 0) {
        printf("ERRO: A expressão dentro do if está incorreta.\n");
        return 0;
    }

    nextToken = updateToken(source);
    if (nextToken != CP) {
        printf("ERRO: Dentro do if é preciso ter algum sinal de comparação, e.g., \"==\", \">\", \"<\".\n");
        return 0;
    }

    success = checkExpression(source);

    if (success == 0) {
        printf("ERRO: A expressão dentro do if está incorreta.\n");
        return 0;
    }

    nextToken = updateToken(source);
    if (nextToken != FP) {
        printf("ERRO: Todo if deve ser finalizado com \")\".\n");
        return 0;
    }

    nextToken = updateToken(source);
    if (nextToken != VR) {
        printf("ERRO: Após um \"if()\", sempre deve vir uma atribuição de variável.\n");
        return 0;
    }

    return varCheck(source);
}

int instrucionCheck(TokenType token, FILE *source)
{
    switch (token)
    {
    case IT:
        return intCheck(source);
    case VR:
        return varCheck(source);
    case SC:
        return scanfCheck(source);
    case PT:
        return printfCheck(source);
    case IF:
        return ifCheck(source);
    default:
        return 0;
    }
}

int main()
{
    FILE *source = fopen("tokens.txt", "r+");

    TokenType token;
    char token_str[3];
    token_str[2] = '\0';
    int success = 1;

    token_str[0] = fgetc(source);
    token_str[1] = fgetc(source);
    token = getTokenTypeFromName(token_str);

    while (token != EF && success == 1)
    {
        success = instrucionCheck(token, source);

        token = updateToken(source);
    }

    while (fgetc(source) != '\n' && !feof(source))
        ;
    ungetc('\n', source);
    fprintf(source, "  OK\n");

    printf("Success: %d\n", success);

    fclose(source);
    return 0;
}
