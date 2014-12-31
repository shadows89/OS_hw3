SRC = user_threads.c
RANLIB = ranlib
CC = gcc
OBJ = $(SRC:.c=.o)
OUT = libuthreads.a
TARGET = $(OUT)
COMP_FLAG = -Werror -o0
all: $(TARGET) clean
user_threads.o: user_threads.c user_mutex.h user_io.h user_threads.h
	$(CC) $(COMP_FLAG) -c user_threads.c -o user_threads.o

$(TARGET): $(OBJ)
	ar rcs $(OUT) user_threads.o
	ranlib $(OUT)

depend:
	makedepend -- $(COMP_FLAG) -- $(SRC)

clean:
	rm $(OBJ)