#version 450

layout(std140, push_constant) uniform Transform {
    mat4 model;
    mat4 proj;
} transf;

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;

layout(location = 0) out vec3 normal;
layout(location = 1) out vec2 texCoord;

void main() {
    const vec4 worldPos = transf.model * vec4(in_pos, 1.0);
    normal = normalize(worldPos.xyz);

    gl_Position = transf.proj * worldPos;
    texCoord = in_uv;
}