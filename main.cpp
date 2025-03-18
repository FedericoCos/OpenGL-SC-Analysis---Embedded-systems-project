#include "main.h"
#include "math.h"

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

    init_VAO();
    init_buffers();
    Shader tempShade("shaders/vertex.glsl", "shaders/fragment.glsl");
    mainShader = &tempShade;
    init_shaders();

    return 0;
}

void Engine::process_input(){
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }
}

void Engine::init_VAO(){
    // Genrating VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
}

void Engine::init_buffers(){
    // Vertex buffer Object
    glGenBuffers(1, &VBO); // first parameter is ID
    glBindBuffer(GL_ARRAY_BUFFER, VBO);


    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // copies vertices on the GPU (buffer memory)

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

void Engine::init_shaders(){
    

    /**
     * We have to tell how to read the vertices
     * Arguments in order
     * position (as in layout = 0)
     * number of values per vertex
     * type of data
     * whether normalized or not
     * distance between consecutive vertices
     * offset of first vertex from buffer init position
     */
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3* sizeof(float)));
    glEnableVertexAttribArray(1); 

}

void Engine::render_loop(){

    while(!glfwWindowShouldClose(window))
    {
        process_input();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        draw();

        glfwSwapBuffers(window);
        glfwPollEvents(); 
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    //glDeleteProgram(shaderProgram);

    glfwTerminate();
}

void Engine::draw(){
    mainShader -> use();

    /* float timeValue = glfwGetTime();
    float greenValue = sin(timeValue) / 2.f + .5f;
    int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
    glUniform4f(vertexColorLocation, .0f, greenValue, .0f, 1.f); */

    glBindVertexArray(VAO);
    // glDrawArrays(GL_TRIANGLES, 0, 3);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}


int main(){
    Engine engine;
    if(engine.init()){
        return -1;
    }

    engine.render_loop();

    return 0;
}



