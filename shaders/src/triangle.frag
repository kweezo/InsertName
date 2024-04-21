#version 450

layout(location = 0) out vec4 outColor;

layout (location = 0) in vec2 oTexCoord;

layout (binding = 1) uniform sampler2D tex;

void main() {
    outColor = texture(tex, oTexCoord);
}
