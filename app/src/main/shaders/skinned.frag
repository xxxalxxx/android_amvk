#version 450
#extension GL_ARB_separate_shader_objects : enable
#define SAMPLER_LIST_SIZE 16
layout(set = 1, binding = 0) uniform sampler2D texSampler[SAMPLER_LIST_SIZE];
//layout(binding = 2) uniform sampler2D texSpecSampler;
//layout(binding = 3) uniform sampler2D texHeightSampler;
//layout(binding = 4) uniform sampler2D texAmbientSampler;

layout(location = 0) in vec2 inTexCoord;
layout(location = 1) flat in uvec4 inSamplerIndices;
layout(location = 2) in vec3 inWorldPos;
layout(location = 3) in vec3 inNormal;

//layout(location = 0) out vec4 outColor;
layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outAlbedo;

void main() {
    //outColor = texture(texSampler[inSamplerIndices.x], inTexCoord);
	outPosition = vec4(inWorldPos, 1.0);
	outNormal = vec4(inNormal, 1.0);
	outAlbedo = texture(texSampler[inSamplerIndices.x], inTexCoord);
}
