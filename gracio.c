#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void read(char **word_m)
{
    int ch = 0, size = 0;
    char word[10000];

    while ((ch = getchar()) != '\n' && ch != EOF)
    {
        word[size] = (char)ch;
        size++;
    }

    *word_m = (char *)malloc((size + 1) * sizeof(char));

    for (int i = 0; i < size; i++)
    {
        *word_m[i] = word[i];
    }

    *word_m[size] = '\0';
}

int main()
{
    char *word_m;

    read(&word_m);

    printf("Impressao: %s\n", word_m);
}