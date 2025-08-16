#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/scene.h>

#include "Shader.h"
#include "stb_image.h"
#include "Camera.h"
#include "Model.h"
#include "Globals.h"
#include "Buffers/Framebuffer.h"
#include "Buffers/UBO.h"
#include "VertexInformation.h"
#include "Buffers/ArrayBuffer.h"
#include "Input.h"

#include <iostream>
#include <cmath>
#include <vector>
#include <string>

static void framebuffer_size_callback(GLFWwindow* window,int width, int height);
static void mouse_callback(GLFWwindow* window,double xpos, double ypos);

void printFPS(double& zeroFrame,int& nFrames);

void renderBackPack(const Shader& shader,const Camera& camera,const Model& backpack,std::vector<glm::vec3>& movingLight);
void renderLightCubes(const Shader& lightShader,const Camera& camera,const ArrayBuffer& lightBuffer,const std::vector<glm::vec3>& movingLight);
void renderPlane(const Shader& lightShader,const ArrayBuffer& planeBuffer,const uint floorTexture);
void renderSkybox(const Shader& skyboxShader,const ArrayBuffer& cubeMapBuffer,glm::mat4& view,const uint cubemapTexture);
void renderWindows(const Shader& stencilShader,const Camera& camera,const ArrayBuffer& grassBuffer,const uint grassTexture);
void renderQuad(const Shader& frameBufferShader,const ArrayBuffer& quadBuffer,const uint textureFramebuffer);

int main() {
    glfwInit(); //the opengl version being used and initialising the state
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);


    GLFWwindow* window = glfwCreateWindow(Globals::SCREEN_WIDTH, Globals::SCREEN_HEIGHT, "Daemon Engine", nullptr, nullptr); // creating the pointer window with the necessary attributes
    if (!window) // if the pointer is null,end the process;
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window); // makes the window the context(the state machine)

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) // initialising glad
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;

    }


    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); // the function uses the window to resize it as appropriate

    stbi_set_flip_vertically_on_load(true);

    { // made this scope so as to properly delete the buffers
        Camera myCamera(glm::vec3(0.0f,0.0f,3.0f));
        Camera depthCamera(glm::vec3(0.0,5.0,0.0f));
        Model myModel("backpack.obj",true); // loading model

        stbi_set_flip_vertically_on_load(false); // keep it like this so the grass texture comes up in the right way
        uint grassTexture(Model::TextureFromFile("transparent_window.png","",TextureType::transparent));
        uint floorTexture{Model::TextureFromFile("temp_container2.png")};
        uint cubemapTexture = Model::loadCubemap(TemporaryVertices::faces);

        Shader myShader("shader.vs","shader.gs","shader.fs");
        Shader lightShader("lightingshader.vs","lightingshader.fs");
        Shader stencilShader("shader.vs","stencilshader.fs");
        Shader frameBufferShader("frameBufferShader.vs","frameBufferShader.fs");
        Shader skyboxShader("skyboxshader.vs","skyboxshader.fs");
        Shader normalShader("NORMALSONLYSHADER.vs","NORMALSONLYSHADER.gs","NORMALSONLYSHADER.fs");
        Shader depthShader("depthshader.vs","depthshader.fs");

        UBO ubo(2*sizeof(glm::mat4),0,GL_STATIC_DRAW);
        //uint uniformBuffer = ubo.getBufferID();

        Framebuffer myFrameBuffer{Globals::SCREEN_WIDTH,Globals::SCREEN_HEIGHT};
        uint framebuffer = myFrameBuffer.getFrameBufferID();
        uint textureFramebuffer = myFrameBuffer.getTextureBufferID();
        //uint renderObject = myFrameBuffer.getRenderBufferID();

        Framebuffer msBuffer{Globals::SCREEN_WIDTH,Globals::SCREEN_HEIGHT,Framebuffer::Type::MULTISAMPLE};
        uint sampleFrameBuffer = msBuffer.getFrameBufferID();
        //uint sampleTexture = msBuffer.getTextureBufferID();
        //uint sampleRenderBuffer = msBuffer.getRenderBufferID();

        Framebuffer depthBuffer(Globals::SHADOW_WIDTH,Globals::SHADOW_HEIGHT,Framebuffer::Type::DEPTH);
        uint depthBufferID = depthBuffer.getFrameBufferID();

        ArrayBuffer lightBuffer(sizeof(TemporaryVertices::cubeVertices),TemporaryVertices::cubeVertices);
        lightBuffer.setupAttribute(0,3,GL_FLOAT,6*sizeof(float),0);
        lightBuffer.setupAttribute(1,3,GL_FLOAT,6*sizeof(float),3*sizeof(float));

        ArrayBuffer planeBuffer(sizeof(TemporaryVertices::planeVertices),TemporaryVertices::planeVertices);
        planeBuffer.setupAttribute(0,3,GL_FLOAT,5*sizeof(float),0);
        planeBuffer.setupAttribute(2,2,GL_FLOAT,5*sizeof(float),3*sizeof(float));

        ArrayBuffer grassBuffer(sizeof(TemporaryVertices::vegetationPosition),TemporaryVertices::vegetationPosition);
        grassBuffer.setupAttribute(0,3,GL_FLOAT,5*sizeof(float),0);
        grassBuffer.setupAttribute(2,2,GL_FLOAT,5*sizeof(float),3*sizeof(float));

        ArrayBuffer quadBuffer(sizeof(TemporaryVertices::quadVertices),TemporaryVertices::quadVertices);
        quadBuffer.setupAttribute(0,2,GL_FLOAT,4*sizeof(float),0);
        quadBuffer.setupAttribute(1,2,GL_FLOAT,4*sizeof(float),2*sizeof(float));

        ArrayBuffer cubeMapBuffer(sizeof(TemporaryVertices::skyboxVertices),TemporaryVertices::skyboxVertices);
        cubeMapBuffer.setupAttribute(0,3,GL_FLOAT,3*sizeof(float),0);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        stencilShader.setInt("grass",0);
        lightShader.setInt("lightTexture",0);
        frameBufferShader.setInt("screenTexture",0);
        skyboxShader.setInt("skybox",0);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINES);
        glEnable(GL_MULTISAMPLE);
        glDepthFunc(GL_LEQUAL);
        glDepthRange(0,1);

        //glViewport(0, 0, Globals::SHADOW_WIDTH, Globals::SHADOW_HEIGHT); // for shadow mapping
        glViewport(0, 0, Globals::SCREEN_WIDTH, Globals::SCREEN_HEIGHT); // for shadow mapping

        ubo.uniformBlockBinding(myShader.getProgramID(),"Perspective");
        ubo.uniformBlockBinding(lightShader.getProgramID(),"Perspective");
        ubo.uniformBlockBinding(skyboxShader.getProgramID(),"Perspective");

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetWindowUserPointer(window, &myCamera);
        glfwSetCursorPosCallback(window, mouse_callback);

        auto zeroFrame = glfwGetTime();
        int nFrames{0};

        float deltaTime{0.0f};
        float lastFrame{0.0f};

        while(!glfwWindowShouldClose(window)) // the while loop checks continuously whether the necessary keys have been pressed to close the window
        {
            //glEnable(GL_DEPTH_TEST); // for one,for testing
            auto currentFrame = static_cast<float>(glfwGetTime());
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;
            nFrames++;

            glm::mat4 projection = glm::perspective(glm::radians(Globals::FOV),
                static_cast<float>(Globals::SCREEN_WIDTH) / static_cast<float>(Globals::SCREEN_HEIGHT),0.1f,100.0f); // determines the perspective
            glm::mat4 view = myCamera.GetViewMatrix(); // view matrix

            ubo.updateUniform(0,sizeof(glm::mat4),projection);
            ubo.updateUniform(sizeof(glm::mat4),sizeof(glm::mat4),view);

            std::vector<glm::vec3> movingLight(8);

            printFPS(zeroFrame,nFrames);

            Input::generalInput(window);
            Input::movementInput(window,myCamera,deltaTime);

            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // glClear(GL_DEPTH_BUFFER_BIT);
            // depthShader.use();
            //
            // float near_plane = 1.0f;
            // float far_plane = 7.5f;
            // glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
            // for (glm::vec3& i :  movingLight)
            // {
            //     glm::mat4 lightView = glm::lookAt(i,glm::vec3(0.0f,0.0f,0.0f),glm::vec3(0.0f,1.0f,0.0f));
            //     glm::mat4 lightMatrix = lightProjection * lightView;
            //     depthShader.setMat4("lightMatrix",lightMatrix);
            // }
            //
            // glm::mat4 lightView = glm::lookAt(glm::vec3(0.0f,5.0f,0.0f),glm::vec3(0.0f,0.0f,0.0f),glm::vec3(0.0f,1.0f,0.0f));
            // glm::mat4 lightMatrix = lightProjection * lightView;
            // depthShader.setMat4("lightMatrix",lightMatrix);
            //
            //
            //
            // glViewport(0, 0, Globals::SHADOW_WIDTH, Globals::SHADOW_HEIGHT); // for shadow mapping
            // glBindFramebuffer(GL_FRAMEBUFFER,depthBufferID);
            // glClear(GL_DEPTH_BUFFER_BIT);
            // glActiveTexture(GL_TEXTURE0);
            // glBindTexture(GL_TEXTURE_2D,depthBuffer.getTextureBufferID());



            // renderBackPack(myShader,myCamera,myModel,movingLight); // function for rendering the backpack
            // renderLightCubes(lightShader,myCamera,lightBuffer,movingLight);
            // renderPlane(lightShader,planeBuffer,floorTexture);
            // renderSkybox(skyboxShader,cubeMapBuffer,view,cubemapTexture);
            // renderWindows(stencilShader,myCamera,grassBuffer,grassTexture);



            glBindFramebuffer(GL_FRAMEBUFFER,sampleFrameBuffer);
            glEnable(GL_DEPTH_TEST);

            // glViewport(0, 0, Globals::SCREEN_WIDTH, Globals::SCREEN_HEIGHT); // for shadow mapping
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            renderBackPack(myShader,myCamera,myModel,movingLight); // function for rendering the backpack
            renderLightCubes(lightShader,myCamera,lightBuffer,movingLight);
            renderPlane(lightShader,planeBuffer,floorTexture);
            renderSkybox(skyboxShader,cubeMapBuffer,view,cubemapTexture);
            renderWindows(stencilShader,myCamera,grassBuffer,grassTexture);

            glBindFramebuffer(GL_READ_FRAMEBUFFER, sampleFrameBuffer);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
            glBlitFramebuffer(0, 0, Globals::SCREEN_WIDTH, Globals::SCREEN_HEIGHT, 0, 0,
                Globals::SCREEN_WIDTH, Globals::SCREEN_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);


            glBindFramebuffer(GL_FRAMEBUFFER,0); // default framebuffer
            glDisable(GL_DEPTH_TEST);

            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            renderQuad(frameBufferShader,quadBuffer,textureFramebuffer);

            glfwSwapBuffers(window); // double buffers(front and back) used simultaneously to make whatever is on screen appear smooth
            glfwPollEvents(); // this calls the callback functions,takes input,updates the window;
        }

        myShader.end();
        lightShader.end();
        stencilShader.end();
        frameBufferShader.end();
        skyboxShader.end();
        normalShader.end();
    }

    glfwTerminate(); // end the window

    return 0;
}

static void framebuffer_size_callback(GLFWwindow* window [[maybe_unused]],int width, int height) // changes the dimensions of the window
{
    glViewport(0, 0, width, height);
}
static void mouse_callback(GLFWwindow* window,double xpos, double ypos)
{
    auto* myCamera = static_cast<Camera*>(glfwGetWindowUserPointer(window)); // I did this because I couldn't pass by reference
    if (Globals::firstLook) // sets the xpos and ypos to the middle of the screen
    {
        Globals::lastX = static_cast<float>(xpos);
        Globals::lastY = static_cast<float>(ypos);
        Globals::firstLook = false;
    }
    float xoffset = static_cast<float>(xpos) - Globals::lastX;
    float yoffset = Globals::lastY - static_cast<float>(ypos);
    Globals::lastX = static_cast<float>(xpos);
    Globals::lastY = static_cast<float>(ypos);
    myCamera -> ProcessMouseMovement(xoffset,yoffset);
}

void printFPS(double& zeroFrame,int& nFrames)
{
    auto currentFrame =glfwGetTime();

    if (currentFrame - zeroFrame > 1.0f) // FPS calculator
    {
        std::cout << "FPS: " << nFrames  << '\n';
        nFrames = 0.0;
        zeroFrame = currentFrame;
    }
}

void renderBackPack(const Shader& shader,const Camera& camera,const Model& backpack,std::vector<glm::vec3>& movingLight){

    auto currentFrame = static_cast<float>(glfwGetTime());

    shader.use();
    shader.setVec3("viewPos",camera.Position);
    shader.setFloat("time",currentFrame);


    shader.setFloat("material.shininess",100.0f);

    for (unsigned int i{0}; i < movingLight.size();++i)
    {
        movingLight[i] = glm::vec3(std::sin(glfwGetTime()) * 1.0f * i ,static_cast<float>(i) * 1.0f,std::cos(glfwGetTime()) * 3.0f * i);
        std::string index{std::to_string(i)};
        shader.setVec3("pointLights[" + index + "].position",movingLight[i]);
        shader.setFloat("pointLights["+ index +"].constant",1.0f);
        shader.setFloat("pointLights["+ index +"].linear",0.09f);
        shader.setFloat("pointLights["+ index +"].quadratic",0.032f);
        shader.setVec3("pointLights["+ index +"].ambient",  0.05f,0.05f,0.05f);
        shader.setVec3("pointLights["+ index +"].diffuse",  0.15f,0.15f,0.15f); // darken diffuse light a bit
        shader.setVec3("pointLights["+ index +"].specular", 1.0f, 1.0f, 1.0f);
    }
    shader.setBool("hasFlashed",Globals::hasFlashed); // spotlight turning off and on
    shader.setBool("blinnPhong",Globals::blinnPhong);
            // spotlight details
    shader.setFloat("spotLight.constant",1.0f);
    shader.setFloat("spotLight.linear",0.09f);
    shader.setFloat("spotLight.quadratic",0.032f);
    shader.setVec3("spotLight.position",camera.Position);
    shader.setVec3("spotLight.direction",camera.Front);
    shader.setFloat("spotLight.cutOff",glm::cos(glm::radians(10.5f)));
    shader.setFloat("spotLight.outerCutOff",glm::cos(glm::radians(18.0f)));
    shader.setVec3("spotLight.ambient",  glm::vec3(0.0f));
    shader.setVec3("spotLight.diffuse",  1.0f,1.0f,1.0f); // darken diffuse light a bit
    shader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
            //direction light details
    shader.setVec3("dirLight.direction",-1.0f,-1.0f,-1.0f); // direction light
    shader.setVec3("dirLight.ambient",  0.05f,0.05f,0.05f);
    shader.setVec3("dirLight.diffuse", 0.05f,0.05f,0.05f); // darken diffuse light a bit
    shader.setVec3("dirLight.specular", 1.0f, 1.0f, 1.0f);

    auto model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, -20.0f)); // translate it down so it's at the center of the scene
    model = glm::scale(model, glm::vec3(2.3f, 2.3f, 2.3f));	// it's a bit too big for our scene, so scale it down


    shader.setMat4("transform",model);
    backpack.Draw(shader);
}
void renderLightCubes(const Shader& lightShader,const Camera& camera,const ArrayBuffer& lightBuffer,const std::vector<glm::vec3>& movingLight) {

    lightShader.use();
    lightShader.setInt("skybox",0);
    lightShader.setVec3("cameraPos",camera.Position);

    for (const glm::vec3& light : movingLight) {
        auto lightModel = glm::mat4(1.0f);
        lightModel = glm::translate(lightModel,glm::vec3(std::sin(light.x),0.0f,light.z));
        lightModel = glm::scale(lightModel,glm::vec3(1.4f));

        lightShader.setMat4("transform",lightModel);

        glBindVertexArray(lightBuffer.getVAO());
        glDrawArraysInstanced(GL_TRIANGLES, 0, 36,100);
    }
    auto model = glm::mat4(1.0f);
    model = glm::translate(model,glm::vec3(0.0f,5.0f,0.0f));
    model = glm::scale(model,glm::vec3(2.0f,2.0f,2.0f));
    lightShader.setMat4("transform",model);

    glBindVertexArray(lightBuffer.getVAO());
    glActiveTexture(GL_TEXTURE0);
    glDrawArrays(GL_TRIANGLES,0,36);

}
void renderPlane(const Shader& lightShader,const ArrayBuffer& planeBuffer,const uint floorTexture) {

    glBindVertexArray(planeBuffer.getVAO());
    glBindTexture(GL_TEXTURE_2D,floorTexture);
    auto model = glm::mat4(1.0f);
    lightShader.setMat4("transform",model);
    glDrawArrays(GL_TRIANGLES,0,6);

}

void renderSkybox(const Shader& skyboxShader,const ArrayBuffer& cubeMapBuffer,glm::mat4& view,const uint cubemapTexture) {

    skyboxShader.use();
    view = glm::mat4(glm::mat3(view));

    skyboxShader.setMat4("view",view);
    glDepthMask(GL_FALSE);
    glBindVertexArray(cubeMapBuffer.getVAO());
    glBindTexture(GL_TEXTURE_CUBE_MAP,cubemapTexture);
    glDrawArrays(GL_TRIANGLES,0,36);
    glDepthMask(GL_TRUE);

}

void renderWindows(const Shader& stencilShader,const Camera& camera,const ArrayBuffer& grassBuffer,const uint grassTexture) {

    stencilShader.use();
    glBindVertexArray(grassBuffer.getVAO());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,grassTexture);

    std::map<float,glm::vec3> sortedWindows;

    for (glm::vec3& pos : TemporaryVertices::vegetation) {
        float distance = glm::length(camera.Position - pos);
        sortedWindows[distance] = pos;
    }
    for(auto it = sortedWindows.rbegin(); it != sortedWindows.rend(); ++it)
    {
        auto model = glm::mat4(1.0f);
        model = glm::translate(model, it->second);
        stencilShader.setMat4("transform", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

}

void renderQuad(const Shader& frameBufferShader,const ArrayBuffer& quadBuffer,const uint textureFramebuffer) {

    frameBufferShader.use();
    frameBufferShader.setBool("gammaCorrected",Globals::gammaCorrected);
    glBindVertexArray(quadBuffer.getVAO());
    glBindTexture(GL_TEXTURE_2D,textureFramebuffer);
    glDrawArrays(GL_TRIANGLES,0,6);

}