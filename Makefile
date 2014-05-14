#the makefile

all:
	mkdir -p build
	cd build && clang -Wall -Wextra -pedantic ../*.c -Os -std=c99 -o l1c
	#cd build && clang -Wall -Wextra -pedantic ../*.c ../dependencies/lua/*.c -g -O0 -std=c99 -o l1c

run:
	./build/l1c -it "a x = x; a" --json
	./build/l1c -it "a x = []; a" --json
	./build/l1c -it "a [] = []; a" --json
	./build/l1c -it "a [] = [x ...xs]; a" --json
	./build/l1c -it "a [] = [x, ...xs]; a" --json
	./build/l1c -it "a [] = [x, y,]; a" --json
	./build/l1c -it "a x y = (b z w = x; b); a" --json
	./build/l1c -it "a x y = (z w -> x); a" --json
	./build/l1c -it "x y -> (z w -> x)" --json
	./build/l1c -it "a [] = []; a" --json
	./build/l1c -it "a x y = (.b z w = x; b); a" --json
	./build/l1c -it "a = f ('x -> (.z = o; .s 'x = q; z)); a" --json
	./build/l1c -it "'n -> (.z = __construct []; .s 'x = __construct [x]; z | s 'x)" --json
	./build/l1c -it "('n -> (.z = __construct []; .s 'x = __construct [x]; z | s 'x))" --json
	./build/l1c -it "f ('n -> (.z = __construct []; .s 'x = __construct [x]; z | s 'x))" --json
	./build/l1c -it "n = f ('n -> (.z = __construct []; .s 'x = __construct [x]; z | s 'x)); n" --json
	./build/l1c -it "nat = fpc ('nat -> (.zero = __construct []; .successor 'x = __construct [x]; zero | successor 'x)); nat" --json
	./build/l1c -i sample.l1 --json

parser:
	lua ParserGenerator.lua > L1ParserGeneratedPortion

clean:
	rm -rf build
