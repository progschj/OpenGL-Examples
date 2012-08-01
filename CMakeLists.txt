cmake_minimum_required(VERSION 2.6)
 
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules")
 
project(OPENGLEXAMPLES)
 
find_package(OpenGL REQUIRED)
find_package(GLFW REQUIRED)
find_package(GLEW REQUIRED)
 
set(CMAKE_CXX_FLAGS "-O2 -Wall -Wextra")

link_directories (${OPENGLEXAMPLES_BINARY_DIR}/bin)
 
add_executable (00skeleton 00skeleton.cpp)
target_link_libraries(00skeleton ${OPENGL_LIBRARY} ${GLFW_LIBRARY} ${GLEW_LIBRARY} )

add_executable (01shader_vbo1 01shader_vbo1.cpp)
target_link_libraries(01shader_vbo1 ${OPENGL_LIBRARY} ${GLFW_LIBRARY} ${GLEW_LIBRARY} )

add_executable (01shader_vbo2 01shader_vbo2.cpp)
target_link_libraries(01shader_vbo2 ${OPENGL_LIBRARY} ${GLFW_LIBRARY} ${GLEW_LIBRARY} )

add_executable (02indexed_vbo 02indexed_vbo.cpp)
target_link_libraries(02indexed_vbo ${OPENGL_LIBRARY} ${GLFW_LIBRARY} ${GLEW_LIBRARY} )

add_executable (03texture 03texture.cpp)
target_link_libraries(03texture ${OPENGL_LIBRARY} ${GLFW_LIBRARY} ${GLEW_LIBRARY} )

add_executable (04perspective 04perspective.cpp)
target_link_libraries(04perspective ${OPENGL_LIBRARY} ${GLFW_LIBRARY} ${GLEW_LIBRARY} )

add_executable (05fbo_fxaa 05fbo_fxaa.cpp)
target_link_libraries(05fbo_fxaa ${OPENGL_LIBRARY} ${GLFW_LIBRARY} ${GLEW_LIBRARY} )

add_executable (06instancing1 06instancing1.cpp)
target_link_libraries(06instancing1 ${OPENGL_LIBRARY} ${GLFW_LIBRARY} ${GLEW_LIBRARY} )

add_executable (06instancing2_buffer_texture 06instancing2_buffer_texture.cpp)
target_link_libraries(06instancing2_buffer_texture ${OPENGL_LIBRARY} ${GLFW_LIBRARY} ${GLEW_LIBRARY} )

add_executable (06instancing3_uniform_buffer 06instancing3_uniform_buffer.cpp)
target_link_libraries(06instancing3_uniform_buffer ${OPENGL_LIBRARY} ${GLFW_LIBRARY} ${GLEW_LIBRARY} )

add_executable (07geometry_shader_blending 07geometry_shader_blending.cpp)
target_link_libraries(07geometry_shader_blending ${OPENGL_LIBRARY} ${GLFW_LIBRARY} ${GLEW_LIBRARY} )

add_executable (08map_buffer 08map_buffer.cpp)
target_link_libraries(08map_buffer ${OPENGL_LIBRARY} ${GLFW_LIBRARY} ${GLEW_LIBRARY} )

add_executable (09transform_feedback 09transform_feedback.cpp)
target_link_libraries(09transform_feedback ${OPENGL_LIBRARY} ${GLFW_LIBRARY} ${GLEW_LIBRARY} )

add_executable (10queries_conditional_render 10queries_conditional_render.cpp)
target_link_libraries(10queries_conditional_render ${OPENGL_LIBRARY} ${GLFW_LIBRARY} ${GLEW_LIBRARY} )

add_executable (11tesselation 11tesselation.cpp)
target_link_libraries(11tesselation ${OPENGL_LIBRARY} ${GLFW_LIBRARY} ${GLEW_LIBRARY} )

add_executable (12shader_image_load_store 12shader_image_load_store.cpp)
target_link_libraries(12shader_image_load_store ${OPENGL_LIBRARY} ${GLFW_LIBRARY} ${GLEW_LIBRARY} )
