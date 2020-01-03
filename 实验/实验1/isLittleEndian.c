#include <stdio.h>

int main()
{
    int a=1;
    int *p=&a;
    char b=*p;
    if(a==b)
        printf("small");
    else
        printf("big");
    return 0;
}



