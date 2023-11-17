#include "syscall.h"

int main() {
    int result;
    int i, j, k;
    k = 10;
    i = MyThreadFork(1);
    if (i == 0) {
        k = 11;
    }
    i = MyThreadFork(2);
}