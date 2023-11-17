// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "syscall.h"
#include "ksyscall.h"
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// If you are handling a system call, don't forget to increment the pc
// before returning (by calling move_program_counter()). (Or else you'll loop
// making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions
//	is in machine.h.
//----------------------------------------------------------------------

/**
 * @brief Convert user string to system string
 *
 * @param addr addess of user string
 * @param convert_length set max length of string to convert, leave
 * blank to convert all characters of user string
 * @return char*
 */
char* stringUser2System(int addr, int convert_length = -1) {
    int length = 0;
    bool stop = false;
    char* str;

    do {
        int oneChar;
        kernel->machine->ReadMem(addr + length, 1, &oneChar);
        length++;
        // if convert_length == -1, we use '\0' to terminate the process
        // otherwise, we use convert_length to terminate the process
        stop = ((oneChar == '\0' && convert_length == -1) ||
                length == convert_length);
    } while (!stop);

    str = new char[length];
    for (int i = 0; i < length; i++) {
        int oneChar;
        kernel->machine->ReadMem(addr + i, 1,
                                 &oneChar);  // copy characters to kernel space
        str[i] = (unsigned char)oneChar;
    }
    return str;
}

/**
 * @brief Convert system string to user string
 *
 * @param str string to convert
 * @param addr addess of user string
 * @param convert_length set max length of string to convert, leave
 * blank to convert all characters of system string
 * @return void
 */
void StringSys2User(char* str, int addr, int convert_length = -1) {
    int length = (convert_length == -1 ? strlen(str) : convert_length);
    for (int i = 0; i < length; i++) {
        kernel->machine->WriteMem(addr + i, 1,
                                  str[i]);  // copy characters to user space
    }
    kernel->machine->WriteMem(addr + length, 1, '\0');
}

/**
 * Modify program counter
 * This code is adapted from `../machine/mipssim.cc`, line 667
 **/
void move_program_counter() {
    /* set previous programm counter (debugging only)
     * similar to: registers[PrevPCReg] = registers[PCReg];*/
    kernel->machine->WriteRegister(PrevPCReg,
                                   kernel->machine->ReadRegister(PCReg));

    /* set programm counter to next instruction
     * similar to: registers[PCReg] = registers[NextPCReg]*/
    kernel->machine->WriteRegister(PCReg,
                                   kernel->machine->ReadRegister(NextPCReg));

    /* set next programm counter for brach execution
     * similar to: registers[NextPCReg] = pcAfter;*/
    kernel->machine->WriteRegister(
        NextPCReg, kernel->machine->ReadRegister(NextPCReg) + 4);
}

/**
 * Handle not implemented syscall
 * This method will write the syscall to debug log and increase
 * the program counter.
 */
void handle_not_implemented_SC(int type) {
    DEBUG(dbgSys, "Not yet implemented syscall " << type << "\n");
    return move_program_counter();
}

void handle_SC_Halt() {
    DEBUG(dbgSys, "Shutdown, initiated by user program.\n");
    SysHalt();
    ASSERTNOTREACHED();
}

void handle_SC_Add() {
    DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + "
                         << kernel->machine->ReadRegister(5) << "\n");

    /* Process SysAdd Systemcall*/
    int result;
    result = SysAdd(
        /* int op1 */ (int)kernel->machine->ReadRegister(4),
        /* int op2 */ (int)kernel->machine->ReadRegister(5));

    DEBUG(dbgSys, "Add returning with " << result << "\n");
    /* Prepare Result */
    kernel->machine->WriteRegister(2, (int)result);

    return move_program_counter();
}

void handle_SC_ReadNum() {
    int result = SysReadNum();
    kernel->machine->WriteRegister(2, result);
    return move_program_counter();
}

void handle_SC_PrintNum() {
    int character = kernel->machine->ReadRegister(4);
    SysPrintNum(character);
    return move_program_counter();
}

void handle_SC_ReadChar() {
    char result = SysReadChar();
    kernel->machine->WriteRegister(2, (int)result);
    return move_program_counter();
}

void handle_SC_PrintChar() {
    char character = (char)kernel->machine->ReadRegister(4);
    SysPrintChar(character);
    return move_program_counter();
}

void handle_SC_RandomNum() {
    int result;
    result = SysRandomNum();
    kernel->machine->WriteRegister(2, result);
    return move_program_counter();
}

#define MAX_READ_STRING_LENGTH 255
void handle_SC_ReadString() {
    int memPtr = kernel->machine->ReadRegister(4);  // read address of C-string
    int length = kernel->machine->ReadRegister(5);  // read length of C-string
    if (length > MAX_READ_STRING_LENGTH) {  // avoid allocating large memory
        DEBUG(dbgSys, "String length exceeds " << MAX_READ_STRING_LENGTH);
        SysHalt();
    }
    char* buffer = SysReadString(length);
    StringSys2User(buffer, memPtr);
    delete[] buffer;
    return move_program_counter();
}

void handle_SC_PrintString() {
    int memPtr = kernel->machine->ReadRegister(4);  // read address of C-string
    char* buffer = stringUser2System(memPtr);

    SysPrintString(buffer, strlen(buffer));
    delete[] buffer;
    return move_program_counter();
}

void handle_SC_PrintStringUC() {
    int memPtr = kernel->machine->ReadRegister(4);  // read address of C-string
    char* buffer = stringUser2System(memPtr);
    int length = strlen(buffer);
    // convert to uppercase
    for (int i = 0; i < length; i++) {
        if (buffer[i] >= 'a' && buffer[i] <= 'z') {
            buffer[i] -= 32;
        }
    }

    SysPrintString(buffer, length);
    delete[] buffer;
    return move_program_counter();
}

void handle_SC_CreateFile() {
    int virtAddr = kernel->machine->ReadRegister(4);
    char* fileName = stringUser2System(virtAddr);

    if (SysCreateFile(fileName))
        kernel->machine->WriteRegister(2, 0);
    else
        kernel->machine->WriteRegister(2, -1);

    delete[] fileName;
    return move_program_counter();
}

void handle_SC_Open() {
    int virtAddr = kernel->machine->ReadRegister(4);
    char* fileName = stringUser2System(virtAddr);
    int type = kernel->machine->ReadRegister(5);

    kernel->machine->WriteRegister(2, SysOpen(fileName, type));

    delete fileName;
    return move_program_counter();
}

void handle_SC_Close() {
    int id = kernel->machine->ReadRegister(4);
    kernel->machine->WriteRegister(2, SysClose(id));

    return move_program_counter();
}

void handle_SC_Read() {
    int virtAddr = kernel->machine->ReadRegister(4);
    int charCount = kernel->machine->ReadRegister(5);
    char* buffer = stringUser2System(virtAddr, charCount);
    int fileId = kernel->machine->ReadRegister(6);

    DEBUG(dbgFile,
          "Read " << charCount << " chars from file " << fileId << "\n");

    kernel->machine->WriteRegister(2, SysRead(buffer, charCount, fileId));
    StringSys2User(buffer, virtAddr, charCount);

    delete[] buffer;
    return move_program_counter();
}

void handle_SC_Write() {
    int virtAddr = kernel->machine->ReadRegister(4);
    int charCount = kernel->machine->ReadRegister(5);
    char* buffer = stringUser2System(virtAddr, charCount);
    int fileId = kernel->machine->ReadRegister(6);

    DEBUG(dbgFile,
          "Write " << charCount << " chars to file " << fileId << "\n");

    kernel->machine->WriteRegister(2, SysWrite(buffer, charCount, fileId));
    StringSys2User(buffer, virtAddr, charCount);

    delete[] buffer;
    return move_program_counter();
}

/**
 * Handle SC_Seek
 * This method will seek the file to the given position.
 * @param seekPos: seek position (use -1 to seek to end of file) (get from R4)
 * @param fileId: file descriptor (get from R5)
 * @return -1 if failed to seek, otherwise return the new position
 */
void handle_SC_Seek() {
    int seekPos = kernel->machine->ReadRegister(4);
    int fileId = kernel->machine->ReadRegister(5);

    kernel->machine->WriteRegister(2, SysSeek(seekPos, fileId));

    return move_program_counter();
}

/**
 * @brief handle System Call Exec
 * @param virtAddr: virtual address of user string name (get from R4)
 * @return -1 if failed to Exec, otherwise return id of new process
 * (write result to R2)
 */
void handle_SC_Exec() {
    int virtAddr;
    virtAddr = kernel->machine->ReadRegister(
        4);  // doc dia chi ten chuong trinh tu thanh ghi r4
    char* name;
    name = stringUser2System(virtAddr);  // Lay ten chuong trinh, nap vao kernel
    if (name == NULL) {
        DEBUG(dbgSys, "\n Not enough memory in System");
        ASSERT(false);
        kernel->machine->WriteRegister(2, -1);
        return move_program_counter();
    }

    kernel->machine->WriteRegister(2, SysExec(name));
    // DO NOT DELETE NAME, THE THEARD WILL DELETE IT LATER
    // delete[] name;

    return move_program_counter();
}

/**
 * @brief handle System Call Join
 * @param id: thread id (get from R4)
 * @return -1 if failed to join, otherwise return exit code of
 * the thread. (write result to R2)
 */
void handle_SC_Join() {
    int id = kernel->machine->ReadRegister(4);
    kernel->machine->WriteRegister(2, SysJoin(id));
    return move_program_counter();
}

/**
 * @brief handle System Call Exit
 * @param id: thread id (get from R4)
 * @return -1 if failed to exit, otherwise return exit code of
 * the thread. (write result to R2)
 */
void handle_SC_Exit() {
    int id = kernel->machine->ReadRegister(4);
    kernel->machine->WriteRegister(2, SysExit(id));
    return move_program_counter();
}

void handle_SC_CreateSemaphore() {
    int virtAddr = kernel->machine->ReadRegister(4);
    int semval = kernel->machine->ReadRegister(5);

    char* name = stringUser2System(virtAddr);
    if (name == NULL) {
        DEBUG(dbgSys, "\n Not enough memory in System");
        ASSERT(false);
        kernel->machine->WriteRegister(2, -1);
        delete[] name;
        return move_program_counter();
    }

    kernel->machine->WriteRegister(2, SysCreateSemaphore(name, semval));
    delete[] name;
    return move_program_counter();
}

void handle_SC_Wait() {
    int virtAddr = kernel->machine->ReadRegister(4);

    char* name = stringUser2System(virtAddr);
    if (name == NULL) {
        DEBUG(dbgSys, "\n Not enough memory in System");
        ASSERT(false);
        kernel->machine->WriteRegister(2, -1);
        delete[] name;
        return move_program_counter();
    }

    kernel->machine->WriteRegister(2, SysWait(name));
    delete[] name;
    return move_program_counter();
}

void handle_SC_Signal() {
    int virtAddr = kernel->machine->ReadRegister(4);

    char* name = stringUser2System(virtAddr);
    if (name == NULL) {
        DEBUG(dbgSys, "\n Not enough memory in System");
        ASSERT(false);
        kernel->machine->WriteRegister(2, -1);
        delete[] name;
        return move_program_counter();
    }

    kernel->machine->WriteRegister(2, SysSignal(name));
    delete[] name;
    return move_program_counter();
}

void handle_SC_GetPid() {
    kernel->machine->WriteRegister(2, SysGetPid());
    return move_program_counter();
}

void handle_SC_ThreadSleep() {
    int ticks = kernel->machine->ReadRegister(4);
    IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);
    Thread* oldThread = kernel->currentThread;
    kernel->scheduler->Sleep(oldThread, ticks);
    kernel->scheduler->Run(kernel->scheduler->FindNextToRun(), false);
    (void)kernel->interrupt->SetLevel(oldLevel);
    return move_program_counter();
}

void handle_SC_ThreadFork() {
    int x = kernel->machine->ReadRegister(4);

    printf("FC  %d %d %d -- %d\n", kernel->currentThread->processID,
           kernel->currentThread->parrentID, x,
           kernel->machine->ReadRegister(PCReg));

    if (kernel->currentThread->isClone == true) {
        kernel->currentThread->isClone = false;
        kernel->machine->WriteRegister(2, 0);
    } else {
        int pid = SysVFork();
        kernel->machine->WriteRegister(2, pid);
    }

    return move_program_counter();
}

void handle_PageFault(int badVAdrr) {
    kernel->addrLock->P();
    int vpn = (unsigned)badVAdrr / PageSize;
    int offset = (unsigned)badVAdrr % PageSize;
    if (kernel->machine->tlb == NULL) {
        kernel->machine->pageTable[vpn].virtualPage =
            vpn;  // for now, virtual page # = phys page #
        kernel->machine->pageTable[vpn].physicalPage =
            kernel->gPhysPageBitMap->FindAndSet();
        // cerr << kernel->machine->pageTable[vpn].physicalPage << endl;
        kernel->machine->pageTable[vpn].valid = TRUE;
        kernel->machine->pageTable[vpn].use = FALSE;
        kernel->machine->pageTable[vpn].dirty = FALSE;
        kernel->machine->pageTable[vpn].readOnly =
            FALSE;  // if the code segment was entirely on
        // a separate page, we could set its
        // pages to be read-only
        // xóa các trang này trên memory
        bzero(&(kernel->machine
                    ->mainMemory[kernel->machine->pageTable[vpn].physicalPage *
                                 PageSize]),
              PageSize);
        DEBUG(dbgAddr,
              "phyPage " << kernel->machine->pageTable[vpn].physicalPage);

        if (kernel->currentThread->noffH.code.size > 0) {
            // for (vpn = 0; vpn < numPages; vpn++)
            kernel->currentThread->executable->ReadAt(
                &(kernel->machine->mainMemory[kernel->currentThread->noffH.code
                                                  .virtualAddr]) +
                    (kernel->machine->pageTable[vpn].physicalPage * PageSize),
                PageSize,
                kernel->currentThread->noffH.code.inFileAddr +
                    (vpn * PageSize));
        }

        if (kernel->currentThread->noffH.initData.size > 0) {
            // for (vpn = 0; vpn < numPages; vpn++)
            kernel->currentThread->executable->ReadAt(
                &(kernel->machine->mainMemory[kernel->currentThread->noffH
                                                  .initData.virtualAddr]) +
                    (kernel->machine->pageTable[vpn].physicalPage * PageSize),
                PageSize,
                kernel->currentThread->noffH.initData.inFileAddr +
                    (vpn * PageSize));
        }
    }
    kernel->addrLock->V();
}

void ExceptionHandler(ExceptionType which) {
    int type = kernel->machine->ReadRegister(2);

    DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

    switch (which) {
        case NoException:  // return control to kernel
            kernel->interrupt->setStatus(SystemMode);
            DEBUG(dbgSys, "Switch to system mode\n");
            break;
        case PageFaultException: {
            int badVAdrr = kernel->machine->ReadRegister(39);
            DEBUG(dbgSys, "PageFaultException: " << badVAdrr << "\n");
            return handle_PageFault(badVAdrr);
        }
        case ReadOnlyException:
        case BusErrorException:
        case AddressErrorException:
        case OverflowException:
        case IllegalInstrException:
        case NumExceptionTypes:
            cerr << "Error " << which << " occurs\n";
            SysHalt();
            ASSERTNOTREACHED();

        case SyscallException:
            switch (type) {
                case SC_Halt:
                    return handle_SC_Halt();
                case SC_Add:
                    return handle_SC_Add();
                case SC_ReadNum:
                    return handle_SC_ReadNum();
                case SC_PrintNum:
                    return handle_SC_PrintNum();
                case SC_ReadChar:
                    return handle_SC_ReadChar();
                case SC_PrintChar:
                    return handle_SC_PrintChar();
                case SC_RandomNum:
                    return handle_SC_RandomNum();
                case SC_ReadString:
                    return handle_SC_ReadString();
                case SC_PrintString:
                    return handle_SC_PrintString();
                case SC_CreateFile:
                    return handle_SC_CreateFile();
                case SC_Open:
                    return handle_SC_Open();
                case SC_Close:
                    return handle_SC_Close();
                case SC_Read:
                    return handle_SC_Read();
                case SC_Write:
                    return handle_SC_Write();
                case SC_Seek:
                    return handle_SC_Seek();
                case SC_Exec:
                    return handle_SC_Exec();
                case SC_Join:
                    return handle_SC_Join();
                case SC_Exit:
                    return handle_SC_Exit();
                case SC_CreateSemaphore:
                    return handle_SC_CreateSemaphore();
                case SC_Wait:
                    return handle_SC_Wait();
                case SC_Signal:
                    return handle_SC_Signal();
                case SC_GetPid:
                    return handle_SC_GetPid();
                case SC_PrintStringUC:
                    return handle_SC_PrintStringUC();
                case SC_ThreadSleep:
                    return handle_SC_ThreadSleep();
                case SC_MyThreadFork:
                    return handle_SC_ThreadFork();
                /**
                 * Handle all not implemented syscalls
                 * If you want to write a new handler for syscall:
                 * - Remove it from this list below
                 * - Write handle_SC_name()
                 * - Add new case for SC_name
                 */
                case SC_Create:
                case SC_Remove:
                case SC_ThreadFork:
                case SC_ThreadYield:
                case SC_ExecV:
                case SC_ThreadExit:
                case SC_ThreadJoin:
                    return handle_not_implemented_SC(type);

                default:
                    cerr << "Unexpected system call " << type << "\n";
                    break;
            }
            break;
        default:
            cerr << "Unexpected user mode exception" << (int)which << "\n";
            break;
    }

    ASSERTNOTREACHED();
}
