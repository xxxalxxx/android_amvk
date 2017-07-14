#ifndef AMVK_VULKAN_STATE_H
#define AMVK_VULKAN_STATE_H


#ifdef __ANDROID__
#include "vulkan_wrapper.h"
#else
#include <vulkan/vulkan.h>
#endif

#include "swap_chain_desc.h"

struct DeviceInfo {
	DeviceInfo():
		samplerAnisotropy(VK_FALSE),
		maxPushConstantsSize(0),
		minUniformBufferOffsetAlignment(0) {}
	VkBool32 samplerAnisotropy;
	uint32_t maxPushConstantsSize;
	VkDeviceSize minUniformBufferOffsetAlignment;
};

struct PipelineInfo {
	PipelineInfo(): 
		pipeline(VK_NULL_HANDLE),
		cache(VK_NULL_HANDLE),
		layout(VK_NULL_HANDLE) {}

	VkPipeline pipeline;
	VkPipelineCache cache;
	VkPipelineLayout layout;
};

struct ShaderInfo {
	ShaderInfo() {}

	VkPipelineShaderStageCreateInfo vertex;
	VkPipelineShaderStageCreateInfo fragment;
	VkPipelineShaderStageCreateInfo geometry;
};

struct Pipelines {
	PipelineInfo tquad;
	PipelineInfo model;
	PipelineInfo skinned;
};

struct DescriptorSets {

};

struct DescriptorSetLayouts {
	VkDescriptorSetLayout tquad;
	VkDescriptorSetLayout model;
	VkDescriptorSetLayout uniform;
	VkDescriptorSetLayout sampler;
	VkDescriptorSetLayout samplerList;
};

struct Shaders {
	ShaderInfo tquad;
	ShaderInfo model;
	ShaderInfo skinned;
};

struct VulkanState {
	VulkanState(): 
		instance(VK_NULL_HANDLE), 
		physicalDevice(VK_NULL_HANDLE), 
		device(VK_NULL_HANDLE), 
		swapChain(VK_NULL_HANDLE),
		graphicsQueue(VK_NULL_HANDLE), 
		presentQueue(VK_NULL_HANDLE),
		commandPool(VK_NULL_HANDLE),
		descriptorPool(VK_NULL_HANDLE)
	{};
	
	// Disallow copy constructor for VulkanState.
	// Only references for VulkanState are allowed
	VulkanState(VulkanState const& vulkanState) = delete;
    VulkanState& operator=(VulkanState const& vulkanState) = delete;
	
	VkInstance instance;
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	
	VkSwapchainKHR swapChain;
	SwapChainDesc swapChainDesc;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	VkQueue graphicsQueue;
	VkQueue presentQueue; 
	uint32_t graphicsQueueIndex;
	uint32_t presentQueueIndex;

	VkRenderPass renderPass;
	
	VkCommandPool commandPool;
	VkDescriptorPool descriptorPool;

	VkFormat depthFormat;

	DeviceInfo deviceInfo;
	Pipelines pipelines;
	Shaders shaders;
	DescriptorSetLayouts descriptorSetLayouts;
};

#endif
