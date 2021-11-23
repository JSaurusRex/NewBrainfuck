#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAXINT 65535
#define BUFFERSIZE 1

//DEFINE COMMANDS
#define ADD 0
#define MOVE 1
#define PRINT 2
#define INPUT 3
#define OLOOP 4
#define CLOOP 5
#define RESET 6
//#define DEBUG

//crosspiled code
char * code;
int * value;
int codeSize = 40;
char * text;
int textSize = 0;

bool loadFile(char * file);
void crosspile();
void run();
void optimization();

int main (int argc, char*argv[])
{
    if (argc < 2)
    {
        printf("No file given\n");
        return 0;
    }
    loadFile(argv[1]);
    crosspile();
    optimization();
    printf("NEWRUN\n\n");
    run();
    printf("\n");
}

void run ()
{
    unsigned char * cells = (unsigned char *) calloc(sizeof(unsigned char), MAXINT);
    unsigned char * currentCell = cells;
    unsigned long long int cycles = 0;
    #ifdef DEBUG
        printf("beginning run\n");
    #endif
    for (int i = 0; i < codeSize; i++)
    {
        cycles++;
        #ifdef DEBUG
            //printf("%i %i %i\n", code[i], i, codeSize);
            //printf("c %i %i\n", *currentCell, (int)currentCell - (int)&cells);
            printf("i %i c %i   ", i, *currentCell);
        #endif
        switch(code[i])
        {
            case ADD:
                *currentCell += value[i];
                #ifdef DEBUG
                    printf("ADD %i\n", value[i]);
                #endif
                break;
            case MOVE:
                currentCell += value[i];
                if (currentCell < cells)
                    currentCell = &cells + MAXINT-1;
                if (currentCell > cells + MAXINT)
                    currentCell = &cells;
                #ifdef DEBUG
                    printf("MOVE %i\n", value[i]);
                #endif
                break;
            case PRINT:
                printf("%c", *currentCell);
                #ifdef DEBUG
                    printf("PRINT\n");
                #endif
                break;
            case INPUT:
                scanf("%c", currentCell);
                #ifdef DEBUG
                    printf("INPUT\n");
                #endif
                break;
            case OLOOP:
                if(*currentCell == 0)
                    i = value[i]-1;
                #ifdef DEBUG
                    printf("OLOOP %i\n", value[i]);
                #endif
                break;
            case CLOOP:
                if(*currentCell != 0)
                    i = value[i]-1;
                #ifdef DEBUG
                    printf("CLOOP %i\n", value[i]);
                    if(value[i] == 3664)
                    {
                        for (int j = 5; j >= 0; j--)
                        {
                            printf("code %i   %i\n", code[i-j], i-j);
                        }
                    }
                #endif
                break;
            case RESET:
                *currentCell = 0;
                #ifdef DEBUG
                    printf("RESET\n");
                #endif
                break;
            default:
                printf("\n help!  %i   %i\n ", code[i], value[i]);
                return;

        }
    }
    printf("\nCycles %llu\n", cycles);
}

void optimization ()
{
    char * newCode = calloc(codeSize, sizeof(char));
    int * newValue = calloc(codeSize, sizeof(int));

    int pos = 0;

    for(int i = 0; i < codeSize; i++)
    {
        if (i == codeSize -1)
            codeSize = pos+1;
        if (i < codeSize - 3)
        {
            //case reset
            if(code[i] == OLOOP && code[i+1] == ADD && code[i+2] == CLOOP)
            {
                printf("found reset\n");
                newCode[pos] = RESET;
                i+=2;
                pos++;
                continue;
            }
        }

        //copying
        newCode[pos] = code[i];
        newValue[pos] = value[i];
        pos++;
    }

    free(code);
    free(value);
    code = newCode;
    value = newValue;

    //recalculating loops
    for (int i = 0; i < codeSize; i++)
    {
        if(code[i] == OLOOP)
        {
            int loopsDeep = 0;
            for (int j = i+1; j < codeSize; j++)
            {
                if(code[j] == OLOOP)
                    loopsDeep +=1;
                else if(code[j] == CLOOP)
                {
                    loopsDeep -= 1;
                    if(loopsDeep == -1)
                    {
                        //printf("Found loop %i  %i\n", i, j);
                        value[i] = j;
                        value[j] = i;
                    }
                }
            }
        }
    }
}

void writeCode(int pos, char codeL, int valueL)
{
    if(codeSize <= pos+3)
    {
        codeSize+= 80;
        #ifdef DEBUG
            printf("reallocating code %d, %d, %i\n", code, value, codeSize);
        #endif
        code = realloc(code, codeSize * sizeof(char));
        #ifdef DEBUG
            printf("reallocating code %d, %d, %i\n", code, value, codeSize*sizeof(int));
        #endif
        value = realloc(value, codeSize * sizeof(int));
        #ifdef DEBUG
            printf("reallocating code, %i\n", codeSize);
        #endif
    }
    #ifdef DEBUG
        printf("%d %i %i\n", &value, codeSize, pos);
    #endif
    code[pos] = codeL;
    value[pos] = valueL;
    #ifdef DEBUG
    switch(codeL)
        {
            case ADD:
                printf("ADD ");
                break;
            case MOVE:
                printf("MOVE ");
                break;
            case PRINT:
                printf("PRINT ");
                break;
            case INPUT:
                printf("INPUT ");
                break;
            case OLOOP:
                printf("OLOOP ");
                break;
            case CLOOP:
                printf("CLOOP ");
                break;
        }
        printf("%i\n", valueL);
    #endif
}

void crosspile()
{
    #ifdef DEBUG
        printf("Crosspiling TextSize: %i\n", textSize);
    #endif
    int pos = 0;
    bool addition = false;
    int addValue = 0;
    bool moving = false;
    int movingValue = 0;
    int loopDeep = 0;
    int loops [100];
    code = calloc(codeSize, sizeof(char));
    value = calloc(codeSize, sizeof(int));
    for(int i = 0; i < textSize; i++)
    {
        char ch = text[i];
        #ifdef DEBUG
            printf("%c  pos: %i\n", ch, pos);
        #endif


        if(ch == '<' || ch == '>') moving = true;
        
        if(moving && !addition)
        {
            if (ch == '<')
                movingValue--;
            else if (ch == '>')
                movingValue++;
            else
            {
                if(movingValue != 0)
                {
                    writeCode(pos, MOVE, movingValue);
                    pos++;
                    moving = false;
                    movingValue =0;
                }
            }
        }
        if(addition && moving)
        {
            moving = false;
            i--;
        }

        if(ch == '+' || ch == '-') addition = true;
        if(addition)
        {
            if (ch == '+')
                addValue++;
            else if (ch == '-')
                addValue--;
            else
            {
                if(addValue != 0)
                {
                    writeCode(pos, ADD, addValue);
                    pos++;
                    addition = false;
                    addValue = 0;
                }
            }
        }
        if (ch == '[')
        {   
            loops[loopDeep] = pos;
            loopDeep++;
            pos++;
        }
        if (ch == ']')
        {
            loopDeep--;
            #ifdef DEBUG
                printf("O pos: %i\nC pos: %i\n", loops[loopDeep], pos);
            #endif

            writeCode(loops[loopDeep], OLOOP, pos);
            writeCode(pos, CLOOP, loops[loopDeep]);
            pos++;
        }
        if (ch == '.')
        {
            writeCode(pos, PRINT, 0);
            pos++;
        }
        if (ch == ',')
        {
            writeCode(pos, INPUT, 0);
            pos++;
        }
        if (i == textSize - 1)
        {
            codeSize = pos;
        }
    }
}

bool loadFile(char * file)
{
    FILE *fp = fopen(file, "r");

    if (fp == NULL)
    {
        printf("Error: could not open file %s", file);
        return 1;
    }

    text = calloc(sizeof(char), 10);
    textSize = 10;

    char ch;
    int size = 0;
    while ((ch = fgetc(fp)) != EOF)
        if (ch == '<' || ch == '>' || ch == '.' || ch == ',' || ch == '+' || ch == '-' || ch == '[' || ch == ']')
        {
            size++;
            if (size >= textSize)
            {
                textSize += 10;
                text = realloc(text, textSize * sizeof(char));
            }
            text[size-1] = ch;
        }
    textSize = size;
}
