
int main() {
    int counter = 0;
    while (counter < 21) {
        ++counter;
        if ((counter & 1) != 0) {
            /* do something when the counter is odd */
        }
        else {
            /* do something when the counter is even */
        }
    }

    return 0;
}
