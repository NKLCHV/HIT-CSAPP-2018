#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char a='l';
signed char b='r';
unsigned char c='x';

int d=300821;
signed int e=300821;
unsigned int f=300821;

short g=117;
long h=117;
signed short int i=821;
unsigned short int j=821;
signed long int k=1170300;
unsigned long int l=1170300;
signed long long int m=1170300821;
unsigned long long int n=1170300821;

float o=152201.1999;
double p=1999.0923;

int *q=&d;
char *r=&a;
float *s=&o;
double *t=&p;

int ar[4]={1,2,3,4};

struct str{
    int aa;
    char bb;
    float cc;
    double dd;
};

union uni{
    int aa;
    char bb;
    float cc;
    double dd;
};

enum box {pencil,pen};

typedef unsigned char *byte_pointer;

void show_bytes(byte_pointer start,size_t len)
{
    size_t i;
    for(i=0; i<len; i++)
        printf(" %.2x",start[i]);
}

int main()
{
    unsigned char z[30];
    printf("         char  a  %c  %p  %x\n", a, &a, a);
    printf("signed   char  b  %c  %p  %x\n", b, &b, b);
    printf("unsigned char  c  %c  %p  %x\n", c, &c, c);
    printf("\n");
    printf("         int  d  %d  %p ", d, &d);  show_bytes((byte_pointer)&d,sizeof(int));  printf("\n");
    printf("signed   int  e  %d  %p ", e, &e);  show_bytes((byte_pointer)&e,sizeof(signed int));  printf("\n");
    printf("unsigned int  f  %d  %p ", f, &f);  show_bytes((byte_pointer)&f,sizeof(unsigned int));  printf("\n");
    printf("\n");

    printf("                 short  g  %10d  %p ", g, &g);  show_bytes((byte_pointer)&g,sizeof(short));  printf("\n");
    printf("                  long  h  %10d  %p ", h, &h);  show_bytes((byte_pointer)&h,sizeof(long));  printf("\n");
    printf("      signed short int  i  %10d  %p ", i, &i);  show_bytes((byte_pointer)&i,sizeof(signed short int));  printf("\n");
    printf("    unsigned short int  j  %10d  %p ", j, &j);  show_bytes((byte_pointer)&j,sizeof(unsigned short int));  printf("\n");
    printf("       signed long int  k  %10d  %p ", k, &k);  show_bytes((byte_pointer)&k,sizeof(signed long int));  printf("\n");
    printf("     unsigned long int  l  %10d  %p ", l, &l);  show_bytes((byte_pointer)&l,sizeof(unsigned long int));  printf("\n");
    printf("  signed long long int  m  %10d  %p ", m, &m);  show_bytes((byte_pointer)&m,sizeof(signed long long int));  printf("\n");
    printf("unsigned long long int  n  %10d  %p ", n, &n);  show_bytes((byte_pointer)&n,sizeof(unsigned long long int));  printf("\n");
    printf("\n");

    printf(" float  o  %15f  %p ", o, &o);  show_bytes((byte_pointer)&o,sizeof(float));  printf("\n");
    printf("double  p  %15lf  %p ", p, &p);  show_bytes((byte_pointer)&p,sizeof(double));  printf("\n");
    printf("\n");

    printf("  int *  q  %10d  %p  %p ", *q, q, &q);   show_bytes((byte_pointer)&q,(size_t)sizeof(int*));  printf("\n");
    printf("  char*  r  %10c  %p  %p ", *r, r, &r);  show_bytes((byte_pointer)&r,(size_t)sizeof(char*));  printf("\n");
    printf(" float*  s  %10f  %p  %p ", *s, s, &s);  show_bytes((byte_pointer)&s,(size_t)sizeof(float*));  printf("\n");
    printf("double*  t  %10lf  %p  %p ", *t, t, &t);  show_bytes((byte_pointer)&t,(size_t)sizeof(double*));  printf("\n");
    printf("\n");

    printf("array   int  ar[0]  %d  %p ", ar[0], &ar[0]);  show_bytes((byte_pointer)&ar[0],sizeof(int));  printf("\n");
    printf("array   int  ar[1]  %d  %p ", ar[1], &ar[1]);  show_bytes((byte_pointer)&ar[1],sizeof(int));  printf("\n");
    printf("array   int  ar[2]  %d  %p ", ar[2], &ar[2]);  show_bytes((byte_pointer)&ar[2],sizeof(int));  printf("\n");
    printf("array   int  ar[3]  %d  %p ", ar[3], &ar[3]);  show_bytes((byte_pointer)&ar[3],sizeof(int));  printf("\n");
    printf("\n");

    struct str stru;
    stru.aa=11703;  stru.bb='l';  stru.cc=1522.01;  stru.dd=1999.0923;
    union uni unio;
    unio.aa=300821;  unio.bb='r';  unio.cc=15.2201;  stru.dd=199.90923;
    enum box penbox;

    printf("struct   int  aa  %15d  %p ", stru.aa, &stru.aa);  show_bytes((byte_pointer)&stru.aa,sizeof(int));  printf("\n");
    printf("struct  char  bb  %15c  %p ", stru.bb, &stru.bb);  show_bytes((byte_pointer)&stru.bb,sizeof(char));  printf("\n");
    printf("struct float  cc  %15f  %p ", stru.cc, &stru.cc);  show_bytes((byte_pointer)&stru.cc,sizeof(float));  printf("\n");
    printf("struct double dd  %15lf  %p ", stru.dd, &stru.dd);  show_bytes((byte_pointer)&stru.dd,sizeof(double));  printf("\n");
    printf("\n");

    printf("union   int  aa  %15d  %p ", unio.aa, &unio.aa);  show_bytes((byte_pointer)&unio.aa,sizeof(int));  printf("\n");
    printf("union  char  bb  %15c  %p ", unio.bb, &unio.bb);  show_bytes((byte_pointer)&unio.bb,sizeof(char));  printf("\n");
    printf("union float  cc  %15f  %p ", unio.cc, &unio.cc);  show_bytes((byte_pointer)&unio.cc,sizeof(float));  printf("\n");
    printf("union double dd  %15lf  %p ", unio.dd, &unio.dd);  show_bytes((byte_pointer)&unio.dd,sizeof(double));  printf("\n");
    printf("\n");

    printf("enum  penbox  值: %d  %d,地址: %p,字节:",pencil,pen,&penbox);
    int zz=pencil;int zzz=pen;
    show_bytes((byte_pointer)&zz,sizeof(int)); printf("\t");
    show_bytes((byte_pointer)&zzz,sizeof(int));
    printf("\n");

    printf("main()函数的地址=%p\n",&main);
    printf("printf的地址是=%p\n",&printf);
    return 0;
}
