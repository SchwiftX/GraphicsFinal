#version 150 core

in vec3 Color;
in vec3 normal;
in vec3 pos;
in vec3 lightDir;
out vec4 outColor;

const float ambient = 0;
void main() {
   vec3 diffuseC = Color*max(dot(-lightDir,normal),0.0);
   vec3 ambC = Color*ambient;
   vec3 viewDir = normalize(-pos); //We know the eye is at (0,0)
   vec3 reflectDir = reflect(viewDir,normal);
   float spec = max(dot(reflectDir,lightDir),0.0);
   if (dot(-lightDir,normal) <= 0.0)spec = 0;
   vec3 specC = .8*vec3(1.0,1.0,1.0)*pow(spec,4);
   vec3 oColor = ambC+diffuseC+specC;
   outColor = vec4(oColor,1);
}