R"FOO(


#version 330 core


uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;


layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;


out vec3 frag_pos;
out vec3 normal;
out vec2 tex_coords;


void main() {
    gl_Position = projection_matrix * view_matrix * model_matrix * vec4(aPos, 1.0);
    frag_pos = vec3(model_matrix * vec4(aPos, 1.0));
    normal = mat3(transpose(inverse(model_matrix))) * aNormal;
    tex_coords = aTexCoords;
}


)FOO"
