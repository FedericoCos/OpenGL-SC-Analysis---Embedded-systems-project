CXX = g++

run:
	$(CXX) *.cpp -lm `sdl2-config --cflags` `sdl2-config --libs` -lEGL -lGLESv2 -o gles_linux
	./gles_linux

clean:
	-rm -f gles_linux
	-rm -f *.o

refresh:
	-rm -f gles_linux
	-rm -f *.o
	$(CXX) *.cpp -lm `sdl2-config --cflags` `sdl2-config --libs` -lEGL -lGLESv2 -o gles_linux
	./gles_linux