#the makefile

all:
	mkdir -p build
	cd build && cc -Wall -Wextra -pedantic ../*.c -O0 -g -flto -fvisibility=hidden -std=c99 -Wno-unused-parameter -Wno-unused-function -o l1c

parser:
	lua ParserGenerator.lua > L1ParserGeneratedPortion

test:
	./build/l1c -i sample.l1

clean:
	rm -rf L1ParserGeneratedPortion
	rm -rf build
