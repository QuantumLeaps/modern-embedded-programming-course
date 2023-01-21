#include <assert.h> /* standard assert() facility */
#include <stdio.h>  /* for printf() */

int int_div(int x, int y) {
    assert(y != 0);  /* <-- assertion (precondition) */

    printf("(%d, %d)", x, y);
    return x/y;
}

int int_log2(int x) { /* binary logarithm */
    static int const log2_lut[] = {
       -1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
    };

    /* assertion (precondition) */
    assert((0 < x) && (x < sizeof(log2_lut)/sizeof(log2_lut[0])));

    printf("(%d)", x);
    return log2_lut[x];
}

/* the main function */
int main() {
    int y;

    printf("begin\n");

    printf("int_log2 ");
    y = int_log2(23);
    printf("=%d\n", y);

    printf("int_div ");
    y = int_div(10, 2);
    printf("=%d\n", y);

    printf("int_log2 ");
    y = int_log2(0); //???
    printf("=%d\n", y);

    printf("int_div ");
    y = int_div(63, 0); //???
    printf("=%d\n", y);

    printf("end\n");
}
