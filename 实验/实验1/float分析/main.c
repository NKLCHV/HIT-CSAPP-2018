#include <stdio.h>
#include <stdlib.h>

int main()
{
    float f;

    for(;;)
    {
        printf("请输入一个浮点数:");
        scanf("%f",&f);
        printf("这个浮点数的数值是:%f\n",f);
        if(f==0)
            break;
    }
    return 0;
}
