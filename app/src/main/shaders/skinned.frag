#version 450
#extension GL_ARB_separate_shader_objects : enable
#define SAMPLER_LIST_SIZE 16
layout(set = 1, binding = 0) uniform sampler2D texSampler[SAMPLER_LIST_SIZE];
//layout(binding = 2) uniform sampler2D texSpecSampler;
//layout(binding = 3) uniform sampler2D texHeightSampler;
//layout(binding = 4) uniform sampler2D texAmbientSampler;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) flat in uvec4 samplerIndices;
layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler[samplerIndices.x], fragTexCoord);
}
