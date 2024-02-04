#include "rr.h"

static struct {
  ll *head;
  ll *tail;
  int size;
} RrState;

void RrInit() {
  ll *newList = (ll *)calloc(1, sizeof(ll));
  RrState.tail = RrState.head = newList;
  RrState.size = 0;
}

void RrShutdown() {
  // loop through list and free everything
  for (ll *cur = RrState.head; cur != NULL; cur = cur->next) {
    ll *prev = cur;
    cur = cur->next;
    free(prev->val);
    free(prev);
  }
}
void RrAdmit(thread *newThead) {
  bool isFirstThread = RrState.head == NULL;
  if (isFirstThread) {
    RrState.tail->val = RrState.head->val = newThead;
    return;
  }
  appendLl(RrState.head, &RrState.tail, newThead);
  RrState.size++;
}

void remove(thread *victim) {
  // search for victim in list
  for (ll *cur = RrState.head; cur != NULL; cur = cur->next) {
    if (cur->val == victim) {
      removeLl(RrState.head, cur);
      return;
    }
  }
  RrState.size--;
}

thread *next() {
  if (RrState.size == 0) {
    return NULL;
  }
  // move head to next thread
  thread *prev = RrState.head->val;
  RrState.head = popLl(RrState.head, &RrState.tail);
  return prev;
}

int qlen() { return RrState.size; }
