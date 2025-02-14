// scheduler.h
//	Data structures for the thread dispatcher and scheduler.
//	Primarily, the list of threads that are ready to run.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "copyright.h"
#include "list.h"
#include "thread.h"
#include <queue>
#include <cstdlib>
#include <cmath>

// The following class defines the scheduler/dispatcher abstraction --
// the data structures and operations needed to keep track of which
// thread is running, and which threads are ready but not running.

class Scheduler {
   public:
    Scheduler();  // Initialize list of ready threads
    Scheduler(bool priority);
    ~Scheduler();  // De-allocate ready list

    void ReadyToRun(Thread* thread);
    // Thread can be dispatched.
    Thread* FindNextToRun();  // Dequeue first thread on the ready
                              // list, if any, and return thread.
    void Run(Thread* nextThread, bool finishing);
    // Cause nextThread to start running
    void CheckToBeDestroyed();  // Check if thread that had been
                                // running needs to be deleted
    void Print();               // Print contents of ready list

    void Sleep(Thread* thread, int ticks);
    void WakeUp();

    // void SetPriority(bool hasPriority) { this->hasPriority = hasPriority; }
    bool GetPriority() { return hasPriority; }

    // SelfTest for scheduler is implemented in class Thread

   private:
    bool hasPriority;

    List<Thread*>* readyList;  // queue of threads that are ready to run,
                               // but not running
    List<Thread*>* sleepList;  // queue of threads that are sleeping
                               // but not running

    Thread* toBeDestroyed;  // finishing thread to be destroyed
                            // by the next thread that runs

    struct CustomComparator {
        bool operator()(const pair<int, Thread*>& a,
                        const pair<int, Thread*>& b) {
            // Compare the first elements of the pairs (the integers) in
            // ascending order.
            return a.first > b.first;
        }
    };

    priority_queue<pair<int, Thread*>, vector<pair<int, Thread*>>,
                   CustomComparator>
        pq;
};

#endif  // SCHEDULER_H
