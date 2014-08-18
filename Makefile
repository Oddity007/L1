#the makefile

all:
	mkdir -p build
	cd build && clang -Wall -Wextra -pedantic ../*.c -O0 -g -flto -fvisibility=hidden -std=c99 -Wno-unused-parameter -Wno-unused-function -o l1c

parser:
	lua ParserGenerator.lua > L1ParserGeneratedPortion

testparsing:
	./build/l1c -i sample.l1

test:
	./build/l1c -i sample.l1 | lua L1GenerateIR.lua

clean:
	rm -rf L1ParserGeneratedPortion
	rm -rf build
