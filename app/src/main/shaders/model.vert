#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBitangent;
layout(location = 4) in vec2 inTexCoord;


layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out vec3 outWorldPos;
layout(location = 2) out vec3 outNormal;

void main() { 
	mat3 normalTransform = transpose(inverse(mat3(ubo.model)));
    vec4 worldPos = ubo.model * vec4(inPosition, 1.0);

    outTexCoord = inTexCoord;
	outWorldPos = vec3(worldPos);
	outNormal = normalTransform * inNormal;

	gl_Position = ubo.proj * ubo.view * worldPos;
}
