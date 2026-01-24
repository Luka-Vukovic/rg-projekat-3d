#version 330 core
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec4 inCol;
layout(location = 2) in vec2 inTex;
layout(location = 3) in vec3 inNormal;

uniform mat4 uM; //Matrica transformacije
uniform mat4 uV; //Matrica kamere
uniform mat4 uP; //Matrica projekcija

out vec4 channelCol;
out vec2 channelTex;
out vec3 fragPos;
out vec3 fragNormal;

void main()
{
	gl_Position = uP * uV * uM * vec4(inPos, 1.0);
	
	fragPos = vec3(uM * vec4(inPos, 1.0));
	fragNormal = mat3(transpose(inverse(uM))) * inNormal;
	
	channelCol = inCol;
	channelTex = inTex;
}