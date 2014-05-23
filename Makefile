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
	#./build/l1c -it "-> -> a"
	./build/l1c -it "a -> (-> a)"
	#./build/l1c -it "-> a -> a"
	#./build/l1c -it "a : b : c"
	#./build/l1c -it "(a : b : c a) -> a"
	#./build/l1c -it "a | b | c"
	#./build/l1c -it "a b | c | d f e a | g"
	./build/l1c -i sample.l1 --json

runirgentests:
	./build/l1c -i sample.l1 --json | lua main.lua

parser:
	lua ParserGenerator.lua > L1ParserGeneratedPortion

clean:
	rm -rf build
