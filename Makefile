#just an ordinary makefile

all:
	mkdir build
	cd build && clang ../*.c -std=c99 -g -o l1c
	rm -f *.o

run:
	./build/l1c

clean:
	rm -rf build