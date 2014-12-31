
CFLAGS=-I. -Wall -g3 -O0

libmylist.a: mylist.o lock.o
	ar rcs libmylist.a mylist.o lock.o

%.o: %.c
	gcc -o $@ -c $(CFLAGS) $<

clean:
	rm -rf *.o libmylist.a
