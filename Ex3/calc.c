#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <memory.h>
#include <ctype.h>
#include <stdbool.h>
#include <math.h>
#include "stack.h"

#define MAX_LINE 100
#define LEFT_PAREN '('
#define RIGHT_PAREN ')'
#define ADD '+'
#define SUB '-'
#define MULT '*'
#define DIV '/'
#define POW '^'
#define CHAR_PTR 0
#define INT_PTR 1
#define ZERO_DIV_MSG "Division by 0!\n"
#define MEM_FAULT "Segmentation fault\n"
#define INFIX "Infix:"
#define OUTPUT "The value is"

/**
 * Operand precedence enum.
 */
enum Precedence
{
    none,
    low,
    med,
    high
};

/**
 * Struct that hold pointer to generic data and data type.
 */
typedef struct GenericData
{
    void* data;
    int type;
} GenericData;

/**
 * free all alocated data.
 * @param postfix the data to free.
 * @param size size of the given array.
 */
void cleanUp(GenericData** postfix, int size);
/**
 * evaluate b op a and store in out.
 * @param op mathematical operation
 * @param a pointer to integer
 * @param b pointer to integer
 * @param out for the evaluation of b op a
 * @return -1 upon zero division, 0 otherwise.
 */
int bOpA(const char* op, const int* a, const int* b, int* out);
/**
 * Check if the char is an operator.
 * @param op pointer to a char.
 * @param precedence enu, of type Precedence.
 * @return true if char is an operator, false otherwise.
 */
bool checkPrecedence(const char* op, enum Precedence *precedence);

/**
 * Convert char* to int.
 * @param target char* to convert
 * @param start starting index.
 * @param end end point.
 * @return converted char
 */
int a2i(const char* target, int start, int end);
/**
 * Push a number to the postfixStack. converting char* to integer according to the adjacent chars.
 * @param infix char* to the string data.
 * @param len len of postfix
 * @param i starting point
 * @param postfixStack pointer to the postfix stack.
 * @return -1 upon erro, 0 otherwise.
 */
int pushNumber(char* infix, int* len, int* i, Stack* postfixStack);
/**
 * pop from working stack and add to postfix stack.
 * @param workingStack current workingStack
 * @param postfixStack current postfixStack
 * @return -1 upon error, 0 otherwise.
 */
int popGenericData(Stack* workingStack, Stack* postfixStack);
/**
 * return 0 upon succes, -1 upon failure.
 * @param infix char* to the infix data.
 * @return array of pointer to structs of type GenericData.
 * Each pointer to struct holds a data of the postfix.
 */
GenericData** infix2postfix(char* infix, GenericData** postfix, int* postfixSize);
/**
 * Print postfix.
 * @param postfix the postfix to print
 * @param arrSize the array of its size
 */
void printFcn(GenericData** postfix, int arrSize);

/**
 * evaluate postfix phrase.
 * @param postfix
 * @param n
 * @param output
 * @return
 */
int postfixRevaluation(GenericData **postfix, int n, int* output);


int main()
{
    char line[MAX_LINE + 1];  // +1 for end of string mark.
    GenericData** postfix;
    int postfixSize, output;
    while(*fgets(line, MAX_LINE + 1, stdin) != EOF)
    {
        printf("%s %s\n", INFIX, line);
        postfix = infix2postfix(line, postfix, &postfixSize);
        if (postfix == NULL)
        {
            printf("%s", MEM_FAULT);
            return -1;
        }
        printFcn(postfix, postfixSize);
        if(postfixRevaluation(postfix, postfixSize, &output) < 0)
        {
            printf("%s", ZERO_DIV_MSG);
            return -1;
        }
        printf("%s: %d\n", OUTPUT, output);
        cleanUp(postfix, postfixSize);
    }
}


GenericData** infix2postfix(char* infix, GenericData** postfix, int* postfixSize)
{
    int len = (int)strlen(infix);
    char* temp = (char*)malloc(sizeof(char) * 2);
    enum Precedence *prec1 = (enum Precedence*)malloc(sizeof(enum Precedence));
    enum Precedence *prec2 = (enum Precedence*)malloc(sizeof(enum Precedence));
    Stack *postfixStack = stackAlloc(sizeof(GenericData*));
    Stack *workingStack = stackAlloc(sizeof(char));
    if(!prec1 && !prec2 && !postfixStack && !workingStack)
    {
        return NULL;
    }
    int i = 0;
    while(i < len)
    {
        if(infix[i] == ' ')
        {
            i++;
            continue;
        }
        else if(isdigit(infix[i]))
        {
            if(pushNumber(infix, &len, &i, postfixStack) < 0)
            {
                return NULL;
            }

        }
        else if(infix[i] == LEFT_PAREN)
        {
            push(workingStack, infix + i);
            i++;
        }
        else if(infix[i] == RIGHT_PAREN)
        {
            while((!isEmptyStack(workingStack))
                  && (*(char*)workingStack->_top->_data != LEFT_PAREN))
            {
                if(popGenericData(workingStack, postfixStack) < 0)
                {
                    return NULL;
                }
            }
            pop(workingStack, temp);
            i++;
        }
        else if(checkPrecedence(infix + i, prec1))
        {
            if(isEmptyStack(workingStack) || (*(char*)workingStack->_top->_data == LEFT_PAREN))
            {
                push(workingStack, infix + i);
            }
            else
            {
                checkPrecedence(workingStack->_top->_data, prec2);
                while((!isEmptyStack(workingStack))
                        && (*(char*)workingStack->_top->_data != LEFT_PAREN)
                        && (*prec1 <= *prec2))
                {
                    if(popGenericData(workingStack, postfixStack) < 0)
                    {
                        return NULL;
                    }
                }
                push(workingStack, infix + i);
            }
            i++;
        }
    }
    while(!isEmptyStack(workingStack))
    {
        if(popGenericData(workingStack, postfixStack) < 0)
        {
            return NULL;
        }
    }

    postfix = (GenericData**)malloc((postfixStack->_stackSize + 1) * sizeof(GenericData*));
    if(postfix == NULL)
    {
        return NULL;
    }
    size_t j = postfixStack->_stackSize - 1;
    *postfixSize = (int)postfixStack->_stackSize;
    while(!isEmptyStack(postfixStack))
    {
        pop(postfixStack, postfix + j);
        j--;
    }

    freeStack(&workingStack);
    freeStack(&postfixStack);
    free(prec1);
    free(prec2);
    free(temp);
    return postfix;
}

void printFcn(GenericData** postfix, int arrSize)
{
    printf("%s", "Postfix: ");
    char c = ' ';
    for(int i = 0; i < arrSize; i++)
    {
        if(i == arrSize -1)
        {
            c = '\n';
        }
        GenericData *genData = postfix[i];
        if(genData->type == CHAR_PTR)
        {
            printf("%s%c", (char*)genData->data, c);
        }
        else
        {
            printf("%d%c", *((int*)genData->data), c);
        }
    }
}
int pushNumber(char* infix, int* len, int* i, Stack* postfixStack)
{
    int j = 1;
    while(i + j < len && isdigit(infix[*i + j]))
    {
        j++;
    }
    GenericData *newData = (GenericData*)malloc(sizeof(GenericData));
    if(newData == NULL)
    {
        return -1; // mem fault
    }
    int* val = (int*)malloc(sizeof(int));
    if(val == NULL)
    {
        free(newData);
        return -1; // mem fault
    }
    *val = a2i(infix, *i, *i + j);
    newData->data = val;
    newData->type = INT_PTR;
    push(postfixStack, &newData);
    int temp = *i + j;
    *i = temp;
    return 0;
}


int popGenericData(Stack* workingStack, Stack* postfixStack)
{
    char* item = (char*)malloc(sizeof(char) * 2);
    if(item == NULL)
    {
        return -1; // mem fault
    }
    GenericData *newData = (GenericData*)malloc(sizeof(GenericData));
    if(newData == NULL)
    {
        free(item);
        return -1; // meme fault
    }
    pop(workingStack, item);
    item[1] = '\0';
    newData->data = item;
    newData->type = CHAR_PTR;
    push(postfixStack, &newData);
    return 0;
}


int postfixRevaluation(GenericData **postfix, int n, int* output)
{
    GenericData* a = (GenericData*)malloc(sizeof(GenericData));
    GenericData* b = (GenericData*)malloc(sizeof(GenericData));
    enum Precedence *prec = (enum Precedence*)malloc(sizeof(enum Precedence));
    Stack *workingStack = stackAlloc(sizeof(GenericData*));
    for(int i = 0; i < n; i++)
    {
        GenericData* genData = postfix[i];
        if(genData->type == INT_PTR)
        {
            push(workingStack, postfix[i]);
        }
        else if(checkPrecedence(genData->data, prec))
        {
            pop(workingStack, a);
            pop(workingStack, b);
            int* tot = (int*)malloc(sizeof(int));
            if(tot == NULL)
            {
                return -1;
            }
            if(bOpA(genData->data, a->data, b->data, tot) < 0)
            {
                printf("%s", ZERO_DIV_MSG);
                return -1;
            }
            GenericData *newData = (GenericData*)malloc(sizeof(GenericData));
            if(newData == NULL)
            {
                return -1;
            }
            newData->data = tot;
            newData->type = INT_PTR;
            push(workingStack, &newData);
        }
    }
    pop(workingStack, a);
    int *result = (int*)(a->data);
    *output = *result;
    free(a);
    free(b);
    free(prec);
    return 0;
}


int bOpA(const char* op, const int* a, const int* b, int* out)
{
    if(*op == ADD)
    {
        *out = *b + *a;
    }
    else if(*op == SUB)
    {
        *out = *b - *a;
    }
    else if(*op == MULT)
    {
        *out = *b * *a;
    }
    else if(*op == DIV)
    {
        if(*a == 0)
        {
            return -1;
        }
        *out = *b / *a;
    }
    else if(*op == POW)
    {
        *out = (int)pow(*b, *a);
    }
    return 0;
}


int a2i(const char* target, int start, int end)
{
    int sign = 1;
    int i = 0;
    if(target[start] == '-')
    {
        sign = -1;
        i++;
    }
    int num = 0;
    while(start + i < end)
    {
        num = ((target[start + i]) - '0') + (num * 10);
        i++;
    }
    return num * sign;
}


bool checkPrecedence(const char* op, enum Precedence *precedence)
{
    if(*op == ADD)
    {
        *precedence = low;
        return true;
    }
    if(*op == SUB)
    {
        *precedence = low;
        return true;
    }
    if(*op == MULT)
    {
        *precedence = med;
        return true;
    }
    if(*op == DIV)
    {
        *precedence = med;
        return true;
    }
    if(*op == POW)
    {
        *precedence = high;
        return true;
    }
    *precedence = none;
    return false;
}

void cleanUp(GenericData** postfix, int size)
{
    for(int i = 0; i < size; i++)
    {
        GenericData* s = postfix[i];
        if(s == NULL)
        {
            break;
        }
        free(s->data);
        free(s);
    }
}




