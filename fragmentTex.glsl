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
uniform float u_numShades; // number of shades


const float ambient = .3;

// calculate diffuse component of lighting
float diffuseSimple(vec3 L, vec3 N){
   return clamp(dot(L,N),0.0,1.0);
}

// calculate specular component of lighting
float specularSimple(vec3 L,vec3 N,vec3 R){
   if(dot(N,L)>0){
      return pow(clamp(dot(L,R),0.0,1.0),4.0);
   }
   return 0.0;
}

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
   vec3 viewDir = normalize(-pos); //We know the eye is at (0,0)!
   vec3 reflectOfViewDir = reflect(viewDir,normal);

   vec3 diffuseC = color*max(dot(-lightDir,normal),0.0);
   float idiff = diffuseSimple(-lightDir,normal);
   float ispec = specularSimple(-lightDir,normal,-reflectOfViewDir);
   float intensity = ambient + idiff + ispec;
   // quantize intensity for cel shading
   float shadeIntensity = ceil(intensity * u_numShades)/ u_numShades;

   outColor = vec4(color*shadeIntensity,1);
}