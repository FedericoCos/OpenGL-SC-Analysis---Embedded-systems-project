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
    glfwSetFramebufferSizeCallback(window, Engine::framebuffer_size_callback);


    stbi_set_flip_vertically_on_load(true);

    init_VAO();
    init_textures();
    init_buffers();
    init_shaders();

    return 0;
}

void Engine::process_input(){
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }
    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
        mix_val = std::min(mix_val+0.1f, 1.0f);
    }
    if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
        mix_val = std::max(mix_val-0.1f, 0.0f);
    }

    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        cam -> position += cam -> vel * cam -> front;
    }
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        cam -> position -= cam -> vel * cam -> front;
    }
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
        cam -> position -= glm::normalize(glm::cross(cam ->front, glm::vec3(0.0f, 1.0f, 0.0f))) * cam -> vel;
    }
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
        cam -> position += glm::normalize(glm::cross(cam ->front, glm::vec3(0.0f, 1.0f, 0.0f))) * cam -> vel;
    }
    
}

void Engine::init_VAO(){
    // Genrating VAO
    // glGenVertexArrays(NUM, VAO);
    glGenVertexArrays(1, &cVAO);
}

void Engine::init_buffers(){
    // Vertex buffer Object
    /*
    glGenBuffers(NUM, VBO); 
    glGenBuffers(NUM, EBO);

    for(int i = 0; i < NUM; i++){
        glBindVertexArray(VAO[i]);

        glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[i]), vertices[i], GL_STATIC_DRAW); // copies vertices on the GPU (buffer memory)

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[i]), indices[i], GL_STATIC_DRAW);

        /**
         * We have to tell how to read the vertices
         * Arguments in order
         * position (as in layout = 0)
         * number of values per vertex
         * type of data
         * whether normalized or not
         * distance between consecutive vertices
         * offset of first vertex from buffer init position
         *//*
        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // color attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3* sizeof(float)));
        glEnableVertexAttribArray(1); 
        // texture attribute
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6* sizeof(float)));
        glEnableVertexAttribArray(2); 
    }
    glBindVertexArray(0); // Good practice
    */

    glGenBuffers(1, &cVBO);
    glGenBuffers(1, &cEBO);

    glBindVertexArray(cVAO);

    glBindBuffer(GL_ARRAY_BUFFER, cVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW); // copies vertices on the GPU (buffer memory)

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3* sizeof(float)));
    glEnableVertexAttribArray(1); 
    // texture attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6* sizeof(float)));
    glEnableVertexAttribArray(2); 

    glBindVertexArray(0);

}

void Engine::init_shaders(){
    shaders[0] = new Shader("shaders/vertex.glsl", "shaders/fragment.glsl");
    shaders[1] = new Shader("shaders/vertex.glsl", "shaders/fragment_yellow.glsl");

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
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glEnable(GL_DEPTH_TEST);

    for(int i = 0; i < CUBES; i++){
        tr[i] = std::max((rand() % 50 * 0.1f), 0.5f) * glm::normalize(glm::vec3(rand() % 10 - 5, rand() % 10 - 5, rand() % 10 - 5));
        sc[i] = glm::vec3(std::max(rand() % 10 * 0.1f, 0.1f));
        rot[i] = glm::normalize(glm::vec3(std::min(rand() % 10 * 0.1f, 0.1f), std::min(rand() % 10 * 0.1f, 0.1f), std::min(rand() % 10 * 0.1f, 0.1f)));
    }

    glm::vec3 pos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 tar = glm::vec3(0.0f, 0.0f, -1.0f);
    cam = new Camera(pos, tar, 0.05f);

    projection = glm::perspective(glm::radians(45.f), 800.f/600.f, 0.1f, 100.f);

    while(!glfwWindowShouldClose(window))
    {
        process_input();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        draw();

        glfwSwapBuffers(window);
        glfwPollEvents(); 
    }

    glDeleteVertexArrays(1, &cVAO);
    glDeleteBuffers(1, &cVBO);
    glDeleteBuffers(1, &cEBO);
    //glDeleteProgram(shaderProgram);

    glfwTerminate();
}

void Engine::draw(){
    /* for(int i = 0; i < NUM; i++){
        shaders[i] -> use();
        glBindVertexArray(VAO[i]);
        if(i == 0){
            glUniform1i(glGetUniformLocation(shaders[i]->ID, "texture1"), 0);
            shaders[i] -> setInt("texture2", 1);
            shaders[i] -> setFloat("mix_val", mix_val);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture[0]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, texture[1]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        }

        trans[i] = glm::mat4(1.0f);
        trans[i] = glm::rotate(trans[i], (float)glfwGetTime(), glm::vec3(1.0f, 0.0f, 1.0f));
        trans[i] = glm::translate(trans[i], glm::vec3(0.5f, -0.5f * (i > 0 ? -1 : 1), 0.0f));
        shaders[i] -> setMatrix("transform", trans[i]);

        // glDrawArrays(GL_TRIANGLES, 0, 3);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    } */

    shaders[0] -> use();
    glBindVertexArray(cVAO);
    glUniform1i(glGetUniformLocation(shaders[0]->ID, "texture1"), 0);
    shaders[0] -> setInt("texture2", 1);
    shaders[0] -> setFloat("mix_val", mix_val);
    // bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

    for(int i = 0; i < CUBES; i++){

        trans[i] = glm::mat4(1.0f);
        trans[i] = glm::scale(trans[i], sc[i]);
        trans[i] = glm::translate(trans[i], tr[i]);
        trans[i] = glm::rotate(trans[i], (float)glfwGetTime(), rot[i]);

        trans[i] = cam -> viewAtMat() * trans[i];
        trans[i] = projection * trans[i];

        shaders[0] -> setMatrix("transform", trans[i]);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    }
    glBindVertexArray(0);

}


int main(){
    Engine engine;
    if(engine.init()){
        return -1;
    }

    engine.render_loop();

    return 0;
}



