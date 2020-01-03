#include <stdio.h>

typedef unsigned float_bit;

float_bit float_absval(float_bit f);

int main()
{
	float_bit f,g;
	printf("please input a float_bit:");
	scanf_s("%u", &f);
	g = float_absval(f);
	printf("%u", g);
}

float_bit float_absval(float_bit f)
{
	float_bit sign = f >> 31;
	float_bit exp = f >> 23 & 0xFF;
	float_bit frac = f & 0x7FFFFF;
	if ((exp == 0xFF) && (frac != 0))
	{
		return f;
	}
	else
	{
		sign = 0;
	}
	f = (sign << 31) | (exp << 23) | frac;
	return f;
}