#ifndef LL_H
#define LL_H

// linked list
struct ll {
  void *val;
  struct ll *next;
};
typedef struct ll ll;
#endif

void appendLl(ll *head, ll **tail, void *val);
ll *createLl(void *val);
int removeLl(ll *head, ll *node);
ll *popLl(ll *head, ll **tail);
