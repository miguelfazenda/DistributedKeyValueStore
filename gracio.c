#include <stdio.h>
#include <stdlib.h>

int main()
{
    int a = 10;
    int* b;
    int* c; 

    c = (int*) malloc(sizeof(int));
    *c = 100;
    b = &a;

    printf("a: %d, b: %p, c:%p\n", a, b, c);
    
    *c = *b;
    printf("a: %d, b: %p, c:%p\n", a, b, c);
    printf("c:%d\n", *c);

    return(0);
}