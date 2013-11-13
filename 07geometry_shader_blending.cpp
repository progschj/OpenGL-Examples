/* OpenGL example code - Geometry Shader and Blending
 * 
 * Uses a geometry shader to expand points to billboard quads.
 * The billboards are then blended while drawing to create a galaxy
 * made of particles.
 * 
 * Autor: Jakob Progsch
 */

#include <GLXW/glxw.h>
#include <GLFW/glfw3.h>

//glm is used to create perspective and transform matrices
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>


// helper to check and display for shader compiler errors
bool check_shader_compile_status(GLuint obj) {
    GLint status;
    glGetShaderiv(obj, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE) {
        GLint length;
        glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(length);
        glGetShaderInfoLog(obj, length, &length, &log[0]);
        std::cerr << &log[0];
        return false;
    }
    return true;
}

// helper to check and display for shader linker error
bool check_program_link_status(GLuint obj) {
    GLint status;
    glGetProgramiv(obj, GL_LINK_STATUS, &status);
    if(status == GL_FALSE) {
        GLint length;
        glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(length);
        glGetProgramInfoLog(obj, length, &length, &log[0]);
        std::cerr << &log[0];
        return false;   
    }
    return true;
}

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
    if((window = glfwCreateWindow(width, height, "07geometry_shader_blending", 0, 0)) == 0) {
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
        
    // the vertex shader simply passes through data
    std::string vertex_source =
        "#version 330\n"
        "layout(location = 0) in vec4 vposition;\n"
        "void main() {\n"
        "   gl_Position = vposition;\n"
        "}\n";

    // the geometry shader creates the billboard quads
    std::string geometry_source =
        "#version 330\n"
        "uniform mat4 View;\n"
        "uniform mat4 Projection;\n"
        "layout (points) in;\n"
        "layout (triangle_strip, max_vertices = 4) out;\n"
        "out vec2 txcoord;\n"
        "void main() {\n"
        "   vec4 pos = View*gl_in[0].gl_Position;\n"
        "   txcoord = vec2(-1,-1);\n"
        "   gl_Position = Projection*(pos+vec4(txcoord,0,0));\n"
        "   EmitVertex();\n"
        "   txcoord = vec2( 1,-1);\n"
        "   gl_Position = Projection*(pos+vec4(txcoord,0,0));\n"
        "   EmitVertex();\n"
        "   txcoord = vec2(-1, 1);\n"
        "   gl_Position = Projection*(pos+vec4(txcoord,0,0));\n"
        "   EmitVertex();\n"
        "   txcoord = vec2( 1, 1);\n"
        "   gl_Position = Projection*(pos+vec4(txcoord,0,0));\n"
        "   EmitVertex();\n"
        "}\n";    
    
    // the fragment shader creates a bell like radial color distribution    
    std::string fragment_source =
        "#version 330\n"
        "in vec2 txcoord;\n"
        "layout(location = 0) out vec4 FragColor;\n"
        "void main() {\n"
        "   float s = 0.2*(1/(1+15.*dot(txcoord, txcoord))-1/16.);\n"
        "   FragColor = s*vec4(1,0.9,0.6,1);\n"
        "}\n";
   
    // program and shader handles
    GLuint shader_program, vertex_shader, geometry_shader, fragment_shader;
    
    // we need these to properly pass the strings
    const char *source;
    int length;

    // create and compiler vertex shader
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    source = vertex_source.c_str();
    length = vertex_source.size();
    glShaderSource(vertex_shader, 1, &source, &length); 
    glCompileShader(vertex_shader);
    if(!check_shader_compile_status(vertex_shader)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
    
    // create and compiler geometry shader
    geometry_shader = glCreateShader(GL_GEOMETRY_SHADER);
    source = geometry_source.c_str();
    length = geometry_source.size();
    glShaderSource(geometry_shader, 1, &source, &length); 
    glCompileShader(geometry_shader);
    if(!check_shader_compile_status(geometry_shader)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
 
    // create and compiler fragment shader
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    source = fragment_source.c_str();
    length = fragment_source.size();
    glShaderSource(fragment_shader, 1, &source, &length);   
    glCompileShader(fragment_shader);
    if(!check_shader_compile_status(fragment_shader)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
    
    // create program
    shader_program = glCreateProgram();
    
    // attach shaders
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, geometry_shader);
    glAttachShader(shader_program, fragment_shader);
    
    // link the program and check for errors
    glLinkProgram(shader_program);
    check_program_link_status(shader_program);
    
    // obtain location of projection uniform
    GLint View_location = glGetUniformLocation(shader_program, "View");
    GLint Projection_location = glGetUniformLocation(shader_program, "Projection");
    
    // vao and vbo handle
    GLuint vao, vbo;
 
    // generate and bind the vao
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    // generate and bind the vertex buffer object
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    
    const int particles = 128*1024;

    // create a galaxylike distribution of points
    std::vector<GLfloat> vertexData(particles*3);
    for(int i = 0;i<particles;++i)
    {
        int arm = 3*(std::rand()/float(RAND_MAX));
        float alpha = 1/(0.1f+std::pow(std::rand()/float(RAND_MAX),0.7f))-1/1.1f;
        float r = 4.0f*alpha;
        alpha += arm*2.0f*3.1416f/3.0f;
        
        vertexData[3*i+0] = r*std::sin(alpha);
        vertexData[3*i+1] = 0;
        vertexData[3*i+2] = r*std::cos(alpha);
        
        vertexData[3*i+0] += (4.0f-0.2*alpha)*(2-(std::rand()/float(RAND_MAX)+std::rand()/float(RAND_MAX)+
                                                  std::rand()/float(RAND_MAX)+std::rand()/float(RAND_MAX)));
        vertexData[3*i+1] += (2.0f-0.1*alpha)*(2-(std::rand()/float(RAND_MAX)+std::rand()/float(RAND_MAX)+
                                                  std::rand()/float(RAND_MAX)+std::rand()/float(RAND_MAX)));
        vertexData[3*i+2] += (4.0f-0.2*alpha)*(2-(std::rand()/float(RAND_MAX)+std::rand()/float(RAND_MAX)+
                                                  std::rand()/float(RAND_MAX)+std::rand()/float(RAND_MAX)));
    }

    // fill with data
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*vertexData.size(), &vertexData[0], GL_STATIC_DRAW);
                    
           
    // set up generic attrib pointers
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (char*)0 + 0*sizeof(GLfloat));

    // we are blending so no depth testing
    glDisable(GL_DEPTH_TEST);
    
    // enable blending
    glEnable(GL_BLEND);
    //  and set the blend function to result = 1*source + 1*destination
    glBlendFunc(GL_ONE, GL_ONE);
    
    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // get the time in seconds
        float t = glfwGetTime();

        
        // clear first
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // use the shader program
        glUseProgram(shader_program);
        
        // calculate ViewProjection matrix
        glm::mat4 Projection = glm::perspective(90.0f, 4.0f / 3.0f, 0.1f, 100.f);
        
        // translate the world/view position
        glm::mat4 View = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -50.0f));
        
        // make the camera rotate around the origin
        View = glm::rotate(View, 30.0f*std::sin(0.1f*t), glm::vec3(1.0f, 0.0f, 0.0f)); 
        View = glm::rotate(View, -22.5f*t, glm::vec3(0.0f, 1.0f, 0.0f)); 
        
        
        // set the uniform
        glUniformMatrix4fv(View_location, 1, GL_FALSE, glm::value_ptr(View)); 
        glUniformMatrix4fv(Projection_location, 1, GL_FALSE, glm::value_ptr(Projection)); 
        
        // bind the vao
        glBindVertexArray(vao);

        // draw
        glDrawArrays(GL_POINTS, 0, particles);
       
        // check for errors
        GLenum error = glGetError();
        if(error != GL_NO_ERROR) {
            std::cerr << error << std::endl;
            break;
        }
        
        // finally swap buffers
        glfwSwapBuffers(window);        
    }
    
    // delete the created objects
        
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    
    glDetachShader(shader_program, vertex_shader);	
    glDetachShader(shader_program, geometry_shader);	
    glDetachShader(shader_program, fragment_shader);
    glDeleteShader(vertex_shader);
    glDeleteShader(geometry_shader);
    glDeleteShader(fragment_shader);
    glDeleteProgram(shader_program);
    
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

