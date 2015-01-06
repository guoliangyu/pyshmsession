#include <stdio.h>

struct BitSet{
    unsigned char a:16;
    unsigned int b:4;
};

int main()
{
    printf("%d\n", sizeof(BitSet));
    return 0;
}

