chip8: main.cpp chip8.cpp instructions.cpp chip8.h font.h
	g++ -std=c++17 -Wall main.cpp chip8.cpp instructions.cpp -o chip8 $(shell sdl2-config --cflags --libs)

clean:
	rm -f chip8