#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void get(char * entra)
{
    char* word = (char*) malloc(10*sizeof(char));
    strcpy(word, "mekie");
    entra = word;
}

int main()
{
    char *entra;
    entra = (char*) malloc (sizeof(char)*10);
    strcpy(entra, "ola");
    printf("%s\n", entra);
    free(entra);

    get(entra);
    printf("%s\n", entra);
}