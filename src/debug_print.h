#ifndef PROJ2_453_DEBUG_PRINT_H
#define PROJ2_453_DEBUG_PRINT_H

#define debug_print(fmt, ...)                                                  \
  do {                                                                         \
    if (DEBUG)                                                                 \
      fprintf(stderr, fmt, __VA_ARGS__);                                       \
  } while (0)


#endif //PROJ2_453_DEBUG_PRINT_H
