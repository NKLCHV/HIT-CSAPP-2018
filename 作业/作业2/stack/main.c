#include <stdio.h>
#include <stdlib.h>

void TextStack(int a0,int a1,int a2,int a3,int a4,int a5,int a6,int a7,int a8,int a9);

int main()
{
    int a0,a1,a2,a3,a4,a5,a6,a7,a8,a9;
    a0=1; a1=2; a2=3; a3=4; a4=5; a5=6; a6=7; a7=8; a8=9; a9=10;
    TextStack(a0,a1,a2,a3,a4,a5,a6,a7,a8,a9);
    return 0;
}

void TextStack(int a0,int a1,int a2,int a3,int a4,int a5,int a6,int a7,int a8,int a9)
{
    int b[10];
    b[0]=a0; b[1]=a1; b[2]=a2; b[3]=a3; b[4]=a4;
    b[5]=a5; b[6]=a6; b[7]=a7; b[8]=a8; b[9]=a9;
}
