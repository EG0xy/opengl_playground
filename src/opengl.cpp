#include <iostream>
// #define _ALLOW_KEYWORD_MACROS

#pragma comment(lib, "user32")
#pragma comment(lib, "gdi32")
#pragma comment(lib, "winmm")
// #define WIN32_LEAN_AND_MEAN
#include <windows.h>
// #include <mmsystem.h> // @note excluded in lean and mean, used for timeBeginPeriod to set scheduler granularity

#pragma comment(lib, "opengl32")
// #include <gl/gl.h>
// #pragma comment(lib, "glu32")
// #include <gl/glu.h>


#include <glad/glad.h>
#pragma comment(lib, "glfw3")
#include <GLFW/glfw3.h>


#include "iml_general.h"
#include "iml_types.h"


void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

void process_input(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}


int main() {
    glfwInit();
    defer {
        glfwTerminate();
    };
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    GLFWwindow *window = glfwCreateWindow(800, 600, "OpenGL Playground", NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        return -1;
    }
    
    glViewport(0, 0, 800, 600);
    
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    
    u32 vao; // @note Vertex array object
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    
#if 1
    // @note: Draw Triangle
    // Normalized Device Coordinates (NDC)
    f32 vertices[] = {
        -0.5, -0.5,  0.0f, // Vertex containing one 3D position
        0.5f, -0.5f, 0.0f,
        0.0f,  0.5f, 0.0f,
        
        0.0f, 0.0f,  0.0f,
        0.5f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.0f
    };
#else
    // @note: Draw Rectangle
    f32 vertices[] = {
        0.5f,   0.5f, 0.0f, // top right
        0.5f,  -0.5f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f, // bottom left
        -0.5f,  0.5f, 0.0f, // top left
    };
#endif
    
    u32 vbo; // @note Vertex buffer object
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    u32 indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };
    
    u32 ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    b32 success;
    char info_log[512];
    
    // @note: Vertex shader
    const char *vertex_shader_source = R"FOO(
                                                                                                                #version 330 core
                                                                                                                layout (location = 0) in vec3 aPos;
                                                                                                                
                                                                                                                void main() {
                                                                                                                    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
                                                                                                                }
                                                                                                                                                                                                        )FOO";
    u32 vertex_shader;
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
        printf("Error: Vertex shader compilation failed\nInfoLog:\n%s", info_log);
    }
    
    // @note: Fragment shader
    const char *fragment_shader_source = R"FOO(
                                                                                                                                                                                                                                                                                                                                        #version 330 core
                                                                                                                                                                                                                                                                                                            out vec4 frag_color;
                                                                                                                                                                                                                                                                                                                                        
                                                                                                                                                                                                                                                                                                                                        void main() {
                                                                                                                                                                                                                                frag_color = vec4(1.0f, 0.5f, 0.2f, 1.0f);
                                                                                                                                                                                                                                                                                                                                        }
                                                                                                                                                                                                                                                                                                                                                                                                                                )FOO";
    u32 fragment_shader;
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
        printf("Error: Fragment shader compilation failed\nInfoLog:\n%s", info_log);
    }
    
    // @note: Shader program
    u32 shader_program;
    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_program, 512, NULL, info_log);
        printf("Error: Shader program linking failed\nInfoLog:\n%s", info_log);
    }
    else {
        glUseProgram(shader_program);
    }
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), (void *)0);
    glEnableVertexAttribArray(0);
    
    
#if 0
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#else
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif
    
    
    while (!glfwWindowShouldClose(window)) {
        // @note: Handle events / input
        glfwPollEvents();
        process_input(window);
        
        // @note: Update and Render
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glUseProgram(shader_program);
        glBindVertexArray(vao);
        defer {
            glBindVertexArray(0);
        };
#if 1
        glDrawArrays(GL_TRIANGLES, 0, 6);
#else
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
#endif
        
        glfwSwapBuffers(window);
    }
    
    
    return 0;
}
