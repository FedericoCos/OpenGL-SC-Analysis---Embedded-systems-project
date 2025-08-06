#include "main.h"
#include "math.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void Engine::framebuffer_size_callback(GLFWwindow* window, int width, int height){
    glViewport(0, 0, width, height);
}

int Engine::init(int cubes, bool imgui, bool save){
    // INIZIALIZZAZIONE FINESTRA

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
    width = WIN_WIDTH;
    height = WIN_HEIGHT;
    glfwSetFramebufferSizeCallback(window, Engine::framebuffer_size_callback);


    stbi_set_flip_vertically_on_load(true);
    tracker.init(save);
    is_imgui = imgui;

    init_shaders();
    init_VAO();
    init_buffers();
    init_textures();

    // Camera initialization
    glm::vec3 pos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 tar = glm::vec3(0.0f, 0.0f, -1.0f);
    cam = new Camera(pos, tar, 30.0f, 2.5f);

    // Loading models
    backpack_model = Model("/home/zancanonzanca/Desktop/OpenGL-SC-Analysis---Embedded-systems-project/resources/backpack/backpack.obj");


    // Setting lights
    light_positions.resize(num_lights);
    light_color.resize(num_lights);
    for(int i = 0; i < num_lights; i++){
        glm::vec3 dir = glm::sphericalRand(1.0f); 

    // Random distance between min and max
    float dist = glm::linearRand(min_light_dis, max_light_dis);

    // Final position (random direction * random distance)
    light_positions[i] = dir * dist;

    // Random bright color (each component between 0.5 and 1.0)
    light_color[i] = glm::vec3(
            glm::linearRand(0.5f, 1.0f),
            glm::linearRand(0.5f, 1.0f),
            glm::linearRand(0.5f, 1.0f)
        );
    }

    // To disable vsync
    glfwSwapInterval(0);

    return 0;
}

void Engine::process_input(){
    // Refreshing the input
    right_input.x = 0;
    right_input.y = 0;
    left_input.x = 0;
    left_input.y = 0;

    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }

    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        right_input.x = 1;
    }
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        right_input.x = -1;
    }
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
        right_input.y = -1;
    }
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
        right_input.y = 1;
    }

    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
        left_input.x = 1;
    }
    if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
        left_input.x = -1;
    }
    if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS){
        left_input.y = -1;
    }
    if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){
        left_input.y = 1;
    }
    
}

void Engine::init_VAO(){
    glGenVertexArrays(1, &wallVAO);
    glGenVertexArrays(1, &lightVAO);
}

void Engine::init_buffers(){    
    glGenBuffers(1, &wallVBO);
    glGenBuffers(1, &wallEBO);

    // guarda qui, bindi il VAO per dirgli "le prossime cose che definisco, mettile qui"
    glBindVertexArray(wallVAO);

    // Il VBO sono effettivamente i vertici, normali e company
    glBindBuffer(GL_ARRAY_BUFFER, wallVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(wall_vertices), wall_vertices, GL_STATIC_DRAW); // copies vertices on the GPU (buffer memory)
    tracker.trackVramAllocation(sizeof(wall_vertices));

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // GLI EBO sono gli indici, perchè un vertice può fare parte di più triangoli
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wallEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(wall_indices), wall_indices, GL_STATIC_DRAW);
    tracker.trackVramAllocation(sizeof(wall_indices));



    glGenBuffers(1, &lightVBO);
    glGenBuffers(1, &lightEBO);

    glBindVertexArray(lightVAO);

    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lightEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);


    glBindVertexArray(0);

}

void Engine::init_shaders(){
    // CREAZIONE SHADER
    backpack_shader = Shader("shaders/vertex_model.glsl", "shaders/fragment_model.glsl");
    wall_shader = Shader("shaders/vertex_wall.glsl", "shaders/fragment_wall.glsl");
    light_shader = Shader("shaders/vertex_light.glsl", "shaders/fragment_light.glsl");
}

void Engine::init_textures(){
    int width, height, nrChannels;
    unsigned char *data = stbi_load("textures/container2.png", &width, &height, &nrChannels, 0); 
    if(!data){
        std::cout << "Failed to load texture" << std::endl;
        exit(0);
    }

    glGenTextures(2, texture);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    tracker.countTextureBind();
    tracker.trackVramAllocation(width * height * 3);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    

    glBindTexture(GL_TEXTURE_2D, texture[1]);
    tracker.countTextureBind();
    data = stbi_load("textures/container2_specular.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        tracker.trackVramAllocation(width * height * 3);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
}

void Engine::render_loop(){
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glEnable(GL_DEPTH_TEST);

    projection = glm::perspective(glm::radians(fov), width * 1.f/height, near_plane, far_plane);

    while(!glfwWindowShouldClose(window))
    {   
        tracker.beginFrame();

        float t = (float)glfwGetTime();
        dtime = t - past_time;
        past_time = t;
        process_input();
        cam -> update(right_input, left_input, dtime);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        draw();

        glfwSwapBuffers(window);
        glfwPollEvents(); 

        tracker.endFrame();
        //tracker.printStats();
    }

    glfwTerminate();
}

void Engine::draw(){
    tracker.beginCpuRender();

    glm::mat4 view = cam -> viewAtMat();

    glBindVertexArray(wallVAO);
    wall_shader.use();

    wall_shader.setMatrix("view", view);
    wall_shader.setMatrix("projection", projection);

    glm::vec3 pos (.0f, 0.f, 0.f);
    wall_shader.setInt("numLights", num_lights);
    wall_shader.setVector3("viewPos", cam -> position);
    for(int i = 0; i < num_lights; i++){
        wall_shader.setVector3("lights[" + std::to_string(i) + "].position", light_positions[i]);
        wall_shader.setVector3("lights[" + std::to_string(i) + "].ambient", light_color[i]);
        wall_shader.setVector3("lights[" + std::to_string(i) + "].diffuse", light_color[i]);
        wall_shader.setVector3("lights[" + std::to_string(i) + "].specular", light_color[i]);
    }

    glm::vec3 ambientLight(.2f, .2f, .2f);
    glm::vec3 diffuseLight(.5f, .5f, .5f);
    glm::vec3 specularLight(1.f, 1.f, 1.f);
    
    glm::vec3 direction(-1.f, -1.f, 0.f);

    wall_shader.setVector3("directionalLight.direction", direction);
    wall_shader.setVector3("directionalLight.ambient", ambientLight);
    wall_shader.setVector3("directionalLight.diffuse", diffuseLight);
    wall_shader.setVector3("directionalLight.specular", specularLight);

    for(int i = 0; i < 6; i++){
        glm::mat4 model(1.0f);
        model = glm::translate(model, wall_positions[i]);
        model = glm::scale(model, wall_scale);
        model = glm::rotate(model, glm::radians(wall_rotation[i].w), glm::vec3(wall_rotation[i]));

        wall_shader.setMatrix("model", model);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(lightVAO);
    light_shader.use();

    light_shader.setMatrix("view", view);
    light_shader.setMatrix("projection", projection);

    for(int i = 0; i < num_lights; i++){
        light_shader.setVector3("color", light_color[i]);

        glm::mat4 model(1.f);
        model = glm::translate(model, light_positions[i]);
        model = glm::scale(model, light_scale);

        light_shader.setMatrix("model", model);

        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);

    tracker.endCpuRender();

}

void Engine::draw_imgui(){

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



