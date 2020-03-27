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

global Camera camera = {};


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
    
    u32 vao; // @note Vertex array object
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    
#if 0
    // @note: Draw Triangle
    // Normalized Device Coordinates (NDC)
    f32 vertices[] = {
        // positions         // colors
        -0.5, -0.5,  0.0f,  1.0f, 0.0f, 0.0f, // Vertex containing one 3D position
        0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,
        0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f
            
            /*
                    0.0f, 0.0f,  0.0f,
                    0.5f, 0.0f, 0.0f,
                    0.5f, 0.5f, 0.0f
            */
    };
    // @note: Draw Texture
    f32 vertices[] = {
        // positions         // colors          // texture coordinates
        0.5f,   0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f, // top right
        0.5f,  -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, // bottom left
        -0.5f,  0.5f, 0.0f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f, // top left
    };
#else
    // @note: Draw textured cube
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
        
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };
    // world space positions of our cubes
    glm::vec3 cube_positions[] = {
        glm::vec3( 0.0f,  0.0f,  0.0f),
        glm::vec3( 2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3( 2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3( 1.3f, -2.0f, -2.5f),
        glm::vec3( 1.5f,  2.0f, -2.5f),
        glm::vec3( 1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
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
    
    // @note: Vertex shader
    const char *vertex_shader_source =
#include "vertex_shader.glsl"
    ;
    // @note: Fragment shader
    const char *fragment_shader_source =
#include "fragment_shader.glsl"
    ;
    Shader shader = create_shader(vertex_shader_source, fragment_shader_source);
    use_shader(shader);
    
    // @note: Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void *)0);
    glEnableVertexAttribArray(0);
    
    // @note: Texture coordinates
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void *)(3 * sizeof(f32)));
    glEnableVertexAttribArray(1);
    
    
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
    
    use_shader(shader);
    glUniform1i(glGetUniformLocation(shader.id, "texture1"), 0);
    glUniform1i(glGetUniformLocation(shader.id, "texture2"), 1);
    
    
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
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        use_shader(shader);
        
        //
        // @note: Coordinate systems
        //
        glm::mat4 view_matrix = get_view_matrix(&camera);
        
        glm::mat4 projection_matrix;
        projection_matrix = glm::perspective(glm::radians(camera.fov), (f32)SCREEN_WIDTH / (f32)SCREEN_HEIGHT, 0.1f, 100.0f);
        
        int view_matrix_location = glGetUniformLocation(shader.id, "view_matrix");
        glUniformMatrix4fv(view_matrix_location, 1, GL_FALSE, glm::value_ptr(view_matrix));
        int projection_matrix_location = glGetUniformLocation(shader.id, "projection_matrix");
        glUniformMatrix4fv(projection_matrix_location, 1, GL_FALSE, glm::value_ptr(projection_matrix));
        
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);
        
        glBindVertexArray(vao);
        defer {
            glBindVertexArray(0);
        };
        for (u32 i = 0; i < 10; ++i) {
            glm::mat4 model_matrix = glm::mat4(1.0f);
            model_matrix = glm::translate(model_matrix, cube_positions[i]);
            f32 angle = 20.0f * i;
            model_matrix = glm::rotate(model_matrix, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            
            int model_matrix_location = glGetUniformLocation(shader.id, "model_matrix");
            glUniformMatrix4fv(model_matrix_location, 1, GL_FALSE, glm::value_ptr(model_matrix));
            
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
#if 0
        // plane glDrawArrays(GL_TRIANGLES, 0, 6);
        // #else
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
#endif
        
        glfwSwapBuffers(window);
        
        
        f32 current_frame = glfwGetTime();
        dt = current_frame - last_frame;
        last_frame = current_frame;
    }
    
    
    return 0;
}
