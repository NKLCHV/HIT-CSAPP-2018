#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int saturating_add (int x,int y);
int main()
{
    int a,b,c;
    for(;;)
    {
        printf("input two numbers a,b:");
        scanf("%d,%d",&a,&b);
        c=saturating_add(a,b);
        printf("%d\n",c);
    }
    return 0;
}
//2147483648

int saturating_add(int x,int y)
{
    int a,b,c,d;
	a = sizeof(int) << 3;
	b = x + y;
	c = (( x ^ b) & (y ^ b)) >> ( a - 1);
	d = x >> ( a  - 1);
	return (b | c) - ( (c & 1) << ( a - 1))^ (d & c);
}

