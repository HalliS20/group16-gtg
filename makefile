.PHONY: all run clean aarch64-release aarch64-debug x86_64-debug

# By default build the aarch64 debug version
all: aarch64-debug

x86_64-debug:
	cmake --preset native-x86_64-debug
	cmake --build --preset native-x86_64-debug
	ln -sf x86_64-debug/compile_commands.json build/cmake/compile_commands.json

aarch64-release:
	cmake --preset cross-aarch64-release
	cmake --build --preset cross-aarch64-release
	ln -sf aarch64/compile_commands.json build/cmake/compile_commands.json

aarch64-debug:
	cmake --preset cross-aarch64-debug
	cmake --build --preset cross-aarch64-debug
	ln -sf aarch64-debug/compile_commands.json build/cmake/compile_commands.json

run: all
	./build/bin/gtg

clean:
	rm -rf ./build
