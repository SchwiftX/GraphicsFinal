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
uniform float alpha;

uniform struct PointLight{
    vec3 position;
    vec3 intensity;
}pl1,pl2;


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

    //add the diffuse/specular color with point light
   vec3 Pl1direction = pl1.position-pos;
   vec3 Pl2direction = pl2.position-pos;
   float length1 = length(Pl1direction);
   float length2 = length(Pl2direction);
   Pl1direction = normalize(Pl1direction);
   Pl2direction = normalize(Pl1direction);
   vec3 diffuseP1 = color*pl1.intensity/length1*max(dot(Pl1direction,normal),0.0);
   vec3 diffuseP2 = color*pl2.intensity/length2*max(dot(Pl2direction,normal),0.0);

   vec3 diffuseC = color*max(dot(-lightDir,normal),0.0);
   vec3 ambC = color*ambient;
   vec3 viewDir = normalize(vec3(2.5,7,2.5)-pos); //We know the eye is at (0,0)!
   vec3 reflectDir = reflect(viewDir,normal);
   float spec = max(dot(reflectDir,lightDir),0.0);
   float spec2 = max(dot(reflectDir,Pl1direction),0.0);
   float spec3 = max(dot(reflectDir,Pl2direction),0.0);
   if (dot(-lightDir,normal) <= 0.0)spec = 0;
   if (dot(-Pl1direction,normal) <= 0.0)spec2 = 0;
   if (dot(-Pl2direction,normal) <= 0.0)spec3 = 0;
   vec3 specC = .8*vec3(1.0,1.0,1.0)*pow(spec,4);
   vec3 specP2 = .2*vec3(1.0,1.0,1.0)*pow(spec2,4);
   vec3 specP3 = .2*vec3(1.0,1.0,1.0)*pow(spec3,4);
   vec3 oColor = ambC+diffuseC+diffuseP1+diffuseP2+specC+specP2+specP3;
   outColor = vec4(oColor,alpha);
}
