#version 450

//shader input
layout (location = 0) in vec3 inColor;
layout(set = 0, binding = 0) uniform sampler3D volumeTex;

//output write
layout (location = 0) out vec4 outFragColor;


void main()
{
	//return color
	// outFragColor = vec4(inColor,1.0f);
	outFragColor = texture(volumeTex, vec3(0.5, 0.5, 0.5));
}