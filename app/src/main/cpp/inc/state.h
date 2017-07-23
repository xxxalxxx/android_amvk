#ifndef AMVK_VULKAN_STATE_H
#define AMVK_VULKAN_STATE_H

#include "vulkan.h"

#include <vector>
#include <glm/glm.hpp>

struct DeviceInfo {
	DeviceInfo():
		samplerAnisotropy(VK_FALSE),
		maxPushConstantsSize(0),
		minUniformBufferOffsetAlignment(0),
		maxDescriptorSetUniformBuffersDynamic(0) {}
	VkBool32 samplerAnisotropy;
	uint32_t maxPushConstantsSize;
	VkDeviceSize minUniformBufferOffsetAlignment;
	uint32_t maxDescriptorSetUniformBuffersDynamic;

	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	std::vector<VkSurfaceFormatKHR> surfaceFormats;
	std::vector<VkPresentModeKHR> presentModes;
};

struct SurfaceInfo {

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
	PipelineInfo fullscreenQuad;
	PipelineInfo quad;
	PipelineInfo pointLight;
	PipelineInfo tquad;
	PipelineInfo model;
	PipelineInfo skinned;
	PipelineInfo deferred;
};

struct DescriptorSets {

};

struct DescriptorSetLayouts {
	VkDescriptorSetLayout quad;
	VkDescriptorSetLayout fullscreenQuad;
	VkDescriptorSetLayout tquad;
	VkDescriptorSetLayout pointLight;
	VkDescriptorSetLayout model;
	VkDescriptorSetLayout uniformVertex;
	VkDescriptorSetLayout uniformFragment;
	VkDescriptorSetLayout dynamicUniformVertex;
	VkDescriptorSetLayout dynamicUniformFragment;
	VkDescriptorSetLayout sampler;
	VkDescriptorSetLayout samplerList;
	VkDescriptorSetLayout deferred;
};

struct Shaders {
	ShaderInfo quad;
	ShaderInfo fullscreenQuad;
	ShaderInfo tquad;
	ShaderInfo pointLight;
	ShaderInfo model;
	ShaderInfo skinned;
	ShaderInfo deferred;
};

struct State {
	
	struct UBO {
		glm::mat4 view;
		glm::mat4 proj;
	};

	UBO ubo;

	State():
		instance(VK_NULL_HANDLE), 
		physicalDevice(VK_NULL_HANDLE), 
		device(VK_NULL_HANDLE), 
		swapChain(VK_NULL_HANDLE),
		graphicsQueue(VK_NULL_HANDLE), 
		presentQueue(VK_NULL_HANDLE),
		commandPool(VK_NULL_HANDLE),
		descriptorPool(VK_NULL_HANDLE)
	{};
	
	// Disallow copy constructor for State.
	// Only references for State are allowed
	State(State const& vulkanState) = delete;
    State& operator=(State const& vulkanState) = delete;
	
	VkInstance instance;
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	
	VkSwapchainKHR swapChain;

	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;

	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkQueue transferQueue;
	VkQueue computeQueue;

	uint32_t graphicsQueueIndex;
	uint32_t presentQueueIndex;
	uint32_t transferQueueIndex;
	uint32_t computeQueueIndex;

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
