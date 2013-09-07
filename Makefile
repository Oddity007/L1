#the makefile

debug:
	mkdir -p build
	cd build && clang ../*.c -g -std=c11 -o l1c
	#&& ./l1c

clean:
	rm -rf build
