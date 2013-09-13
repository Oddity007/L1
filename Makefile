#the makefile

all:
	mkdir -p build
	cd build && clang ../*.c -g -O0 -std=c11 -o l1c

run:
	cd build && ./l1c

parser:
	lua ParserGenerator.lua > L1ParserGeneratedPortion

clean:
	rm -rf build