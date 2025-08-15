
#ifndef MYOPENPROJECT_ARRAYBUFFER_H
#define MYOPENPROJECT_ARRAYBUFFER_H

#include <glad/glad.h>
#include <iostream>

class ArrayBuffer {
public:

    ArrayBuffer(const unsigned int size,const void* data)
    {
        setupBuffer(size,data);
        std::cout << "ARRAY BUFFER CREATED" << '\n';
    }

    ~ArrayBuffer()
    {
        std::cout << "ARRAY BUFFER DELETED" << '\n';
        if (VBO) {
            glBindBuffer(GL_ARRAY_BUFFER,0);
            glDeleteBuffers(1,&VBO);
        }


        std::cout << "VERTEX ARRAY DELETED" << '\n';
        if (VAO) {
            glBindVertexArray(0);
            glDeleteVertexArrays(1,&VAO);
        }

    }

    [[nodiscard]]unsigned int getVAO() const {return VAO;}
    [[nodiscard]]unsigned int getVBO() const {return VBO;}

private:
    unsigned int VBO{};
    unsigned int VAO{};

    void setupBuffer(const unsigned int size,const void* data)
    {
        glGenVertexArrays(1,&VAO);
        glGenBuffers(1,&VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER,VBO);
        glBufferData(GL_ARRAY_BUFFER,size,data,GL_STATIC_DRAW);
    }

public:

    void setupAttribute(const unsigned int index,const int size,const GLenum type,const int stride,const unsigned int startPoint) const
    {
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER,VBO);
        glVertexAttribPointer(index, size, type, GL_FALSE, stride, reinterpret_cast<void*>(startPoint));
        glEnableVertexAttribArray(index);

        glBindBuffer(GL_ARRAY_BUFFER,0);
        glBindVertexArray(0);
    }

};
#endif //MYOPENPROJECT_ARRAYBUFFER_H