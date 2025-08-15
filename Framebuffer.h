
#ifndef MYOPENPROJECT_FRAMEBUFFER_H
#define MYOPENPROJECT_FRAMEBUFFER_H

#include <glad/glad.h>
#include "Globals.h"
#include <iostream>

class Framebuffer
{
public:
    enum Type
    {
        MULTISAMPLE,
        NORMAL,
    };

    Framebuffer(const int width,const int height,const Type type = NORMAL)
           :bufferType{type}
    {
        setupFramebuffer(width,height);
        std::cout << "FRAMEBUFFER CREATED" << '\n';
    }

    ~Framebuffer()
    {
        std::cout << "FRAMEBUFFER DELETED" << '\n';
        glDeleteFramebuffers(1,&framebufferID);

        std::cout << "FRAMEBUFFER TEXTURE DELETED" << '\n';
        glDeleteTextures(1,&textureFramebuffer);

        std::cout << "RENDER BUFFER OBJECT DELETED" << '\n';
        glDeleteRenderbuffers(1,&renderBuffer);
    }

    [[nodiscard]]unsigned int getFrameBufferID() const {return framebufferID;}
    [[nodiscard]]unsigned int getTextureBufferID() const {return textureFramebuffer;}
    [[nodiscard]]unsigned int getRenderBufferID() const {return renderBuffer;}
    [[nodiscard]]Type getBufferType() const {return bufferType;}

private:

    unsigned int framebufferID{};
    unsigned int textureFramebuffer{};
    unsigned int renderBuffer{};
    Type bufferType{};

    void setupFramebuffer(const int width,const int height)
    {
        glGenFramebuffers(1,&framebufferID);
        glBindFramebuffer(GL_FRAMEBUFFER,framebufferID);

        glGenTextures(1,&textureFramebuffer);
        bindTexture(width,height);

        glGenRenderbuffers(1,&renderBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER,renderBuffer);
        setRenderBufferStorage(width,height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void bindTexture(const int width,const int height) const
    {
        if (bufferType == MULTISAMPLE ){
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureFramebuffer);
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,Globals::SAMPLE_NUMBER,
                GL_RGB,width,height,GL_TRUE);
            glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D_MULTISAMPLE,textureFramebuffer,0);
        }
        else {
            glBindTexture(GL_TEXTURE_2D,textureFramebuffer);
            glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,width,height,0,GL_RGB,GL_UNSIGNED_BYTE,nullptr);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
            glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,textureFramebuffer,0);
        }
    }
    void setRenderBufferStorage(const int width,const int height) const
    {
        if (bufferType == MULTISAMPLE)
            glRenderbufferStorageMultisample(GL_RENDERBUFFER,Globals::SAMPLE_NUMBER,
                GL_DEPTH24_STENCIL8,width,height);
        else
            glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH24_STENCIL8,width,height);
    }
};
#endif //MYOPENPROJECT_FRAMEBUFFER_H