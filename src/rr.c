#include "rr.h"

static struct {
    ll *head;
    ll *tail;
    int size;
} RrState;

void rrInit() {
    ll *newList = (ll *) calloc(1, sizeof(ll));
    RrState.tail = RrState.head = newList;
    RrState.size = 0;
}

void rrShutdown() {
    // loop through list and free everything
    for (ll *cur = RrState.head; cur != NULL; cur = cur->next) {
        ll *prev = cur;
        cur = cur->next;
        free(prev->val);
        free(prev);
    }
}

void rrAdmit(thread newThead) {
    bool isFirstThread = RrState.size == 0;
    if (isFirstThread) {
        RrState.head->val = newThead;
        RrState.tail->val = newThead;
        RrState.size++;
        return;
    }
    appendLl(RrState.head, &RrState.tail, newThead);
    RrState.size++;
}

void rrRemove(thread victim) {
    // search for victim in list
    for (ll *cur = RrState.head; cur != NULL; cur = cur->next) {
        if (cur->val == victim) {
            removeLl(RrState.head, cur);
            return;
        }
    }
    RrState.size--;
}

thread rrNext() {
    if (RrState.size == 0) {
        return NULL;
    }
    // move head to next thread
    thread prev = RrState.head->val;
    RrState.head = popLl(RrState.head, &RrState.tail);
    return prev;
}

int rrQlen() { return RrState.size; }

struct scheduler rrPublish = {rrInit, rrShutdown, rrAdmit,
                              rrRemove, rrNext, rrQlen};

scheduler RoundRobin = &rrPublish;
