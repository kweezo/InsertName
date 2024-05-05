#version 450

layout (location = 0) in vec3 vertex;

layout (location = 1) in vec4 m1;
layout (location = 2) in vec4 m2;
layout (location = 3) in vec4 m3;
layout (location = 4) in vec4 m4;

layout (location = 5) in vec2 texCoord;
layout (location = 6) in vec3 normal;


layout (location = 0) out vec2 oTexCoord;

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;


void main() {
    mat4 model = mat4(m1, m2, m3, m4);
    gl_Position =  ubo.proj * ubo.view * model * vec4(vertex, 1.0);

    oTexCoord = texCoord;
}
