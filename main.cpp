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

#include <iostream>
#include <cmath>
#include <vector>
#include <string>

static void framebuffer_size_callback(GLFWwindow* window,int width, int height);
static void mouse_callback(GLFWwindow* window,double xpos, double ypos);

static void processInput(GLFWwindow *window);
static void movementInput(GLFWwindow *window,Camera& myCamera,float deltaTime);

namespace Globals
{
    static constexpr unsigned int SCREEN_WIDTH{800};
    static constexpr unsigned int SCREEN_HEIGHT{600};
    static float FOV{60.0f};
    static bool firstLook{true};


    static float lastX{static_cast<float>(SCREEN_WIDTH) / 2.0f};
    static float lastY{static_cast<float>(SCREEN_HEIGHT) / 2.0f};
    static bool hasFlashed{false};
    static bool closeFlash{false};

}


int main()
{
    glfwInit(); //the opengl version being used and initialising the state
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(Globals::SCREEN_WIDTH, Globals::SCREEN_HEIGHT, "LearnOpenGL", nullptr, nullptr); // creating the pointer window with the necessary attributes
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

    float vertices[] = {// vertices for the cubes;
            //vertices         texture        normals
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,0.0f,-1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,0.0f,-1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,0.0f,-1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,0.0f,-1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,0.0f,-1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,0.0f,-1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,0.0f, 1.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -1.0f,0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, -1.0f,0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -1.0f,0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -1.0f,0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, -1.0f,0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -1.0f,0.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,0.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,-1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,-1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,-1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,-1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,-1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,-1.0f, 0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,1.0f, 0.0f
    };

    stbi_set_flip_vertically_on_load(true);

    Camera myCamera(glm::vec3(0.0f,0.0f,3.0f));
    Model myModel("backpack.obj"); // loading model
    Shader myShader("shader.vs","shader.fs");
    Shader lightShader("lightingshader.vs","lightingshader.fs");

    unsigned int lightVBO;
    glGenBuffers(1, &lightVBO); //initializing the buffer object and giving it an id(VBO)
    glBindBuffer(GL_ARRAY_BUFFER, lightVBO); //making the created buffer be associated to a target,the gl_array_buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // passing the data about the vertices we'd like to display with the way in which they should appear(static_draw)

    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINES);
    glEnable(GL_DEPTH_TEST);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetWindowUserPointer(window, &myCamera);
    glfwSetCursorPosCallback(window, mouse_callback);

    float deltaTime{0.0f};
    float lastFrame{0.0f};

    while(!glfwWindowShouldClose(window)) // the while loop checks continuously whether the necessary keys have been pressed to close the window
    {
        auto currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        movementInput(window,myCamera,deltaTime);


        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        myShader.use();
        myShader.setVec3("viewPos",myCamera.Position);
        myShader.setFloat("material.shininess",32.0f);
        std::vector<glm::vec3> movingLight(8);

        for (unsigned int i{0}; i < movingLight.size();++i)
        {
            movingLight[i] = glm::vec3(1.0f,1.0f,1.0f) * glm::vec3(std::sin(glfwGetTime()) * 1.0f ,static_cast<float>(i) * 1.0f,std::cos(glfwGetTime()) * 3.0f );
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
        myShader.setVec3("dirLight.ambient",  0.025f,0.025f,0.025f);
        myShader.setVec3("dirLight.diffuse", 0.05f,0.05f,0.05f); // darken diffuse light a bit
        myShader.setVec3("dirLight.specular", 1.0f, 1.0f, 1.0f);

        glm::mat4 view = myCamera.GetViewMatrix(); // view matrix
        glm::mat4 projection = glm::perspective(glm::radians(Globals::FOV),
            static_cast<float>(Globals::SCREEN_WIDTH) / static_cast<float>(Globals::SCREEN_HEIGHT),0.1f,100.0f); // determines the perspective

        myShader.setMat4("projection",projection);
        myShader.setMat4("view",view);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down

        myShader.setMat4("transform",model);

        myModel.Draw(myShader);

        lightShader.use();
        for (glm::vec3& light : movingLight)
        {
            lightShader.setMat4("projection",projection);
            lightShader.setMat4("view",view);

            glm::mat4 lightModel = glm::mat4(1.0f);
            lightModel = glm::translate(lightModel,light);
            lightModel = glm::scale(lightModel,glm::vec3(0.3f));

            lightShader.setMat4("transform",lightModel);

            glBindVertexArray(lightVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glfwSwapBuffers(window); // double buffers(front and back) used simultaneously to make whatever is on screen appear smooth
        glfwPollEvents(); // this calls the callback functions,takes input,updates the window;
    }

    glDeleteVertexArrays(1,&lightVAO);
    myShader.end();
    lightShader.end();

    glfwTerminate(); // end the window

    return 0;
}

static void framebuffer_size_callback(GLFWwindow* window [[maybe_unused]],int width, int height) // changes the dimensions of the window
{
    glViewport(0, 0, width, height);
}

static void processInput(GLFWwindow *window) // defines which keys,when pressed,perform which functions;escape closes the window
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window,GLFW_KEY_Z) == GLFW_PRESS)
    {
        Globals::FOV -= 0.3f;
        if (Globals::FOV < 10.0f)
            Globals::FOV = 10.0f;
    }
    if (glfwGetKey(window,GLFW_KEY_X) == GLFW_PRESS)
    {
        Globals::FOV += 0.3f;
        if (Globals::FOV > 60.0f)
            Globals::FOV = 60.0f;
    }

    if (glfwGetKey(window,GLFW_KEY_F) == GLFW_PRESS && !Globals::closeFlash) {
        Globals::hasFlashed = !Globals::hasFlashed;
        Globals::closeFlash = true;
    } else if (glfwGetKey(window,GLFW_KEY_F) == GLFW_RELEASE) {Globals::closeFlash  = false;}
}

static void movementInput(GLFWwindow *window,Camera& myCamera,float deltaTime)
{
    if (glfwGetKey(window,GLFW_KEY_W) == GLFW_PRESS)
    {
        myCamera.ProcessKeyboard(FORWARD,deltaTime);
        if (glfwGetKey(window,GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)  // n0$$
            myCamera.ProcessKeyboard(ULTRASPEED,deltaTime);

    }
    if (glfwGetKey(window,GLFW_KEY_S) == GLFW_PRESS)
        myCamera.ProcessKeyboard(BACKWARD,deltaTime);

    if (glfwGetKey(window,GLFW_KEY_A) == GLFW_PRESS)
        myCamera.ProcessKeyboard(LEFT,deltaTime);

    if (glfwGetKey(window,GLFW_KEY_D) == GLFW_PRESS)
        myCamera.ProcessKeyboard(RIGHT,deltaTime);
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


