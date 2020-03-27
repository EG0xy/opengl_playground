R"FOO(


#version 330 core

uniform vec3 object_color;
uniform vec3 light_color;

out vec4 frag_color;

void main() {
    frag_color = vec4(light_color * object_color, 1.0);
}


)FOO"