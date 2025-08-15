#ifndef MYOPENPROJECT_INPUT_H
#define MYOPENPROJECT_INPUT_H
#include <GLFW/glfw3.h>
#include "Globals.h"
#include "Camera.h"

namespace Input
{
    inline void generalInput(GLFWwindow *window) // defines which keys,when pressed,perform which functions;escape closes the window
    {
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        if (glfwGetKey(window,GLFW_KEY_Z) == GLFW_PRESS)
        {
            Globals::FOV -= 0.3f;
            if (Globals::FOV < Globals::MIN_FOV)
                Globals::FOV = Globals::MIN_FOV;
        }
        if (glfwGetKey(window,GLFW_KEY_X) == GLFW_PRESS)
        {
            Globals::FOV += 0.3f;
            if (Globals::FOV > Globals::MAX_FOV)
                Globals::FOV = Globals::MAX_FOV;
        }

        if (glfwGetKey(window,GLFW_KEY_F) == GLFW_PRESS && !Globals::closeFlash) {
            Globals::hasFlashed = !Globals::hasFlashed;
            Globals::closeFlash = true;
        } else if (glfwGetKey(window,GLFW_KEY_F) == GLFW_RELEASE) {Globals::closeFlash  = false;}

        if (glfwGetKey(window,GLFW_KEY_B) == GLFW_PRESS && !Globals::hasChangedLight){   //blinn-phong model changing at run-time
            Globals::blinnPhong = !Globals::blinnPhong;
            Globals::hasChangedLight = true;
        }else if (glfwGetKey(window,GLFW_KEY_B) == GLFW_RELEASE) {Globals::hasChangedLight = false;}
    }

    inline void movementInput(GLFWwindow *window,Camera& myCamera,const float deltaTime)
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
}
#endif //MYOPENPROJECT_INPUT_H