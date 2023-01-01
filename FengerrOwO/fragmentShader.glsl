#version 460 core

out vec4 color;

in vec3 v_normal;
in vec3 v_position;

in vec3 fragmentColor;

uniform vec3 LightPosition;
uniform vec3 LightDirection;


uniform vec3 ambient_color;
uniform vec3 diffuse_color;
uniform vec3 specular_color;

void main(){
    
  vec3 light_direction = normalize(LightPosition - v_position);

    if (dot(light_direction, v_normal) < 0.0) {
		  light_direction = normalize(v_position - LightPosition);
	}

    // Calculate the view direction
    vec3 camera_dir = normalize(-v_position);

    // Calculate the half direction
    vec3 half_direction = normalize(light_direction + camera_dir);

    // Normalize the normal vector
    vec3 normal = normalize(v_normal);

    // Calculate the diffuse term
    float diffuse = max(dot(normal, light_direction), 0.0);

    // Calculate the specular term
    float specular = pow(max(dot(normal, half_direction), 0.0), 16.0);

    // Calculate the final color of the fragment
    color = vec4((ambient_color + diffuse * diffuse_color + specular * specular_color) * fragmentColor, 1.0);
}