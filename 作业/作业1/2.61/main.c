#include <stdio.h>
#include <stdlib.h>

int main()
{
    int a,b,c,d,x;
    int i;
    int flagA=1,flagB=1,flagC=1,flagD=1;
    printf("please input a int:");
    scanf("%d",&x);

    a=x;
    for(i=0;i<32;i++)
    {
        if(~(a%2))  flagA=0;

        a=a>>1;
    }

    b=x;
    for(i=0;i<32;i++)
    {
        if(b%2) flagB=0;

        b=b>>1;
    }

    c=x;
    for(i=0;i<4;i++)
    {
        if(!(c%2))  flagC=0;

        c=c>>1;
    }

    d=x;
    d=d>>27;
    for(i=0;i<4;i++)
    {
        if(d%2) flagD=0;

        d=d>>1;
    }

    if(flagA+flagB+flagC+flagD>0)
        return 1;
    else
        return 0;
}
