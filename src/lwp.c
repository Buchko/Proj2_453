#include <stdlib.h>
#include <stdio.h>
#include "lwp.h"
#include "rr.h"
#include <unistd.h>
#include "sys/mman.h"
#include "sys/resource.h"
#include "debug_print.h"

#define DEBUG 1

static struct {
    scheduler scheduler;
    int nextTid;
    thread currentThread;
    ll *waitHead;
    ll *waitTail;
} lwpState;
static bool lwpHasInited = false;

long getStackSize() {
    long pageSize = sysconf(_SC_PAGESIZE);
    struct rlimit stackLimits;
    getrlimit(RLIMIT_STACK, &stackLimits);
    long softLimit = stackLimits.rlim_cur;
    if (softLimit == -1 || softLimit == RLIM_INFINITY) {
        return pageSize * 1000 * 8; //approx 8 MB
    } else {
        return ((softLimit + (pageSize - 1)) / pageSize) * pageSize; //dividing and rounding up
    }
}

tid_t lwp_wait(int *status) {
    while (1) {
        bool outOfThreads = lwpState.scheduler->qlen() <= 1;
        if (outOfThreads) {
            return NO_THREAD;
        }

        //search through threads until we get to the first terminated one or we get back to the start
        debug_print("waiting, currentThead is %lu\n", lwpState.currentThread->tid);
        thread originalHead = lwpState.currentThread;
        thread cur = originalHead;
        do {
            cur = lwpState.scheduler->next();
        } while (cur != originalHead && !LWPTERMINATED(cur->status));
        bool didTerminate = LWPTERMINATED(cur->status);
        if (didTerminate) {
            thread toRemove = cur;
            lwpState.scheduler->remove(toRemove);
            //cycle through scheduler until we get back to where we started
            while (cur != originalHead) {
                cur = lwpState.scheduler->next();
            }
            tid_t tid = toRemove->tid;
            free(toRemove);
            return tid;
        } else {
            //handling the case when no threads are finished
            //remove the current thread
            lwpState.scheduler->remove(lwpState.currentThread);
            //add it to the list of waiting threads
            lwpState.waitHead = pushLl(lwpState.waitHead, &lwpState.waitTail, lwpState.currentThread);
            //yield to the next thing
            lwp_yield();
        }
    }
}


void lwp_exit(int exitval) {
    debug_print("exitting, current thread is %lu\n", lwpState.currentThread->tid);
    lwpState.currentThread->status = MKTERMSTAT(LWP_TERM, exitval);
    //if we have a waiting thread, pop it from the waiting queue and put it into the scheduler;
    if (lwpState.waitHead != NULL) {
        thread waitingThread = (thread) popLl(&lwpState.waitHead);
        debug_print("admitted waiting thread is %lu\n", waitingThread->tid);
        lwpState.scheduler->admit(waitingThread);
    }
    lwp_yield();
}


void lwp_yield() {
    thread newThread = lwpState.currentThread;
    do {
        newThread = lwpState.scheduler->next();
    } while (LWPTERMINATED(newThread->status) && newThread != lwpState.currentThread);
    bool outOfThreads = (newThread == lwpState.currentThread);
    if (outOfThreads){
        int returnVal = LWPTERMSTAT(newThread->status);
        exit(returnVal);
    }

    debug_print("new thead is %lu\n", newThread->tid);
    //save the old state and put in the state of the new thread
    if (lwpState.currentThread != NULL) {
        thread oldThread = lwpState.currentThread;
        lwpState.currentThread = newThread;
        swap_rfiles(&(oldThread->state), &(newThread->state));
    } else {
        lwpState.currentThread = newThread;
        swap_rfiles(NULL, &(newThread->state));
    }
}

static void lwp_wrap(lwpfun fun, void *arg) {
    int rval;
    rval = fun(arg);
    lwp_exit(rval);
}

tid_t lwp_create(lwpfun function, void *arg) {
    if (!lwpHasInited) {
        lwpHasInited = true;
        lwpState.scheduler = RoundRobin;
        lwpState.scheduler->init();
        lwpState.nextTid = 1;
        lwpState.waitHead = lwpState.waitTail = NULL;
    };

    //init stack up stack
    long stackSize = getStackSize();
    void *s = mmap(NULL, stackSize, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);

    //convert everything to the same size to make math easier
    //if we have overflow errors tdhen just up the -1 to something else maybe? (give more back padding)
    unsigned long *s2 = ((unsigned long *) (s + stackSize)) - 16;

    thread t = (struct threadinfo_st *) calloc(sizeof(struct threadinfo_st), 1);
    //setup the registers we need - function pointer as arg1 and arg pointer as arg2
    t->state.rdi = (unsigned long) function;
    t->state.rsi = (unsigned long) arg;
    t->state.fxsave = FPU_INIT;
    t->stack = s;
    t->stacksize = stackSize;
    t->tid = lwpState.nextTid;
    t->status = LWP_LIVE;
    lwpState.nextTid++;

    //setup stack
    //put the return address on the top of stack
    *s2 = (unsigned long) &lwp_wrap;
    s2 -= 1;
    //make sure the program knows that there is nothing else back on the stack and set rbp to it
    *s2 = (unsigned long) NULL;
    t->state.rbp = (unsigned long) s2;
    //decrement and set rsp to the next value down in memory
    s2 -= 1;
    t->state.rsp = (unsigned long) s2;

    debug_print("admited_thread is %lu\n", t->tid);
    lwpState.scheduler->admit(t);
    return t->tid;
};

void lwp_start() {
    lwpState.currentThread = NULL;
    thread t = (struct threadinfo_st *) calloc(sizeof(struct threadinfo_st), 1);
    t->tid = lwpState.nextTid;
    lwpState.nextTid++;
    t->status = LWP_LIVE;

    swap_rfiles(&(t->state), NULL);
    debug_print("start admited_thread is %lu\n", t->tid);
    lwpState.scheduler->admit(t);
    lwp_yield();
}