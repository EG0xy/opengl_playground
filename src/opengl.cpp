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

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#undef assert


#include "iml_general.h"
#include "iml_types.h"


#include "shader.h"
#include "camera.h"


struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 tex_coords;
};

struct Texture {
    u32 id;
    std::string type; // @todo Maybe a enum could be used?
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<u32> indices;
    std::vector<Texture> textures;
    
    u32 vao, vbo, ebo;
};

void init_mesh(Mesh *mesh) {
    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->vbo);
    glGenBuffers(1, &mesh->ebo);
    
    glBindVertexArray(mesh->vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh->vertices.size() * sizeof(Vertex), &mesh->vertices[0], GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices.size() * sizeof(u32), &mesh->indices[0], GL_STATIC_DRAW);
    
    // Vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
    // Vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));
    // Vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FLOAT, sizeof(Vertex), (void *)offsetof(Vertex, tex_coords));
    
    glBindVertexArray(0);
}

Mesh make_mesh(std::vector<Vertex> vertices, std::vector<u32> indices, std::vector<Texture> textures) {
    Mesh result = {};
    
    result.vertices = vertices;
    result.indices  = indices;
    result.textures = textures;
    
    init_mesh(&result);
    return result;
}

void draw_mesh(Mesh *mesh, Shader shader) {
    u32 diffuse_map_count  = 1;
    u32 specular_map_count = 1;
    for (u32 i = 0; i < mesh->textures.size(); ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        std::string number;
        std::string name = mesh->textures[i].type;
        if (name == "texture_diffuse") {
            number = std::to_string(diffuse_map_count++);
        }
        else if (name == "texture_specular") {
            number = std::to_string(specular_map_count++);
        }
        
        set_uniform(shader, ("material." + name + number).c_str(), (float)i);
        glBindTexture(GL_TEXTURE_2D, mesh->textures[i].id);
    }
    glActiveTexture(GL_TEXTURE0);
    
    // Draw mesh
    glBindVertexArray(mesh->vao);
    glDrawElements(GL_TRIANGLES, mesh->indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}


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

u32
load_texture(char const * path) {
    u32 texture_id;
    // @note: Generating a texture
    glGenTextures(1, &texture_id);
    int width, height, nr_channels;
    stbi_set_flip_vertically_on_load(true);
    u8 *data = stbi_load(path, &width, &height, &nr_channels, 0);
    defer {
        stbi_image_free(data);
    };
    
    if (data) {
        GLenum format;
        if (nr_channels == 1)  format = GL_RED;
        else if (nr_channels == 3)  format = GL_RGB;
        else if (nr_channels == 4)  format = GL_RGBA;
        
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
    }
	
	return texture_id;
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
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
        
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
        
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
        
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };
    // positions all containers
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
    // positions of the point lights
    glm::vec3 point_light_positions[] = {
        glm::vec3( 0.7f,  0.2f,  2.0f),
        glm::vec3( 2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f,  2.0f, -12.0f),
        glm::vec3( 0.0f,  0.0f, -3.0f)
    };
	
    
    // first, configure the cube's VAO (and vbo)
    unsigned int vbo, cube_vao;
    glGenVertexArrays(1, &cube_vao);
    glGenBuffers(1, &vbo);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindVertexArray(cube_vao);
    
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(f32)));
    glEnableVertexAttribArray(1);
    
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(f32)));
    glEnableVertexAttribArray(2);
    
    // second, configure the light's VAO (vbo stays the same; the vertices are the same for the light object which is also a 3D cube)
    unsigned int light_vao;
    glGenVertexArrays(1, &light_vao);
    glBindVertexArray(light_vao);
    
    // we only need to bind to the vbo (to link it with glVertexAttribPointer), no need to fill it; the vbo's data already contains all we need (it's already bound, but we do it again for educational purposes)
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
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
    
    // @note: Diffuse map
    u32 diffuse_map = load_texture("../assets/container2.png");
	u32 specular_map = load_texture("../assets/container2_specular.png");
    use_shader(lighting_shader);
    set_int(lighting_shader, "material.diffuse_map", (int)0);
	set_int(lighting_shader, "material.specular_map", (int)1);
    
    
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
		set_uniform(lighting_shader, "view_pos", camera.position);
		set_uniform(lighting_shader, "material.shininess", 32.0f);
        // directional light
        set_uniform(lighting_shader, "directional_light.direction", -0.2f, -1.0f, -0.3f);
        set_uniform(lighting_shader, "directional_light.ambient", 0.05f, 0.05f, 0.05f);
        set_uniform(lighting_shader, "directional_light.diffuse", 0.4f, 0.4f, 0.4f);
        set_uniform(lighting_shader, "directional_light.specular", 0.5f, 0.5f, 0.5f);
        // point light 1
        set_uniform(lighting_shader, "point_lights[0].position", point_light_positions[0]);
        set_uniform(lighting_shader, "point_lights[0].ambient", 0.05f, 0.05f, 0.05f);
        set_uniform(lighting_shader, "point_lights[0].diffuse", 0.8f, 0.8f, 0.8f);
        set_uniform(lighting_shader, "point_lights[0].specular", 1.0f, 1.0f, 1.0f);
        set_uniform(lighting_shader, "point_lights[0].constant", 1.0f);
        set_uniform(lighting_shader, "point_lights[0].linear", 0.09f);
        set_uniform(lighting_shader, "point_lights[0].quadratic", 0.032f);
        // point light 2
        set_uniform(lighting_shader, "point_lights[1].position", point_light_positions[1]);
        set_uniform(lighting_shader, "point_lights[1].ambient", 0.05f, 0.05f, 0.05f);
        set_uniform(lighting_shader, "point_lights[1].diffuse", 0.8f, 0.8f, 0.8f);
        set_uniform(lighting_shader, "point_lights[1].specular", 1.0f, 1.0f, 1.0f);
        set_uniform(lighting_shader, "point_lights[1].constant", 1.0f);
        set_uniform(lighting_shader, "point_lights[1].linear", 0.09f);
        set_uniform(lighting_shader, "point_lights[1].quadratic", 0.032f);
        // point light 3
        set_uniform(lighting_shader, "point_lights[2].position", point_light_positions[2]);
        set_uniform(lighting_shader, "point_lights[2].ambient", 0.05f, 0.05f, 0.05f);
        set_uniform(lighting_shader, "point_lights[2].diffuse", 0.8f, 0.8f, 0.8f);
        set_uniform(lighting_shader, "point_lights[2].specular", 1.0f, 1.0f, 1.0f);
        set_uniform(lighting_shader, "point_lights[2].constant", 1.0f);
        set_uniform(lighting_shader, "point_lights[2].linear", 0.09f);
        set_uniform(lighting_shader, "point_lights[2].quadratic", 0.032f);
        // point light 4
        set_uniform(lighting_shader, "point_lights[3].position", point_light_positions[3]);
        set_uniform(lighting_shader, "point_lights[3].ambient", 0.05f, 0.05f, 0.05f);
        set_uniform(lighting_shader, "point_lights[3].diffuse", 0.8f, 0.8f, 0.8f);
        set_uniform(lighting_shader, "point_lights[3].specular", 1.0f, 1.0f, 1.0f);
        set_uniform(lighting_shader, "point_lights[3].constant", 1.0f);
        set_uniform(lighting_shader, "point_lights[3].linear", 0.09f);
        set_uniform(lighting_shader, "point_lights[3].quadratic", 0.032f);
        // spot_light
        set_uniform(lighting_shader, "spot_light.position", camera.position);
        set_uniform(lighting_shader, "spot_light.direction", camera.direction);
        set_uniform(lighting_shader, "spot_light.ambient", 0.0f, 0.0f, 0.0f);
        set_uniform(lighting_shader, "spot_light.diffuse", 1.0f, 1.0f, 1.0f);
        set_uniform(lighting_shader, "spot_light.specular", 1.0f, 1.0f, 1.0f);
        set_uniform(lighting_shader, "spot_light.constant", 1.0f);
        set_uniform(lighting_shader, "spot_light.linear", 0.09f);
        set_uniform(lighting_shader, "spot_light.quadratic", 0.032f);
        set_uniform(lighting_shader, "spot_light.cut_off", glm::cos(glm::radians(12.5f)));
        set_uniform(lighting_shader, "spot_light.outer_cut_off", glm::cos(glm::radians(15.0f)));
        
        // @note: Draw color cube
        glm::mat4 view_matrix = get_view_matrix(&camera);
        glm::mat4 projection_matrix = glm::perspective(glm::radians(camera.fov), (f32)SCREEN_WIDTH / (f32)SCREEN_HEIGHT, 0.1f, 100.0f);
        set_uniform(lighting_shader, "view_matrix", view_matrix);
        set_uniform(lighting_shader, "projection_matrix", projection_matrix);
        
		glm::mat4 model_matrix = glm::mat4(1.0f);
		set_uniform(lighting_shader, "model_matrix", model_matrix);
		
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuse_map);
		
		glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specular_map);
		
        glBindVertexArray(cube_vao);
		for (u32 i = 0; i < 10; ++i) {
			glm::mat4 model_matrix = glm::mat4(1.0f);
			model_matrix = glm::translate(model_matrix, cube_positions[i]);
			float angle = 20.0f * i;
			model_matrix = glm::rotate(model_matrix, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
			set_uniform(lighting_shader, "model_matrix", model_matrix);
			
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
        
        // @note: Draw lamp
        use_shader(lamp_shader);
        set_uniform(lamp_shader, "view_matrix", view_matrix);
        set_uniform(lamp_shader, "projection_matrix", projection_matrix);
		glBindVertexArray(light_vao);
		for (u32 i = 0; i < 4; i++) {
            model_matrix = glm::mat4(1.0f);
            model_matrix = glm::translate(model_matrix, point_light_positions[i]);
            model_matrix = glm::scale(model_matrix, glm::vec3(0.2f)); // Make it a smaller cube
            set_uniform(lamp_shader, "model_matrix", model_matrix);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        
        glfwSwapBuffers(window);
        
        
        f32 current_frame = glfwGetTime();
        dt = current_frame - last_frame;
        last_frame = current_frame;
    }
    
    
    return 0;
}
