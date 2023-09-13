/* exec.c
 *	Simple program to test multi programming with the exec system call.
 */

#include "syscall.h"
#define stdin 0
#define stdout 1

int main() {
    int pid1, pid2;
    pid1 = Exec("add");
    pid2 = Exec("help");
    if (pid1 < 0) {
        Write("Exec failed: ", 14, stdout);
        PrintNum(pid1);
    } else if (pid2 < 0) {
        Write("Exec failed: ", 14, stdout);
        PrintNum(pid2);
    } else {
        Join(pid1);
        Join(pid2);
    }
}
