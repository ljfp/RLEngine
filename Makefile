###########################################################
# Makefile variables
###########################################################
CXX = g++
COMPILER_FLAGS = -Wall -std=c++2b -O2 -ggdb
LINKER_FLAGS = -L"./lib/" -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -llua54
INCLUDE_FLAGS = -I"./third_party/"
SRC = ./src/*.cpp \
	./src/AssetManager/*.cpp \
	./src/ECS/*.cpp \
	./src/Game/*.cpp \
	./third_party/imgui/*.cpp
BIN = bin/
EXECUTABLE = $(BIN)RoguelikeEngine

###########################################################
# Makefile rules
###########################################################
.PHONY: all build clean run

# Default target
all: clean build run

# Rule to build the executable
build:
	$(CXX) $(COMPILER_FLAGS) $(INCLUDE_FLAGS) $(SRC) -o $(EXECUTABLE) $(LINKER_FLAGS)

# Rule to clean the build
clean:
	rm -f $(BIN)*

# Rule to run the executable
run:
	./$(EXECUTABLE)