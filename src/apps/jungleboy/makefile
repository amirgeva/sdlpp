CC=g++
DEBUG=-O2
PROJ=jungleboy
CFLAGS=-c $(DEBUG) -std=c++11 -DLINUX -I ../../../include -I /usr/include/SDL
LFLAGS=$(DEBUG) -L/usr/lib -L../../../src -lsdlpp -lSDL -lSDL_image -lSDL_ttf -lSDLmain -ldl
OBJS=globals.o jungleboy.o main.o pickup.o 

$(PROJ): $(OBJS) ../../../src/libsdlpp.a
	$(CC) -o $(PROJ) $(OBJS) $(LFLAGS)

globals.o: globals.cpp
	$(CC) $(CFLAGS) globals.cpp

jungleboy.o: jungleboy.cpp
	$(CC) $(CFLAGS) jungleboy.cpp

main.o: main.cpp
	$(CC) $(CFLAGS) main.cpp

pickup.o: pickup.cpp
	$(CC) $(CFLAGS) pickup.cpp

../../../src/libsdlpp.a:
	make -C ../../../src

clean:
	rm -f *.o ../../../src/*.o ../../../src/GUI/*.o ../../../src/libsdlpp.a


