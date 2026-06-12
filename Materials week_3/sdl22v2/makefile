# Example SDL22V2
EXE=sdl22v2

# Main target
all: $(EXE)

#  Msys/MinGW
ifeq "$(OS)" "Windows_NT"
CFLG=-O3 -Wall -DUSEGLEW -DSDL2
LIBS=-lmingw32 -lSDL2main -lSDL2 -mwindows -lSDL2_mixer -lglew32 -lglu32 -lopengl32 -lm
CLEAN=rm -f *.exe *.o *.a
else
#  OSX
ifeq "$(shell uname)" "Darwin"
CFLG=-O3 -Wall -Wno-deprecated-declarations -DSDL2
LIBS=-lSDL2main -lSDL2 -lSDL2_mixer -framework Cocoa -framework OpenGL
#  Linux/Unix/Solaris
else
CFLG=-O3 -Wall -DSDL2
LIBS=-lSDL2 -lSDL2_mixer -lGLU -lGL -lm
endif
#  OSX/Linux/Unix/Solaris
CLEAN=rm -f $(EXE) *.o *.a
endif

# Dependencies
sdl22v2.o: sdl22v2.c CSCIx229.h wall.h
wall.o: wall.c CSCIx229.h wall.h
fatal.o: fatal.c CSCIx229.h
errcheck.o: errcheck.c CSCIx229.h
print-dl.o: print-dl.c CSCIx229.h
loadtexbmp.o: loadtexbmp.c CSCIx229.h
loadobj.o: loadobj.c CSCIx229.h
projection.o: projection.c CSCIx229.h

#  Create archive
CSCIx229.a:fatal.o errcheck.o print-dl.o loadtexbmp.o loadobj.o projection.o
	ar -rcs $@ $^

# Compile rules
.c.o:
	gcc -c $(CFLG)  $<
.cpp.o:
	g++ -c $(CFLG)  $<

#  Link
sdl22v2:sdl22v2.o wall.o  CSCIx229.a
	gcc $(CFLG) -o $@ $^  $(LIBS)

#  Clean
clean:
	$(CLEAN)
