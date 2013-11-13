/* OpenGL example code - Tesselation
 * 
 * This example shows the usage of tesselation for terrain LOD.
 * The terrain is given as a texture of 3d samples (generalized
 * heightfield) and gets rendered without use of a vbo/vao. Instead
 * sample coordinates are generated from InstanceID and VertexID.
 * Tessellation is used to dynamically change the amount of vertices
 * depending on distance from the viewer.
 * This example requires at least OpenGL 4.0
 * 
 * Autor: Jakob Progsch
 */

#include <GLXW/glxw.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 
#include <glm/gtx/noise.hpp> 

#include <iostream>
#include <string>
#include <vector>


// helper to check and display for shader compiler errors
bool check_shader_compile_status(GLuint obj) {
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
bool check_program_link_status(GLuint obj) {
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

int main() {
    int width = 640;
    int height = 480;
    
    if(glfwInit() == GL_FALSE) {
        std::cerr << "failed to init GLFW" << std::endl;
        return 1;
    }

    // select opengl version
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
 
    // create a window
    GLFWwindow *window;
    if((window = glfwCreateWindow(width, height, "11tesselation", 0, 0)) == 0) {
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

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

    // shader source code
    std::string vertex_source =
        "#version 400\n"
        "uniform uint width;\n"
        "uniform uint height;\n"
        "out vec4 tposition;\n"
        "const vec2 quad_offsets[6] = vec2[](\n"
        "   vec2(0,0),vec2(1,0),vec2(1,1),\n"
        "   vec2(0,0),vec2(1,1),vec2(0,1)\n"
        ");\n"
        "void main() {\n"
        "   vec2 base = vec2(gl_InstanceID/width, gl_InstanceID%width);\n"
        "   vec2 offset = quad_offsets[gl_VertexID];\n"
        "   vec2 pos = (base + offset)/vec2(width+1, height+1);\n"
        "   tposition = vec4(pos,0,1);\n"
        "}\n";    

    std::string tess_control_source =
        "#version 400\n"
        "uniform vec3 ViewPosition;\n"
        "uniform float tess_scale;\n"
        "layout(vertices = 3) out;\n"
        "in vec4 tposition[];\n"
        "out vec4 tcposition[];\n"
        "void main()\n"
        "{\n"
        "   tcposition[gl_InvocationID] = tposition[gl_InvocationID];\n"
        "   if(gl_InvocationID == 0) {\n"
        "       vec3 terrainpos = ViewPosition;\n"
        "       terrainpos.z -= clamp(terrainpos.z,-0.1, 0.1);\n"        
        "       vec4 center = (tposition[1]+tposition[2])/2.0;\n"
        "       gl_TessLevelOuter[0] = min(6.0, 1+tess_scale*0.5/distance(center.xyz, terrainpos));\n"
        "       center = (tposition[2]+tposition[0])/2.0;\n"
        "       gl_TessLevelOuter[1] = min(6.0, 1+tess_scale*0.5/distance(center.xyz, terrainpos));\n"
        "       center = (tposition[0]+tposition[1])/2.0;\n"
        "       gl_TessLevelOuter[2] = min(6.0, 1+tess_scale*0.5/distance(center.xyz, terrainpos));\n"
        "       center = (tposition[0]+tposition[1]+tposition[2])/3.0;\n"
        "       gl_TessLevelInner[0] = min(7.0, 1+tess_scale*0.7/distance(center.xyz, terrainpos));\n"
        "   }\n"
        "}\n";

    std::string tess_eval_source =
        "#version 400\n"
        "uniform mat4 ViewProjection;\n"        
        "uniform sampler2D displacement;\n"
        "layout(triangles, equal_spacing, cw) in;\n"
        "in vec4 tcposition[];\n"
        "out vec2 tecoord;\n"
        "out vec4 teposition;\n"
        "void main()\n"
        "{\n"
        "   teposition = gl_TessCoord.x * tcposition[0];\n"
        "   teposition += gl_TessCoord.y * tcposition[1];\n"
        "   teposition += gl_TessCoord.z * tcposition[2];\n"
        "   tecoord = teposition.xy;\n"
        "   vec3 offset = texture(displacement, tecoord).xyz;\n"
        "   teposition.xyz = offset;\n"
        "   gl_Position = ViewProjection*teposition;\n"
        "}\n";
        
    std::string fragment_source =
        "#version 400\n"
        "uniform vec3 ViewPosition;\n"
        "uniform sampler2D displacement;\n"
        "in vec4 teposition;\n"
        "in vec2 tecoord;\n"
        "layout(location = 0) out vec4 FragColor;\n"
        "void main() {\n"
        "   vec3 x = textureOffset(displacement, tecoord, ivec2(0,0)).xyz;\n"
        "   vec3 t0 = x-textureOffset(displacement, tecoord, ivec2(1,0)).xyz;\n"
        "   vec3 t1 = x-textureOffset(displacement, tecoord, ivec2(0,1)).xyz;\n"
        "   vec3 normal = (gl_FrontFacing?1:-1)*normalize(cross(t0, t1));\n"
        "   vec3 light = normalize(vec3(2, -1, 3));\n"
        "   vec3 reflected = reflect(normalize(ViewPosition-teposition.xyz), normal);\n"
        "   float ambient = 0.1;\n"
        "   float diffuse = max(0,dot(normal, light));\n"
        "   float specular = pow(max(0,dot(reflected, light)), 64);\n"
        "   FragColor = vec4(vec3(ambient + 0.5*diffuse + 0.4*specular), 1);\n"
        "}\n";

    // program and shader handles
    GLuint shader_program, vertex_shader, tess_control_shader, tess_eval_shader, fragment_shader;
    
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

    // create and compiler tesselation control shader
    tess_control_shader = glCreateShader(GL_TESS_CONTROL_SHADER);
    source = tess_control_source.c_str();
    length = tess_control_source.size();
    glShaderSource(tess_control_shader, 1, &source, &length); 
    glCompileShader(tess_control_shader);
    if(!check_shader_compile_status(tess_control_shader)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
    
    // create and compiler tesselation evaluation shader
    tess_eval_shader = glCreateShader(GL_TESS_EVALUATION_SHADER);
    source = tess_eval_source.c_str();
    length = tess_eval_source.size();
    glShaderSource(tess_eval_shader, 1, &source, &length); 
    glCompileShader(tess_eval_shader);
    if(!check_shader_compile_status(tess_eval_shader)) {
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
    glAttachShader(shader_program, tess_control_shader);
    glAttachShader(shader_program, tess_eval_shader);
    glAttachShader(shader_program, fragment_shader);
    
    // link the program and check for errors
    glLinkProgram(shader_program);
    check_program_link_status(shader_program);

    GLint width_Location = glGetUniformLocation(shader_program, "width");
    GLint height_Location = glGetUniformLocation(shader_program, "height");
    GLint ViewProjection_Location = glGetUniformLocation(shader_program, "ViewProjection");
    GLint ViewPosition_Location = glGetUniformLocation(shader_program, "ViewPosition");
    GLint displacement_Location = glGetUniformLocation(shader_program, "displacement");
    GLint tess_scale_Location = glGetUniformLocation(shader_program, "tess_scale");


    int terrainwidth = 1024, terrainheight = 1024;
    std::vector<glm::vec3> displacementData(terrainwidth*terrainheight);
    
    glm::vec3 layernorm = glm::normalize(glm::vec3(0.1f,0.3f,1.0f));
    glm::vec3 layerdir(0,0,1);
    layerdir -= layernorm*glm::dot(layernorm, layerdir);
    layerdir = glm::normalize(layerdir);
    
    for(int y = 0;y<terrainheight;++y) {
        for(int x = 0;x<terrainwidth;++x) {
            glm::vec2 pos(float(x)/terrainwidth,float(y)/terrainheight);
            glm::vec3 tmp = glm::vec3( pos, 0.15f*glm::perlin(5.0f*pos));
            displacementData[y*terrainwidth+x] = tmp + 0.04f*layerdir*glm::perlin(glm::vec2(30.0f*glm::dot(layernorm, tmp), 0.5f));
        }
    }

     // texture handle
    GLuint displacement;
    
    // generate texture
    glGenTextures(1, &displacement);

    // bind the texture
    glBindTexture(GL_TEXTURE_2D, displacement);
    
    // set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // set texture content
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, terrainwidth, terrainheight, 0, GL_RGB, GL_FLOAT, &displacementData[0]);

    // camera position and orientation
    glm::vec3 position;
    glm::mat4 rotation = glm::mat4(1.0f);
    
    float t = glfwGetTime();
    bool tessellation = true;
    bool space_down = false;
    
    glEnable(GL_DEPTH_TEST);
    
    // disable mouse cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // mouse position
    double mousex, mousey;
    glfwGetCursorPos(window, &mousex, &mousey);
    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
            
           // calculate timestep
        float new_t = glfwGetTime();
        float dt = new_t - t;
        t = new_t;

        // update mouse differential
        double tmpx, tmpy;
        glfwGetCursorPos(window, &tmpx, &tmpy);
        glm::vec2 mousediff(tmpx-mousex, tmpy-mousey);
        mousex = tmpx;
        mousey = tmpy;
        
        // find up, forward and right vector
        glm::mat3 rotation3(rotation);
        glm::vec3 up = glm::transpose(rotation3)*glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 right = glm::transpose(rotation3)*glm::vec3(1.0f, 0.0f, 0.0f);
        glm::vec3 forward = glm::transpose(rotation3)*glm::vec3(0.0f, 0.0f,-1.0f);
        
        // apply mouse rotation
        rotation = glm::rotate(rotation,  0.2f*mousediff.x, up);
        rotation = glm::rotate(rotation,  0.2f*mousediff.y, right);
        
        // roll
        if(glfwGetKey(window, 'Q')) {
            rotation = glm::rotate(rotation, 180.0f*dt, forward); 
        }  
        if(glfwGetKey(window, 'E')) {
            rotation = glm::rotate(rotation,-180.0f*dt, forward); 
        }
        
        float speed = 0.1f;
        // movement
        if(glfwGetKey(window, 'W')) {
            position += speed*dt*forward; 
        }  
        if(glfwGetKey(window, 'S')) {
            position -= speed*dt*forward;
        }
        if(glfwGetKey(window, 'D')) {
            position += speed*dt*right; 
        }  
        if(glfwGetKey(window, 'A')) {
            position -= speed*dt*right;
        }

        if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        
        // toggle tesselation
        if(glfwGetKey(window, GLFW_KEY_SPACE) && !space_down) {
            tessellation = !tessellation;
        }
        space_down = glfwGetKey(window, GLFW_KEY_SPACE);
        
        // calculate ViewProjection matrix
        glm::mat4 Projection = glm::perspective(60.0f, float(width) / height, 0.001f, 10.f);
        glm::mat4 View = rotation*glm::translate(glm::mat4(1.0f), -position);
        glm::mat4 ViewProjection = Projection*View;
        
        // clear first
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, displacement);
        
        // use the shader program
        glUseProgram(shader_program);
        glUniform1ui(width_Location, 64); // 64x64 base grid without tessellation
        glUniform1ui(height_Location, 64);
        glUniformMatrix4fv(ViewProjection_Location, 1, GL_FALSE, glm::value_ptr(ViewProjection));
        glUniform3fv(ViewPosition_Location, 1, glm::value_ptr(position));
        
        if(tessellation) {
            glUniform1f(tess_scale_Location, 1.0f);
        } else {
            glUniform1f(tess_scale_Location, 0.0f);
        }
        
        // set texture uniform
        glUniform1i(displacement_Location, 0);
        
        // draw
        glDrawArraysInstanced(GL_PATCHES, 0, 6, 64*64);
        
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
    glDetachShader(shader_program, vertex_shader);
    glDetachShader(shader_program, tess_control_shader);
    glDetachShader(shader_program, tess_eval_shader);
    glDetachShader(shader_program, fragment_shader);
    glDeleteShader(vertex_shader);
    glDeleteShader(tess_control_shader);
    glDeleteShader(tess_eval_shader);
    glDeleteShader(fragment_shader);
    glDeleteProgram(shader_program);
    
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

