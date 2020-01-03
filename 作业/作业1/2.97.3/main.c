#include<stdio.h>

#include<limits.h>

typedef unsigned float_bits;

float_bits float_i2f(int I)

{

    unsigned sign,frac,exp,k,flag;

    sign=((unsigned)I)>>31;

    if(I==0)

        return 0.0;

    if(I<0)

        I=~(I-1);

    k=31;

    frac=(unsigned)I;

    while(((frac>>k)&1)==0)

    {

        k--;

    }

    exp=127+k;

    if(k<23)

    {

        frac<<(23-k);

    }

    else

    {

        frac>>(k-23);

        flag=(1<<(k-23))-1;

        if((I&flag)>(1<<(k-24)))

        {

            frac++;

        }

        else if(((I&flag)==(1<<(k-24)))&&(frac&1))/* rounding to even number*/

        {

            frac++;

        }

        if(frac==(1<<24))

            exp++;

    }

    frac=frac&0x7fffff;

    return (sign<<31)+(exp<<23)+frac;

}

float_bits test(int x)

{

    float f=(float)x;

    unsigned I=*(unsigned *)&x;

    return I;

}

int main()

{

    int I=1;

    printf("%x\n",float_i2f(I));

    printf("%x\n",test(I));

    return 0;

}
