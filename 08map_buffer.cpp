/* OpenGL example code - buffer mapping
 * 
 * This example uses the geometry shader again for particle drawing.
 * The particles are animated on the cpu and uploaded every frame by
 * mapping vbos. Multiple vbos are used to triple buffer the particle
 * data.
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
#include <cmath>


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
    if((window = glfwCreateWindow(width, height, "08map_buffer", 0, 0)) == 0) {
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
        "   gl_Position = Projection*(pos+0.2*vec4(txcoord,0,0));\n"
        "   EmitVertex();\n"
        "   txcoord = vec2( 1,-1);\n"
        "   gl_Position = Projection*(pos+0.2*vec4(txcoord,0,0));\n"
        "   EmitVertex();\n"
        "   txcoord = vec2(-1, 1);\n"
        "   gl_Position = Projection*(pos+0.2*vec4(txcoord,0,0));\n"
        "   EmitVertex();\n"
        "   txcoord = vec2( 1, 1);\n"
        "   gl_Position = Projection*(pos+0.2*vec4(txcoord,0,0));\n"
        "   EmitVertex();\n"
        "}\n";    
    
    // the fragment shader creates a bell like radial color distribution    
    std::string fragment_source =
        "#version 330\n"
        "in vec2 txcoord;\n"
        "layout(location = 0) out vec4 FragColor;\n"
        "void main() {\n"
        "   float s = 0.2*(1/(1+15.*dot(txcoord, txcoord))-1/16.);\n"
        "   FragColor = s*vec4(0.3,0.3,1.0,1);\n"
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

    const int particles = 128*1024;

    // randomly place particles in a cube
    std::vector<glm::vec3> vertexData(particles);
    std::vector<glm::vec3> velocity(particles);
    for(int i = 0;i<particles;++i) {
        vertexData[i] = glm::vec3(0.5f-float(std::rand())/RAND_MAX,
                                  0.5f-float(std::rand())/RAND_MAX,
                                  0.5f-float(std::rand())/RAND_MAX);
        vertexData[i] = glm::vec3(0.0f,20.0f,0.0f) + 5.0f*vertexData[i];
    }

    
    int buffercount = 3;
    // generate vbos and vaos
    GLuint vao[buffercount], vbo[buffercount];
    glGenVertexArrays(buffercount, vao);
    glGenBuffers(buffercount, vbo);
    
    for(int i = 0;i<buffercount;++i) {
        glBindVertexArray(vao[i]);
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo[i]);

        // fill with initial data
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*vertexData.size(), &vertexData[0], GL_DYNAMIC_DRAW);
                        
        // set up generic attrib pointers
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (char*)0 + 0*sizeof(GLfloat));
    }
    
    // we are blending so no depth testing
    glDisable(GL_DEPTH_TEST);
    
    // enable blending
    glEnable(GL_BLEND);
    //  and set the blend function to result = 1*source + 1*destination
    glBlendFunc(GL_ONE, GL_ONE);

    // define spheres for the particles to bounce off
    const int spheres = 3;
    glm::vec3 center[spheres];
    float radius[spheres];
    center[0] = glm::vec3(0,12,1);
    radius[0] = 3;
    center[1] = glm::vec3(-3,0,0);
    radius[1] = 7;
    center[2] = glm::vec3(5,-10,0);
    radius[2] = 12;

    // physical parameters
    float dt = 1.0f/60.0f;
    glm::vec3 g(0.0f, -9.81f, 0.0f);
    float bounce = 1.2f; // inelastic: 1.0f, elastic: 2.0f

    int current_buffer=0;    
    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // get the time in seconds
        float t = glfwGetTime();

        // update physics
        for(int i = 0;i<particles;++i) {
            // resolve sphere collisions
            for(int j = 0;j<spheres;++j) {
                glm::vec3 diff = vertexData[i]-center[j];
                float dist = glm::length(diff);
                if(dist<radius[j] && glm::dot(diff, velocity[i])<0.0f)
                    velocity[i] -= bounce*diff/(dist*dist)*glm::dot(diff, velocity[i]);
            }
            // euler iteration
            velocity[i] += dt*g;
            vertexData[i] += dt*velocity[i];
            // reset particles that fall out to a starting position
            if(vertexData[i].y<-30.0) {
                vertexData[i] = glm::vec3(
                                    0.5f-float(std::rand())/RAND_MAX,
                                    0.5f-float(std::rand())/RAND_MAX,
                                    0.5f-float(std::rand())/RAND_MAX
                                );
                vertexData[i] = glm::vec3(0.0f,20.0f,0.0f) + 5.0f*vertexData[i];
                velocity[i] = glm::vec3(0,0,0);
            }
        }
        
        // bind a buffer to upload to
        glBindBuffer(GL_ARRAY_BUFFER, vbo[(current_buffer+buffercount-1)%buffercount]);
        
        // explicitly invalidate the buffer
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*vertexData.size(), 0, GL_DYNAMIC_DRAW);

        // map the buffer
        glm::vec3 *mapped = 
            reinterpret_cast<glm::vec3*>(
                glMapBufferRange(GL_ARRAY_BUFFER, 0,
                    sizeof(glm::vec3)*vertexData.size(),
                    GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT
                )
            );
            
        // copy data into the mapped memory
        std::copy(vertexData.begin(), vertexData.end(), mapped);
        
        // unmap the buffer
        glUnmapBuffer(GL_ARRAY_BUFFER);

        
        // clear first
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // use the shader program
        glUseProgram(shader_program);
        
        // calculate ViewProjection matrix
        glm::mat4 Projection = glm::perspective(90.0f, 4.0f / 3.0f, 0.1f, 100.f);
        
        // translate the world/view position
        glm::mat4 View = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -30.0f));
        
        // make the camera rotate around the origin
        View = glm::rotate(View, 30.0f, glm::vec3(1.0f, 0.0f, 0.0f)); 
        View = glm::rotate(View, -22.5f*t, glm::vec3(0.0f, 1.0f, 0.0f)); 
        
        // set the uniform
        glUniformMatrix4fv(View_location, 1, GL_FALSE, glm::value_ptr(View)); 
        glUniformMatrix4fv(Projection_location, 1, GL_FALSE, glm::value_ptr(Projection)); 
        
        // bind the current vao
        glBindVertexArray(vao[current_buffer]);

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
        
        // advance buffer index
        current_buffer = (current_buffer + 1) % buffercount;       
    }
    
    // delete the created objects
        
    glDeleteVertexArrays(buffercount, vao);
    glDeleteBuffers(buffercount, vbo);
    
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

