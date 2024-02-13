#ifndef LL_H
#define LL_H

#include "lwp.h"
// linked list
struct ll {
  void *val;
  struct ll *next;
};
typedef struct ll ll;
#endif

void appendLl(ll *head, ll **tail, void *val);
ll* pushLl(ll *head, ll **tail, void *val);
ll *createLl(void *val);
int removeLl(ll *head, ll *node);
ll *popAndPushLl(ll *head, ll **tail);
void *popLl(ll **head);
