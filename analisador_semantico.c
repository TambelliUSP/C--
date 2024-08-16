#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define NUMBEROFREGISTERS 10

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

int freeRegister = 0;
int labelCounter = 0;

typedef enum
{
    R3,
    R4,
    R5,
    R6,
    R7,
    R8,
    R9,
    R10,
    R11,
    R12
} Registers;

const char *registerNames[] = {
    "R3", "R4", "R5", "R6", "R7", "R8", "R9", "R10", "R11", "R12"};

typedef struct
{
    Registers reg;
    char name[100];
} Variable;

Variable variableTable[NUMBEROFREGISTERS];

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

int variableToReg(FILE *source)
{
    char ch;
    char variableName[100];
    int index = 0;

    while ((ch = fgetc(source)) != '\n')
    {
        if (isspace(ch))
        {
            continue;
        }
        else if (isalpha(ch) || ch == '_')
        {
            variableName[index++] = ch;
            while (isalnum(ch = fgetc(source)) || ch == '_')
            {
                variableName[index++] = ch;
            }
            ungetc(ch, source);
            variableName[index] = '\0';
        }
    }
    ungetc(ch, source);

    for (int i = 0; i < NUMBEROFREGISTERS; i++)
    {
        if (strcmp(variableName, variableTable[i].name) == 0)
            return (variableTable[i].reg);
    }

    printf("Variavel '%s' nao foi declarada.\n", variableName);
    return -1;
}

int readNumber(FILE *source)
{
    char ch;
    char numberString[30];
    int index = 0;

    while ((ch = fgetc(source)) != '\n')
    {
        if (isspace(ch))
        {
            continue;
        }
        else if (isdigit(ch))
        {
            numberString[index++] = ch;
            while (isdigit(ch = fgetc(source)))
            {
                numberString[index++] = ch;
            }
            ungetc(ch, source);
            numberString[index] = '\0';
        }
    }
    ungetc(ch, source);

    return atoi(numberString);
}

void readComparator(FILE *source, char *comparatorString)
{
    char ch;
    int index = 0;

    while ((ch = fgetc(source)) != '\n')
    {
        if (isspace(ch))
        {
            continue;
        }
        else if (ch == '=' || ch == '<' || ch == '>')
        {
            comparatorString[index++] = ch;
            if ((ch = fgetc(source)) == '=')
            {
                comparatorString[index++] = ch;
            }
            else
            {
                ungetc(ch, source);
            }
            comparatorString[index] = '\0';
        }
    }
    ungetc(ch, source);

    return;
}

int defineOperation(FILE *source, FILE *destination, char *destinationReg)
{
    char instruction[100];
    int isImmediateOperation;
    int firstRegOrNum;

    TokenType nextToken = updateToken(source);

    if (nextToken == VR)
    {
        isImmediateOperation = 0;

        firstRegOrNum = variableToReg(source);

        if (firstRegOrNum < 0)
            return 0;
    }
    else
    {
        isImmediateOperation = 1;

        firstRegOrNum = readNumber(source);
    }

    nextToken = updateToken(source);
    if (nextToken == OP)
    {
        char ch;
        while (isspace(ch = fgetc(source)))
            ;

        switch (ch)
        {
        case '+':
            strcpy(instruction, "add ");
            break;
        case '-':
            strcpy(instruction, "sub ");
            break;
        case '*':
            strcpy(instruction, "mul ");
            break;
        case '/':
            strcpy(instruction, "sdiv ");
            break;
        default:
            return 0;
            break;
        }
        strcat(instruction, destinationReg);
        strcat(instruction, ", ");

        nextToken = updateToken(source);
        if (isImmediateOperation == 0)
        {
            strcat(instruction, registerNames[firstRegOrNum]);
        }
        else if (isImmediateOperation == 1)
        {
            char assistantInstruction[100];

            strcpy(assistantInstruction, "mov R0, ");
            strcat(assistantInstruction, "#");
            char numberString[30];
            sprintf(numberString, "%d", firstRegOrNum);
            strcat(assistantInstruction, numberString);

            fprintf(destination, "%s\n", assistantInstruction);

            strcat(instruction, "R0");
        }

        strcat(instruction, ", ");
        if (nextToken == NM)
        {
            if (ch == '/')
            {
                char assistantInstruction[100];

                strcpy(assistantInstruction, "mov R1, ");
                strcat(assistantInstruction, "#");
                char numberString[30];
                int number = readNumber(source);
                sprintf(numberString, "%d", number);
                strcat(assistantInstruction, numberString);

                fprintf(destination, "%s\n", assistantInstruction);

                strcat(instruction, "R1");
            }
            else
            {
                strcat(instruction, "#");
                char numberString[30];
                int number = readNumber(source);
                sprintf(numberString, "%d", number);
                strcat(instruction, numberString);
            }
        }
        else
        {
            int secondReg = variableToReg(source);
            strcat(instruction, registerNames[secondReg]);
        }

        fprintf(destination, "%s\n", instruction);
    }
    else
    {
        rollbackToken(source, nextToken);

        strcpy(instruction, "mov ");
        strcat(instruction, destinationReg);
        strcat(instruction, ", ");
        if (isImmediateOperation == 1)
        {
            strcat(instruction, "#");
            char numberString[30];
            sprintf(numberString, "%d", firstRegOrNum);
            strcat(instruction, numberString);
        }
        else
        {
            strcat(instruction, registerNames[firstRegOrNum]);
        }

        fprintf(destination, "%s\n", instruction);
    }

    return 1;
}

int defineRegister(FILE *source)
{
    updateToken(source); // VR

    char ch;
    Variable variable;
    int index = 0;

    while ((ch = fgetc(source)) != '\n')
    {
        if (isspace(ch))
        {
            continue;
        }
        else if (isalpha(ch) || ch == '_')
        {
            variable.name[index++] = ch;
            while (isalnum(ch = fgetc(source)) || ch == '_')
            {
                variable.name[index++] = ch;
            }
            ungetc(ch, source);
            variable.name[index] = '\0';
        }
    }
    ungetc(ch, source);

    variable.reg = freeRegister;
    variableTable[freeRegister] = variable;

    freeRegister++;

    TokenType token = updateToken(source); // PV

    return 1;
}

int executeOperation(FILE *source, FILE *destination)
{
    int destinationReg = variableToReg(source);

    if (destinationReg < 0)
        return 0;

    updateToken(source); // AT

    defineOperation(source, destination, registerNames[destinationReg]);

    updateToken(source); // PV

    return 1;
}

int executeConditionalOperation(FILE *source, FILE *destination)
{
    char branchInstruction[100];

    updateToken(source); // AP esperado

    defineOperation(source, destination, "R2");

    updateToken(source); // CP esperado

    char comparatorString[3];
    readComparator(source, comparatorString);

    defineOperation(source, destination, "R1");

    fprintf(destination, "cmp R0, R2, R1\n");

    // gera o salto condicional
    if (strcmp(comparatorString, "==") == 0)
    {
        strcpy(branchInstruction, "bne ");
    }
    else if (strcmp(comparatorString, "<") == 0)
    {
        strcpy(branchInstruction, "bge ");
    }
    else if (strcmp(comparatorString, ">") == 0)
    {
        strcpy(branchInstruction, "ble ");
    }
    else if (strcmp(comparatorString, "<=") == 0)
    {
        strcpy(branchInstruction, "bgt ");
    }
    else if (strcmp(comparatorString, ">=") == 0)
    {
        strcpy(branchInstruction, "blt ");
    }
    else
    {
        return 0;
    }

    // cria uma label para onde pular se a condição não for atendida
    strcat(branchInstruction, "label_");

    fprintf(destination, "%s%d\n", branchInstruction, labelCounter);

    updateToken(source); // FP esperado

    updateToken(source); // VR esperado

    // executa a operação dentro do if
    executeOperation(source, destination);

    // gerar a label de continuação
    fprintf(destination, "label_%d:\n", labelCounter);
    labelCounter++;

    return 1;
}

int checkPV(FILE *source)
{
    TokenType nextToken;

    nextToken = updateToken(source);
    if (nextToken == PV)
        return 1;

    return 0;
}

int checkExpression(FILE *source)
{
    TokenType nextToken;

    nextToken = updateToken(source);
    if (nextToken != NM && nextToken != VR)
        return 0;

    nextToken = updateToken(source);
    if (nextToken == OP)
    {
        nextToken = updateToken(source);
        if (nextToken != NM && nextToken != VR)
            return 0;
    }
    else
    {
        rollbackToken(source, nextToken);
    }

    return 1;
}

int scanfCheck(FILE *source)
{
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

int printfCheck(FILE *source)
{
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

int instrucionCheck(TokenType token, FILE *source, FILE *destination)
{
    switch (token)
    {
    case IT:
        return defineRegister(source);
    case VR:
        return executeOperation(source, destination);
    case SC:
        return scanfCheck(source); // mantido para ignorar o scan na tradução
    case PT:
        return printfCheck(source); // mantido para ignorar o print na tradução
    case IF:
        return executeConditionalOperation(source, destination);
    default:
        return 0;
    }
}

int main()
{
    FILE *source = fopen("tokens.txt", "r");
    FILE *destination = fopen("assemblyCode.s", "w");

    TokenType token;
    char token_str[3];
    token_str[2] = '\0';
    int success = 1;

    token_str[0] = fgetc(source);
    token_str[1] = fgetc(source);
    token = getTokenTypeFromName(token_str);

    while (token != EF && success == 1)
    {
        success = instrucionCheck(token, source, destination);

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
