#just an ordinary makefile

all:
	clang *.c -std=c99 -o l1c
	rm -f *.o

clean:
	rm -f l1c