#include <stdlib.h>
#include <stdio.h>
#include "lwp.h"
#include "rr.h"
#include <unistd.h>
#include "sys/mman.h"
#include "sys/resource.h"

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

tid_t lwp_wait(int* status){
    bool outOfThreads = lwpState.scheduler->qlen() <= 1;
    if (outOfThreads){
        return NO_THREAD;
    }

    //search through threads until we get to the first terminated one or we get back to the start
    thread originalHead = lwpState.currentThread;
    thread cur = originalHead;
    do {
        cur = lwpState.scheduler->next();
    } while (cur != originalHead ||LWPTERMINATED(cur->status));
    bool didTerminate = LWPTERMINATED(cur->status);
    if (didTerminate) {
        lwpState.scheduler->remove(cur);
        //cycle through scheduler until we get back to where we started
        while (cur != originalHead) {
            cur = lwpState.scheduler->next();
        }
        tid_t tid = cur->tid;
        free(cur);
        return tid;
    } else {
        //handling the case when no threads are finished
        //remove the current thread
        lwpState.scheduler->remove(lwpState.currentThread);
        //add it to the list of waiting threads
        pushLl(lwpState.waitHead, &lwpState.waitTail, lwpState.currentThread);
    }
}


void lwp_exit(int exitval) {
    lwpState.currentThread->status = MKTERMSTAT(LWP_TERM, exitval);
//    lwpState.scheduler->remove(lwpState.currentThread);
    lwp_yield();
}


void lwp_yield() {
    printf("yield\n");
    //get the next thing from the scheduler
    thread newThread = lwpState.scheduler->next();
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
        lwpState.nextTid = 0;
        lwpState.waitHead = lwpState.waitTail = NULL;
    };

    //init stack up stack
    long stackSize = getStackSize();
    void *s = mmap(NULL, stackSize, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);

    //convert everything to the same size to make math easier
    //if we have overflow errors then just up the -1 to something else maybe? (give more back padding)
    unsigned long *s2 = ((unsigned long *) (s + stackSize)) - 16;

    thread t = (struct threadinfo_st *) calloc(sizeof(struct threadinfo_st), 1);
    //setup the registers we need - function pointer as arg1 and arg pointer as arg2
    t->state.rdi = (unsigned long) function;
    t->state.rsi = (unsigned long) arg;
    t->state.fxsave = FPU_INIT;
    t->stack = s;
    t->stacksize = stackSize;
    t->tid = lwpState.nextTid;
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

    lwpState.scheduler->admit(t);
};

void lwp_start() {
    lwpState.currentThread = NULL;
    thread t = (struct threadinfo_st *) calloc(sizeof(struct threadinfo_st), 1);
    swap_rfiles(&(t->state), NULL);
    lwpState.scheduler->admit(t);
    lwp_yield();
}