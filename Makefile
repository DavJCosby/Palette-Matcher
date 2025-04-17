
BUILD_DIR = ./build

OBJS = $(BUILD_DIR)/glad.o $(BUILD_DIR)/rendering.o $(BUILD_DIR)/ui.o $(BUILD_DIR)/spotlight.o $(BUILD_DIR)/scene.o $(BUILD_DIR)/mesh.o $(BUILD_DIR)/lodepng.o $(BUILD_DIR)/pixelartfx.o $(BUILD_DIR)/paletteparser.o
EXECUTABLE_NAME = App.exe

CC = g++
INCLUDE_PATHS = -I./include -I/usr/local/include/ -I/opt/homebrew/opt/glfw/include
LIBRARY_PATHS = -L/usr/local/lib/ -L/opt/homebrew/opt/glfw/lib
COMPILER_FLAGS = -Wall -Wextra -Wdeprecated-declarations -Wno-unused-parameter -fsanitize=address -std=c++23 -g
LINKER_FLAGS = -L/opt/homebrew/opt/glfw/lib -lglfw -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo

FULL_CC = $(COMPILER_FLAGS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(LINKER_FLAGS)


app : ./src/main.cpp $(OBJS) fmt
	$(CC) ./src/main.cpp $(FULL_CC) -o $(EXECUTABLE_NAME) $(OBJS)

$(BUILD_DIR)/lodepng.o: ./src/lodepng.cpp
	$(CC) ./src/lodepng.cpp $(COMPILER_FLAGS) $(INCLUDE_PATHS) -c -o $(BUILD_DIR)/lodepng.o

$(BUILD_DIR)/glad.o: ./src/glad.c
	$(CC) ./src/glad.c $(COMPILER_FLAGS) $(INCLUDE_PATHS) -c -o $(BUILD_DIR)/glad.o

$(BUILD_DIR)/ui.o: ./src/ui.cpp
	$(CC) ./src/ui.cpp $(FULL_CC) -c -o $(BUILD_DIR)/ui.o

$(BUILD_DIR)/scene.o: ./src/scene.cpp
	$(CC) ./src/scene.cpp $(FULL_CC) -c -o $(BUILD_DIR)/scene.o

$(BUILD_DIR)/mesh.o: ./src/mesh.cpp
	$(CC) ./src/mesh.cpp $(FULL_CC) -c -o $(BUILD_DIR)/mesh.o

$(BUILD_DIR)/spotlight.o: ./src/spotlight.cpp
	$(CC) ./src/spotlight.cpp $(FULL_CC) -c -o $(BUILD_DIR)/spotlight.o

$(BUILD_DIR)/rendering.o: ./src/rendering.cpp
	$(CC) ./src/rendering.cpp $(FULL_CC) -c -o $(BUILD_DIR)/rendering.o

$(BUILD_DIR)/pixelartfx.o: ./src/pixelartfx.cpp
	$(CC) ./src/pixelartfx.cpp $(FULL_CC) -c -o $(BUILD_DIR)/pixelartfx.o

$(BUILD_DIR)/paletteparser.o: ./src/paletteparser.cpp
	$(CC) ./src/paletteparser.cpp $(FULL_CC) -c -o $(BUILD_DIR)/paletteparser.o

fmt:
	clang-format -i ./src/*.cpp ./include/internal/*
