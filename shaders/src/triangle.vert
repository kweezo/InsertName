#version 450

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

layout (location = 0) out vec3 oColor;

void main() {
    gl_Position = vec4(aPos, 1.0);

    oColor = aColor;
}
