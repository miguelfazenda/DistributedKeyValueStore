#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void get(char ** entra)
{
    char* word = (char*) malloc(10*sizeof(char));
    strcpy(word, "mekie");
    *entra = word;
}

int get2(int* x)
{
    int y = 3;

    *x = y;
}

int main()
{
    int x;
    char *entra;
    get2(&x);

    get(&entra);
    printf("%s\n", entra);
}