#version 450

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;

layout (location = 0) out vec2 oTexCoord;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main() {
    gl_Position =  ubo.proj * ubo.view * vec4(aPos, 0.0, 1.0);

    oTexCoord = aPos+vec2(0.5, 0.5);
}
