#the makefile

all:
	mkdir -p generated
	lua Source/ParserGenerator.lua > generated/L1ParserGeneratedPortion
	cd Source && lua IRManagementGenerator.lua ../generated
	mkdir -p build
	cd build && cc -Weverything -I ../Generated -I ../Source ../Source/*.c -Os -flto -fvisibility=hidden -std=c11 -Wno-unused-parameter -Wno-unused-function -o l1c

test:
	./build/l1c -i sample.l1

clean:
	rm -rf generated
	rm -rf build
