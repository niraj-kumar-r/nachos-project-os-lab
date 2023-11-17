/* exec.c
 *	Simple program to test multi programming with the exec system call.
 */

#include "syscall.h"
#define stdin 0
#define stdout 1

int main() {
    while (1) {
        int pid1, pid2;
        pid1 = Exec("add");
        pid2 = Exec("help");
    }
}
