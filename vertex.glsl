#version 150 core

in vec3 position;
in vec3 inNormal;

//in vec3 inColor;
uniform vec3 inColor;

//const vec3 inColor = vec3(0.f,0.7f,0.f);
const vec3 inLightDir = normalize(vec3(-1,-1,-1));

out vec3 Color;
out vec3 normal;
out vec3 pos;
out vec3 lightDir;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {
   Color = inColor;
   gl_Position = proj * view * model * vec4(position,1.0);
   pos = (view * model * vec4(position,1.0)).xyz;
   lightDir = (view * vec4(inLightDir,0.0)).xyz; //It's a vector!
   vec4 norm4 = transpose(inverse(view*model)) * vec4(inNormal,0.0);
   normal = normalize(norm4.xyz);
}