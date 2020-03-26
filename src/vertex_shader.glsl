R"FOO(


#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;
uniform mat4 transform;

out vec3 our_color;
out vec2 tex_coord;

void main() {
    gl_Position = transform * vec4(aPos, 1.0);
    our_color = aColor;
    tex_coord = aTexCoord;
}


)FOO"
