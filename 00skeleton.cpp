/* OpenGL example code - Skeleton
 * 
 * Skeleton code that all the other examples are based on
 * 
 * Autor: Jakob Progsch
 */
 
/* index
 * line   34: glfw initialization
 * line   56: glew initialization    
 * line   68: main loop    
 * line   92: cleanup
 */

#include <GL3/gl3w.h>
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
    
    if(glfwInit() == GL_FALSE)
    {
        std::cerr << "failed to init GLFW" << std::endl;
        return 1;
    }

    // sadly glew doesn't play nice with core profiles... 
    glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);
 
    // create a window
    if(glfwOpenWindow(width, height, 0, 0, 0, 8, 24, 8, GLFW_WINDOW) == GL_FALSE)
    {
        std::cerr << "failed to open window" << std::endl;
        glfwTerminate();
        return 1;
    }
    
    // setup windows close callback
    glfwSetWindowCloseCallback(closedWindow);

    
    
    if (gl3wInit())
    {
        std::cerr << "failed to init GL3W" << std::endl;
        glfwCloseWindow();
        glfwTerminate();
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


    glfwCloseWindow();
    glfwTerminate();
    return 0;
}

