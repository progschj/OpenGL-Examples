/* OpenGL example code - Skeleton
 * 
 * Skeleton code that all the other examples are based on
 * 
 * Autor: Jakob Progsch
 */

#include <GL/glew.h>
#include <GL/glfw.h>

#include <iostream>

bool running;

// window close callback function
int closedWindow()
{
    running = false;
    return GL_TRUE;
}

int main()
{
    int width = 640;
    int height = 480;
    
    glfwInit();

    // sadly glew doesn't play nice with core profiles... 
    glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);
 
    // create a window
    glfwOpenWindow(width, height, 0, 0, 0, 0, 0, 0, GLFW_WINDOW);
    
    // setup windows close callback
    glfwSetWindowCloseCallback(closedWindow);
    
    glewExperimental = GL_TRUE;
    if (GLEW_OK != glewInit())
    {
        std::cerr << "failed to init GLEW" << std::endl;
        return 1;
    }
    
    // creation and initialization of stuff goes here
    
    running = true;
    while(running)
    {    
        // terminate on excape 
        if(glfwGetKey(GLFW_KEY_ESC))
        {
            running = false;
        }
        
        // drawing etc goes here
        // ...
       
        // check for errors
        GLenum error = glGetError();
        if(error != GL_NO_ERROR)
        {
            std::cerr << gluErrorString(error);
            running = false;       
        }
        
        // finally swap buffers
        glfwSwapBuffers();       
    }

    glfwTerminate();
    return 0;
}
