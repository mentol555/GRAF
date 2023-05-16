#version 410
layout (location = 0) in vec3 vp; 
layout (location = 1) in vec3 vColor; 
out vec3 outColor; 
void main () { 
	gl_Position = vec4(vp.x, vp.y, vp.z, 1.0); 
	outColor = vColor; 
};