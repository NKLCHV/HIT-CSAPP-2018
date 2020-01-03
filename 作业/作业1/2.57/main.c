#include <stdio.h>
#include <stdlib.h>

typedef unsigned char *byte_pointer;

void show_short(short x);
void show_long(long x);
void show_double(double x);
void show_bytes(byte_pointer start,size_t len);

int main()
{
    short a=821;
    long b=1170300821;
    double c=117030.0821;
    printf("short a:%d\t",a);
    show_short(a);
    printf("long b:%ld\t",b);
    show_long(b);
    printf("double c:%lf\t",c);
    show_double(c);
    return 0;
}

void show_bytes(byte_pointer start,size_t len)
{
    size_t i;
    for(i=0; i<len; i++)
        printf(" %.2x",start[i]);
    printf("\n");
}

void show_short(short x)
{
    show_bytes((byte_pointer)&x,sizeof(short));
}

void show_long(long x)
{
    show_bytes((byte_pointer)&x,sizeof(long int));
}

void show_double(double x)
{
    show_bytes((byte_pointer)&x, sizeof(double));
}


