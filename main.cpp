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
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // load open gl functions with no back compatibility

    window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "OpenGL", NULL, NULL);
    if(window == NULL){
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // GLAD initialization
    /**
     * GLAD contains alla the fucntions not available in system OpenGL Library
     */
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT); // lower-left corner of the window, and dimension
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


    stbi_set_flip_vertically_on_load(true);

    init_VAO();
    init_textures();
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3* sizeof(float)));
    glEnableVertexAttribArray(1); 
    // texture attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2); 

}

void Engine::init_textures(){
    int width, height, nrChannels;
    unsigned char *data = stbi_load("textures/wall.jpg", &width, &height, &nrChannels, 0); 
    if(!data){
        std::cout << "Failed to load texture" << std::endl;
        exit(0);
    }

    glGenTextures(2, texture);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    /**
     * 
    The first argument specifies the texture target; setting this to GL_TEXTURE_2D means this operation will generate a texture on the currently bound texture object at the same target (so any textures bound to targets GL_TEXTURE_1D or GL_TEXTURE_3D will not be affected).
    The second argument specifies the mipmap level for which we want to create a texture for if you want to set each mipmap level manually, but we'll leave it at the base level which is 0.
    The third argument tells OpenGL in what kind of format we want to store the texture. Our image has only RGB values so we'll store the texture with RGB values as well.
    The 4th and 5th argument sets the width and height of the resulting texture. We stored those earlier when loading the image so we'll use the corresponding variables.
    The next argument should always be 0 (some legacy stuff).
    The 7th and 8th argument specify the format and datatype of the source image. We loaded the image with RGB values and stored them as chars (bytes) so we'll pass in the corresponding values.
    The last argument is the actual image data.
     */
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    

    glBindTexture(GL_TEXTURE_2D, texture[1]);
    data = stbi_load("textures/awesomeface.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
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
    glUniform1i(glGetUniformLocation(mainShader->ID, "texture1"), 0);
    mainShader -> setInt("texture2", 1);

    /* float timeValue = glfwGetTime();
    float greenValue = sin(timeValue) / 2.f + .5f;
    int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
    glUniform4f(vertexColorLocation, .0f, greenValue, .0f, 1.f); */

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture[1]);
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



