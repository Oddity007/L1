#a basic makefile
all:
	lemon L1ParserInternal.y -q
	clang *.c -g -o l1c

clean:
	rm -rf L1ParserInternal.h
	rm -rf L1ParserInternal.c
	rm -rf l1c
	rm -rf l1c.dYSM