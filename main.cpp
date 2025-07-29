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

    tr.resize(CUBES);
    sc.resize(CUBES);
    rot.resize(CUBES);
    trans.resize(CUBES);

    cubes_tot = 1000;

    // Random variables for cubes
    for(int i = 0; i < CUBES; i++){
        tr[i] = std::max((rand() % 1000 * 0.1f), 0.5f) * glm::normalize(glm::vec3(rand() % 10 - 5, rand() % 10 - 5, rand() % 10 - 5));
        sc[i] = glm::vec3(std::max(rand() % 10 * 0.1f, 0.1f));
        rot[i] = glm::normalize(glm::vec3(std::min((rand() % 10 - 5) * 0.1f, 0.1f), std::min((rand() % 10 - 5) * 0.1f, 0.1f), std::min((rand() % 10 - 5) * 0.1f, 0.1f)));
    }

    init_VAO();
    init_textures();
    init_buffers();
    init_shaders();

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
}

void Engine::init_buffers(){
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

    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, CUBES * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);

    std::size_t vec4Size = sizeof(glm::vec4);
    for (int i = 0; i < 4; i++) {
        glEnableVertexAttribArray(3 + i);
        glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(i * vec4Size));
        glVertexAttribDivisor(3 + i, 1); 
    }

    glGenBuffers(1, &rotAxisVBO);
    glBindBuffer(GL_ARRAY_BUFFER, rotAxisVBO);
    glBufferData(GL_ARRAY_BUFFER, CUBES * sizeof(glm::vec3), rot.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glVertexAttribDivisor(7, 1);

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

    projection = glm::perspective(glm::radians(45.f), 800.f/600.f, 0.1f, 100.f);

    // ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    while(!glfwWindowShouldClose(window))
    {   
        float t = (float)glfwGetTime();
        dtime = t - past_time;
        past_time = t;
        process_input();
        cam -> update(right_input, left_input, dtime);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

        draw();

        ImGui::Begin("STATS");
        ImGui::Text("FPS: %.1f", 1.0f / dtime);
        ImGui::End();
        
		ImGui::Begin("CUBES");
        ImGui::InputInt("Num Cubes", &cubes_tot);
        cubes_tot = std::min(CUBES, cubes_tot);
        ImGui::InputFloat("Spread fact", &spread);
        ImGui::InputFloat("Rot Speed", &rot_speed);

		// Ends the window
		ImGui::End();

        ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents(); 
    }

    // Deletes all ImGUI instances
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

    glDeleteVertexArrays(1, &cVAO);
    glDeleteBuffers(1, &cVBO);
    glDeleteBuffers(1, &cEBO);
    //glDeleteProgram(shaderProgram);

    glfwTerminate();
}

void Engine::draw(){
    shaders[0] -> use();
    glBindVertexArray(cVAO);
    glUniform1i(glGetUniformLocation(shaders[0]->ID, "texture1"), 0);
    shaders[0] -> setInt("texture2", 1);
    shaders[0] -> setFloat("mix_val", mix_val);

    glm::mat4 view = cam -> viewAtMat();
    shaders[0] -> setMatrix("view", view);
    shaders[0] -> setMatrix("projection", projection);

    shaders[0]->setFloat("time", (float)glfwGetTime());
    shaders[0]->setFloat("rotSpeed", rot_speed);
    
    // bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

    for (int i = 0; i < cubes_tot; i++) {
        trans[i] = glm::mat4(1.0f);
        trans[i] = glm::scale(trans[i], sc[i]);
        trans[i] = glm::translate(trans[i], spread * tr[i]);
    }

    glBindBuffer(GL_ARRAY_BUFFER, rotAxisVBO);

    // Upload all instance transforms in one go
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, cubes_tot * sizeof(glm::mat4), trans.data());

    // One draw call only
    glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, cubes_tot);
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



