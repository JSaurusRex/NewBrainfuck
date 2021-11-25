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
#define MOVEL 7
#define MULTI 8
#define TABLE 9
//#define DEBUG
//#define PROFILE

//crosspiled code
unsigned char * code;
int * value;
int codeSize = 40;
char * text;
int textSize = 0;
typedef struct {
    int firstAdd, move, secondAdd;
}MultiplyTable;
MultiplyTable * multiplyTable;
int mtSize = 0;

int * tables;
int tableSize=0;


bool loadFile(char * file);
void crosspile();
void run();
void optimization();


char toBrainfuck(int bfCode, int value);

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

int pInfo [9]; 

void run ()
{
    unsigned char * cells = (unsigned char *) calloc(sizeof(unsigned char), MAXINT);
    unsigned char * currentCell = cells;
    unsigned char * currentCode = code;
    int * currentValue = value;

    unsigned long long int cycles = 0;
    #ifdef DEBUG
        printf("beginning run\n");
        for (int i = 0; i< 6; i++)
        {
            printf("%i   %i\n", value[i], *(currentValue + i));
        }
    #endif
    for (; currentCode - code < codeSize; currentCode++, currentValue++)
    {
        cycles++;
        #ifdef DEBUG
            //printf("%i %i %i\n", code[i], i, codeSize);
            //printf("c %i %i\n", *currentCell, (int)currentCell - (int)&cells);
            printf("i %i %i c %i   ", currentCode - code, currentValue - value, *currentCell);
            if (currentValue - value < 0) return;
        #endif
        switch(*currentCode)
        {
            case ADD:
                *currentCell += *currentValue;
                #ifdef DEBUG
                    printf("ADD %i\n", *currentValue);
                #endif
                #ifdef PROFILE
                    pInfo[ADD] += 1;
                #endif
                break;
            case MOVE:
                currentCell += *currentValue;
                // if (currentCell < cells)
                //     currentCell = cells + MAXINT-1;
                // else if (currentCell > cells + MAXINT)
                //     currentCell = cells;
                #ifdef DEBUG
                    printf("MOVE %i\n", *currentValue);
                #endif
                 #ifdef PROFILE
                    pInfo[MOVE] += 1;
                #endif
                break;
            case PRINT:
                printf("%c", *currentCell);
                #ifdef DEBUG
                    printf("PRINT\n");
                #endif
                 #ifdef PROFILE
                    pInfo[PRINT] += 1;
                #endif
                break;
            case INPUT:
                scanf("%c", currentCell);
                #ifdef DEBUG
                    printf("INPUT\n");
                #endif
                 #ifdef PROFILE
                    pInfo[INPUT] += 1;
                #endif
                break;
            case OLOOP:
                if(*currentCell == 0)
                {
                    currentCode = (unsigned char *)(code + *currentValue);
                    currentValue = (int *)(value + (*currentValue));
                }
                #ifdef DEBUG
                    printf("OLOOP %i\n", *currentValue);
                #endif
                 #ifdef PROFILE
                    pInfo[OLOOP] += 1;
                #endif
                break;
            case CLOOP:
                if(*currentCell != 0)
                {
                    currentCode = (unsigned char *)(code + (*currentValue));
                    currentValue = (int *)(value + (*currentValue));
                }
                #ifdef DEBUG
                    printf("CLOOP %i\n", *currentValue);
                #endif
                 #ifdef PROFILE
                    pInfo[CLOOP] += 1;
                #endif
                break;
            case RESET:
                *currentCell = 0;
                #ifdef DEBUG
                    printf("RESET\n");
                #endif
                 #ifdef PROFILE
                    pInfo[RESET] += 1;
                #endif
                break;
            case MOVEL:
                while(*currentCell != 0)
                {
                    currentCell+= *currentValue;
                }
                 #ifdef PROFILE
                    pInfo[MOVEL] += 1;
                #endif
                break;
            case MULTI:
                MultiplyTable tmp = multiplyTable[*currentValue];
                //printf("multi %i    %i %i %i\n", value[i], tmp.firstAdd, tmp.move, tmp.secondAdd);
                // int amount = 0;
                // int rest = 1;
                // while(rest != 0)
                // {
                //     if((*currentCell) < value[i])
                //     {
                //         *currentCell -= value[i];
                //     }
                //     rest = (*currentCell) % value[i];
                //     amount += (*currentCell) / value[i];
                //     *currentCell /= value[i];
                // }

                while(*currentCell != 0)
                {
                    *currentCell += tmp.firstAdd;
                    *(currentCell + tmp.move) += tmp.secondAdd;
                }
                 #ifdef PROFILE
                    pInfo[MULTI] += 1;
                #endif
                break;
            case TABLE:
                //printf("\ntable\n");
                //size move start data data data data ...
                int size = tables[*currentValue];
                int move = tables[*currentValue+1]; 
                int start = tables[*currentValue+2];
                printf("table: size %i  move %i  start %i\n", size, move, start);

                for(int j = 0; j <size-3; j++)
                {
                    *(currentCell+j+start) = tables[j+3];
                }
                currentCell+=move;
                break;
                //return;
            default:
                printf("\n help!  %i   %i\n ", *currentCode, *currentValue);
                return;
        }
    }
    printf("\nCycles %llu\n", cycles);
    #ifdef PROFILE
        for(int i = 0; i < 9; i++)
        {
            printf("%c %i\n", toBrainfuck(i, 0), pInfo[i]);
        }
    #endif
}

void optimization ()
{
    char * newCode = calloc(codeSize, sizeof(char));
    int * newValue = calloc(codeSize, sizeof(int));

    int pos = 0;

    multiplyTable = malloc(sizeof(MultiplyTable) * 40);

    tables = malloc(sizeof(int)* 40);

    for(int i = 0; i < codeSize; i++)
    {
        if (i == codeSize -1)
            codeSize = pos+1;

        int j = i;
        while((code[j] == ADD || code[j] == MOVE) && j < codeSize)
        {
            j++;
        }
        if(j - i > 2)
        {
            int move = 0; //from start position
            int lowest = 999, highest = -999;
            int currentPos = 0;
            for(int k = i; k < j; k++)
            {
                if(code[k] == MOVE)
                {
                    currentPos += value[k];
                    if(currentPos < lowest)
                        lowest = currentPos;
                    if(currentPos > highest)
                        highest = currentPos;
                }
            }
            move = currentPos;
            int size = (highest - lowest) + 4;
            printf("current size = %i  h %i l  %i  m %i\n", size, highest, lowest, move);
            
            
            int tmpTS = tableSize;
            tableSize += size;
            printf("----    TAbleSize: %i\n", tableSize);
            tables = realloc(tables, sizeof(int)*(tableSize + 10));
            tables[tmpTS] = size; //size move start data data data data ...
            tables[tmpTS+1] = move;
            tables[tmpTS+2] = lowest;
            currentPos = -lowest; //-lowest - lowest = 0
            for(int k = j; k < j; k++)
            {
                if(code[k] == MOVE)
                {
                    currentPos += value[k];
                    if(currentPos < lowest)
                        lowest = currentPos;
                    if(currentPos > highest)
                        highest = currentPos;
                }
                if(currentPos+tmpTS+3 >= tableSize)
                {
                    printf("uuuhm\n\n");
                }
                if(code[k] == ADD)
                    tables[currentPos+tmpTS+3] = value[k];
            }
            i = j;
            newCode[pos] = TABLE;
            newValue[pos] = tmpTS;
            pos++;
            continue;
        }

        if (i < codeSize - 6)
        {
            //case multiply
            // if(code[i] == OLOOP && code[i+1] == ADD && code[i+2] == MOVE && code[i+3] == ADD && code[i+4] == MOVE && code[i+5] == CLOOP
            // && value[i+2] + value[i+4] == 0)
            // {
            //     printf("     found multiply  %i   %i   %i\n", value[i+1], value[i+2], value[i+3]);
                
            //     if(mtSize % 40 == 39)
            //     {
            //         printf("\ntest\n");
            //         multiplyTable = realloc(multiplyTable,sizeof(MultiplyTable) * (mtSize+41));
            //     }
            //     newCode[pos] = MULTI;
            //     newValue[pos] = mtSize;
            //     multiplyTable[mtSize] = (MultiplyTable){.firstAdd=value[i+1],.move=value[i+2],.secondAdd=value[i+3]};
            //     i+=5;
            //     pos++;
            //     mtSize++;
            //     continue;
            // }

            // //case multiply 2 (different order)
            // if(code[i] == OLOOP && code[i+1] == MOVE && code[i+2] == ADD && code[i+3] == MOVE && code[i+4] == ADD && code[i+5] == CLOOP
            // && value[i+1] + value[i+3] == 0)
            // {
            //     printf("     found multiply  222222 %i   %i   %i\n", value[i+4], value[i+1], value[i+2]);
                
            //     if(mtSize % 40 == 39)
            //     {
            //         printf("\ntest\n");
            //         multiplyTable = realloc(multiplyTable,sizeof(MultiplyTable) * (mtSize+41));
            //     }
            //     newCode[pos] = MULTI;
            //     newValue[pos] = mtSize;
            //     multiplyTable[mtSize] = (MultiplyTable){.firstAdd=value[i+4],.move=value[i+1],.secondAdd=value[i+2]};
            //     i+=5;
            //     pos++;
            //     mtSize++;
            //     continue;
            // }
        }
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

            //case move loop
            if(code[i] == OLOOP && code[i+1] == MOVE && code[i+2] == CLOOP)
            {
                printf("found moveloop\n");
                newCode[pos] = MOVEL;
                newValue[pos] = value[i+1];
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

    #ifdef PROFILE
        char loopProfile[200][11];
        int lPIndex = 0;
    #endif

    //recalculating loops
    for (int i = 0; i < codeSize; i++)
    {
        if(code[i] == OLOOP)
        {
            int loopsDeep = 0;
            #ifdef PROFILE
                char tmpLP [10]; 
            #endif
            //printf("%c ", toBrainfuck(code[i], value[i]));
            for (int j = i+1; j < codeSize; j++)
            {
                //printf("%c", toBrainfuck(code[j], value[j]));
                if(code[j] == OLOOP)
                    loopsDeep +=1;
                else if(code[j] == CLOOP)
                {
                    loopsDeep -= 1;
                    if(loopsDeep == -1)
                    {
                        // printf("Found loop %i  %i\n", i, j);
                        value[i] = j;
                        value[j] = i;
                        #ifdef PROFILE
                            if(j-i-1 < 9)
                            {
                                lPIndex++;
                                for(int k =j-i; k < 10;k++)
                                    tmpLP[k] = -1;
                                bool foundOne = false;
                                int whichOne = 0;
                                //compare it to others
                                for(int k =0; k < lPIndex; k++)
                                {
                                    
                                    for(int l = 0; l < 10; l++)
                                    {
                                        if(loopProfile[lPIndex][l+1] != tmpLP[l])
                                            break; //arrays/loops are not even
                                        else if(l == 9)
                                                foundOne = true;
                                    }
                                    if(foundOne)
                                    {
                                        whichOne = k;
                                        break;
                                    }
                                }
                                if(foundOne) // add to existing counter
                                {
                                    loopProfile[whichOne][0]++;//first one is amount of identicals counter
                                }else
                                {   //create new entry
                                    if(lPIndex < 200)
                                    {
                                        for(int k = 1; k < 11; k++)
                                        {
                                            loopProfile[lPIndex][k] = tmpLP[k-1];
                                            if(loopProfile[lPIndex][k] == OLOOP)
                                            {
                                                lPIndex--;
                                                break;
                                            }
                                        }
                                        lPIndex++;
                                    }
                                }
                                
                            }
                        #endif
                        break;
                    }
                }
                #ifdef PROFILE
                    if (j-i-1 < 9)
                    {
                        tmpLP[j-i-1] = code[j];
                    }
                #endif
            }
            //printf("\n\n");
        }
    }

    #ifdef PROFILE
        for (int i = 0; i < lPIndex; i++)
        {
            //printf("\n\n%i\n", loopProfile[i][0]);
            for(int j = 1; j < 11 && loopProfile[i][j] != -1; j++)
            {
                //printf("%i%c",(int)loopProfile[i][j],toBrainfuck(loopProfile[i][j] ,0));
            }
            
        }
    #endif
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

char toBrainfuck(int bfCode, int Lvalue)
{
    switch(bfCode)
    {
        case ADD:
            if (Lvalue < 0)
                return '-';
            return '+';
        case MOVE:
            if (Lvalue < 0)
                return '<';
            return '>';
        case OLOOP:
            return '[';
        case CLOOP:
            return ']';
        case PRINT:
            return '.';
        case INPUT:
            return ',';
        
        //optimization cases
        case RESET:
            return 'r';
        case MOVEL:
            if (Lvalue < 0)
                return '{';
            return '}';
        case MULTI:
            return '*';

    }
    printf("brainfuck code given but doesn't exist");
    return -1;
}