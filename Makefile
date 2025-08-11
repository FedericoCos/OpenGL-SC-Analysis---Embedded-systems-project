CC = g++
CFLAGS = -Wall -Wextra -std=c++17
LDFLAGS = -ldl -lglfw -lGL -lX11 -lpthread -lassimp

# ImGui sources and backends
IMGUI_DIR = imgui
IMGUI_BACKENDS = $(IMGUI_DIR)/backends
IMGUI_SRC = \
    $(IMGUI_DIR)/imgui.cpp \
    $(IMGUI_DIR)/imgui_draw.cpp \
    $(IMGUI_DIR)/imgui_tables.cpp \
    $(IMGUI_DIR)/imgui_widgets.cpp \
    $(IMGUI_BACKENDS)/imgui_impl_glfw.cpp \
    $(IMGUI_BACKENDS)/imgui_impl_opengl3.cpp

# Your project sources
SRC_FILES = $(wildcard *.cpp)
GLAD_FILE = glad.c
TARGET = output

# All sources together
ALL_SRC = $(SRC_FILES) $(GLAD_FILE) $(IMGUI_SRC)

# Include paths
INCLUDES = -I$(IMGUI_DIR) -I$(IMGUI_BACKENDS)

all: $(TARGET)

$(TARGET): $(ALL_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)

run: $(TARGET)
	./$(TARGET) 

clean:
	rm -f $(TARGET)

.PHONY: all run clean
