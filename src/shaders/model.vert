#version 450

// Per-vertex attributes
layout(location = 0) in vec3 inPosition;
// layout(location = 1) in vec3 inNormal;   // optional, for lighting
layout(location = 1) in vec2 inTexCoord; // optional, for textures

// Per-object transform (model matrix)
layout(push_constant) uniform PushConstants {
    mat4 transform;
} push;

// Outputs to fragment shader
// layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    fragTexCoord = vec2(inPosition.y, 0.0);

    // MVP transform
    gl_Position = push.transform * vec4(inPosition, 1.0);
}
