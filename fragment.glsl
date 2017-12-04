//////////////////////////////////
// FRAGMENT SHADER ///////////////
#version 330

uniform vec3 u_color1;

layout(location = 0) out vec4 o_FragColor;

void main(void){
    o_FragColor = vec4(u_color1, 1.0);
}
