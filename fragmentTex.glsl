#version 150 core

in vec3 Color;
in vec3 normal;
in vec3 pos;
in vec3 lightDir;
in vec2 texcoord;

out vec4 outColor;

uniform sampler2D tex0;
uniform sampler2D tex1;

uniform int texID;


const float ambient = .3;
void main() {
   vec3 color;
   if (texID == -1)
   	 color = Color;
   else if (texID == 0)
     color = texture(tex0, texcoord).rgb;
   else if (texID == 1)
     color = texture(tex1, texcoord).rgb;  
   else{
   	 outColor = vec4(1,0,0,1);
   	 return; //This was an error, stop lighting!
   	}
   vec3 diffuseC = color*max(dot(-lightDir,normal),0.0);
   vec3 ambC = color*ambient;
   vec3 viewDir = normalize(-pos); //We know the eye is at (0,0)!
   vec3 reflectDir = reflect(viewDir,normal);
   float spec = max(dot(reflectDir,lightDir),0.0);
   if (dot(-lightDir,normal) <= 0.0)spec = 0;
   vec3 specC = .8*vec3(1.0,1.0,1.0)*pow(spec,4);
   vec3 oColor = ambC+diffuseC+specC;
   outColor = vec4(oColor,1);
}