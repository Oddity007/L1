#the makefile

all:
	mkdir -p build
	cd build && clang -Wall -Wextra -pedantic ../*.c -Os -std=c99 -o l1c
	#cd build && clang -Wall -Wextra -pedantic ../*.c ../dependencies/lua/*.c -g -O0 -std=c99 -o l1c

runparsetests:
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
	./build/l1c -it "b = a.x; x = a.y; x" --json	
	./build/l1c -it "b = a.x; x = a.y; a.y.x.s" --json
	./build/l1c -it "factorial = fix ('f -> (((nat.zero) -> (nat.successor) (nat.zero)) | ('sx -> (nat.multiply) (sx) (f x)))); factorial" --json
	./build/l1c -it "('sx : (nat.successor) 'x) -> (nat.multiply)" --json
	./build/l1c -it "('sx : (nat.successor) 'x) -> (nat.multiply) (sx) (f x)" --json
	./build/l1c -it "('sx -> (nat.multiply) (sx) (f x))" --json
	./build/l1c -it "(('sx : 'x) -> (sx) (f x))" --json
	./build/l1c -it "(('sx : (nat.successor) 'x) -> (nat.multiply) (sx) (f x))" --json
	./build/l1c -it "((nat.zero) -> (nat.successor) (nat.zero)) | (('sx : (nat.successor) 'x) -> (nat.multiply) (sx) (f x))" --json
	./build/l1c -it "(((nat.zero) -> (nat.successor) (nat.zero)) | ('sx -> (nat.multiply) (sx) (f x)))" --json
	./build/l1c -it "((z -> s z) | (('sx : s 'x) -> m (sx) (f x)))" --json
	./build/l1c -it "(((nat.zero) -> (nat.successor) (nat.zero)) | (('sx) -> (nat.multiply) (sx) (f x)))" --json
	./build/l1c -it "(((nat.zero) -> (nat.successor) (nat.zero)) | (('sx : (nat.successor) 'x) -> (nat.multiply) (sx) (f x)))" --json
	./build/l1c -it "'f -> (((nat.zero) -> (nat.successor) (nat.zero)) | (('sx : (nat.successor) 'x) -> (nat.multiply) (sx) (f x)))" --json
	./build/l1c -it "('f -> (((nat.zero) -> (nat.successor) (nat.zero)) | (('sx : (nat.successor) 'x) -> (nat.multiply) (sx) (f x))))" --json
	./build/l1c -it "fix ('f -> (((nat.zero) -> (nat.successor) (nat.zero)) | (('sx : ((nat.successor) 'x) -> (nat.multiply) (sx) (f x))))" --json
	./build/l1c -it "factorial = fix ('f -> (((nat.zero) -> (nat.successor) (nat.zero)) | (('sx : (nat.successor) 'x) -> (nat.multiply) (sx) (f x)))); factorial" --json

runirgentests:
	./build/l1c -i sample.l1 --json | lua main.lua

parser:
	lua ParserGenerator.lua > L1ParserGeneratedPortion

clean:
	rm -rf build
