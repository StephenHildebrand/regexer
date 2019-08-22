# Compiler for the default rule to use.
CC = gcc
# Compile options for the default rule.
CFLAGS = -g -Wall -std=c99
# Build the mygrep executable as default target
mygrep: mygrep.o pattern.o
mygrep.o: mygrep.c pattern.h
pattern.o: pattern.c pattern.h
# Delete any temporary files made during build or by tests.
clean:  # Only run when explicitly called on command line as a target.
	rm -f mygrep
	rm -f *.o
