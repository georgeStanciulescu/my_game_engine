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
#include "Framebuffer.h"
#include "UBO.h"
#include "VertexInformation.h"
#include "ArrayBuffer.h"
#include "Input.h"


#include <iostream>
#include <cmath>
#include <vector>
#include <string>

static void framebuffer_size_callback(GLFWwindow* window,int width, int height);
static void mouse_callback(GLFWwindow* window,double xpos, double ypos);

void printFPS(double& zeroFrame,int& nFrames);

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

    glViewport(0, 0, Globals::SCREEN_WIDTH, Globals::SCREEN_HEIGHT); // where rendering happens,the x and the y are 0,so the starting point is the left bottom corner
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); // the function uses the window to resize it as appropriate

    stbi_set_flip_vertically_on_load(true);

    Camera myCamera(glm::vec3(0.0f,0.0f,3.0f));
    Model myModel("backpack.obj"); // loading model

    stbi_set_flip_vertically_on_load(false); // keep it like this so the grass texture comes up in the right way
    uint grassTexture(Model::TextureFromFile("transparent_window.png","",TextureType::transparent));
    uint floorTexture{Model::TextureFromFile("container2.png")};
    uint cubemapTexture = Model::loadCubemap(TemporaryVertices::faces);

    Shader myShader("shader.vs","shader.gs","shader.fs");
    Shader lightShader("lightingshader.vs","lightingshader.fs");
    Shader stencilShader("shader.vs","stencilshader.fs");
    Shader frameBufferShader("frameBufferShader.vs","frameBufferShader.fs");
    Shader skyboxShader("skyboxshader.vs","skyboxshader.fs");
    Shader normalShader("NORMALSONLYSHADER.vs","NORMALSONLYSHADER.gs","NORMALSONLYSHADER.fs");

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
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    // glEnableVertexAttribArray(0);

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
        auto currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        nFrames++;

        printFPS(zeroFrame,nFrames);

        Input::generalInput(window);
        Input::movementInput(window,myCamera,deltaTime);

        glBindFramebuffer(GL_FRAMEBUFFER,sampleFrameBuffer);
        glEnable(GL_DEPTH_TEST);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        myShader.use();
        myShader.setVec3("viewPos",myCamera.Position);
        myShader.setFloat("material.shininess",100.0f);

        std::vector<glm::vec3> movingLight(8);

        for (unsigned int i{0}; i < movingLight.size();++i)
        {
            movingLight[i] = glm::vec3(std::sin(glfwGetTime()) * 1.0f * i ,static_cast<float>(i) * 1.0f,std::cos(glfwGetTime()) * 3.0f * i);
            std::string index{std::to_string(i)};
            myShader.setVec3("pointLights[" + index + "].position",movingLight[i]);
            myShader.setFloat("pointLights["+ index +"].constant",1.0f);
            myShader.setFloat("pointLights["+ index +"].linear",0.09f);
            myShader.setFloat("pointLights["+ index +"].quadratic",0.032f);
            myShader.setVec3("pointLights["+ index +"].ambient",  0.05f,0.05f,0.05f);
            myShader.setVec3("pointLights["+ index +"].diffuse",  0.15f,0.15f,0.15f); // darken diffuse light a bit
            myShader.setVec3("pointLights["+ index +"].specular", 1.0f, 1.0f, 1.0f);
        }
        myShader.setBool("hasFlashed",Globals::hasFlashed); // spotlight turning off and on
        myShader.setBool("blinnPhong",Globals::blinnPhong);

        myShader.setFloat("time",currentFrame);
        // spotlight details
        myShader.setFloat("spotLight.constant",1.0f);
        myShader.setFloat("spotLight.linear",0.09f);
        myShader.setFloat("spotLight.quadratic",0.032f);
        myShader.setVec3("spotLight.position",myCamera.Position);
        myShader.setVec3("spotLight.direction",myCamera.Front);
        myShader.setFloat("spotLight.cutOff",glm::cos(glm::radians(10.5f)));
        myShader.setFloat("spotLight.outerCutOff",glm::cos(glm::radians(18.0f)));
        myShader.setVec3("spotLight.ambient",  glm::vec3(0.0f));
        myShader.setVec3("spotLight.diffuse",  1.0f,1.0f,1.0f); // darken diffuse light a bit
        myShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        //direction light details
        myShader.setVec3("dirLight.direction",-1.0f,-1.0f,-1.0f); // direction light
        myShader.setVec3("dirLight.ambient",  0.05f,0.05f,0.05f);
        myShader.setVec3("dirLight.diffuse", 0.05f,0.05f,0.05f); // darken diffuse light a bit
        myShader.setVec3("dirLight.specular", 1.0f, 1.0f, 1.0f);

        glm::mat4 projection = glm::perspective(glm::radians(Globals::FOV),
            static_cast<float>(Globals::SCREEN_WIDTH) / static_cast<float>(Globals::SCREEN_HEIGHT),0.1f,100.0f); // determines the perspective
        glm::mat4 view = myCamera.GetViewMatrix(); // view matrix

        ubo.updateUniform(0,sizeof(glm::mat4),projection);
        ubo.updateUniform(sizeof(glm::mat4),sizeof(glm::mat4),view);

        auto model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -20.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(2.3f, 2.3f, 2.3f));	// it's a bit too big for our scene, so scale it down

        myShader.setMat4("transform",model);
        myModel.Draw(myShader);

        // normalShader.use(); //drawing normals
        // normalShader.setMat4("projection",projection);
        // normalShader.setMat4("view",view);
        // normalShader.setMat4("model",model);
        // myModel.Draw(normalShader);

        lightShader.use();
        lightShader.setInt("skybox",0);
        lightShader.setVec3("cameraPos",myCamera.Position);

        for (glm::vec3& light : movingLight) {
            glm::mat4 lightModel = glm::mat4(1.0f);
            lightModel = glm::translate(lightModel,glm::vec3(std::sin(light.x),0.0f,light.z));
            lightModel = glm::scale(lightModel,glm::vec3(1.4f));

            lightShader.setMat4("transform",lightModel);

            glBindVertexArray(lightBuffer.getVAO());
            glDrawArraysInstanced(GL_TRIANGLES, 0, 36,100);
        }
        model = glm::translate(model,glm::vec3(0.0f,5.0f,0.0f));
        model = glm::scale(model,glm::vec3(2.0f,2.0f,2.0f));
        lightShader.setMat4("transform",model);

        glBindVertexArray(lightBuffer.getVAO());
        glActiveTexture(GL_TEXTURE0);
        glDrawArrays(GL_TRIANGLES,0,36);

        glBindVertexArray(planeBuffer.getVAO());
        glBindTexture(GL_TEXTURE_2D,floorTexture);
        model = glm::mat4(1.0f);
        lightShader.setMat4("transform",model);
        glDrawArrays(GL_TRIANGLES,0,6);

        skyboxShader.use();
        view = glm::mat4(glm::mat3(view));

        skyboxShader.setMat4("view",view);
        glDepthMask(GL_FALSE);
        glBindVertexArray(cubeMapBuffer.getVAO());
        glBindTexture(GL_TEXTURE_CUBE_MAP,cubemapTexture);
        glDrawArrays(GL_TRIANGLES,0,36);
        glDepthMask(GL_TRUE);

        stencilShader.use();
        glBindVertexArray(grassBuffer.getVAO());
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,grassTexture);

        std::map<float,glm::vec3> sortedWindows;

        for (glm::vec3& pos : TemporaryVertices::vegetation) {
            float distance = glm::length(myCamera.Position - pos);
            sortedWindows[distance] = pos;
        }
        for(auto it = sortedWindows.rbegin(); it != sortedWindows.rend(); ++it)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, it->second);
            stencilShader.setMat4("transform", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        //passing the anti-aliased scene to the framebuffer
        glBindFramebuffer(GL_READ_FRAMEBUFFER, sampleFrameBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
        glBlitFramebuffer(0, 0, Globals::SCREEN_WIDTH, Globals::SCREEN_HEIGHT, 0, 0,
            Globals::SCREEN_WIDTH, Globals::SCREEN_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        glBindFramebuffer(GL_FRAMEBUFFER,0); // default framebuffer
        glDisable(GL_DEPTH_TEST);

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        frameBufferShader.use();
        glBindVertexArray(quadBuffer.getVAO());
        glBindTexture(GL_TEXTURE_2D,textureFramebuffer);
        glDrawArrays(GL_TRIANGLES,0,6);

        glfwSwapBuffers(window); // double buffers(front and back) used simultaneously to make whatever is on screen appear smooth
        glfwPollEvents(); // this calls the callback functions,takes input,updates the window;
    }
    myShader.end();
    lightShader.end();
    stencilShader.end();
    frameBufferShader.end();
    skyboxShader.end();
    normalShader.end();

    ubo.~UBO();
    myFrameBuffer.~Framebuffer();
    msBuffer.~Framebuffer();
    lightBuffer.~ArrayBuffer();
    planeBuffer.~ArrayBuffer();
    grassBuffer.~ArrayBuffer();
    quadBuffer.~ArrayBuffer();
    cubeMapBuffer.~ArrayBuffer();

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

    if (currentFrame - zeroFrame > 0.2f) // FPS calculator
    {
        std::cout << "FPS: " << nFrames * 5.0 << '\n';
        nFrames = 0.0;
        zeroFrame = currentFrame;
    }
}


