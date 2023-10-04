#include "syscall.h"

int main() {
    int i, j;
    Exec("testSleep2");
    while (1) {
        ThreadSleep(5000000);
        for (i = 0; i < 1000; i++) {
            for (j = 0; j < 200; j++) {
            }
        }
        PrintString("In testSleep\n");
    }
}
