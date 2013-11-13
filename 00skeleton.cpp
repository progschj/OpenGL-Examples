/* OpenGL example code - Skeleton
 *
 * Skeleton code that all the other examples are based on
 *
 * Autor: Jakob Progsch
 */

#include <GLXW/glxw.h>
#include <GLFW/glfw3.h>

#include <iostream>

int main() {
    int width = 640;
    int height = 480;

    if(glfwInit() == GL_FALSE) {
        std::cerr << "failed to init GLFW" << std::endl;
        return 1;
    }

    // select opengl version
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    // create a window
    GLFWwindow *window;
    if((window = glfwCreateWindow(width, height, "00skeleton", 0, 0)) == 0) {
        std::cerr << "failed to open window" << std::endl;
        glfwTerminate();
        return 1;
    }
    
    glfwMakeContextCurrent(window);

    if(glxwInit()) {
        std::cerr << "failed to init GL3W" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    // creation and initialization of stuff goes here
    // ...

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // drawing etc goes here
        // ...

        // check for errors
        GLenum error = glGetError();
        if(error != GL_NO_ERROR) {
            std::cerr << error << std::endl;
            break;
        }

        // finally swap buffers
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

