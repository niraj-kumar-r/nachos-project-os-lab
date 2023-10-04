#include "syscall.h"

int main() {
    int i, j;
    while (1) {
        for (i = 0; i < 1000; i++) {
            for (j = 0; j < 200; j++)
                ;
        }
        PrintString("In testSleep2\n");
    }
}
