#ifndef MYOPENPROJECT_UBO_H
#define MYOPENPROJECT_UBO_H

#include <glad/glad.h>
#include <string>
#include <glm/gtc/type_ptr.hpp>

class UBO
{
public:

    explicit UBO(const unsigned int size,const unsigned int index,const GLenum usage)
        :indexID{index}
    {
        setupUBO(size,usage);
        std::cout << "UNIFORM BUFFER CREATED" << '\n';
    }

    ~UBO()
    {
        std::cout << "UNIFORM BUFFER DELETED" << '\n';
        glDeleteBuffers(1,&bufferID);
    }

    [[nodiscard]]unsigned int getBufferID() const {return bufferID;}
    [[nodiscard]]unsigned int getIndexID() const {return indexID;}

private:
    unsigned int bufferID{};
    unsigned int indexID{};

    void setupUBO(const unsigned int size,const GLenum usage)
    {
        glGenBuffers(1,&bufferID);
        glBindBuffer(GL_UNIFORM_BUFFER,bufferID);
        glBufferData(GL_UNIFORM_BUFFER,size,nullptr,usage);
        glBindBufferRange(GL_UNIFORM_BUFFER,indexID,bufferID,0,size);
    }
public:
    void uniformBlockBinding(const unsigned int shaderProgramID,const std::string& name) const
    {
        glUniformBlockBinding(shaderProgramID,glGetUniformBlockIndex(shaderProgramID,name.c_str()),indexID);
    }

    template<typename T>
    void updateUniform(const unsigned int offset,const unsigned int size,const T& uniformObject) const  {
        glBindBuffer(GL_UNIFORM_BUFFER,bufferID);
        glBufferSubData(GL_UNIFORM_BUFFER,offset,size,glm::value_ptr(uniformObject));
        glBindBuffer(GL_UNIFORM_BUFFER,0);
    }

};

#endif //MYOPENPROJECT_UBO_H