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
    AC,
    FC,
    OP,
    CP,
    EF,
    UK
} TokenType;

const char *tokenNames[] = {
    "VR", "NM", "IT", "SC", "PT", "IF", "PV", "AT", "AP", "FP", "AC", "FC", "OP", "CP", "EF", "UK"};

typedef struct
{
    TokenType type;
    char word[100];
} Token;

void printToken(Token token, FILE *destination)
{
    fprintf(destination, "%s  %s\n", tokenNames[token.type], token.word);
    // printf("%s  %s\n", tokenNames[token.type], token.word);
}

int isKeyword(char *str)
{
    return (strcmp(str, "int") == 0 || strcmp(str, "scanf") == 0 || strcmp(str, "printf") == 0 || strcmp(str, "if") == 0);
}

TokenType getKeywordTokenType(char *str)
{
    if (strcmp(str, "int") == 0)
        return IT;
    if (strcmp(str, "scanf") == 0)
        return SC;
    if (strcmp(str, "printf") == 0)
        return PT;
    if (strcmp(str, "if") == 0)
        return IF;
    printf("ERRO: Keyword não reconhecida.\n");
    return UK;
}

TokenType getCharTokenType(char c)
{
    switch (c)
    {
    case ';':
        return PV;
    case '(':
        return AP;
    case ')':
        return FP;
    case '{':
        return AC;
    case '}':
        return FC;
    case '+':
        return OP;
    case '-':
        return OP;
    case '*':
        return OP;
    case '/':
        return OP;
    default:
        printf("ERRO: Caracter não reconhecido: '%c'.\n", c);
        return UK;
    }
}

TokenType getCompOrAtTokenType(char *str)
{
    if (strcmp(str, "=") == 0)
        return AT;
    if (strcmp(str, "==") == 0 || strcmp(str, "<") == 0 || strcmp(str, ">") == 0 || strcmp(str, "<=") == 0 || strcmp(str, ">=") == 0)
        return CP;
    printf("ERRO: Caracter não reconhecido: '%s'.\n", str);
    return UK;
}

Token getNextToken(FILE *source)
{
    Token token;
    token.type = UK;
    int index = 0;
    char ch;

    while ((ch = fgetc(source)) != EOF)
    {
        if (isspace(ch))
        {
            continue;
        }
        else if (isalpha(ch) || ch == '_')
        {
            token.word[index++] = ch;
            while (isalnum(ch = fgetc(source)) || ch == '_')
            {
                token.word[index++] = ch;
            }
            ungetc(ch, source);
            token.word[index] = '\0';

            if (isKeyword(token.word))
            {
                token.type = getKeywordTokenType(token.word);
            }
            else
            {
                token.type = VR;
            }
            return token;
        }
        else if (isdigit(ch))
        {
            token.word[index++] = ch;
            while (isdigit(ch = fgetc(source)))
            {
                token.word[index++] = ch;
            }
            ungetc(ch, source);
            token.word[index] = '\0';
            token.type = NM;
            return token;
        }
        else if (ch == '=' || ch == '<' || ch == '>')
        {
            token.word[index++] = ch;
            if ((ch = fgetc(source)) == '=')
            {
                token.word[index++] = ch;
            }
            else
            {
                ungetc(ch, source);
            }
            token.word[index] = '\0';
            token.type = getCompOrAtTokenType(token.word);
            return token;
        }
        else
        {
            token.word[index++] = ch;
            token.word[index] = '\0';
            token.type = getCharTokenType(ch);
            return token;
        }
    }

    token.type = EF;
    strcpy(token.word, "EOF");
    return token;
}

int main()
{
    FILE *source = fopen("input.c", "r");
    FILE *destination = fopen("tokens.txt", "w");
    Token token;

    do
    {
        token = getNextToken(source);
        printToken(token, destination);
    } while (token.type != EF);

    fclose(source);
    return 0;
}
