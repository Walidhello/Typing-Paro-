#include "word.h"
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char words[MAX_WORDS][WORD_LENGTH];

int totalWords = 0;

//----------------------------------
// Load words from file
//----------------------------------

void LoadWords(const char *filename)
{
    FILE *file = fopen(filename, "r");

    if(file == NULL)
    {
        printf("Could not open %s\n", filename);
        return;
    }

    totalWords = 0;

    while(fscanf(file,"%31s",words[totalWords]) == 1)
    {
        totalWords++;

        if(totalWords >= MAX_WORDS)
            break;
    }

    fclose(file);

    printf("%d words loaded.\n", totalWords);
}

//----------------------------------
// Return random word
//----------------------------------

const char *GetRandomWord(void)
{
    if(totalWords == 0)
        return "error";

    int index = GetRandomValue(0, totalWords - 1);

    return words[index];
}