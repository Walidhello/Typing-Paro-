#ifndef WORD_H
#define WORD_H

#define MAX_WORDS 500
#define WORD_LENGTH 32

void LoadWords(const char *filename);

const char *GetRandomWord(void);

#endif