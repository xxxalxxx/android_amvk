#ifndef AMVK_VULKAN_PIPELINE_CREATOR_H
#define AMVK_VULKAN_PIPELINE_CREATOR_H


#ifdef __ANDROID__
#include "vulkan_wrapper.h"
#else
#include <vulkan/vulkan.h>
#endif

#include "macro.h"
#include "file_manager.h"
#include "vulkan_utils.h"
#include "vulkan_state.h"

namespace PipelineCreator 
{


inline VkPipelineViewportStateCreateInfo viewportStateDynamic() 
{
	VkPipelineViewportStateCreateInfo viewportState;
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.pNext = nullptr;
	viewportState.flags = 0;
	viewportState.viewportCount = 1;
	viewportState.pViewports = nullptr;
	viewportState.scissorCount = 1;
	viewportState.pScissors = nullptr;
	
	return viewportState;
}

inline VkPipelineViewportStateCreateInfo viewportStateDefault(VkExtent2D& extent)
{
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float) extent.width;
	viewport.height = (float) extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	
	VkRect2D scissor = {};
	scissor.offset = {0, 0};
	scissor.extent = extent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;	
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	return viewportState;
}


inline VkPipelineDynamicStateCreateInfo dynamicState(VkDynamicState* dynamicStates, uint32_t stateCount)
{
	VkPipelineDynamicStateCreateInfo dynamicInfo;
	dynamicInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicInfo.pNext = nullptr;
	dynamicInfo.flags = 0;
	dynamicInfo.dynamicStateCount = stateCount;
	dynamicInfo.pDynamicStates = dynamicStates;

	return dynamicInfo;
}

inline VkPipelineShaderStageCreateInfo shaderStage(VkShaderModule& shaderModule, VkShaderStageFlagBits stage)
{
	VkPipelineShaderStageCreateInfo vertStageCreateInfo = {};
	vertStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertStageCreateInfo.stage = stage;
	vertStageCreateInfo.module = shaderModule;
	vertStageCreateInfo.pName = "main";

	return vertStageCreateInfo;
}

inline VkPipelineShaderStageCreateInfo shaderStage(const VkDevice& device, const char* path, VkShaderStageFlagBits stage)
{
	auto shaderSpvCode = FileManager::getInstance().readShader(path);

	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = shaderSpvCode.size();
	createInfo.pCode = (uint32_t*) shaderSpvCode.data();
	VkShaderModule shaderModule;
	VK_CHECK_RESULT(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule));

	VkPipelineShaderStageCreateInfo vertStageCreateInfo = {};
	vertStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertStageCreateInfo.stage = stage;
	vertStageCreateInfo.module = shaderModule;
	vertStageCreateInfo.pName = "main";

	return vertStageCreateInfo;
}


inline VkPipelineColorBlendAttachmentState blendAttachmentStateDisabled() 
{
	VkPipelineColorBlendAttachmentState blendAttachmentState = {};
	blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT 
										| VK_COLOR_COMPONENT_G_BIT 
										| VK_COLOR_COMPONENT_B_BIT 
										| VK_COLOR_COMPONENT_A_BIT;
	blendAttachmentState.blendEnable = VK_FALSE;

	return blendAttachmentState;
}


inline VkPipelineColorBlendStateCreateInfo blendStateDisabled(
		VkPipelineColorBlendAttachmentState* attachmentStates,  
		uint32_t attachmentCount) 
{
	VkPipelineColorBlendStateCreateInfo blendState = {};
	blendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blendState.logicOpEnable = VK_FALSE;
	blendState.logicOp = VK_LOGIC_OP_COPY;
	blendState.attachmentCount = attachmentCount;
	blendState.pAttachments = attachmentStates;
	blendState.blendConstants[0] = 0.0f;
	blendState.blendConstants[1] = 0.0f;
	blendState.blendConstants[2] = 0.0f;
	blendState.blendConstants[3] = 0.0f;
	
	return blendState;
}


inline VkPushConstantRange pushConstantRange(const VulkanState& state, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size)
{
	if (size > state.deviceInfo.maxPushConstantsSize)
		throw std::runtime_error("Push Constants exceed max size. Use uniform buffer.");
	if (size % 4 != 0)
		throw std::runtime_error("Push Constants size must be a multiple of 4");

	VkPushConstantRange pushConstantRange = {};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstantRange.offset = offset;
	pushConstantRange.size = size; 

	return pushConstantRange;
} 

inline VkPipelineInputAssemblyStateCreateInfo inputAssemblyNoRestart(VkPrimitiveTopology topology) 
{
	VkPipelineInputAssemblyStateCreateInfo assemblyInfo = {};
	assemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	assemblyInfo.topology = topology;
	assemblyInfo.primitiveRestartEnable = VK_FALSE;
	
	return assemblyInfo;
}



inline VkPipelineMultisampleStateCreateInfo multisampleStateNoMultisampleNoSampleShading()
{
	VkPipelineMultisampleStateCreateInfo multisampleState = {};
	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.sampleShadingEnable = VK_FALSE;
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	return multisampleState;
}

inline VkPipelineDepthStencilStateCreateInfo depthStencilStateDepthLessNoStencil() 
{
	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.stencilTestEnable = VK_FALSE;

	return depthStencil;
}

inline VkPipelineRasterizationStateCreateInfo rasterizationStateCullBackCCW() 
{
	VkPipelineRasterizationStateCreateInfo rasterizationState = {};
	rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationState.depthClampEnable = VK_FALSE;
	rasterizationState.rasterizerDiscardEnable = VK_FALSE;
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationState.lineWidth = 1.0f;
	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationState.depthBiasEnable = VK_FALSE;

	return rasterizationState;
}

inline VkPipelineRasterizationStateCreateInfo rasterizationStateCullBackCW() 
{
	VkPipelineRasterizationStateCreateInfo rasterizationState = {};
	rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationState.depthClampEnable = VK_FALSE;
	rasterizationState.rasterizerDiscardEnable = VK_FALSE;
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationState.lineWidth = 1.0f;
	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizationState.depthBiasEnable = VK_FALSE;

	return rasterizationState;
}

inline VkPipelineLayoutCreateInfo layout(
		VkDescriptorSetLayout* setLayouts, 
		uint32_t setLayoutCount,
		VkPushConstantRange* pushConstantRanges,
		uint32_t pushConstantRangeCount)
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.pNext = nullptr;
	pipelineLayoutInfo.flags = 0;
	pipelineLayoutInfo.setLayoutCount = setLayoutCount;
	pipelineLayoutInfo.pSetLayouts = setLayouts;
	pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges;
	pipelineLayoutInfo.pushConstantRangeCount = pushConstantRangeCount;

	return pipelineLayoutInfo;
}

inline VkPipelineVertexInputStateCreateInfo vertexInputState(
		VkVertexInputBindingDescription* bindings, 
		uint32_t bindingsCount,
		VkVertexInputAttributeDescription* attributes,
		uint32_t attributesCount)
{
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = bindingsCount;
	vertexInputInfo.pVertexBindingDescriptions = &bindings[0];
	vertexInputInfo.vertexAttributeDescriptionCount = attributesCount;
	vertexInputInfo.pVertexAttributeDescriptions = &attributes[0];
	
	return vertexInputInfo;
}


inline void pipelineCache(const VkDevice& device, const std::vector<char>& cache, VkPipelineCacheCreateInfo& out)
{
	out = {};
	out.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	if (cache.empty())
		return;
	out.initialDataSize = sizeof(char) * cache.size();
	out.pInitialData = cache.data();
}


};

#endif
