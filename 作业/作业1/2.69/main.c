#include <stdio.h>
#include <stdlib.h>

unsigned rotate_left (unsigned x,int n);

int main()
{
    unsigned x;
    int n;
    printf("please input the number:");
    scanf("%x",&x);
    printf("please input the times of rotating:");
    scanf("%d",&n);
    x=rotate_left(x,n);
    printf("the number after rotating:");
    printf("%x",x);
    return 0;
}

unsigned rotate_left (unsigned x,int n)
{
    int i;
    unsigned y,z;
    for(i=0;i<n;i++)
    {
        y=x;
        z=x;
        y=y<<1;
        z=z>>31;
        x=y+z;
    }
    return x;
}
