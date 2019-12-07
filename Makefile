# Generic MinGW makefile (C source only)
# Modify variables/macros to fit your program

INSTALL_DIR=/usr/local
SBIN_DIR=$(INSTALL_DIR)/sbin

# make / make all | will compile your program.
# make clean      | deletes all compiled object and executable files.
# make depend     | rebuilds the dependancy list
# make run        | compiles (if needed) and runs your program

# Compiler command
CC = gcc

# Linker command
LD = gcc

# Flags to pass to the compiler - add "-g" to include debug information
CFLAGS = -Wall

# Flags to pass to the linker
LDFLAGS =

# Command used to delete files
RM = rm

# List your object files here
OBJS = port-open.o

# List your source files here
SRCS = port-open.c

# Define your compile target here.
PROG = port-open

# Compile everything.
all: $(PROG)

# Link the program
$(PROG): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -s -o $(PROG)

# All .o files depend on their corresponding .c file
%.o: %.c
	$(CC) $(CFLAGS) -c $<

install:
	cp -a port-open $(SBIN_DIR)/
	cd $(SBIN_DIR) && ln -fs port-open port-close

clean:
	$(RM) -f $(PROG)
	$(RM) -f *.o port-close

distclean: clean
	$(RM) -f depend


depend:
	$(CC) $(CFLAGS) -MM $(SRCS) > depend


include depend
