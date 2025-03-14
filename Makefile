CC = g++
CFLAGS = -Wall -Wextra -std=c++17
LDFLAGS = -ldl -lglfw

SRC_FILES = $(wildcard *.cpp)
GLAD_FILE = glad.c
TARGET = output

all: $(TARGET)

$(TARGET): $(SRC_FILES) $(GLAD_FILE)
	$(CC) $^ -o $@ $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean