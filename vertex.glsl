#version 330

layout(location = 0) in vec3 i_position;
layout(location = 1) in vec3 i_normal;

uniform mat4 u_mvp_mat; // model-view-projection matrix

uniform float u_offset1; // offset along normal

void main(void){
    vec4 tPos   = vec4(i_position + i_normal * u_offset1, 1.0);
    gl_Position    = u_mvp_mat * tPos;
}
