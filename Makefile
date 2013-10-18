#the makefile

all:
	mkdir -p build
	cd build && clang ../*.c -g -O0 -std=c99 -o l1c

run:
	cd build && ./l1c -i ../sample.l1 -o sample.l1ir

parser:
	lua ParserGenerator.lua > L1ParserGeneratedPortion

clean:
	rm -rf build
