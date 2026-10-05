.PHONY: all run clean

all: build/cmake/CMakeCache.txt
	cmake --build build/cmake

build/cmake/CMakeCache.txt:
	cmake -S . -B build/cmake -G Ninja

run: all
	./build/bin/gtg

clean:
	rm -rf build
