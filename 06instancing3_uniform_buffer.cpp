/* OpenGL example code - Instancing with uniform buffer object
 * 
 * create 8 instances of the cube from the perspective example
 * the per instance data is passed with a uniform buffer object
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
    if((window = glfwCreateWindow(width, height, "06instancing3_uniform_buffer", 0, 0)) == 0) {
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

    // shader source code
    std::string vertex_source =
        "#version 330\n"
        "layout(std140) uniform Matrices {\n"
        "   mat4 ViewProjection;\n"
        "   mat4 Model[8];\n"
        "};\n"
        "layout(location = 0) in vec4 vposition;\n"
        "layout(location = 1) in vec4 vcolor;\n"
        "out vec4 fcolor;\n"
        "void main() {\n"
        "   fcolor = vcolor;\n"
        "   gl_Position = ViewProjection*Model[gl_InstanceID]*vposition;\n"
        "}\n";
        
    std::string fragment_source =
        "#version 330\n"
        "in vec4 fcolor;\n"
        "layout(location = 0) out vec4 FragColor;\n"
        "void main() {\n"
        "   FragColor = fcolor;\n"
        "}\n";
   
    // program and shader handles
    GLuint shader_program, vertex_shader, fragment_shader;
    
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
    glAttachShader(shader_program, fragment_shader);
    
    // link the program and check for errors
    glLinkProgram(shader_program);
    check_program_link_status(shader_program);

    // obtain location of the uniform block
    GLuint Matrices_binding = 0;
    GLint uniform_block_index = glGetUniformBlockIndex(shader_program, "Matrices");
    // assign the block binding
    glUniformBlockBinding(shader_program, uniform_block_index, Matrices_binding);
    
    // create uniform buffer
    GLuint ubo;
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, 9*sizeof(float)*4*4, 0, GL_STREAM_DRAW);
    
    // fill the Model matrix array
    glm::mat4 ModelMatrices[8];
    ModelMatrices[0] = glm::translate(glm::mat4(1.0f), glm::vec3( 2.0f, 2.0f, 2.0f));
    ModelMatrices[1] = glm::translate(glm::mat4(1.0f), glm::vec3( 2.0f, 2.0f,-2.0f));
    ModelMatrices[2] = glm::translate(glm::mat4(1.0f), glm::vec3( 2.0f,-2.0f, 2.0f));
    ModelMatrices[3] = glm::translate(glm::mat4(1.0f), glm::vec3( 2.0f,-2.0f,-2.0f));
    ModelMatrices[4] = glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 2.0f, 2.0f));
    ModelMatrices[5] = glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 2.0f,-2.0f));
    ModelMatrices[6] = glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f,-2.0f, 2.0f));
    ModelMatrices[7] = glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f,-2.0f,-2.0f));
    
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float)*4*4, 8*sizeof(float)*4*4, ModelMatrices);
    
    
    // vao and vbo handle
    GLuint vao, vbo, ibo;
 
    // generate and bind the vao
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    // generate and bind the vertex buffer object
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
            
    // data for a cube
    GLfloat vertexData[] = {
    //  X     Y     Z           R     G     B
    // face 0:
       1.0f, 1.0f, 1.0f,       1.0f, 0.0f, 0.0f, // vertex 0
      -1.0f, 1.0f, 1.0f,       1.0f, 0.0f, 0.0f, // vertex 1
       1.0f,-1.0f, 1.0f,       1.0f, 0.0f, 0.0f, // vertex 2
      -1.0f,-1.0f, 1.0f,       1.0f, 0.0f, 0.0f, // vertex 3

    // face 1:
       1.0f, 1.0f, 1.0f,       0.0f, 1.0f, 0.0f, // vertex 0
       1.0f,-1.0f, 1.0f,       0.0f, 1.0f, 0.0f, // vertex 1
       1.0f, 1.0f,-1.0f,       0.0f, 1.0f, 0.0f, // vertex 2
       1.0f,-1.0f,-1.0f,       0.0f, 1.0f, 0.0f, // vertex 3

    // face 2:
       1.0f, 1.0f, 1.0f,       0.0f, 0.0f, 1.0f, // vertex 0
       1.0f, 1.0f,-1.0f,       0.0f, 0.0f, 1.0f, // vertex 1
      -1.0f, 1.0f, 1.0f,       0.0f, 0.0f, 1.0f, // vertex 2
      -1.0f, 1.0f,-1.0f,       0.0f, 0.0f, 1.0f, // vertex 3
      
    // face 3:
       1.0f, 1.0f,-1.0f,       1.0f, 1.0f, 0.0f, // vertex 0
       1.0f,-1.0f,-1.0f,       1.0f, 1.0f, 0.0f, // vertex 1
      -1.0f, 1.0f,-1.0f,       1.0f, 1.0f, 0.0f, // vertex 2
      -1.0f,-1.0f,-1.0f,       1.0f, 1.0f, 0.0f, // vertex 3

    // face 4:
      -1.0f, 1.0f, 1.0f,       0.0f, 1.0f, 1.0f, // vertex 0
      -1.0f, 1.0f,-1.0f,       0.0f, 1.0f, 1.0f, // vertex 1
      -1.0f,-1.0f, 1.0f,       0.0f, 1.0f, 1.0f, // vertex 2
      -1.0f,-1.0f,-1.0f,       0.0f, 1.0f, 1.0f, // vertex 3

    // face 5:
       1.0f,-1.0f, 1.0f,       1.0f, 0.0f, 1.0f, // vertex 0
      -1.0f,-1.0f, 1.0f,       1.0f, 0.0f, 1.0f, // vertex 1
       1.0f,-1.0f,-1.0f,       1.0f, 0.0f, 1.0f, // vertex 2
      -1.0f,-1.0f,-1.0f,       1.0f, 0.0f, 1.0f, // vertex 3
    }; // 6 faces with 4 vertices with 6 components (floats)

    // fill with data
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*6*4*6, vertexData, GL_STATIC_DRAW);
                    
           
    // set up generic attrib pointers
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), (char*)0 + 0*sizeof(GLfloat));
 
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), (char*)0 + 3*sizeof(GLfloat));
    
    
    // generate and bind the index buffer object
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
            
    GLuint indexData[] = {
        // face 0:
        0,1,2,      // first triangle
        2,1,3,      // second triangle
        // face 1:
        4,5,6,      // first triangle
        6,5,7,      // second triangle
        // face 2:
        8,9,10,     // first triangle
        10,9,11,    // second triangle
        // face 3:
        12,13,14,   // first triangle
        14,13,15,   // second triangle
        // face 4:
        16,17,18,   // first triangle
        18,17,19,   // second triangle
        // face 5:
        20,21,22,   // first triangle
        22,21,23,   // second triangle
    };

    // fill with data
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*6*2*3, indexData, GL_STATIC_DRAW);
    
    // we are drawing 3d objects so we want depth testing
    glEnable(GL_DEPTH_TEST);
    
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
        glm::mat4 View = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));
        
        // make the camera rotate around the origin
        View = glm::rotate(View, 90.0f*t, glm::vec3(1.0f, 1.0f, 1.0f)); 
        
        glm::mat4 ViewProjection = Projection*View;
        
        // set the ViewProjection in the uniform buffer
        glBindBuffer(GL_UNIFORM_BUFFER, ubo);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float)*4*4, glm::value_ptr(ViewProjection));
        glBindBufferRange(GL_UNIFORM_BUFFER, Matrices_binding, ubo, 0, sizeof(float)*4*4*9);
        
        // bind the vao
        glBindVertexArray(vao);

        // draw
        // the additional parameter indicates how many instances to render
        glDrawElementsInstanced(GL_TRIANGLES, 6*6, GL_UNSIGNED_INT, 0, 8);
       
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
    glDeleteBuffers(1, &ibo);
    
    glDetachShader(shader_program, vertex_shader);	
    glDetachShader(shader_program, fragment_shader);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    glDeleteProgram(shader_program);
    
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

