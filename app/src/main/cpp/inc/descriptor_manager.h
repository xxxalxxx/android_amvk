#ifndef AMVK_DESCRIPTOR_MANAGER_H
#define AMVK_DESCRIPTOR_MANAGER_H

#include "macro.h"
#include "vulkan_state.h"
#include "pipeline_creator.h"

/*
class DescriptorManager {
public:
	DescriptorManager(VulkanState& state);
	void createDescriptorSetLayouts();
	void createDescriptorPool();

private:
	void createQuadDescriptorSetLayout();
	void createModelDescriptorSetLayout();
	void createSamplerDescriptorSetLayout();
	void createUniformDescriptorSetLayout();
	VulkanState& state;
	DescriptorSetLayouts& state.descriptorSetLayouts;
};
*/

namespace DescriptorManager 
{

inline void createQuadDescriptorSetLayout(VulkanState& state)
{
	VkDescriptorSetLayoutBinding descSetBinding = {};
	descSetBinding.binding = 0;
	descSetBinding.descriptorCount = 1;
	descSetBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descSetBinding.pImmutableSamplers = nullptr;
	descSetBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	
	VkDescriptorSetLayoutBinding bindings[] = {
		descSetBinding, 
		samplerLayoutBinding
	};

	VkDescriptorSetLayoutCreateInfo descSetLayoutInfo = {};
	descSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descSetLayoutInfo.bindingCount = ARRAY_SIZE(bindings);
	descSetLayoutInfo.pBindings = bindings;

	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(state.device, &descSetLayoutInfo, nullptr, &state.descriptorSetLayouts.tquad));
}


inline void createSamplerDescriptorSetLayout(VulkanState& state)
{
	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 0;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutCreateInfo descSetLayoutInfo = {};
	descSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descSetLayoutInfo.bindingCount = 1; 
	descSetLayoutInfo.pBindings = &samplerLayoutBinding;

	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(state.device, &descSetLayoutInfo, nullptr, &state.descriptorSetLayouts.sampler));
}



inline void createSamplerListDescriptorSetLayout(VulkanState& state)
{
	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 0;
	samplerLayoutBinding.descriptorCount = SAMPLER_LIST_SIZE;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutCreateInfo descSetLayoutInfo = {};
	descSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descSetLayoutInfo.bindingCount = 1; 
	descSetLayoutInfo.pBindings = &samplerLayoutBinding;

	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(state.device, &descSetLayoutInfo, nullptr, &state.descriptorSetLayouts.samplerList));
}



inline void createUniformDescriptorSetLayout(VulkanState& state)
{
	VkDescriptorSetLayoutBinding descSetBinding = {};
	descSetBinding.binding = 0;
	descSetBinding.descriptorCount = 1;
	descSetBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descSetBinding.pImmutableSamplers = nullptr;
	descSetBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutCreateInfo descSetLayoutInfo = {};
	descSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descSetLayoutInfo.bindingCount = 1;
	descSetLayoutInfo.pBindings = &descSetBinding;

	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(state.device, &descSetLayoutInfo, nullptr, &state.descriptorSetLayouts.uniform));
}

inline void createModelDescriptorSetLayout(VulkanState& state)
{
	VkDescriptorSetLayoutBinding descSetBinding = {};
	descSetBinding.binding = 0;
	descSetBinding.descriptorCount = 1;
	descSetBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descSetBinding.pImmutableSamplers = nullptr;
	descSetBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding samplerSpecLayoutBinding = {};
	samplerSpecLayoutBinding.binding = 2;
	samplerSpecLayoutBinding.descriptorCount = 0;
	samplerSpecLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerSpecLayoutBinding.pImmutableSamplers = nullptr;
	samplerSpecLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding samplerHeightLayoutBinding = {};
	samplerHeightLayoutBinding.binding = 3;
	samplerHeightLayoutBinding.descriptorCount = 0;
	samplerHeightLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerHeightLayoutBinding.pImmutableSamplers = nullptr;
	samplerHeightLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding samplerAmbientLayoutBinding = {};
	samplerAmbientLayoutBinding.binding = 4;
	samplerAmbientLayoutBinding.descriptorCount = 0;
	samplerAmbientLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerAmbientLayoutBinding.pImmutableSamplers = nullptr;
	samplerAmbientLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding bindings[] = {
		descSetBinding, 
		samplerLayoutBinding,
		samplerSpecLayoutBinding,
		samplerHeightLayoutBinding,
		samplerAmbientLayoutBinding
	};

	VkDescriptorSetLayoutCreateInfo descSetLayoutInfo = {};
	descSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descSetLayoutInfo.bindingCount = ARRAY_SIZE(bindings);
	descSetLayoutInfo.pBindings = bindings;

	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(state.device, &descSetLayoutInfo, nullptr, &state.descriptorSetLayouts.model));

	LOG("MODEL DESC LAYOUT CREATED");
}

inline void createDescriptorPool(VulkanState& state) 
{
	VkDescriptorPoolSize uboSize = {};
	uboSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboSize.descriptorCount = 1;
	
	VkDescriptorPoolSize samplerSize = {};
	samplerSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerSize.descriptorCount = 1;

	VkDescriptorPoolSize poolSizes[] = {
		uboSize,
		samplerSize
	};
	
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = ARRAY_SIZE(poolSizes);
	poolInfo.pPoolSizes = poolSizes;
	poolInfo.maxSets = 1;

	VK_CHECK_RESULT(vkCreateDescriptorPool(state.device, &poolInfo, nullptr, &state.descriptorPool));
}

inline void createDescriptorSetLayouts(VulkanState& state)
{
	createQuadDescriptorSetLayout(state);
	createModelDescriptorSetLayout(state);
	createSamplerDescriptorSetLayout(state);
	createSamplerListDescriptorSetLayout(state);
	createUniformDescriptorSetLayout(state);
	LOG("DESC LAYOUTS CREATED");
}



};

#endif
