static int counter = 0;

int main(void) {
    int volatile *p_int;
    p_int = &counter;
    while (*p_int < 21) {
        ++(*p_int);
    }

    p_int = (int *)0x20000002U;
    *p_int = (int)0xDEADBEEF;

    return 0;
}
