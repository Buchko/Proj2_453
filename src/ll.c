#include "ll.h"
#include <stdbool.h>
#include <stdlib.h>

void appendLl(ll *head, ll **tail, void *val) {
    ll *newNode = (ll *) calloc(1, sizeof(ll));
    newNode->val = val;
    (*tail)->next = newNode;
    *tail = newNode;
}

ll *createLl(void *val) {
    ll *node = (ll *) calloc(1, sizeof(ll));
    node->val = val;
    node->next = node;
    return node;
}

int removeLl(ll *head, ll *node) {
    // returns 1 if this deletes the last node in the list, 0 otherwise
    bool onlyOne = (node->next == node);
    if (onlyOne) {
        free(node);
        return 1;
    }

    // search from head until we get to previous node
    ll *cur = head;
    for (; cur->next != node; cur = cur->next) { ;
    }
    cur->next = node->next;
    free(node);
    return 0;
}

ll *popAndPushLl(ll *head, ll **tail) {
    // returns the current head and pushes it to back
    ll *newHead = head->next;
    (*tail)->next = head;
    head->next = newHead;
    *tail = head;
    return newHead;
}

ll *pushLl(ll *head, ll **tail, void *val) {
    //creates if head is NULL, or appends if it isn't
    if (head == NULL) {
        head = createLl(val);
    } else {
        appendLl(head, tail, val);
    }
    return head;
}

void *popLl(ll **head) {
    //returns the current head and removes it from the queue
    bool onlyOne = (*head)->next = *head;
    ll *prevHead = *head;
    if (onlyOne){
        *head = NULL;
    } else {
        *head = (*head)->next;
    }
    void *ans = prevHead->val;
    free(prevHead);
    return ans;
};