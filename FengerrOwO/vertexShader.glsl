#version 460 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec3 vertexNormal;


uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

out vec3 fragmentColor;
out vec3 v_normal;
out vec3 v_position;

void main(){
	gl_Position = Projection * View * Model * vec4(vertexPosition_modelspace, 1.0);
	fragmentColor = vertexColor;
	v_normal = transpose(inverse(mat3(View * Model))) * vertexNormal;
	v_position = gl_Position.xyz / gl_Position.w;
}