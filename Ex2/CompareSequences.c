#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <memory.h>
#include <ctype.h>

#define READ_MODE "r"
#define SEQUENCE_HEADER '>'
#define INV_ARG "Usage: <file path> <match> <mismatch> <gap>\n"
#define INVALID_INP "Invalid input\n"
#define OPEN_FILE_ERR "Can not open file\n. Please enter a valid path.\n"
#define MEM_FAULT "Memory allocation failed!\n"
#define SCORE_MSG "Score for alignment of seq%d to seq%d is %d\n"
#define INV_SEQ "The input file should contain at least 2 sequences!\n"
#define MAX_SEQUENCES 100
#define MAX_LINE 100


/**
 * Convert char* to int.
 * @param convertMe char* to a string.
 * @param converted fill its value with converted item value. (output).
 * @return 0 uppon success, otherwise string can not be converted and -1 is returned.
 */
int s2i(const char* convertMe, int *converted);

/**
 * Frees all allocated data.
 * @param sequences : char** array (of pointers) to be freed.
 */
void cleanUp(char **sequences);


/**
 * @param x first integer
 * @param y second integer
 * @return * Return the pointer of the lerger integer. If x==y, y is returned.
 */
const int *myMax(const int *x, const int *y);

/**
 *
 * @param file pointer to a file to read from relevant sequences
 * @param sequences pointer to an array of pointers to be filled with read sequences.
 * fixed size of 100. (output)
 * @param sequencesRead number of sequences that are read from file to fill. (output)
 * @return
 */
int extractSequences(FILE *file, char **sequences, int *sequencesRead);

/**
 * Compare to given sequences and return its maximum sub sequence weight.
 * @param seq1 first sequence to compare
 * @param seq2 second sequence to compare
 * @param match the weight of matching letters
 * @param misMatch the weight of mis match
 * @param gap the weight of gap
 * @return the maximum weight of the best matching sub sequence.
 */
int compareSequences(const char *seq1, const char *seq2,
                     const int *match, const int *misMatch, const int *gap, int *score);

/**
 * Compare all sequences pointed by sequences param. for each comparison it prints
 * its maximum sub sequence match.
 * @param sequences pointer to an array of addresses of the sequences to be analized.
 * @param sequencesRead number of sequences read. (sequences length)
 * @param match weight of match
 * @param misMatch weight of mismatch
 * @param gap weight of gap
 */
void analyzeSequences(char** sequences, const int *sequencesRead,
                      const int *match, const int *misMatch, const int *gap);


int main(int argc, char* argv[])
{
    if (argc != 5)
    {
        fprintf(stderr, INV_ARG);
        exit(EXIT_FAILURE);
    }
    FILE *file;
    file = fopen(argv[1], READ_MODE);
    if (file == NULL)
    {
        fprintf(stderr, OPEN_FILE_ERR);
        exit(EXIT_FAILURE);
    }
    int match, misMatch, gap, retVal;
    retVal = s2i(argv[2], &match);
    if(retVal < 0)
    {
        fprintf(stderr, INVALID_INP);
        exit(EXIT_FAILURE);
    }
    retVal = s2i(argv[3], &misMatch);
    if(retVal < 0)
    {
        fprintf(stderr, INVALID_INP);
        exit(EXIT_FAILURE);
    }
    retVal = s2i(argv[4], &gap);
    if(retVal < 0)
    {
        fprintf(stderr, INVALID_INP);
        exit(EXIT_FAILURE);
    }
    int sequencesRead = 0;
    char *sequences[MAX_SEQUENCES];
    if(extractSequences(file, sequences, &sequencesRead) == 0)
    {
        analyzeSequences(sequences, &sequencesRead, &match, &misMatch, &gap);
    }
    cleanUp(sequences);
    fclose(file);
    return 0;
}


int extractSequences(FILE *file, char **sequences, int *sequencesRead)
{
    char line[MAX_LINE + 1];  // +1 for end of string mark.
    for(int i = 0; i < MAX_SEQUENCES; i++)
    {
        sequences[i] = NULL;
    }
    int linesOfSeq = 1;
    char *seq = NULL;
    char* token = NULL;
    while(fgets(line, MAX_LINE + 1, file) != NULL && *sequencesRead < MAX_SEQUENCES)
    {
        // Sequence header
        if (line[0] == SEQUENCE_HEADER)
        {
            linesOfSeq = 1;
            continue;
        }
        token = strtok(line, "\r\n");  // cut "\r\n"
        // empty line.
        if (token == NULL)
        {
            linesOfSeq = 1;
            continue;
        }
        // Sequence consists of 2 lines or more.
        if (linesOfSeq >= 2)
        {
            // Realloc memory of last inserted sequence
            char* lastSeq = sequences[*(sequencesRead) - 1];
            if (realloc(lastSeq, (strlen(token) + strlen(lastSeq)) * sizeof(char)) == NULL)
            {
                fprintf(stderr, MEM_FAULT);
                return -1;
            }
            strcat(lastSeq, token);
            linesOfSeq++;
            continue;
        }
        // New sequence
        seq = (char*)malloc(strlen(token) * sizeof(char));
        if (seq == NULL)
        {
            fprintf(stderr, MEM_FAULT);
            return -1;
        }
        strcpy(seq, token);
        sequences[*sequencesRead] = seq;
        (*sequencesRead)++;
        linesOfSeq++;
    }
    if(*sequencesRead < 2)
    {
        fprintf(stderr, INV_SEQ);
        return -1;
    }
    return 0;
}


void analyzeSequences(char** sequences, const int *sequencesRead,
                      const int *match, const int *misMatch, const int *gap)
{
    int score;
    for(int i = 0; i < *sequencesRead; i++)
    {
        for(int j = i + 1; j < *sequencesRead; j++)
        {
            if(compareSequences(sequences[i], sequences[j], match, misMatch, gap, &score) < 0)
            {
                return;
            }
            printf(SCORE_MSG, i + 1, j + 1, score);
        }
    }
}


int compareSequences(const char *seq1, const char *seq2,
                     const int *match, const int *misMatch, const int *gap, int *score)
{
    int seqLen1 = (int)strlen(seq1);
    int seqLen2 = (int)strlen(seq2);
    int rowLen = seqLen1 + 1;
    int colLen = seqLen2 + 1;
    int *table = (int*)malloc(rowLen * colLen * sizeof(int));
    if(table == NULL)
    {
        fprintf(stderr, MEM_FAULT);
        return -1;
    }
    // table initialization: first row and column are set.
    for(int i = 0; i <= seqLen1; i++)
    {
        table[i] = (*gap) * i;
    }
    for(int i = 1; i <= seqLen2; i++)
    {
        table[i * rowLen] = (*gap) * i;
    }
    // main loop to fill the table. filling by columns.
    for(int j = 1; j <= seqLen1; j++)
    {
        for(int i = 1; i <= seqLen2; i++)
        {
            int up = table[((i - 1) * rowLen) + j] + (*gap);
            int left = table[(i * rowLen) + j - 1] + (*gap);
            if(seq1[j - 1] == seq2[i - 1])
            {
                int corner = table[((i - 1) * rowLen) + j - 1] + (*match);
                table[(i * rowLen) + j] = *myMax(&corner, myMax(&up, &left));
            }
            else
            {
                int corner = table[(i - 1) * rowLen + j - 1] + (*misMatch);
                table[(i * rowLen) + j] = *myMax(&corner, myMax(&up, &left));
            }
        }
    }
    *score = table[(rowLen * colLen) - 1];
    free(table);
    return 0;
}


const int *myMax(const int *x, const int *y)
{
    if(*x > *y)
    {
        return x;
    }
    if(*x <= *y)
    {
        return y;
    }
    return NULL;  // unreachable.
}


void cleanUp(char **sequences)
{
    for (int i = 0; i < MAX_SEQUENCES; i++)
    {
        if (sequences[i] != NULL)
        {
            free(sequences[i]);
            continue;
        }
        break;
    }
}


int s2i(const char* convertMe, int *converted)
{
    int len = (int)strlen(convertMe);
    int i = 0;
    if(convertMe[0] == '-')
    {
        if(strlen(convertMe) == 1)
        {
            return -1;
        }
        i = 1;
    }
    for(; i < len; i++)
    {
        if(!isdigit(convertMe[i]))
        {
            return -1;
        }
    }
    *converted = atoi(convertMe);
    return 0;
}
