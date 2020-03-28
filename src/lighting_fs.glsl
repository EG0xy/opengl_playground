R"FOO(


#version 330 core


struct Material {
    // Opaque type, only passed as uniform, cant like be passed to a function!
    sampler2D diffuse_map; // diffuse and ambient
    sampler2D specular_map;
    float shininess;
};

struct Directional_Light {
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
struct Spot_Light {
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


uniform Directional_Light directional_light;
#define POINT_LIGHT_COUNT  4
uniform Point_Light point_lights[POINT_LIGHT_COUNT];
uniform Spot_Light spot_light;
uniform Material material;
uniform vec3 view_pos;


in vec2 tex_coords;
in vec3 frag_pos;
in vec3 normal;


out vec4 frag_color;


vec3 calc_directional_light(Directional_Light light, vec3 normal, vec3 view_dir) {
	vec3 light_dir = normalize(-light.direction);
	// Diffuse
	float diff = max(dot(normal, light_dir), 0.0);

	// Specular
	vec3 reflect_dir = reflect(-light_dir, normal);
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
	
	vec3 ambient  = light.ambient  * texture(material.diffuse_map, tex_coords).rgb;
	vec3 diffuse  = light.diffuse  * diff * texture(material.diffuse_map, tex_coords).rgb;
	vec3 specular = light.specular * spec * texture(material.specular_map, tex_coords).rgb;

	return (ambient + diffuse + specular);
}

vec3 calc_point_light(Point_Light light, vec3 normal, vec3 frag_pos, vec3 view_dir) {
	vec3 light_dir = normalize(light.position - frag_pos);
	// Diffuse
	float diff = max(dot(normal, light_dir), 0.0);
	// Specular
	vec3 reflect_dir = reflect(-light_dir, normal);
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
	// Attenuation
	float distance = length(light.position - frag_pos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
	

	vec3 ambient  = light.ambient  * texture(material.diffuse_map, tex_coords).rgb;
	vec3 diffuse  = light.diffuse  * diff * texture(material.diffuse_map, tex_coords).rgb;
	vec3 specular = light.specular * spec * texture(material.specular_map, tex_coords).rgb;
	ambient  *= attenuation;
	diffuse  *= attenuation;
	specular *= attenuation;
	
	return (ambient + diffuse + specular);
}

vec3 calc_spot_light(Spot_Light light, vec3 normal, vec3 frag_pos, vec3 view_dir) {
	vec3 light_dir = normalize(light.position - frag_pos);
	// Diffuse
	float diff = max(dot(normal, light_dir), 0.0);
	// Specular
	vec3 reflect_dir = reflect(-light_dir, normal);
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
	// Spot light smooth/soft edges
	float theta = dot(light_dir, normalize(-light.direction));
	float epsilon = light.cut_off - light.outer_cut_off;
	float intensity = clamp((theta - light.outer_cut_off) / epsilon, 0.0, 1.0);
	// Attenuation
	float distance = length(light.position - frag_pos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
	
	
	vec3 ambient = light.ambient * texture(material.diffuse_map, tex_coords).rgb;
	vec3 diffuse = light.diffuse * diff * texture(material.diffuse_map, tex_coords).rgb;
	vec3 specular = light.specular * spec * texture(material.specular_map, tex_coords).rgb;
	ambient  *= attenuation * intensity;
	diffuse  *= attenuation * intensity;
	specular *= attenuation * intensity;
	return (ambient + diffuse + specular);
}


void main() {
	vec3 norm = normalize(normal);
	vec3 view_dir = normalize(view_pos - frag_pos);
	
	vec3 result = calc_directional_light(directional_light, norm, view_dir);
	for (int i = 0; i < POINT_LIGHT_COUNT; ++i) {
		result += calc_point_light(point_lights[i], norm, frag_pos, view_dir);
	}
	result += calc_spot_light(spot_light, norm, frag_pos, view_dir);
	
	frag_color = vec4(result, 1.0);
}


)FOO"