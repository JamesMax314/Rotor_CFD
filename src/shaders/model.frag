#version 450

// Inputs from vertex shader (if you want to use them later)
// layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;

// Output color
layout(location = 0) out vec4 outColor;

void main() {
    // Flat color for testing
    outColor = vec4(fragTexCoord.x, fragTexCoord.x, 0.0, 1.0);
}
