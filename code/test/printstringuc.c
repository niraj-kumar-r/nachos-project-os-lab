#include "syscall.h"

char buffer[100];
int i = 0;
int main() {
    PrintString("String length: (<= 255):\n");
    ReadString(buffer, ReadNum());
    while (buffer[i] != '\0') {
        // make it uppercase
        if (buffer[i] >= 'a' && buffer[i] <= 'z') {
            buffer[i] = buffer[i] - 'a' + 'A';
        }
        i++;
    }
    PrintString(buffer);
}