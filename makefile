CC=gcc
CFLAGS=-Wall -Wextra -std=c99

all: compiler

compiler: lexer.o parser.o driver.o
	$(CC) $(CFLAGS) lexer.o parser.o driver.o -o compiler

lexer.o: lexer.c lexer.h lexerDef.h
	$(CC) $(CFLAGS) -c lexer.c

parser.o: parser.c parser.h parserDef.h lexer.h
	$(CC) $(CFLAGS) -c parser.c

driver.o: driver.c lexer.h parser.h
	$(CC) $(CFLAGS) -c driver.c

clean:
	rm -f *.o compiler
