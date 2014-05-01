CC=g++
CFLAGS=-g -c -DGL_GLEXT_PROTOTYPES -lGL -lGLU -lGLEW -lglfw -lassimp -lIL
LDFLAGS=-ggdb -DGL_GLEXT_PROTOTYPES -lGL -lGLU -lGLEW -lglfw -lassimp -lIL
SOURCES=Rendering/GLSL_helper.cpp Modeling/CMeshLoaderSimple.cpp GameObject.cpp GameModel.cpp Game.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=smorgasfjord

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) -g $(LDFLAGS) $(OBJECTS) -o $@

%.o: %.cpp
	$(CC) -g $(CFLAGS) $< -o $@

clean:
	-rm -rf *.o $(EXECUTABLE)

