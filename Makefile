EXE=foggymountains

# Main target
all: $(EXE)

#  MinGW
ifeq "$(OS)" "Windows_NT"
CFLG=-O3 -Wall
LIBS=-lglut32cu -lglu32 -lopengl32
CLEAN=del *.exe *.o *.a
else
#  OSX
ifeq "$(shell uname)" "Darwin"
CFLG=-O3 -Wall -Wno-deprecated-declarations
LIBS=-framework GLUT -framework OpenGL
#  Linux/Unix/Solaris
else
CFLG=-O3 -Wall
LIBS=-lglut -lGLU -lGL -lm
endif
#  OSX/Linux/Unix/Solaris
CLEAN=rm -f $(EXE) *.o *.a
endif

# Dependencies
foggymountains.o: foggymountains.cpp customLib.h
fatal.o: fatal.c customLib.h
loadtexbmp.o: loadtexbmp.c customLib.h
print.o: print.c customLib.h
project.o: project.c customLib.h
errcheck.o: errcheck.c customLib.h
object.o: object.c customLib.h
camera.o: camera.c camera.h

#  Create archive
customLib.a:fatal.o loadtexbmp.o print.o project.o errcheck.o object.o
	ar -rcs $@ $^

# Compile rules
.c.o:
	gcc -c $(CFLG) $<
.cpp.o:
	#g++ -std=c++14 -c $(CFLG) $< #Will not compile on Ubuntu
	g++ -std=c++11 -c $(CFLG) $<

#  Link
foggymountains:foggymountains.o customLib.a
	g++ -O3 -o $@ $^   $(LIBS)

#  Clean
clean:
	$(CLEAN)
