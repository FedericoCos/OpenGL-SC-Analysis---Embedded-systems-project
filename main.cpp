#include "main.h"
#include "math.h"

void Engine::framebuffer_size_callback(GLFWwindow* window, int width, int height){
    glViewport(0, 0, width, height);
}

int Engine::init(int cubes, bool imgui, bool save){
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

    tr.resize(CUBES);
    sc.resize(CUBES);
    rot.resize(CUBES);
    trans.resize(CUBES);

    cubes_tot = cubes;
    num_lights = 10;

    // Random variables for cubes
    for(int i = 0; i < CUBES; i++){
        tr[i] = std::max((rand() % 1000 * 0.1f), 0.5f) * glm::normalize(glm::vec3(rand() % 10 - 5, rand() % 10 - 5, rand() % 10 - 5));
        sc[i] = glm::vec3(std::max(rand() % 10 * 0.1f, 0.1f));
        rot[i] = glm::normalize(glm::vec3(std::min((rand() % 10 - 5) * 0.1f, 0.1f), std::min((rand() % 10 - 5) * 0.1f, 0.1f), std::min((rand() % 10 - 5) * 0.1f, 0.1f)));
    }

    init_shaders();
    init_VAO();
    init_buffers();
    init_textures();

    // Camera initialization
    glm::vec3 pos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 tar = glm::vec3(0.0f, 0.0f, -1.0f);
    cam = new Camera(pos, tar, 30.0f, 2.5f);

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
    // Genrating VAO
    // glGenVertexArrays(NUM, VAO);
    glGenVertexArrays(1, &cVAO);
    glGenVertexArrays(1, &lightVAO);
}

void Engine::init_buffers(){
    glGenBuffers(1, &cVBO);
    glGenBuffers(1, &cEBO);

    glBindVertexArray(cVAO);

    glBindBuffer(GL_ARRAY_BUFFER, cVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW); // copies vertices on the GPU (buffer memory)
    tracker.trackVramAllocation(sizeof(cube));

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);
    tracker.trackVramAllocation(sizeof(cube_indices));



    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cEBO);

    glBindVertexArray(0);

}

void Engine::init_shaders(){
    shader = Shader("shaders/vertex.glsl", "shaders/fragment.glsl");
    light_shader = Shader("shaders/vertex.glsl", "shaders/fragment_light.glsl");
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
    data = stbi_load("textures/awesomeface.png", &width, &height, &nrChannels, 0);
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

    // ImGui
    if(is_imgui){
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
    }

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

        if(is_imgui){
            draw_imgui();
        }

        glfwSwapBuffers(window);
        glfwPollEvents(); 

        tracker.endFrame();
        //tracker.printStats();
    }

    // Deletes all ImGUI instances
    if(is_imgui){
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    glDeleteVertexArrays(1, &cVAO);
    glDeleteBuffers(1, &cVBO);
    glDeleteBuffers(1, &cEBO);
    //glDeleteProgram(shaderProgram);

    glfwTerminate();
}

void Engine::draw(){
    tracker.beginCpuRender();

    glm::mat4 view = cam -> viewAtMat();

    light_shader.use();
    tracker.countShaderBind();
    glBindVertexArray(lightVAO);
    light_shader.setMatrix("view", view);
    light_shader.setMatrix("projection", projection);

    int i;
    for (i = 0; i < num_lights; i++) {
        trans[i] = glm::mat4(1.0f);
        trans[i] = glm::rotate(trans[i], rot_speed * (float)glfwGetTime(), rot[i]);
        trans[i] = glm::scale(trans[i], sc[i]);
        trans[i] = glm::translate(trans[i], spread * tr[i]);
        trans[i] = glm::rotate(trans[i], rot_speed * (float)glfwGetTime(), rot[i]);

        light_shader.setMatrix("model", trans[i]);

        tracker.countDrawCall();
        tracker.countTriangles(12 * cubes_tot);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        
    }

    shader.use();
    tracker.countShaderBind();
    glBindVertexArray(cVAO);
    shader.setInt("material.diffuse", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    tracker.countTextureBind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);


    /* glUniform1i(glGetUniformLocation(shader.ID, "texture1"), 0);
    shader.setInt("texture2", 1);
    shader.setFloat("mix_val", mix_val); */
    shader.setMatrix("view", view);
    shader.setMatrix("projection", projection);
    shader.setVector3("viewPos", cam -> position);

    glm::vec3 objColor(1.f, .5f, .31f);
    glm::vec3 lightColor(1.f, 1.f, 1.f);
    shader.setVector3("objectColor", objColor);

    /* shader.setFloat("time", (float)glfwGetTime());
    shader.setFloat("rotSpeed", rot_speed);
    
    // bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    tracker.countTextureBind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    tracker.countTextureBind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); */

    shader.setInt("numLights", num_lights);
    glm::vec3 matAmbient(1.f, .5f, .31f);
    glm::vec3 matDiffuse(1.f, .5f, .31f);
    glm::vec3 matSpecular(.5f, .5f, .5f);
    shader.setVector3("material.ambient", matAmbient);
    shader.setVector3("material.diffuse", matDiffuse);
    shader.setVector3("material.specular", matSpecular);
    shader.setFloat("material.shininess", 32.0f);

    glm::vec3 ambientLight(.2f, .2f, .2f);
    glm::vec3 diffuseLight(.5f, .5f, .5f);
    glm::vec3 specularLight(1.f, 1.f, 1.f);


    for (i = 0; i < num_lights; i++) {
        glm::vec3 position = trans[i] * glm::vec4(0.f, 0.f, 0.f, 1.f);
        shader.setVector3("lights[" + std::to_string(i) + "].position", position);
        shader.setVector3("lights[" + std::to_string(i) + "].ambient", ambientLight);
        shader.setVector3("lights[" + std::to_string(i) + "].diffuse", diffuseLight);
        shader.setVector3("lights[" + std::to_string(i) + "].specular", specularLight);
    }

    for (;i < cubes_tot; i++) {
        trans[i] = glm::mat4(1.0f);
        trans[i] = glm::scale(trans[i], sc[i]);
        trans[i] = glm::translate(trans[i], spread * tr[i]);
        trans[i] = glm::rotate(trans[i], rot_speed * (float)glfwGetTime(), rot[i]);

        shader.setMatrix("model", trans[i]);

        tracker.countDrawCall();
        tracker.countTriangles(12 * cubes_tot);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        
    }

    glBindVertexArray(0);

    tracker.endCpuRender();

}

void Engine::draw_imgui(){
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("STATS");
    ImGui::Text("FPS: %.1f", 1.0f / dtime);
    ImGui::End();
    
    ImGui::Begin("CUBES");
    ImGui::InputInt("Num Cubes", &cubes_tot);
    cubes_tot = std::min(CUBES, cubes_tot);
    ImGui::InputInt("Num Lights", &num_lights);
    num_lights = std::min(num_lights, cubes_tot);
    ImGui::InputFloat("Spread fact", &spread);
    ImGui::InputFloat("Rot Speed", &rot_speed);
    ImGui::End();

    ImGui::Begin("CAMERA");
    int w = width, h = height;
    float np = near_plane, fp = far_plane, f = fov;
    ImGui::InputFloat("FOV", &f);
    ImGui::InputFloat("Near Plane", &np);
    ImGui::InputFloat("Far Plane", &fp);
    glfwGetWindowSize(window, &w, &h);
    if(
        w != width ||
        h != height ||
        np != near_plane ||
        fp != far_plane ||
        f != fov
    ){
        width = w;
        height = h;
        near_plane = np;
        far_plane = fp;
        fov = f;
        projection = glm::perspective(glm::radians(fov), width * 1.f/height, near_plane, far_plane);
    }


    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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



