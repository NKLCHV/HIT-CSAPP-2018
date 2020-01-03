#include <stdio.h>
#include <stdlib.h>

void float_double(void);

int main()
{
	float_double();
	getchar();
	return 0;
}

void float_double(void)
{
	for (;;)
	{
		int x = rand();
		int y = rand();
		int z = rand();

		double dx = (double)x;
		double dy = (double)y;
		double dz = (double)z;

		int a, b, c, d, e;
		a = ((float)x == (float)dx);
		b = (dx - dy == (double)(x - y));
		c = ((dx + dy) + dz == dx + (dy + dz));
		d = ((dx*dy)*dz == dx * (dy*dz));
		e = (dx / dx == dz / dz);
		printf("A=%d,B=%d,C=%d,D=%d,E=%d", a, b, c, d, e);
		printf("     x=%7d, y=%7d, z=%7d\n", x, y, z);
		if (a*b*c*d*e == 0)
		{
			break;
		}
	}
}
