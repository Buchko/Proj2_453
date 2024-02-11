#ifndef RR_H
#define RR_H

#include "lwp.h"
#include "ll.h"
#include <stdbool.h>
#include <stdlib.h>

void rrInit();

void rrShutdown();

void rrAdmit(thread newThread);

void rrRemove(thread victim);

thread rrNext();

int rrQlen();

extern struct scheduler rrPublish;
extern scheduler RoundRobin;
#endif