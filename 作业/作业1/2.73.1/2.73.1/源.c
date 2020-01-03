#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ht (unsigned)pow(2,15)

int saturating_add(int x, int y);
int main()
{
	int a, b, c;
	printf("input two numbers a,b:");
	scanf_s("%d,%d", &a, &b);
	c = saturating_add(a, b);
	printf("%d", c);
	system("pause");
	return 0;
}
//2147483648
int saturating_add(int x, int y)
{
	int f;
	unsigned a, b;
	unsigned s[3] = { 0,0,0 };
	a = (unsigned)x;
	b = (unsigned)y;
	s[0] = (a%ht + b % ht) % ht;
	s[1] = (a / ht + b / ht + (a%ht + b % ht) / ht) % ht;
	s[2] = (a / ht + b / ht + (a%ht + b % ht) / ht) / ht;
	if ((s[2] / 2 > 0) && x > 0 && y > 0)
	{
		f = 2147483647;
	}
	else if()
	return f;
}
