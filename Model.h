
#ifndef MYOPENPROJECT_MODEL_H
#define MYOPENPROJECT_MODEL_H

#include <glad/glad.h>

#include <glm/glm.hpp>

#include "stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "Shader.h"


#include <string>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

enum class TextureType
{
    transparent,
    opaque,
};

unsigned int TextureFromFile(const char *path, const std::string &directory ="",TextureType imageType = TextureType::opaque, bool gamma = false);

class Model
{
public:

    std::vector<Mesh::Texture> textures_loaded;
    std::vector<Mesh> meshes;
    std::string directory;
    bool gammaCorrection;

    explicit Model(char const * path,bool gamma = false) : gammaCorrection(gamma)
    {
        loadModel(path);
    }

    void Draw(const Shader& shader,const int numberOfInstances = 0) const
    {
        for (const Mesh& i : meshes)
        {
            i.Draw(shader,numberOfInstances);
        }
    }
private:

    void loadModel(std::string const &path)
    {
        Assimp::Importer import;
        const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
            return;
        }
        directory = path.substr(0, path.find_last_of('/'));

        processNode(scene->mRootNode, scene);
    }
    void processNode(const aiNode* node, const aiScene *scene)
    {
        // process all the node's meshes (if any)
        for(unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // then do the same for each of its children
        for(unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }
    }

    Mesh processMesh(aiMesh *mesh, const aiScene *scene)
    {
        std::vector<Mesh::Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Mesh::Texture> textures;

        for (unsigned int i{0}; i < mesh ->mNumVertices;i++) {
            Mesh::Vertex vertex{};

            glm::vec3 vector{};

            vector.x = mesh ->mVertices[i].x;
            vector.y = mesh ->mVertices[i].y;
            vector.z = mesh ->mVertices[i].z;

            vertex.Position = vector;

            if (mesh -> HasNormals())
            {
                glm::vec3 normal{};

                normal.x = mesh -> mNormals[i].x;
                normal.y = mesh -> mNormals[i].y;
                normal.z = mesh -> mNormals[i].z;

                vertex.Normal = normal;
            }

            if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;

                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.Tangent = vector;
                // bitangent
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.Bitangent = vector;
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }

        for(unsigned int i{0}; i < mesh->mNumFaces; i++)
        {
                aiFace face = mesh->mFaces[i];
                for(unsigned int j{0}; j < face.mNumIndices; j++)
                    indices.push_back(face.mIndices[j]);
        }

        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<Mesh::Texture> diffuseMaps = loadMaterialTextures(material,
                                                    aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        std::vector<Mesh::Texture> specularMaps = loadMaterialTextures(material,
                                                    aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        std::vector<Mesh::Texture> normalMaps = loadMaterialTextures(material,
                                                    aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        std::vector<Mesh::Texture> heightMaps = loadMaterialTextures(material,
                                                    aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

        return {std::move(vertices), std::move(indices), std::move(textures)};
    }
    std::vector<Mesh::Texture> loadMaterialTextures(const aiMaterial *mat, aiTextureType type,
                                         const std::string& typeName)
    {
        std::vector<Mesh::Texture> textures;
        for(unsigned int i{0}; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            bool skip = false;

            for (Mesh::Texture tex : textures_loaded)
            {
                if(std::strcmp(tex.path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(tex);
                    skip = true;
                    break;
                }
            }
            if(!skip)
            {   // if texture hasn't been loaded already, load it
                Mesh::Texture texture;
                texture.id = TextureFromFile(str.C_Str()); // add directory as second argument if you place your maps somewhere else
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture); // add to loaded textures


            }
        }
        return textures;
    }
public:

    static unsigned int TextureFromFile(const char *path,const std::string &directory = "",
        const TextureType imageType = TextureType::opaque,[[maybe_unused]] bool gamma = false)
    {
        auto filename = std::string(path);

        if (!directory.empty()){filename = directory + '/' + filename;}

        unsigned int textureID{};
        glGenTextures(1, &textureID);

        int width{};
        int height{};
        int nrComponents{};

        unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
        if (!data)
        {
            std::cout << "Texture failed to load at path: " << path << std::endl;
            stbi_image_free(data);
            return textureID;
        }
        GLenum format{0};
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, static_cast<int>(format), width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        std::cout << "TEXTURE: " << path << "     TYPE: ";
        if (imageType == TextureType::opaque)
        {
            std::cout << "OPAQUE" << '\n';
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }
        else if (imageType == TextureType::transparent)
        {
            std::cout << "TRANSPARENT" << '\n';
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);

        return textureID;
    }

    static unsigned int loadCubemap(const std::vector<std::string>& faces) // not part of the class proper,have to move it eventually;
    {
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

        int width, height, nrChannels;
        for (unsigned int i = 0; i < faces.size(); i++)
        {
            unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
            if (!data)
            {

                std::cout << "Texture upload problem for " << faces[i] << std::endl;
                stbi_image_free(data);
                return 0;
            }
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                             0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
                );
            stbi_image_free(data);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        return textureID;
    }
};

#endif //MYOPENPROJECT_MODEL_H