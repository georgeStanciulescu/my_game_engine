
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include "Shader.h"
#include "stb_image.h"


#include <iostream>
#include <cmath>

void framebuffer_size_callback(GLFWwindow* window [[maybe_unused]],int width, int height);
void processInput(GLFWwindow *window,Shader& myShader,float currentSpeed);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);



static float moater = 0.3f;
static float rotationSpeed = 90.0f;
static glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
static glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
static glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
static float movementSpeed = 1.0f;
static bool firstLook{true};
static float lastX{400.0f};
static float lastY{300.0f};
static float yaw{ -90.0f};
static float pitch{0.0f};
static float fov{60.0f};

int main()
{
    glfwInit(); //the opengl version being used and initialising the state
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", nullptr, nullptr); // creating the pointer window with the necessary attributes
    if (!window) // if the pointer is null,end the process;
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window); // makes the window the context(the state machine)

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) // initialising glad
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;

    }

    glViewport(0, 0, 800, 600); // where rendering happens,the x and the y are 0,so the starting point is the left bottom corner


    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); // the function uses the window to resize it as appropriate



   //  float vertices[] = {
   //      0.25f,-0.25f,0.0f,   1.0f,0.0f,0.0f,  1.0f,0.0f, //bottom right   1.0,0.0f
   //      -0.25f,-0.25f,0.0f,  0.0f,1.0f,0.0f,  0.0f,0.0f, // bottom left    0 0 f
   //      -0.25f,0.25f,0.0f,   0.0f,0.0f,1.0f,  0.0f,1.0f, // top left        0.0f,1.0f
   //      0.25f,0.25f,0.0f,    1.0f,1.0f,0.0f,  1.0f,1.0f // top right       1.0f,1.0f
   // };

    float vertices[] = { // vertices for the cubes;
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };

    glm::vec3 cubePositions[] = {
        glm::vec3( 0.0f,  0.0f,  0.0f),
        glm::vec3( 2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3( 2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3( 1.3f, -2.0f, -2.5f),
        glm::vec3( 1.5f,  2.0f, -2.5f),
        glm::vec3( 1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };

    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 2,   // first triangle
        0, 2, 3,
    };

    Shader myShader("shader.vs","shader.fs");

    unsigned int VAO;

    glGenVertexArrays(1, &VAO); // the container for the configurations for each attribute inside the shaders
    glBindVertexArray(VAO);

    unsigned int VBO;

    glGenBuffers(1, &VBO); //initializing the buffer object and giving it an id(VBO)
    glBindBuffer(GL_ARRAY_BUFFER, VBO); //making the created buffer be associated to a target,the gl_array_buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // passing the data about the vertices we'd like to display with the way in which they should appear(static_draw)

    unsigned int texture1;
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    int width, height, nrChannels;
     stbi_set_flip_vertically_on_load(true);
    [[maybe_unused]]unsigned char *data = stbi_load("container.jpg", &width, &height, &nrChannels, 0);

    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load the texture!" << std::endl ;
    }

    stbi_image_free(data);



    unsigned int texture2;
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    data = stbi_load("awesomeface.png", &width, &height, &nrChannels, 0);

    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load the texture!" << std::endl ;
    }

    stbi_image_free(data);

    unsigned int EBO;
    glGenBuffers(1, &EBO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);  //position
    glEnableVertexAttribArray(0);

    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3* sizeof(float))); //colour
    // glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));  //texture
    glEnableVertexAttribArray(2);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINES);

    glEnable(GL_DEPTH_TEST);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


    myShader.use();
    myShader.setInt("texture1",0);
    myShader.setInt("texture2",1);

    float deltaTime{0.0f};
    float lastFrame{0.0f};

    while(!glfwWindowShouldClose(window)) // the while loop checks continuously whether the necessary keys have been pressed to close the window
    {

        auto currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        auto currentSpeed{deltaTime * 2.5f};

        processInput(window,myShader,currentSpeed);
        glfwSetCursorPosCallback(window, mouse_callback);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        rotationSpeed = static_cast<float>(glfwGetTime()) * 10.0f;

        glm::mat4 projection;
        glm::mat4 view;

        view = glm::lookAt(cameraPos,cameraPos + cameraFront,cameraUp); // look at view matrix composed of position,direction and up vector;
        projection = glm::perspective(glm::radians(fov),800.0f/600.0f,0.1f,100.0f); // determines the perspective

        unsigned int projectionLoc = glGetUniformLocation(myShader.getProgramID(), "projection");
        unsigned int viewLoc = glGetUniformLocation(myShader.getProgramID(),"view");


        glUniformMatrix4fv(static_cast<int>(projectionLoc), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(static_cast<int>(viewLoc), 1, GL_FALSE, glm::value_ptr(view));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        myShader.use();
        glBindVertexArray(VAO);

        for (unsigned int i{0};i < std::size(cubePositions);++i) // creating cubes
        {
            float rotation{};
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model,cubePositions[i]);

            if (!i)
            {
                rotation = rotationSpeed * 1.0f;
            }
            else
            {
                rotation = rotationSpeed * static_cast<float>(i) * 10.0f;
            }

            model = glm::rotate(model,glm::radians(rotation),glm::vec3(1.0f,0.3f,0.5f));
            unsigned int modelLoc = glGetUniformLocation(myShader.getProgramID(), "transform");
            glUniformMatrix4fv(static_cast<int>(modelLoc), 1, GL_FALSE, glm::value_ptr(model));

            glDrawArrays(GL_TRIANGLES, 0, 36);

        }

        glBindVertexArray(0);

        glfwSwapBuffers(window); // double buffers(front and back) used simultaneously to make whatever is on screen appear smooth
        glfwPollEvents(); // this calls the callback functions,takes input,updates the window;
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    myShader.end();

    glfwTerminate(); // end the window

    return 0;
}

void framebuffer_size_callback(GLFWwindow* window [[maybe_unused]],int width, int height) // changes the dimensions of the window
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window,Shader& myShader,float currentSpeed) // defines which keys,when pressed,perform which functions;escape closes the window
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window,GLFW_KEY_UP) == GLFW_PRESS)
    {
        myShader.setFloat("floater",moater += 0.01f);
    }
    if (glfwGetKey(window,GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        myShader.setFloat("floater",moater -= 0.01f);
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += currentSpeed * cameraFront * movementSpeed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= currentSpeed * cameraFront * movementSpeed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * currentSpeed * movementSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * currentSpeed * movementSpeed;
    if (glfwGetKey(window,GLFW_KEY_Z) == GLFW_PRESS)
    {
        fov -= 0.3f;
        if (fov < 10.0f)
            fov = 10.0f;
    }
    if (glfwGetKey(window,GLFW_KEY_X) == GLFW_PRESS)
    {
        fov += 0.3f;
        if (fov > 60.0f)
            fov = 60.0f;
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstLook) // sets the xpos and ypos to the middle of the screen
    {
        lastX = xpos;
        lastY = ypos;
        firstLook = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f; // minimizing the offset to make camera movement slower
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset; //adding the offset calculated
    pitch += yoffset;

    if(pitch > 89.0f) // limiting pitch
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch)); // x and z are affected by both pitch and yaw whilst y is affected by pitch only
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));


    cameraFront = glm::normalize(direction); //
}

