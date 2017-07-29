#ifndef AMVK_SHADER_MANAGER_H
#define AMVK_SHADER_MANAGER_H

#include "macro.h"
#include "state.h"
#include "pipeline_builder.h"


namespace ShaderCreator
{

	
inline void createShaders(State& state)
{
	LOG_TITLE("Shader Creator");

	Shaders& shaders = state.shaders;
	
	shaders.tquad.vertex = PipelineBuilder::shaderStage(state.device, "tquad.vert", VK_SHADER_STAGE_VERTEX_BIT);
	shaders.tquad.fragment = PipelineBuilder::shaderStage(state.device, "tquad.frag", VK_SHADER_STAGE_FRAGMENT_BIT);

	shaders.model.vertex = PipelineBuilder::shaderStage(state.device, "model.vert", VK_SHADER_STAGE_VERTEX_BIT);
	shaders.model.fragment = PipelineBuilder::shaderStage(state.device, "model.frag", VK_SHADER_STAGE_FRAGMENT_BIT);

	shaders.skinned.vertex = PipelineBuilder::shaderStage(state.device, "skinned.vert", VK_SHADER_STAGE_VERTEX_BIT);
	shaders.skinned.fragment = PipelineBuilder::shaderStage(state.device, "skinned.frag", VK_SHADER_STAGE_FRAGMENT_BIT);

	shaders.fullscreenQuad.vertex = PipelineBuilder::shaderStage(state.device, "fullscreen_quad.vert", VK_SHADER_STAGE_VERTEX_BIT);
	shaders.fullscreenQuad.fragment = PipelineBuilder::shaderStage(state.device, "fullscreen_quad.frag", VK_SHADER_STAGE_FRAGMENT_BIT);
	
	shaders.pointLight.vertex = PipelineBuilder::shaderStage(state.device, "point_light.vert", VK_SHADER_STAGE_VERTEX_BIT);
	shaders.pointLight.fragment = PipelineBuilder::shaderStage(state.device, "point_light.frag", VK_SHADER_STAGE_FRAGMENT_BIT);

	shaders.deferred.vertex = PipelineBuilder::shaderStage(state.device, "deferred.vert", VK_SHADER_STAGE_VERTEX_BIT);
	shaders.deferred.fragment = PipelineBuilder::shaderStage(state.device, "deferred.frag", VK_SHADER_STAGE_FRAGMENT_BIT);
	shaders.deferred.compute =  PipelineBuilder::shaderStage(state.device, "deferred.comp", VK_SHADER_STAGE_COMPUTE_BIT);
}


};

#endif
