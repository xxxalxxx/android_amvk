#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (binding = 0) uniform sampler2D samplerPosition;
layout (binding = 1) uniform sampler2D samplerNormal;
layout (binding = 2) uniform sampler2D samplerAlbedo;
layout (binding = 3) uniform sampler2D samplerDepth;

layout(location = 0) in vec2 inTexCoord;
layout(location = 0) out vec4 outColor;

#define DEBUG

void main() {
#ifndef DEBUG
    outColor = texture(samplerAlbedo, inTexCoord);
#else
	float x = inTexCoord.x;
	float y = inTexCoord.y;
	if (x < 0.5 && y < 0.5) { // top left
		outColor = texture(samplerPosition, vec2(2.0 * x, 2.0 * y));
	} else if (x >= 0.5 && y < 0.5) { //top right
			outColor = texture(samplerNormal, vec2(2.0 * (x - 0.5), 2.0 * y));
	} else if (x < 0.5 && y >= 0.5) { // bottom left
		outColor = texture(samplerAlbedo, vec2(2.0 * x, 2.0 * (y - 0.5)));
	} else { // bottom right
		float depth = texture(samplerDepth, vec2(2.0 * (x - 0.5), 2.0 * (y - 0.5))).r;
		outColor = vec4(depth, depth, depth, 1.0);
	}
#endif
}
