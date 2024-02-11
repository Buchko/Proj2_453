# Compiler and flags
CC = gcc
CFLAGS = -Wall -g3 -std=gnu99

#Name
NAME = lwp
# Source files
SRCS = src/main.c src/lwp.c src/rr.c src/ll.c src/magic64.S

# Object files
OBJS = $(SRCS:.c=.o)

# Build the executable
all: $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $^

# Compile source files into object files
%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

# Compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up object files and executable
clean:
	rm -f $(OBJS) all
