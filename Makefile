#the makefile

all:
	mkdir -p generated
	lua Source/ParserGenerator.lua > generated/L1ParserGeneratedPortion
	cd Source && lua IRManagementGenerator.lua ../generated
	mkdir -p build
	#cd build && cc -DNDEBUG -Weverything -I ../generated -I ../Source ../Source/*.c -Os -fvisibility=hidden -std=c11 -Wno-unused-parameter -Wno-unused-function -c
	#cd build && cc -DNDEBUG -Weverything -I ../generated -I ../Source ../Source/*.c -Os -flto -fvisibility=hidden -std=c11 -Wno-unused-parameter -Wno-unused-function -o l1c
	cd build && cc -Weverything -I ../generated -I ../Source ../Source/*.c -O0 -g -std=c11 -Wno-unused-parameter -Wno-unused-function -o l1c

test:
	./build/l1c -i sample.l1
	./build/l1c -i Tests/test1.l1

clean:
	rm -rf generated
	rm -rf build
