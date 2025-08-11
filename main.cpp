#include "main.h"
#include "math.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void Engine::framebuffer_size_callback(GLFWwindow* window, int width, int height){
    glViewport(0, 0, width, height);
}

int Engine::init(int cubes, bool imgui, bool save){

    // GLFW window initialization
    glfwInit();
    // First specify version of OpenGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // first argument is what we want to configure
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
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT); // lower-left corner of the window, and dimension
    width = WIN_WIDTH;
    height = WIN_HEIGHT;
    glfwSetFramebufferSizeCallback(window, Engine::framebuffer_size_callback);


    tracker.init(save);

    init_shaders();
    init_VAO();
    init_textures();

    // COMPUTE SETUP: create an RGBA32F texture and bind it to image unit 0
    glGenTextures(1, &compTex);
    glBindTexture(GL_TEXTURE_2D, compTex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, WIN_WIDTH, WIN_HEIGHT);
    tracker.trackVramAllocation( size_t(WIN_WIDTH) * size_t(WIN_HEIGHT) * 16 );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindImageTexture(0, compTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    // compile & link Menger‐sponge compute shader
    mengerCS = new ComputeShader("shaders/compute_advanced_menger.comp");

    std::cout << "GL Version: " << glGetString(GL_VERSION) << "\n";

    return 0;
}

void Engine::process_input(){

    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }    
}

void Engine::init_VAO(){
    // fullscreen quad
    float quadVerts[] = {
    // pos      // uv
    -1,-1,     0,0,
    1,-1,     1,0,
    -1, 1,     0,1,
    1, 1,     1,1,
    };
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);
    tracker.trackVramAllocation(sizeof(quadVerts));
    tracker.trackDataUpload(sizeof(quadVerts));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,4*sizeof(float),(void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,4*sizeof(float),(void*)(2*sizeof(float)));
    glBindVertexArray(0);
}

void Engine::init_shaders(){
    // shader initialization
    screen_shader = Shader("shaders/screen.vert", "shaders/screen.frag");
}

void Engine::init_textures(){
    // load wall.jpg for compute-shader sampling
    int w, h, n;
    unsigned char* wallData = stbi_load("textures/wall.jpg", &w, &h, &n, 0);
    if(!wallData){
        std::cerr << "Failed to load textures/wall.jpg\n";
        exit(1);
    }
    glGenTextures(1, &wallTex);
    glBindTexture(GL_TEXTURE_2D, wallTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);
    GLenum fmt = (n == 4 ? GL_RGBA : GL_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, wallData);
    int bpp = (fmt == GL_RGBA ? 4 : 3);
    wallW = w; wallH = h; wallBpp = bpp;
    tracker.trackVramAllocation( size_t(w) * size_t(h) * bpp );
    tracker.trackDataUpload(    size_t(w) * size_t(h) * bpp );
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(wallData);
}

void Engine::render_loop(){

    while(!glfwWindowShouldClose(window))
    {   
        tracker.beginFrame();

        float t = (float)glfwGetTime();
        dtime = t - past_time;
        past_time = t;
        process_input();
        tracker.beginCpuRender();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT);

        // bind wall texture to image unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, wallTex);
        tracker.countTextureBind();

        // COMPUTE DISPATCH
        mengerCS->use();
        tracker.countShaderBind();
        glUniform1i(glGetUniformLocation(mengerCS->ID, "iChannel0"), 0);
        glUniform1f(glGetUniformLocation(mengerCS->ID, "iTime"),  glfwGetTime());
        glUniform2f(glGetUniformLocation(mengerCS->ID, "iResolution"),
                    float(WIN_WIDTH), float(WIN_HEIGHT));
        mengerCS->dispatch( (WIN_WIDTH+15)/16, (WIN_HEIGHT+15)/16, 1 );
        mengerCS->barrier();

        screen_shader.use();
        tracker.countShaderBind();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, compTex);
        tracker.countTextureBind();
        glUniform1i(glGetUniformLocation(screen_shader.ID, "tex"), 0);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        tracker.countDrawCall();
        tracker.countTriangles(2);

        tracker.endCpuRender();

        glfwSwapBuffers(window);
        glfwPollEvents(); 

        tracker.endFrame();
        tracker.printStats();
    }

    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    tracker.trackVramDeallocation(sizeof(float) * 4 * 4);

    glDeleteTextures(1, &compTex);
    tracker.trackVramDeallocation( size_t(WIN_WIDTH) * size_t(WIN_HEIGHT) * 16 );

    glDeleteTextures(1, &wallTex);
    tracker.trackVramDeallocation(size_t(wallW) * size_t(wallH) * wallBpp);

    glfwTerminate();
}

int main(int argc, char * argv[]){
    if(argc < 4){
        std::cerr << "Not enough parameter passed. You must give, in order, num of cubes, whether to draw imgui and whether to save stats" << std::endl;
        return -1;
    }

    Engine engine;
    int c = std::atoi(argv[1]);
    if(engine.init(c, strcmp(argv[2], "true") == 0, strcmp(argv[3], "true") == 0)){
        return -1;
    }

    engine.render_loop();

    return 0;
}



