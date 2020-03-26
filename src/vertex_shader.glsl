R"FOO(


#version 330 core


uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;


layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 tex_coord;


void main() {
    gl_Position = projection_matrix * view_matrix * model_matrix * vec4(aPos, 1.0);
    tex_coord = aTexCoord;
}


)FOO"
