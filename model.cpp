#include "model.h"



void Model::Draw(Shader &shader, bool use_text, unsigned int shadowID){
    int t = 0;
    for(unsigned int i = 0; i < meshes.size(); i++){
        t += meshes[i].Draw(shader, use_text, shadowID);
    }
    triangles = t;
}

void Model::set_lights(Shader &shader, AmbientLight& ambient_light, std::vector<PointLight> &pointLights, std::vector<SpotLight> &spotLights){
    ambient_light.linkShader(shader);

    shader.setInt("pointLights_num", pointLights.size());
    for(size_t i = 0; i < pointLights.size(); i++){
        pointLights[i].setIndex(i);
        pointLights[i].linkShader(shader);
    }

    shader.setInt("spotLights_num", spotLights.size());
    for(size_t i = 0; i < spotLights.size(); i++){
        spotLights[i].setIndex(i);
        spotLights[i].linkShader(shader);
    }
}


void Model::loadModel(std::string path){
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
    {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode * node, const aiScene *scene){
    // process all the node's meshes (if any)
    for(unsigned int i = 0; i < node -> mNumMeshes; i++){
        aiMesh * mesh = scene -> mMeshes[node -> mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    // then do the same for each of its children
    for(unsigned int i = 0; i < node -> mNumChildren; i++){
        processNode(node -> mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene){
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    for(unsigned int i = 0; i < mesh -> mNumVertices; i++){
        Vertex vertex;
        glm::vec3 vector;
        vector.x = mesh -> mVertices[i].x;
        vector.y = mesh -> mVertices[i].y;
        vector.z = mesh -> mVertices[i].z;
        vertex.position = vector;

        vector.x = mesh -> mNormals[i].x;
        vector.y = mesh -> mNormals[i].y;
        vector.z = mesh -> mNormals[i].z;
        vertex.normal = vector;

        if(mesh -> mTextureCoords[0]){
            glm::vec2 vec;
            vec.x = mesh -> mTextureCoords[0][i].x;
            vec.y = mesh -> mTextureCoords[0][i].y;
            vertex.texCoords = vec;
        }
        else{
            vertex.texCoords = glm::vec2(0.f, 0.f);
        }
        vertices.push_back(vertex);
    }
    // process indices
    for(unsigned int i = 0; i < mesh -> mNumFaces; i++){
        aiFace face = mesh -> mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++){
            indices.push_back(face.mIndices[j]);
        }
    }


    //process material
    if(mesh -> mMaterialIndex >= 0){
        aiMaterial *material = scene -> mMaterials[mesh -> mMaterialIndex];
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }

    return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName){
    std::vector<Texture> textures;
    for(unsigned int i = 0; i < mat -> GetTextureCount(type); i++){
        aiString str;
        mat -> GetTexture(type, i, &str);
        bool skip = false;
        for(unsigned int j = 0; j < textures_loaded.size(); j++){
            if(std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0){
                textures.push_back(textures_loaded[j]);
                skip = true;
                break;
            }
        }

        if(!skip){
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture);
        }
    }

    return textures;
}

unsigned int Model::TextureFromFile(const char *path, const std::string &directory, bool gamma)
{
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    // Flips the texture vertically on load, as OpenGL expects the 0.0 coordinate on the y-axis to be at the bottom
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);

    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RGB;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Set texture wrapping and filtering options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data); // Free the image memory
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

