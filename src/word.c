#include "word.h"
#include "raylib.h"
#include <stdio.h>

char words[MAX_WORDS][WORD_LENGTH];
int totalWords = 0;

void LoadWords(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file) return;

    totalWords = 0;
    while (totalWords < MAX_WORDS && fscanf(file, "%31s", words[totalWords]) == 1)
    {
        totalWords++;
    }
    fclose(file);
}

const char *GetRandomWord(void)
{
    return (totalWords > 0) ? words[GetRandomValue(0, totalWords - 1)] : "error";
}
