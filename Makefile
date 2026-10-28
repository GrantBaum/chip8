chip8: src/*.cpp include/*.h
	g++ -std=c++20 -Wall -Iinclude src/*.cpp -o chip8 $(shell sdl2-config --cflags --libs)

clean:
	rm -f chip8