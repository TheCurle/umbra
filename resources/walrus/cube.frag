#version 450

layout(std140, binding = 1) uniform Light {
    vec4 dirTime;
} light;

layout(binding = 2) uniform sampler2D tex;

layout(location = 0) in vec3 normal;
layout(location = 1) in vec2 tex_coord;

layout(location = 0) out vec4 frag_color;

void main() {
    const vec3 dir = normalize(-light.dirTime.xyz);
    const float diffuseCol = max(dot(normal, dir), 0.0);
    frag_color = diffuseCol * texture(tex, tex_coord);
}