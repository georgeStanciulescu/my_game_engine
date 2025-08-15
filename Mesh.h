#ifndef MYOPENPROJECT_MESH_H
#define MYOPENPROJECT_MESH_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <utility>
#include <vector>
#include <cstddef>
#include "Shader.h"

static constexpr int maxBoneInfluence{4};

class Mesh
{
public:
    struct Vertex {
        glm::vec3 Position{};
        glm::vec3 Normal{};
        glm::vec2 TexCoords{};
        glm::vec3 Tangent{};
        // bitangent
        glm::vec3 Bitangent{};
        //bone indexes which will influence this vertex
        int m_BoneIDs[maxBoneInfluence];
        //weights from each bone
        float m_Weights[maxBoneInfluence];
    };

    struct Texture {
        unsigned int id;
        std::string type;
        std::string path;
    };

    std::vector<Vertex> vertices;
    std::vector <unsigned int> indices;
    std::vector<Texture> textures;

    unsigned int VAO{};

    Mesh(std::vector<Vertex>&& vertex,std::vector<unsigned int>&& index,std::vector<Texture>&& texture )
        :vertices{std::move(vertex)},indices{std::move(index)},textures{std::move(texture)}
    {
        setupMesh();
    }

    void Draw(const Shader& shader,const int numberOfInstances = 0) const
    {
        unsigned int diffuseNr{1};
        unsigned int specularNr{1};
        unsigned int normalNr{1};
        unsigned int heightNr{1};
        for(unsigned int i{0}; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
            // retrieve texture number (the N in diffuse_textureN)
            std::string number;
            std::string name = textures[i].type;
            if(name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            else if(name == "texture_specular")
                number = std::to_string(specularNr++);
            else if(name == "texture_normal")
                number = std::to_string(normalNr++);
            else if(name == "texture_height")
                number = std::to_string(heightNr++);

            name += number;

            shader.setInt("material." + name, static_cast<int>(i));
            glBindTexture(GL_TEXTURE_2D, textures[i].id);


        }

        glActiveTexture(GL_TEXTURE0);

        // draw mesh
        glBindVertexArray(VAO);
        if (!numberOfInstances)
            glDrawElements(GL_TRIANGLES, static_cast<int>(indices.size()), GL_UNSIGNED_INT, nullptr);
        else
            glDrawElementsInstanced(GL_TRIANGLES,static_cast<int>(indices.size()),GL_UNSIGNED_INT,nullptr,numberOfInstances);
        glBindVertexArray(0);
    }

private:
    unsigned int VBO{};
    unsigned int EBO{};

    void setupMesh()
    {
        glGenVertexArrays(1,&VAO);
        glGenBuffers(1,&VBO);
        glGenBuffers(1,&EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER,VBO);

        glBufferData(GL_ARRAY_BUFFER,static_cast<long>(std::size(vertices) * sizeof(Vertex)),&vertices[0],GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,static_cast<long>(std::size(indices) * sizeof(unsigned int)),&indices[0],GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),nullptr);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,Normal)));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,TexCoords)));

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,Tangent)));

        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,Bitangent)));

        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5,4,GL_INT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,m_BoneIDs)));

        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6,4,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,m_Weights)));

        glBindVertexArray(0);
    }
};
#endif //MYOPENPROJECT_MESH_H