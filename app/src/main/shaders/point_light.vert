#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout(set = 1, binding = 0) uniform Model {
    mat4 model;
} uboModel;


layout(location = 0) in vec3 inPosition;

void main() {
    gl_Position = ubo.proj * ubo.view * uboModel.model * vec4(inPosition, 1.0);
}
