#include <stdio.h>
#include "lwp.h"
#include "unistd.h"
#include "sys/resource.h"

static int indentnum(void *num) {
    /* print the number num num times, indented by 5*num spaces
     * Not terribly interesting, but it is instructive.
     */
    long i;
    int howfar;

    howfar = (long) num; /* interpret num as an integer */
    for (i = 0; i < howfar; i++) {
        printf("%*d\n", howfar * 5, howfar);
//        lwp_yield(); /* let another have a turn */
    }
//    lwp_exit(i); /* bail when done.  This should
//                * be unnecessary if the stack has
//                * been properly prepared
//                */
}

int main(void) {
    printf("hello, world\n");
    long res = sysconf(_SC_PAGESIZE);
    struct rlimit foo;
    getrlimit(RLIMIT_STACK,&foo);
    printf("size is %d", res);
    printf("limits are hard: %d, and soft: %d", foo.rlim_cur, foo.rlim_max);

    lwpfun my_func_ptr = &indentnum;
    // Get the size of the function pointer variable
    int size_of_pointer = sizeof(my_func_ptr);

    printf("Size of function pointer: %d bytes\n", size_of_pointer);

    long i = 2;
    long j = 3;
    lwp_create(indentnum, (void*)i);
    lwp_create(indentnum, (void*)j);
    lwp_start();
    printf("returned to main!");
}
