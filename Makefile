CC=gcc
COPTS=-g -std=gnu99 -Wall

all: chromaTerrain

JUNK=*.o *~ core *.dSYM

clean:
  -rm -rf $(JUNK)

clobber:
	-rm -rf $(JUNK) $(ALL)

ifeq "$(shell uname)" "Darwin"
LIBS=-framework GLUT -framework OpenGL -framework Cocoa
else
LIBS=-L/usr/X11R6/lib -lglut -lGLU -lGL -lXext -lX11 -lm
endif

chromaTerrain: chromaTerrain.o matrix.o
	$(CC) $(COPTS) $^ -o $@ $(LIBS)

.c.o:
	$(CC) -c $(COPTS) $<

chromaTerrain.o: chromaTerrain.c matrix.h
matrix.o: matrix.c matrix.h
