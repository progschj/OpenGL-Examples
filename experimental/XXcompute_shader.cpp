/* OpenGL example code - Compute Shader
 * 
 * Simulates and draws the same particles system as the transform
 * feedback example but with a compute shader instead of transform
 * feedback.
 * 
 * note: this example is still "experimental" at this point glew doesn't
 * load 4.3 and a modified gl3w was used.
 * 
 * Autor: Jakob Progsch
 */
 
 /* index
 * line  208: compute shader source code    
 * line  390: compute shader invocation
 */

#include <GL3/gl3w.h>
#include <GL/glfw.h>

//glm is used to create perspective and transform matrices
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cmath>

bool running;

// window close callback function
int closedWindow()
{
    running = false;
    return GL_TRUE;
}

// helper to check and display for shader compiler errors
bool check_shader_compile_status(GLuint obj)
{
    GLint status;
    glGetShaderiv(obj, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE)
    {
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
bool check_program_link_status(GLuint obj)
{
    GLint status;
    glGetProgramiv(obj, GL_LINK_STATUS, &status);
    if(status == GL_FALSE)
    {
        GLint length;
        glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(length);
        glGetProgramInfoLog(obj, length, &length, &log[0]);
        std::cerr << &log[0];
        return false;   
    }
    return true;
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
    glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 4);
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

    // shader source code
    
    // the vertex shader simply passes through data
    std::string vertex_source =
        "#version 430\n"
        "layout(location = 0) in vec4 vposition;\n"
        "void main() {\n"
        "   gl_Position = vposition;\n"
        "}\n";
    
    // the geometry shader creates the billboard quads
    std::string geometry_source =
        "#version 430\n"
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
    if(!check_shader_compile_status(vertex_shader))
    {
        return 1;
    }
    
    // create and compiler geometry shader
    geometry_shader = glCreateShader(GL_GEOMETRY_SHADER);
    source = geometry_source.c_str();
    length = geometry_source.size();
    glShaderSource(geometry_shader, 1, &source, &length); 
    glCompileShader(geometry_shader);
    if(!check_shader_compile_status(geometry_shader))
    {
        return 1;
    }
 
    // create and compiler fragment shader
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    source = fragment_source.c_str();
    length = fragment_source.size();
    glShaderSource(fragment_shader, 1, &source, &length);   
    glCompileShader(fragment_shader);
    if(!check_shader_compile_status(fragment_shader))
    {
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

    std::string compute_source =
        "#version 430\n"
        "layout(local_size_x=256) in;\n"
        
        "uniform vec3 center[3];\n"
        "uniform float radius[3];\n"
        "uniform vec3 g;\n"
        "uniform float dt;\n"
        "uniform float bounce;\n"
        "uniform int seed;\n"
        "uniform layout(rgba32f) imageBuffer particles;\n"
                
        "float hash(int x) {\n"
        "   x = x*1235167 + int(gl_GlobalInvocationID)*948737 + seed*9284365;\n"
        "   x = (x >> 13) ^ x;\n"
        "   return ((x * (x * x * 60493 + 19990303) + 1376312589) & 0x7fffffff)/float(0x7fffffff-1);\n"
        "}\n"
        
        "void main() {\n"
        "   int index = int(gl_GlobalInvocationID);\n"
        "   vec3 inposition = imageLoad(particles, 2*index).xyz;\n"
        "   vec3 invelocity = imageLoad(particles, 2*index+1).xyz;\n"
        "   vec3 outvelocity = invelocity;\n"
        "   for(int j = 0;j<3;++j) {\n"
        "       vec3 diff = inposition-center[j];\n"
        "       float dist = length(diff);\n"
        "       float vdot = dot(diff, invelocity);\n"
        "       if(dist<radius[j] && vdot<0.0)\n"
        "           outvelocity -= bounce*diff*vdot/(dist*dist);\n"
        "   }\n"
        "   outvelocity += dt*g;\n"
        "   vec3 outposition = inposition + dt*outvelocity;\n"
        "   if(outposition.y < -30.0)\n"
        "   {\n"
        "       outvelocity = vec3(0,0,0);\n"
        "       outposition = 0.5-vec3(hash(3*index+0),hash(3*index+1),hash(3*index+2));\n"
        "       outposition = vec3(0,20,0) + 5.0*outposition;\n"
        "   }\n"
        "   imageStore(particles, 2*index, vec4(outposition,1));\n"
        "   imageStore(particles, 2*index+1, vec4(outvelocity,1));\n"
        "}\n";
   
    // program and shader handles
    GLuint compute_program, compute_shader;

    // create and compiler vertex shader
    compute_shader = glCreateShader(GL_COMPUTE_SHADER);
    source = compute_source.c_str();
    length = compute_source.size();
    glShaderSource(compute_shader, 1, &source, &length); 
    glCompileShader(compute_shader);
    if(!check_shader_compile_status(compute_shader))
    {
        return 1;
    }

    // create program
    compute_program = glCreateProgram();
    
    // attach shaders
    glAttachShader(compute_program, compute_shader);
   
    // link the program and check for errors
    glLinkProgram(compute_program);
    check_program_link_status(compute_program);

    GLint center_location = glGetUniformLocation(compute_program, "center");
    GLint radius_location = glGetUniformLocation(compute_program, "radius");
    GLint g_location = glGetUniformLocation(compute_program, "g");
    GLint dt_location = glGetUniformLocation(compute_program, "dt");
    GLint bounce_location = glGetUniformLocation(compute_program, "bounce");
    GLint seed_location = glGetUniformLocation(compute_program, "seed");
    GLint particles_location = glGetUniformLocation(compute_program, "particles");
   
    const int particles = 128*1024;

    // randomly place particles in a cube
    std::vector<glm::vec4> vertexData(2*particles);
    for(int i = 0;i<particles;++i)
    {
        // initial position
        vertexData[2*i+0] = glm::vec4(
                                0.5f-float(std::rand())/RAND_MAX,
                                0.5f-float(std::rand())/RAND_MAX,
                                0.5f-float(std::rand())/RAND_MAX,
                                0
                            );
        vertexData[2*i+0] = glm::vec4(0.0f,20.0f,0.0f,1) + 5.0f*vertexData[2*i+0];
        
        // initial velocity
        vertexData[2*i+1] = glm::vec4(0,0,0,0);
    }
    
    // generate vbos and vaos
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    

    glBindVertexArray(vao);
        
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // fill with initial data
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4)*vertexData.size(), &vertexData[0], GL_STATIC_DRAW);
                        
    // set up generic attrib pointers
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), (char*)0 + 0*sizeof(GLfloat));
    // set up generic attrib pointers
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), (char*)0 + 4*sizeof(GLfloat));


    // "unbind" vao
    glBindVertexArray(0);
    
    // texture handle
    GLuint buffer_texture;
    
    // generate and bind the buffer texture
    glGenTextures(1, &buffer_texture);
    glBindTexture(GL_TEXTURE_BUFFER, buffer_texture);
    
    // tell the buffer texture to use 
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, vbo);
    
    
    
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
    running = true;
    while(running)
    {   
        // get the time in seconds
        float t = glfwGetTime();
        
        // terminate on excape 
        if(glfwGetKey(GLFW_KEY_ESC))
        {
            running = false;
        }



        // use the transform shader program
        glUseProgram(compute_program);

        // set the uniforms
        glUniform3fv(center_location, 3, reinterpret_cast<GLfloat*>(center)); 
        glUniform1fv(radius_location, 3, reinterpret_cast<GLfloat*>(radius));
        glUniform3fv(g_location, 1, glm::value_ptr(g));
        glUniform1f(dt_location, dt);
        glUniform1f(bounce_location, bounce);
        glUniform1i(seed_location, std::rand());
        
        glBindImageTexture(0, buffer_texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
        glUniform1i(particles_location, 0);

        glDispatchCompute(particles/256, 1, 1);

        
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
        glBindVertexArray(vao);

        // draw
        glDrawArrays(GL_POINTS, 0, particles);
       
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
    
    // delete the created objects
        
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteTextures(1, &buffer_texture);
    
    glDetachShader(shader_program, vertex_shader);	
    glDetachShader(shader_program, geometry_shader);	
    glDetachShader(shader_program, fragment_shader);
    glDeleteShader(vertex_shader);
    glDeleteShader(geometry_shader);
    glDeleteShader(fragment_shader);
    glDeleteProgram(shader_program);

    glDetachShader(compute_program, compute_shader);	
    glDeleteShader(compute_shader);
    glDeleteProgram(compute_program);
    
    glfwCloseWindow();
    glfwTerminate();
    return 0;
}

