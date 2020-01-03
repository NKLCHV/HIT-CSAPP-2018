#include <stdio.h>
#include <stdlib.h>

int main()
{
    int x;

    printf("please input the x:");
    scanf("%d",&x);

    printf("A.K=17  x*K=%d\n",x+(x<<4));

    printf("B.K=-7  x*K=%d\n",x-(x<<3));

    printf("C.K=60  x*K=%d\n",(x<<6)-(x<<2));

    printf("D.K=-112  x*K=%d\n",(x<<4)-(x<<7));

    return 0;
}
