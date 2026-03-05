# ======================================================
# Makefile cross-platform (Linux/macOS/Windows)
# ======================================================
CC = gcc
CFLAGS = -Wall -O2 -I. 


SRC = main.c lib/main_lib.c lib/flood_core.c
OBJ = $(patsubst %.c,build/%.o,$(SRC))

ifeq ($(OS),Windows_NT)
    TARGET = bin/synfld.exe
    MKDIR_BUILD = if not exist build mkdir build & if not exist build\lib mkdir build\lib
    MKDIR_BIN = if not exist bin mkdir bin
else
    TARGET = bin/synfld
    MKDIR_BUILD = mkdir -p build/lib
    MKDIR_BIN = mkdir -p bin
endif

all: folders $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

build/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

folders:
	$(MKDIR_BUILD)
	$(MKDIR_BIN)

clean:
ifeq ($(OS),Windows_NT)
	@if exist build (rd /s /q build)
	@if exist bin (rd /s /q bin)
	@if exist generated (rd /s /q generated)
else
	@rm -rf build bin generated
endif

re: clean all

install: all
ifeq ($(OS),Windows_NT)
	@echo "Manual installation: Copy $(TARGET) wherever you want"
else
	sudo cp $(TARGET) /usr/local/bin/
	@echo "Globally installed. Now you can run 'synfld'"
endif