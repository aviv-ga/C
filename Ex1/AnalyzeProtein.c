#include <stdio.h>
#include <stdbool.h>
#include <memory.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>

#define MAX_ATOMS 20000
#define ATOM "ATOM"
#define ARG_ERROR_MSG "Usage: AnalyzeProtein <pdb1> <pdb2> ...\n"
#define FILE_NOT_FOUND_MSG "Error opening file: %s\n"
#define CONVERSION_ERR_MSG "Error in coordinate conversion  %s!\n"
#define FILE_STATS_MSG "PDB file %s, %d atoms were read\n"
#define CENTER_MASS_MSG "Cg = %.3f %.3f %.3f\n"
#define ORB_RADIUS_MSG "Rg = %.3f\n"
#define MAX_DISTANCE_MSG "Dmax = %.3f\n"


/**
 * Analyzes a file and calculates its data. The info in the pdb file allows to extract
 * x,y,z coordinates and with them to calculate center mass, orb radius and max distance
 * between 2 atoms of the protein.
 * Prints relevant info upon success and error.
 * @param file the file being processed.
 * @param file_name name of the file being processed.
 */
void analyzeProtein(FILE *file, const char* file_name);
/**
 * extract from the given line its coordinates: x, y, z and cast them to float.
 * @param line the line to read from.
 * @param cords array of floats, size = 3, for output.
 */
void extractCords(const char *line, float* cords);
/**
 * Calculate center mass of a protein.
 * @param cords All atom's coordinates in 2d array of floats.
 * @param cordsLen len of the first dimention of cords.
 * @param centerMass output destination . centerMass array of len 3 of floats.
 */
void calCenterMass(float cords[][3], const int *cordsLen, float *centerMass);
/**
 * Calc Orb radius of a protein.
 * @param cords All atom's coordinates in 2d array of floats.
 * @param cordsLen len of the first dimention of cords.
 * @param centerMass array of len 3 of floats.
 * @param radius output destination of the calculation.
 */
void calcRadius(float cords[][3], const int *cordsLen, const float *centerMass, float *radius);
/**
 * Calculate max distance between 3D points.
 * @param cords 2D array of all atoms.
 * @param cordsLen number of cords in the array.
 * @param dMax output param to fill.
 * @return
 */
void calcDmax(float cords[][3], const int *cordsLen, float *dMax);
/**
 * Print to screen various info of the protein described in file.
 * @param file_name name of the file being read.
 * @param atoms number of atoms read from file.
 * @param radius average circular radius
 * @param dMax max distance between 2 coordinates
 * @param centerMass protein center mass
 */
void report(const char *file_name, const int *atoms, const float *radius,
            const float *dMax, const float centerMass[]);
/**
 * Cast string to float.
 * @param convertMe the string to convert
 * @param dest the converted string (output)
 */
void stringToFloat(const char convertMe[], float* dest);


int main(int argc, char* argv[])
{
    if(argc <= 1)
    {
        fprintf(stderr, ARG_ERROR_MSG);
        exit(EXIT_FAILURE);
    }

    FILE *file;
    for (int i = 1; i < argc; i++)
    {
        file = fopen(argv[i], "r");
        if (file == NULL)
        {
            fprintf(stderr, FILE_NOT_FOUND_MSG, argv[i]);
            break;
        }
        analyzeProtein(file, argv[i]);
        fclose(file);
    }
    return 0;
}


void analyzeProtein(FILE *file, const char* file_name)
{
    int atomsRead = 0;
    float radius = 0, dMax = 0;
    float centerMass[3];
    float proteinCords[MAX_ATOMS][3];
    char line[81];
    while(fgets(line, 80, file) != NULL || atomsRead > MAX_ATOMS)
    {
        char atom[5];
        strncpy(atom, line, 4);

        if(strcmp(atom, ATOM) == 0)
        {
            extractCords(line, proteinCords[atomsRead]);
            atomsRead++;
            continue;
        }
    }
    calCenterMass(proteinCords, &atomsRead, centerMass);
    calcRadius(proteinCords, &atomsRead, centerMass, &radius);
    calcDmax(proteinCords, &atomsRead, &dMax);
    report(file_name, &atomsRead, &radius, &dMax, centerMass);
}


void report(const char *file_name, const int *atoms, const float *radius,
            const float *dMax, const float centerMass[])
{
    printf(FILE_STATS_MSG, file_name, *atoms);
    printf(CENTER_MASS_MSG, centerMass[0], centerMass[1], centerMass[2]);
    printf(ORB_RADIUS_MSG, *radius);
    printf(MAX_DISTANCE_MSG, *dMax);
}


void extractCords(const char *line, float cords[3])
{
    char xCord[8], yCord[8], zCord[8];
    strncpy (xCord, line + 31, 7);
    strncpy (yCord, line + 39, 7);
    strncpy (zCord, line + 47, 7);

    stringToFloat(xCord, &cords[0]);
    stringToFloat(yCord, &cords[1]);
    stringToFloat(zCord, &cords[2]);
}


void stringToFloat(const char convertMe[], float* dest)
{
    char *end;
    float result = 0;
    errno = 0;
    result = strtof(convertMe, &end);
    if(result == 0 && (errno != 0 || end == convertMe))
    {
        fprintf(stderr, CONVERSION_ERR_MSG, convertMe);
        exit(EXIT_FAILURE);
    }
    *dest = result;
}


void calcDmax(float cords[][3], const int *cordsLen, float *dMax)
{
    for(int i = 0; i < *cordsLen; i++)
    {
        for(int j = 0; j < i; j++)
        {
            float xDiff = powf(cords[i][0] - cords[j][0], 2);
            float yDiff = powf(cords[i][1] - cords[j][1], 2);
            float zDiff = powf(cords[i][2] - cords[j][2], 2);
            float sol = sqrtf(xDiff + yDiff + zDiff);
            if (*dMax < sol)
            {
                *dMax = sol;
            }
        }
    }
}


void calcRadius(float cords[][3], const int *cordsLen, const float *centerMass, float *radius)
{
    float xAverage = 0, yAverage = 0, zAverage = 0;
    for(int i = 0; i < *cordsLen; i++)
    {
        xAverage += powf(centerMass[0] - cords[i][0], 2);
        yAverage += powf(centerMass[1] - cords[i][1], 2);
        zAverage += powf(centerMass[2] - cords[i][2], 2);
    }
    xAverage /= *cordsLen;
    yAverage /= *cordsLen;
    zAverage /= *cordsLen;

    *radius = sqrtf(xAverage + yAverage + zAverage);
}


void calCenterMass(float cords[][3], const int *cordsLen, float *centerMass)
{
    float xAverage = 0, yAverage = 0, zAverage = 0;
    for(int i = 0; i < *cordsLen; i++)
    {
        xAverage += cords[i][0];
        yAverage += cords[i][1];
        zAverage += cords[i][2];
    }
    xAverage /= *cordsLen;
    yAverage /= *cordsLen;
    zAverage /= *cordsLen;

    centerMass[0] = xAverage;
    centerMass[1] = yAverage;
    centerMass[2] = zAverage;
}
