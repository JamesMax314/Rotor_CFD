#version 450

// Inputs
layout(location = 0) in vec2 inUV;
layout(set = 0, binding = 0) uniform sampler3D volumeTex;

// Output
layout(location = 0) out vec4 outFragColor;

layout(push_constant) uniform CamData {
    vec3 pos;
    float _pad1;   // matches padding1
    vec3 camUp;
    float _pad2;   // matches padding2
    vec3 lookAt;
    float _pad3;   // matches padding3
} cam;

// Simple parameters
vec3 cameraPos = cam.pos;
vec3 cameraDir = cam.lookAt - cam.pos;
vec3 cameraUp  = cam.camUp;

float fov      = radians(45.0);
float stepSize = 0.01; // Step size along the ray

float randomStepModifier(vec2 st) {
    return max(0.5, fract(sin(dot(st, vec2(12.9898, 78.233)) * 43758.5453)))/10.0 + 0.9;
}

vec3 computeRayDir(vec3 cameraPos, vec3 cameraDir, vec3 cameraUp, vec2 fragUV, float fov) {
    // Compute camera basis
    vec3 forward = normalize(cameraDir);
    vec3 right = normalize(cross(forward, cameraUp));
    vec3 up = cross(right, forward);

    // Map UV from [0,1] to [-1,1]
    vec2 screenPos = fragUV * 2.0 - 1.0;

    // Adjust for aspect ratio if needed (assume square for now)
    float aspect = 1.0; // set properly if not square
    screenPos.x *= aspect;

    // Compute ray in camera space using FOV
    float tanFov = tan(fov * 0.5);
    vec3 rayDir = normalize(forward + screenPos.x * tanFov * right + screenPos.y * tanFov * up);

    return rayDir;
}

bool intersectBox(vec3 rayOrigin, vec3 rayDir, out float tNear, out float tFar) {
    vec3 invDir = 1.0 / rayDir;
    vec3 t0s = (vec3(0.0) - rayOrigin) * invDir;     // min bounds
    vec3 t1s = (vec3(1.0) - rayOrigin) * invDir;     // max bounds

    vec3 tMin = min(t0s, t1s);
    vec3 tMax = max(t0s, t1s);

    tNear = max(max(tMin.x, tMin.y), tMin.z);
    tFar  = min(min(tMax.x, tMax.y), tMax.z);

    // If ray origin is inside, force tNear = 0
    if (tNear < 0.0) tNear = 0.0;

    return tFar >= tNear;
}

// Assume we have a cube from [0,0,0] to [1,1,1] and inUV maps to a face
void main() {
    vec3 rayDir = computeRayDir(cameraPos, cameraDir, cameraUp, inUV, fov);

    float tNear, tFar;
    if (!intersectBox(cameraPos, rayDir, tNear, tFar)) {
        outFragColor = vec4(0.0); // Ray misses the volume
        return;
    }

    vec3 samplePos = cameraPos + rayDir * tNear;
    float t = tNear;
    vec4 accumulatedColor = vec4(0.0);
    float stepMod = randomStepModifier(inUV + vec2(t, t)); // vary step size a bit


    while (t < tFar) {
        float density = texture(volumeTex, samplePos).r;
        vec4 col = vec4(density, density, density, density);

        // Front-to-back alpha blending
        accumulatedColor.rgb += (1.0 - accumulatedColor.a) * col.rgb * col.a;
        accumulatedColor.a += (1.0 - accumulatedColor.a) * col.a;

        if (accumulatedColor.a >= 0.95)
            break;

        samplePos += rayDir * stepSize * stepMod;
        t += stepSize;
    }

    outFragColor = accumulatedColor;
    // outFragColor = vec4(jitter*20, 1.0, 1.0, 1.0); // visualize jitter
}
