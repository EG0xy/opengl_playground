R"FOO(


#version 330 core


struct Material {
    // Opaque type, only passed as uniform, cant like be passed to a function!
    sampler2D diffuse_map; // diffuse and ambient
    sampler2D specular_map;
    float shininess;
};

struct Directional_Light {
    // vec3 position;
	vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
struct Point_Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
	
	// Attenuation
	float constant;
	float linear;
	float quadratic;
};
struct Light { // Spot
    vec3 position;
	vec3 direction;
	float cut_off; // cosine result of phi
	float outer_cut_off;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
	
	// Attenuation
	float constant;
	float linear;
	float quadratic;
};


uniform Material material;
uniform Light    light;
uniform vec3 view_pos;


in vec2 tex_coords;
in vec3 frag_pos;
in vec3 normal;


out vec4 frag_color;


void main() {
    vec3 light_dir = normalize(light.position - frag_pos);
	// vec3 light_dir = normalize(-light.direction);

	// Ambient
	vec3 ambient = light.ambient * texture(material.diffuse_map, tex_coords).rgb;

	// Diffuse
	vec3 norm = normalize(normal);
	float diff = max(dot(norm, light_dir), 0.0);
	vec3 diffuse = light.diffuse * diff * texture(material.diffuse_map, tex_coords).rgb;

	// Specular
	vec3 view_dir = normalize(view_pos - frag_pos);
	vec3 reflect_dir = reflect(-light_dir, norm);
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
	vec3 specular = light.specular * spec * texture(material.specular_map, tex_coords).rgb;

	// Spot light smooth/soft edges
	float theta = dot(light_dir, normalize(-light.direction));
	float epsilon = light.cut_off - light.outer_cut_off;
	float intensity = clamp((theta - light.outer_cut_off) / epsilon, 0.0, 1.0);
	// ambient: We will leave this unaffected so we always have a little light.
	diffuse *= intensity;
	specular *= intensity;

	// Attenuation
	float distance = length(light.position - frag_pos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
	ambient  *= attenuation;
	diffuse  *= attenuation;
	specular *= attenuation;
		
		
	frag_color = vec4(ambient + diffuse + specular, 1.0);
}


)FOO"