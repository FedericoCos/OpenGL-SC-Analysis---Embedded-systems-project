#include "main.h"

void Engine::framebuffer_size_callback(GLFWwindow* window, int width, int heihgt){
    glViewport(0, 0, width, heihgt);
}

int Engine::init(){
    // GLFW initialization
    glfwInit();
    // First specify version of OpenGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // first argument is what we want to configure
                                                   // second is the value for that option
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "OpenGL", NULL, NULL);
    if(window == NULL){
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // GLAD initialization
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT); // lower-left corner of the window, and dimension
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    return 0;
}

void Engine::process_input(){
    if(glfwGetKey(window, GLFW_KEY_ESCAPE == GLFW_PRESS)){
        glfwSetWindowShouldClose(window, true);
    }
}

void Engine::render_loop(){

    while(!glfwWindowShouldClose(window))
    {
        process_input();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();    
    }


    glfwTerminate();
}


int main(){
    Engine engine;
    if(engine.init()){
        return -1;
    }

    engine.render_loop();

    return 0;
}



