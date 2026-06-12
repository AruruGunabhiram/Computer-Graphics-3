# Example GLFW22
EXE=glfw22

# Main target
all: $(EXE)

#  Msys/MinGW
ifeq "$(OS)" "Windows_NT"
CFLG=-O3 -Wall -DUSEGLEW -DGLFW
LIBS=-lglfw3 -lglew32 -lglu32 -lopengl32 -lm
CLEAN=rm -f *.exe *.o *.a
else
#  OSX
ifeq "$(shell uname)" "Darwin"
CFLG=-O3 -Wall -Wno-deprecated-declarations -DGLFW
LIBS=-lglfw -framework Cocoa -framework OpenGL -framework IOKit
#  Linux/Unix/Solaris
else
CFLG=-O3 -Wall -DGLFW
LIBS=-lglfw -lGLU -lGL -lm
endif
#  OSX/Linux/Unix/Solaris
CLEAN=rm -f $(EXE) *.o *.a
endif

# Dependencies
glfw22.o: glfw22.c CSCIx229.h wall.h
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
glfw22:glfw22.o wall.o  CSCIx229.a
	gcc $(CFLG) -o $@ $^  $(LIBS)

#  Clean
clean:
	$(CLEAN)
