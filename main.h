#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <iostream>

#define WIN_WIDTH 800
#define WIN_HEIGHT 600


class Engine{
public:
    
    int init();
    void render_loop();

private:
    GLFWwindow * window;

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    void process_input();
};