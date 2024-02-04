# Compiler and flags
CC = gcc
CFLAGS = -Wall -g -std=gnu99

#Name
NAME = lwp
# Source files
SRCS = src/main.c 

# Object files
OBJS = $(SRCS:.c=.o)

# Build the executable
all: $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $^


# Compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up object files and executable
clean:
	rm -f $(OBJS) all
