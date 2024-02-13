#include "lwp.h"
#include "sys/resource.h"
#include "unistd.h"
#include <stdio.h>
static int indentnum(void *num) {
    /* print the number num num times, indented by 5*num spaces
     * Not terribly interesting, but it is instructive.
     */
    long i;
    int howfar;

    howfar = (long) num; /* interpret num as an integer */
    for (i = 0; i < howfar; i++) {
        printf("%*d\n", howfar * 5, howfar);
        lwp_yield(); /* let another have a turn */
    }
        lwp_exit(i); /* bail when done.  This should
    //                * be unnecessary if the stack has
    //                * been properly prepared
    //                */
}

int main(void) {
    printf("hello, world\n");
    long res = sysconf(_SC_PAGESIZE);
    struct rlimit foo;
    getrlimit(RLIMIT_STACK, &foo);
    printf("size is %d", res);
    long i;

    printf("Launching LWPS\n");
    int numLwps = 3;

    /* spawn a number of individual LWPs */
    for (i = 1; i <= numLwps; i++) {
        lwp_create((lwpfun) indentnum, (void *) i);
    }

    lwp_start();

    /* wait for the other LWPs */
    for (i = 1; i <= numLwps; i++) {
        int status, num;
        tid_t t;
        t = lwp_wait(&status);
        num = LWPTERMSTAT(status);
        printf("Thread %ld exited with status %d\n", t, num); }

    printf("Back from LWPS.\n");
    lwp_exit(0);
    return 0;
}


