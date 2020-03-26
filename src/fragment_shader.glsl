R"FOO(


#version 330 core

#define lerp(a, t, b)  mix(a, b, t)

// uniform vec4 our_color; // Set in c++ code
in vec2 tex_coord;
uniform sampler2D texture1;
uniform sampler2D texture2;

out vec4 frag_color;

void main() {
    // frag_color = vec4(our_color, 1.0);
    // frag_color = texture(our_texture, tex_coord) * vec4(our_color, 1.0);
    frag_color = lerp(texture(texture1, tex_coord), 0.2, texture(texture2, tex_coord));
}


)FOO"