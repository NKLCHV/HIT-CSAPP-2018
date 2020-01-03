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
    int xx[10][8];
	for (;;)
	{
	    int i=0;

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
			xx[i][0]=x;
			xx[i][1]=y;
			xx[i][2]=z;
			xx[i][3]=a;
			xx[i][4]=b;
			xx[i][5]=c;
			xx[i][6]=d;
			xx[i][7]=e;
		    i++;
		}
		if(i==2)
            break;
	}
	for(int i=0;i<2;i++)
    {
        printf("\n\nx=%d  y=%d  z=%d  a=%d  b=%d  c=%d  d=%d  e=%d\n",xx[i][0],xx[i][1],xx[i][2],xx[i][3],xx[i][4],xx[i][5],xx[i][6],xx[i][7]);
    }
}
