#include "syscall.h"

char buffer[100];
int main() {
    PrintString("String length: (<= 255):\n");
    ReadString(buffer, ReadNum());
    PrintStringUC(buffer);
}