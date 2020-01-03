#include <stdio.h>
#include <stdlib.h>

int sum(int a[],unsigned len);

int main()
{
    int a[30];
    int s,i;
    unsigned len;
    for(i=0; i<30; i++)
        a[i]=i+1;
    printf("Input the len:");
    scanf("%d",&len);
    s=sum(a,len);
    printf("The sum is :%d\n",s);
    return 0;
}

int sum(int a[],unsigned len)
{
    int i,sum=0;
    for(i=0; i<=len-1; i++)
        sum+=a[i];
    return sum;
}
