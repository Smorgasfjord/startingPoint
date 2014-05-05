CC=g++
CFLAGS=-g -c -DGL_GLEXT_PROTOTYPES -lGL -lGLU -lGLEW -lglfw -lassimp -lIL
LDFLAGS=-g -DGL_GLEXT_PROTOTYPES -lGL -lGLU -lGLEW -lglfw -lassimp -lIL
SOURCES=Rendering/GLSL_helper.cpp Modeling/CMeshLoaderSimple.cpp GameObject.cpp GameModel.cpp oldTextureLoader.cpp Game.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=smorgasfjord

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm -rf *.o $(EXECUTABLE)

