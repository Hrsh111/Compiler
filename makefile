
CC = gcc
CFLAGS = -Wall -Wextra -std=c99


TARGET = myCompiler


SOURCES = driver1.c lexer.c parser.c stack.c
OBJECTS = $(SOURCES:.c=.o)


all: $(TARGET)


$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)


driver1.o: driver1.c lexer.h parser.h parserDef.h
	$(CC) $(CFLAGS) -c driver1.c


lexer.o: lexer.c lexer.h lexerDef.h
	$(CC) $(CFLAGS) -c lexer.c


parser.o: parser.c parser.h parserDef.h stack.h lexerDef.h
	$(CC) $(CFLAGS) -c parser.c


stack.o: stack.c
	$(CC) $(CFLAGS) -c stack.c


clean:
	rm -f $(OBJECTS) $(TARGET)
