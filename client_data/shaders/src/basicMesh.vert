#version 450

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 normal;

layout (location = 0) out vec2 oTexCoord;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main() {
    gl_Position =  ubo.proj * ubo.view * ubo.model * vec4(vertex, 1.0);

    oTexCoord = texCoord;
}
