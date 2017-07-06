#version 450
#extension GL_ARB_separate_shader_objects : enable

#define MAX_BONES 64

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
	mat4 bones[MAX_BONES];
} ubo;


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBitangent;
layout(location = 4) in vec2 inTexCoord;
layout(location = 5) in uvec4 inBoneIndices;
layout(location = 6) in vec4 inWeights;
layout(location = 7) in uvec4 inSamplerIndices;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out uvec4 samplerIndices;

void main() { 
	mat4 boneTransform = ubo.bones[inBoneIndices.x] * inWeights.x;
    boneTransform += ubo.bones[inBoneIndices.y] * inWeights.y;
    boneTransform += ubo.bones[inBoneIndices.z] * inWeights.z;  
    boneTransform += ubo.bones[inBoneIndices.w] * inWeights.w;
    gl_Position = ubo.proj * ubo.view * ubo.model * boneTransform * vec4(inPosition, 1.0);
    fragTexCoord = inTexCoord;
	samplerIndices = inSamplerIndices;
}
