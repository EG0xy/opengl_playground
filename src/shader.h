#ifndef SHADER_H


#include <glad/glad.h>


struct Shader {
    u32 id;
};

// @todo Load from a file path
Shader
create_shader(const char *vertex_shader_source, const char *fragment_shader_source) {
    Shader shader = {};
    
    b32 success;
    char info_log[512];
    
    u32 vertex_shader, fragment_shader;
    defer {
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
    };
    
    // @note: Vertex shader
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
        printf("Error: Vertex shader compilation failed\nInfoLog:\n%s", info_log);
    }
    
    // @note: Fragment shader
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
        printf("Error: Fragment shader compilation failed\nInfoLog:\n%s", info_log);
    }
    
    // @note: Shader program
    shader.id = glCreateProgram();
    glAttachShader(shader.id, vertex_shader);
    glAttachShader(shader.id, fragment_shader);
    glLinkProgram(shader.id);
    glGetProgramiv(shader.id, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader.id, 512, NULL, info_log);
        printf("Error: Shader program linking failed\nInfoLog:\n%s", info_log);
    }
    
    return shader;
}

void
use_shader(Shader shader) {
    glUseProgram(shader.id);
}

void
set_uniform(Shader shader, const char *name, bool value) {
    glUniform1i(glGetUniformLocation(shader.id, name), (int)value);
}

void
set_uniform(Shader shader, const char *name, int value) {
    glUniform1i(glGetUniformLocation(shader.id, name), value);
}

void
set_uniform(Shader shader, const char *name, f32 value) {
    glUniform1f(glGetUniformLocation(shader.id, name), value);
}


#define SHADER_H
#endif//SHADER_H
