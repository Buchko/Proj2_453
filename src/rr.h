#include "ll.h"
#include "lwp.h"
#include <stdbool.h>
#include <stdlib.h>

void RrInit();
void RrShutdown();
void RrAdmit(thread *newThread);
void remove(thread *victim);
thread *next();
int qlen();
