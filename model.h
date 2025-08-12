#pragma once

#include "shader.h"
#include "mesh.h"

#include "helpers.h"

#include "ambientlight.h"
#include "pointlight.h"
#include "spotlight.h"


class Model{
public:
    Model(){
        
    }

    Model(char *path){
        loadModel(path);
    }

    void Draw(Shader &shader, bool use_text = true, unsigned int shadowID = 0);
    void set_lights(Shader &shader, AmbientLight& ambient_light, std::vector<PointLight> &pointLights, std::vector<SpotLight> &spotLights);
    int triangle_count(){
        return triangles;
    }

private:
    // model data
    std::vector<Mesh> meshes;
    std::string directory;
    std::vector<Texture> textures_loaded;

    int triangles;

    void loadModel(std::string path);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);
    unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma = false);
};