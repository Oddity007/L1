#the makefile

all:
	mkdir -p build
	cd build && clang -Wall -Wextra -pedantic ../*.c -Os -flto -fvisibility=hidden -std=c99 -o l1c

parser:
	lua ParserGenerator.lua > L1ParserGeneratedPortion

clean:
	rm -rf L1ParserGeneratedPortion
	rm -rf build
