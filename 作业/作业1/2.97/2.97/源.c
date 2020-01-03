#include <stdio.h>
#include<climits>
#include<Windows.h>

typedef unsigned float_bits;

float_bits float_i2f(int i);
float_bits test(int x)

{

	float f = (float)x;

	unsigned I = *(unsigned *)&x;

	return I;

}

int main()

{

	int I;

	printf("%x\n", float_i2f(I));

	printf("%x\n", test(I));
	system("pause");
	return 0;

}

float_bits float_i2f(int i)
{
	unsigned sign, exp, frac, bias,flag;
	bias = 127;
	frac = (unsigned)i;

	if (i == 0)
		return 0;

	if (i == INT_MIN) 
	{ 
		sign = 1;
		exp = 31 + bias;
		frac = 0;
		return sign << 31 | exp << 23 | frac;
	}

	sign = i >> 31;
	int j, k=i;
	if (sign == 1)   k = ~(i-1);
	int first = 0;
	for (j = 1; j <32; j++) 
	{
		if ((k >> j) & 1)
		{
			first = j;
			break;
		}
	}

	exp = bias + first;
	if (first < 23)
		frac << (23 - first);
	else
	{
		frac >> (first - 23);
		flag = (1 << (first- 23)) - 1;
		if( ((k&flag) > (1 << (first - 24))) || (((k&flag) == (1 << (first - 24))) && (frac & 1)))
			frac++;
		if (frac == (1 << 24))
			exp++;
	}
	frac = frac & 0x7fffff;

	return (sign << 31) | (exp << 23) | frac;
}