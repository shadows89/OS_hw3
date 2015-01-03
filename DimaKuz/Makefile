CC=gcc
RANLIB=ranlib
TARGET=main
C_SOURCES=$(wildcard *.c)
C_OBJS=$(C_SOURCES:.c=.o)
CFLAGS=-I. -I. -Wall -Werror -g3 -O0 $(CFLAGS_EXTRA)
LDFLAGS=-L. -lmylist -lpthread

all: $(TARGET)

check: $(TARGET)
	./$(TARGET)

%.o: %.c
	$(CC) -o $@ -c $(CFLAGS) $<

$(TARGET): $(C_OBJS)
	$(CC) $(C_OBJS) $(LDFLAGS) -o $@

clean:
	rm -rf *.o $(TARGET)

.PHONY: all clean check
