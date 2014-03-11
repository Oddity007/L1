#the makefile

all:
	mkdir -p build
	cd build && clang -Wall -Wextra -pedantic ../*.c -g -O0 -std=c99 -o l1c
	#cd build && clang -Wall -Wextra -pedantic ../*.c ../dependencies/lua/*.c -g -O0 -std=c99 -o l1c

run:
	./build/l1c -it "a x = x; a" --json
	./build/l1c -it "a x = []; a" --json
	./build/l1c -it "a [] = []; a" --json
	./build/l1c -it "a [] = [x ...xs]; a" --json
	./build/l1c -it "a [] = [x, ...xs]; a" --json
	./build/l1c -it "a [] = [x, y,]; a" --json
	./build/l1c -it "a [] = []; a" --json
	./build/l1c -i sample.l1 --json -o build/sample_ast.json

parser:
	lua ParserGenerator.lua > L1ParserGeneratedPortion

clean:
	rm -rf build
