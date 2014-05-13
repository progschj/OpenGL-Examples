/* OpenGL example code - fbo & fxaa
 *
 * render the cube from the perspective example to a texture and
 * apply fxaa antialiasing to it.
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
    if((window = glfwCreateWindow(width, height, "05fbo_fxaa", 0, 0)) == 0) {
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
        "uniform mat4 ViewProjection;\n" // the projection matrix uniform
        "layout(location = 0) in vec4 vposition;\n"
        "layout(location = 1) in vec4 vcolor;\n"
        "out vec4 fcolor;\n"
        "void main() {\n"
        "   fcolor = vcolor;\n"
        "   gl_Position = ViewProjection*vposition;\n"
        "}\n";

    std::string fragment_source =
        "#version 330\n"
        "in vec4 fcolor;\n"
        "layout(location = 0) out vec4 FragColor;\n"
        "void main() {\n"
        "   FragColor = fcolor;\n"
        // the following line is required for fxaa (will not work with blending!)
        "   FragColor.a = dot(fcolor.rgb, vec3(0.299, 0.587, 0.114));\n"
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

    // obtain location of projection uniform
    GLint ViewProjection_location = glGetUniformLocation(shader_program, "ViewProjection");


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

    // "unbind" vao
    glBindVertexArray(0);

    // shader source code
    std::string post_effect_vertex_source =
        "#version 330\n"
        "layout(location = 0) in vec4 vposition;\n"
        "layout(location = 1) in vec2 vtexcoord;\n"
        "out vec2 ftexcoord;\n"
        "void main() {\n"
        "   ftexcoord = vtexcoord;\n"
        "   gl_Position = vposition;\n"
        "}\n";

    // this is a Timothy Lottes FXAA 3.11
    // check out the following link for detailed information:
    // http://timothylottes.blogspot.ch/2011/07/fxaa-311-released.html
    //
    // the shader source has been stripped with a preprocessor for
    // brevity reasons (it's still pretty long for inlining...).
    // the used defines are:
    // #define FXAA_PC 1
    // #define FXAA_GLSL_130 1
    // #define FXAA_QUALITY__PRESET 13

    std::string post_effect_fragment_source =
        "#version 330\n"
        "uniform sampler2D intexture;\n"
        "in vec2 ftexcoord;\n"
        "layout(location = 0) out vec4 FragColor;\n"
        "\n"
        "float FxaaLuma(vec4 rgba) {\n"
        "    return rgba.w;\n"
        "}\n"
        "\n"
        "vec4 FxaaPixelShader(\n"
        "    vec2 pos,\n"
        "    sampler2D tex,\n"
        "    vec2 fxaaQualityRcpFrame,\n"
        "    float fxaaQualitySubpix,\n"
        "    float fxaaQualityEdgeThreshold,\n"
        "    float fxaaQualityEdgeThresholdMin\n"
        ") {\n"
        "    vec2 posM;\n"
        "    posM.x = pos.x;\n"
        "    posM.y = pos.y;\n"
        "    vec4 rgbyM = textureLod(tex, posM, 0.0);\n"
        "    float lumaS = FxaaLuma(textureLodOffset(tex, posM, 0.0, ivec2( 0, 1)));\n"
        "    float lumaE = FxaaLuma(textureLodOffset(tex, posM, 0.0, ivec2( 1, 0)));\n"
        "    float lumaN = FxaaLuma(textureLodOffset(tex, posM, 0.0, ivec2( 0,-1)));\n"
        "    float lumaW = FxaaLuma(textureLodOffset(tex, posM, 0.0, ivec2(-1, 0)));\n"
        "    float maxSM = max(lumaS, rgbyM.w);\n"
        "    float minSM = min(lumaS, rgbyM.w);\n"
        "    float maxESM = max(lumaE, maxSM);\n"
        "    float minESM = min(lumaE, minSM);\n"
        "    float maxWN = max(lumaN, lumaW);\n"
        "    float minWN = min(lumaN, lumaW);\n"
        "    float rangeMax = max(maxWN, maxESM);\n"
        "    float rangeMin = min(minWN, minESM);\n"
        "    float rangeMaxScaled = rangeMax * fxaaQualityEdgeThreshold;\n"
        "    float range = rangeMax - rangeMin;\n"
        "    float rangeMaxClamped = max(fxaaQualityEdgeThresholdMin, rangeMaxScaled);\n"
        "    bool earlyExit = range < rangeMaxClamped;\n"
        "    if(earlyExit)\n"
        "        return rgbyM;\n"
        "\n"
        "    float lumaNW = FxaaLuma(textureLodOffset(tex, posM, 0.0, ivec2(-1,-1)));\n"
        "    float lumaSE = FxaaLuma(textureLodOffset(tex, posM, 0.0, ivec2( 1, 1)));\n"
        "    float lumaNE = FxaaLuma(textureLodOffset(tex, posM, 0.0, ivec2( 1,-1)));\n"
        "    float lumaSW = FxaaLuma(textureLodOffset(tex, posM, 0.0, ivec2(-1, 1)));\n"
        "    float lumaNS = lumaN + lumaS;\n"
        "    float lumaWE = lumaW + lumaE;\n"
        "    float subpixRcpRange = 1.0/range;\n"
        "    float subpixNSWE = lumaNS + lumaWE;\n"
        "    float edgeHorz1 = (-2.0 * rgbyM.w) + lumaNS;\n"
        "    float edgeVert1 = (-2.0 * rgbyM.w) + lumaWE;\n"
        "    float lumaNESE = lumaNE + lumaSE;\n"
        "    float lumaNWNE = lumaNW + lumaNE;\n"
        "    float edgeHorz2 = (-2.0 * lumaE) + lumaNESE;\n"
        "    float edgeVert2 = (-2.0 * lumaN) + lumaNWNE;\n"
        "    float lumaNWSW = lumaNW + lumaSW;\n"
        "    float lumaSWSE = lumaSW + lumaSE;\n"
        "    float edgeHorz4 = (abs(edgeHorz1) * 2.0) + abs(edgeHorz2);\n"
        "    float edgeVert4 = (abs(edgeVert1) * 2.0) + abs(edgeVert2);\n"
        "    float edgeHorz3 = (-2.0 * lumaW) + lumaNWSW;\n"
        "    float edgeVert3 = (-2.0 * lumaS) + lumaSWSE;\n"
        "    float edgeHorz = abs(edgeHorz3) + edgeHorz4;\n"
        "    float edgeVert = abs(edgeVert3) + edgeVert4;\n"
        "    float subpixNWSWNESE = lumaNWSW + lumaNESE;\n"
        "    float lengthSign = fxaaQualityRcpFrame.x;\n"
        "    bool horzSpan = edgeHorz >= edgeVert;\n"
        "    float subpixA = subpixNSWE * 2.0 + subpixNWSWNESE;\n"
        "    if(!horzSpan) lumaN = lumaW;\n"
        "    if(!horzSpan) lumaS = lumaE;\n"
        "    if(horzSpan) lengthSign = fxaaQualityRcpFrame.y;\n"
        "    float subpixB = (subpixA * (1.0/12.0)) - rgbyM.w;\n"
        "    float gradientN = lumaN - rgbyM.w;\n"
        "    float gradientS = lumaS - rgbyM.w;\n"
        "    float lumaNN = lumaN + rgbyM.w;\n"
        "    float lumaSS = lumaS + rgbyM.w;\n"
        "    bool pairN = abs(gradientN) >= abs(gradientS);\n"
        "    float gradient = max(abs(gradientN), abs(gradientS));\n"
        "    if(pairN) lengthSign = -lengthSign;\n"
        "    float subpixC = clamp(abs(subpixB) * subpixRcpRange, 0.0, 1.0);\n"
        "    vec2 posB;\n"
        "    posB.x = posM.x;\n"
        "    posB.y = posM.y;\n"
        "    vec2 offNP;\n"
        "    offNP.x = (!horzSpan) ? 0.0 : fxaaQualityRcpFrame.x;\n"
        "    offNP.y = ( horzSpan) ? 0.0 : fxaaQualityRcpFrame.y;\n"
        "    if(!horzSpan) posB.x += lengthSign * 0.5;\n"
        "    if( horzSpan) posB.y += lengthSign * 0.5;\n"
        "    vec2 posN;\n"
        "    posN.x = posB.x - offNP.x * 1.0;\n"
        "    posN.y = posB.y - offNP.y * 1.0;\n"
        "    vec2 posP;\n"
        "    posP.x = posB.x + offNP.x * 1.0;\n"
        "    posP.y = posB.y + offNP.y * 1.0;\n"
        "    float subpixD = ((-2.0)*subpixC) + 3.0;\n"
        "    float lumaEndN = FxaaLuma(textureLod(tex, posN, 0.0));\n"
        "    float subpixE = subpixC * subpixC;\n"
        "    float lumaEndP = FxaaLuma(textureLod(tex, posP, 0.0));\n"
        "    if(!pairN) lumaNN = lumaSS;\n"
        "    float gradientScaled = gradient * 1.0/4.0;\n"
        "    float lumaMM = rgbyM.w - lumaNN * 0.5;\n"
        "    float subpixF = subpixD * subpixE;\n"
        "    bool lumaMLTZero = lumaMM < 0.0;\n"
        "    lumaEndN -= lumaNN * 0.5;\n"
        "    lumaEndP -= lumaNN * 0.5;\n"
        "    bool doneN = abs(lumaEndN) >= gradientScaled;\n"
        "    bool doneP = abs(lumaEndP) >= gradientScaled;\n"
        "    if(!doneN) posN.x -= offNP.x * 1.5;\n"
        "    if(!doneN) posN.y -= offNP.y * 1.5;\n"
        "    bool doneNP = (!doneN) || (!doneP);\n"
        "    if(!doneP) posP.x += offNP.x * 1.5;\n"
        "    if(!doneP) posP.y += offNP.y * 1.5;\n"
        "    if(doneNP) {\n"
        "        if(!doneN) lumaEndN = FxaaLuma(textureLod(tex, posN.xy, 0.0));\n"
        "        if(!doneP) lumaEndP = FxaaLuma(textureLod(tex, posP.xy, 0.0));\n"
        "        if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"
        "        if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"
        "        doneN = abs(lumaEndN) >= gradientScaled;\n"
        "        doneP = abs(lumaEndP) >= gradientScaled;\n"
        "        if(!doneN) posN.x -= offNP.x * 2.0;\n"
        "        if(!doneN) posN.y -= offNP.y * 2.0;\n"
        "        doneNP = (!doneN) || (!doneP);\n"
        "        if(!doneP) posP.x += offNP.x * 2.0;\n"
        "        if(!doneP) posP.y += offNP.y * 2.0;\n"
        "        if(doneNP) {\n"
        "            if(!doneN) lumaEndN = FxaaLuma(textureLod(tex, posN.xy, 0.0));\n"
        "            if(!doneP) lumaEndP = FxaaLuma(textureLod(tex, posP.xy, 0.0));\n"
        "            if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"
        "            if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"
        "            doneN = abs(lumaEndN) >= gradientScaled;\n"
        "            doneP = abs(lumaEndP) >= gradientScaled;\n"
        "            if(!doneN) posN.x -= offNP.x * 2.0;\n"
        "            if(!doneN) posN.y -= offNP.y * 2.0;\n"
        "            doneNP = (!doneN) || (!doneP);\n"
        "            if(!doneP) posP.x += offNP.x * 2.0;\n"
        "            if(!doneP) posP.y += offNP.y * 2.0;\n"
        "            if(doneNP) {\n"
        "                if(!doneN) lumaEndN = FxaaLuma(textureLod(tex, posN.xy, 0.0));\n"
        "                if(!doneP) lumaEndP = FxaaLuma(textureLod(tex, posP.xy, 0.0));\n"
        "                if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"
        "                if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"
        "                doneN = abs(lumaEndN) >= gradientScaled;\n"
        "                doneP = abs(lumaEndP) >= gradientScaled;\n"
        "                if(!doneN) posN.x -= offNP.x * 4.0;\n"
        "                if(!doneN) posN.y -= offNP.y * 4.0;\n"
        "                doneNP = (!doneN) || (!doneP);\n"
        "                if(!doneP) posP.x += offNP.x * 4.0;\n"
        "                if(!doneP) posP.y += offNP.y * 4.0;\n"
        "                if(doneNP) {\n"
        "                    if(!doneN) lumaEndN = FxaaLuma(textureLod(tex, posN.xy, 0.0));\n"
        "                    if(!doneP) lumaEndP = FxaaLuma(textureLod(tex, posP.xy, 0.0));\n"
        "                    if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;\n"
        "                    if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;\n"
        "                    doneN = abs(lumaEndN) >= gradientScaled;\n"
        "                    doneP = abs(lumaEndP) >= gradientScaled;\n"
        "                    if(!doneN) posN.x -= offNP.x * 12.0;\n"
        "                    if(!doneN) posN.y -= offNP.y * 12.0;\n"
        "                    doneNP = (!doneN) || (!doneP);\n"
        "                    if(!doneP) posP.x += offNP.x * 12.0;\n"
        "                    if(!doneP) posP.y += offNP.y * 12.0;\n"
        "                }\n"
        "            }\n"
        "        }\n"
        "    }\n"
        "\n"
        "    float dstN = posM.x - posN.x;\n"
        "    float dstP = posP.x - posM.x;\n"
        "    if(!horzSpan) dstN = posM.y - posN.y;\n"
        "    if(!horzSpan) dstP = posP.y - posM.y;\n"
        "\n"
        "    bool goodSpanN = (lumaEndN < 0.0) != lumaMLTZero;\n"
        "    float spanLength = (dstP + dstN);\n"
        "    bool goodSpanP = (lumaEndP < 0.0) != lumaMLTZero;\n"
        "    float spanLengthRcp = 1.0/spanLength;\n"
        "\n"
        "    bool directionN = dstN < dstP;\n"
        "    float dst = min(dstN, dstP);\n"
        "    bool goodSpan = directionN ? goodSpanN : goodSpanP;\n"
        "    float subpixG = subpixF * subpixF;\n"
        "    float pixelOffset = (dst * (-spanLengthRcp)) + 0.5;\n"
        "    float subpixH = subpixG * fxaaQualitySubpix;\n"
        "\n"
        "    float pixelOffsetGood = goodSpan ? pixelOffset : 0.0;\n"
        "    float pixelOffsetSubpix = max(pixelOffsetGood, subpixH);\n"
        "    if(!horzSpan) posM.x += pixelOffsetSubpix * lengthSign;\n"
        "    if( horzSpan) posM.y += pixelOffsetSubpix * lengthSign;\n"
        "    \n"
        "    return vec4(textureLod(tex, posM, 0.0).xyz, rgbyM.w);\n"
        "}\n"
        "\n"
        "void main() {    \n"
        "    FragColor = FxaaPixelShader(\n"
        "                    ftexcoord,\n"
        "                    intexture,\n"
        "                    1.0/textureSize(intexture,0),\n"
        "                    0.75,\n"
        "                    0.166,\n"
        "                    0.0625\n"
        "                );\n"
        "}\n";

    // program and shader handles
    GLuint post_effect_shader_program, post_effect_vertex_shader, post_effect_fragment_shader;

    // create and compiler vertex shader
    post_effect_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    source = post_effect_vertex_source.c_str();
    length = post_effect_vertex_source.size();
    glShaderSource(post_effect_vertex_shader, 1, &source, &length);
    glCompileShader(post_effect_vertex_shader);
    if(!check_shader_compile_status(post_effect_vertex_shader))
    {
        return 1;
    }

    // create and compiler fragment shader
    post_effect_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    source = post_effect_fragment_source.c_str();
    length = post_effect_fragment_source.size();
    glShaderSource(post_effect_fragment_shader, 1, &source, &length);
    glCompileShader(post_effect_fragment_shader);
    if(!check_shader_compile_status(post_effect_fragment_shader))
    {
        return 1;
    }

    // create program
    post_effect_shader_program = glCreateProgram();

    // attach shaders
    glAttachShader(post_effect_shader_program, post_effect_vertex_shader);
    glAttachShader(post_effect_shader_program, post_effect_fragment_shader);

    // link the program and check for errors
    glLinkProgram(post_effect_shader_program);
    check_program_link_status(post_effect_shader_program);

    // get texture uniform location
    GLint post_effect_texture_location = glGetUniformLocation(post_effect_shader_program, "intexture");

    // vao and vbo handle
    GLuint post_effect_vao, post_effect_vbo, post_effect_ibo;

    // generate and bind the vao
    glGenVertexArrays(1, &post_effect_vao);
    glBindVertexArray(post_effect_vao);

    // generate and bind the vertex buffer object
    glGenBuffers(1, &post_effect_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, post_effect_vbo);

    // data for a fullscreen quad (this time with texture coords)
    GLfloat post_effect_vertexData[] = {
    //  X     Y     Z           U     V
       1.0f, 1.0f, 0.0f,       1.0f, 1.0f, // vertex 0
      -1.0f, 1.0f, 0.0f,       0.0f, 1.0f, // vertex 1
       1.0f,-1.0f, 0.0f,       1.0f, 0.0f, // vertex 2
      -1.0f,-1.0f, 0.0f,       0.0f, 0.0f, // vertex 3
    }; // 4 vertices with 5 components (floats) each

    // fill with data
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*4*5, post_effect_vertexData, GL_STATIC_DRAW);


    // set up generic attrib pointers
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (char*)0 + 0*sizeof(GLfloat));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (char*)0 + 3*sizeof(GLfloat));


    // generate and bind the index buffer object
    glGenBuffers(1, &post_effect_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, post_effect_ibo);

    GLuint post_effect_indexData[] = {
        0,1,2, // first triangle
        2,1,3, // second triangle
    };

    // fill with data
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*2*3, post_effect_indexData, GL_STATIC_DRAW);

    // "unbind" vao
    glBindVertexArray(0);

    // texture handle
    GLuint texture;

    // generate texture
    glGenTextures(1, &texture);

    // bind the texture
    glBindTexture(GL_TEXTURE_2D, texture);

    // set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // set texture content
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);


    // renderbuffer handle
    GLuint rbf;

    // generate renderbuffers
    glGenRenderbuffers(1, &rbf);

    glBindRenderbuffer(GL_RENDERBUFFER, rbf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);

    // framebuffer handle
    GLuint fbo;

    // generate framebuffer
    glGenFramebuffers(1, &fbo);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbf);


    bool fxaa = true;
    bool space_down = false;
    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // get the time in seconds
        float t = glfwGetTime();

        // toggle fxaa on/off with space
        if(glfwGetKey(window, GLFW_KEY_SPACE) && !space_down)
        {
            fxaa = !fxaa;
        }
        space_down = glfwGetKey(window, GLFW_KEY_SPACE);

        glEnable(GL_DEPTH_TEST);

        // bind target framebuffer
        if(fxaa)
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        else
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

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

        // set the uniform
        glUniformMatrix4fv(ViewProjection_location, 1, GL_FALSE, glm::value_ptr(ViewProjection));

        // bind the vao
        glBindVertexArray(vao);

        // draw
        glDrawElements(GL_TRIANGLES, 6*6, GL_UNSIGNED_INT, 0);

        // apply post processing only when fxaa is on
        if(fxaa)
        {
            // bind the "screen frambuffer"
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // we are not 3d rendering so no depth test
            glDisable(GL_DEPTH_TEST);

            // use the shader program
            glUseProgram(post_effect_shader_program);

            // bind texture to texture unit 0
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture);

            // set uniforms
            glUniform1i(post_effect_texture_location, 0);

            // bind the vao
            glBindVertexArray(post_effect_vao);

            // draw
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }

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

    glDeleteVertexArrays(1, &post_effect_vao);
    glDeleteBuffers(1, &post_effect_vbo);
    glDeleteBuffers(1, &post_effect_ibo);

    glDetachShader(post_effect_shader_program, post_effect_vertex_shader);
    glDetachShader(post_effect_shader_program, post_effect_fragment_shader);
    glDeleteShader(post_effect_vertex_shader);
    glDeleteShader(post_effect_fragment_shader);
    glDeleteProgram(post_effect_shader_program);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

