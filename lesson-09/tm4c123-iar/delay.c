#include "delay.h"

void delay(int iter) {
    int volatile counter;
    counter = 0;
    while (counter < iter) {  // delay loop
        ++counter;
    }
}

