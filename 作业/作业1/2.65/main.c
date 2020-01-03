#include <stdio.h>
#include <stdlib.h>

int odd_ones(unsigned x);

int main()
{
    unsigned x;
    for(;;)
    {
        printf("please input an unsigned number:");
        scanf("%u",&x);
        printf("%d\n",odd_ones(x));
    }

    return 0;
}

int odd_ones(unsigned x)
{
	unsigned  a;
	a = x >> 16;
    x = x^a;
	a = x >> 8;
	x = x^a;
	a = x >> 4;
	x = x^a;
	a = x >> 2;
	x = x^a;
	a = x >> 1;
	x = x^a;
    x=x&1;
	return x;
}
