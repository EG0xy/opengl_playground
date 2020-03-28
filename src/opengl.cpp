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

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#undef assert


#include "iml_general.h"
#include "iml_types.h"


#include "shader.h"
#include "camera.h"


#define SCREEN_WIDTH   800
#define SCREEN_HEIGHT  600


global b32 first_mouse = true;
global f32 last_mouse_x = SCREEN_WIDTH / 2.0f;
global f32 last_mouse_y = SCREEN_HEIGHT / 2.0f;

global f32 dt = 0.0f;
global f32 last_frame = 0.0f;

global Camera camera = make_camera(glm::vec3(0.0f, 0.0f, 3.0f));
global glm::vec3 light_pos(1.2f, 1.0f, 2.0f);


void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

void process_input(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        process_keyboard(&camera, FORWARD, dt);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        process_keyboard(&camera, BACKWARD, dt);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        process_keyboard(&camera, LEFT, dt);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        process_keyboard(&camera, RIGHT, dt);
    }
}

void
mouse_callback(GLFWwindow *window, double pos_x, double pos_y) {
    if (first_mouse) {
        last_mouse_x = pos_x;
        last_mouse_y = pos_y;
        first_mouse = false;
    }
    
    f32 offset_x = pos_x - last_mouse_x;
    f32 offset_y = last_mouse_y - pos_y; // Reversed since y-coordinates go from botom to top
    last_mouse_x = pos_x;
    last_mouse_y = pos_y;
    
    process_mouse_movement(&camera, offset_x, offset_y);
}

void
scroll_callback(GLFWwindow *window, double offset_x, double offset_y) {
    process_mouse_scroll(&camera, offset_y);
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
    
    GLFWwindow *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OpenGL Playground", NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        return -1;
    }
    
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    
    glEnable(GL_DEPTH_TEST);
    
    // @note: Vertex shader
    const char *lighting_vs_source =
#include "lighting_vs.glsl"
    ;
    // @note: Fragment shader
    const char *lighting_fs_source =
#include "lighting_fs.glsl"
    ;
    Shader lighting_shader = create_shader(lighting_vs_source, lighting_fs_source);
    
    // @note: Vertex shader
    const char *lamp_vs_source =
#include "lamp_vs.glsl"
    ;
    // @note: Fragment shader
    const char *lamp_fs_source =
#include "lamp_fs.glsl"
    ;
    Shader lamp_shader = create_shader(lamp_vs_source, lamp_fs_source);
    
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
        
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
        
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
        
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };
    
    // first, configure the cube's VAO (and vbo)
    unsigned int vbo, cube_vao;
    glGenVertexArrays(1, &cube_vao);
    glGenBuffers(1, &vbo);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindVertexArray(cube_vao);
    
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(f32)));
    glEnableVertexAttribArray(1);
    
    // second, configure the light's VAO (vbo stays the same; the vertices are the same for the light object which is also a 3D cube)
    unsigned int light_vao;
    glGenVertexArrays(1, &light_vao);
    glBindVertexArray(light_vao);
    
    // we only need to bind to the vbo (to link it with glVertexAttribPointer), no need to fill it; the vbo's data already contains all we need (it's already bound, but we do it again for educational purposes)
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    
    //
    // @note: Wireframe
    //
#if 0
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#else
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif
    
    //
    // @note: Textures
    //
    u32 texture1;
    // @note: Generating a texture
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    
    {
        int width, height, nr_channels;
        u8 *data = stbi_load("../assets/container.jpg", &width, &height, &nr_channels, 0);
        defer {
            stbi_image_free(data);
        };
        
        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
    }
    u32 texture2;
    // @note: Generating a texture
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    {
        int width, height, nr_channels;
        stbi_set_flip_vertically_on_load(true);
        u8 *data = stbi_load("../assets/awesomeface.png", &width, &height, &nr_channels, 0);
        defer {
            stbi_image_free(data);
        };
        
        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
    }
    
#if 0
    use_shader(shader);
    glUniform1i(glGetUniformLocation(shader.id, "texture1"), 0);
    glUniform1i(glGetUniformLocation(shader.id, "texture2"), 1);
#endif
    
    
    //
    // @note: Texture Wrapping
    //
#if 0
    // GL_REPEAT   default
    // GL_MIRRORED_REPEAT
    // GL_CLAMP_TO_EDGE
    // GL_CLAMP_TO_BORDER
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    f32 border_color[] = { 1.0f, 1.0f, 0.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);
#endif
    
    //
    // @note: Texture Filtering
    //
#if 0
    // GL_NEAREST = best for pixel art
    // GL_LINEAR  = (bi)linear
    //
    // Can be set for minifying and magnifying
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#endif
    
    //
    // @note: Mipmaps
    //
#if 0
    // GL_NEAREST_MIPMAP_NEAREST
    // GL_LINEAR_MIPMAP_NEAREST
    // GL_NEAREST_MIPMAP_LINEAR
    // GL_LINEAR_MIPMAP_LINEAR
    // These are only used for downscaling!
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#endif
    
    
    while (!glfwWindowShouldClose(window)) {
        // @note: Handle events / input
        glfwPollEvents();
        process_input(window);
        
        // @note: Update and Render
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        use_shader(lighting_shader);
        set_uniform(lighting_shader, "material.ambient",   1.0f, 0.5f, 0.31f);
        set_uniform(lighting_shader, "material.diffuse",   1.0f, 0.5f, 0.31f);
        set_uniform(lighting_shader, "material.specular",  1.0f, 1.0f, 1.0f);
        set_uniform(lighting_shader, "material.shininess", 32.0f);
        set_uniform(lighting_shader, "light_color",  1.0f, 1.0f, 1.0f);
        set_uniform(lighting_shader, "light_pos",    light_pos);
        set_uniform(lighting_shader, "view_pos",     camera.position);
        
        // @note: Draw color cube
        glm::mat4 view_matrix = get_view_matrix(&camera);
        glm::mat4 projection_matrix = glm::perspective(glm::radians(camera.fov), (f32)SCREEN_WIDTH / (f32)SCREEN_HEIGHT, 0.1f, 100.0f);
        set_uniform(lighting_shader, "view_matrix", view_matrix);
        set_uniform(lighting_shader, "projection_matrix", projection_matrix);
        
        glm::mat4 model_matrix = glm::mat4(1.0f);
        set_uniform(lighting_shader, "model_matrix", model_matrix);
        
        glBindVertexArray(cube_vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        // @note: Draw lamp
        use_shader(lamp_shader);
        set_uniform(lamp_shader, "view_matrix", view_matrix);
        set_uniform(lamp_shader, "projection_matrix", projection_matrix);
        model_matrix = glm::mat4(1.0f);
        model_matrix = glm::translate(model_matrix, light_pos);
        model_matrix = glm::scale(model_matrix, glm::vec3(0.2f));
        set_uniform(lamp_shader, "model_matrix", model_matrix);
        glBindVertexArray(light_vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        glfwSwapBuffers(window);
        
        
        f32 current_frame = glfwGetTime();
        dt = current_frame - last_frame;
        last_frame = current_frame;
    }
    
    
    return 0;
}
