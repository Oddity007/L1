#the makefile

all:
	mkdir -p build
	cd build && clang -Wall -Wextra -pedantic ../*.c -g -O0 -std=c99 -o l1c
	#cd build && clang -Wall -Wextra -pedantic ../*.c ../dependencies/lua/*.c -g -O0 -std=c99 -o l1c

run:
	cd build && ./l1c -i ../sample.l1 --lua -o sample.l1ir

parser:
	lua ParserGenerator.lua > L1ParserGeneratedPortion

clean:
	rm -rf build
