#ifndef AMVK_BUFFER_MANAGER_H
#define AMVK_BUFFER_MANAGER_H


#ifdef __ANDROID__
#include "vulkan_wrapper.h"
#else
#include <vulkan/vulkan.h>
#endif

#include "cmd_pass.h"
#include "vulkan_utils.h"
#include "vulkan_state.h"

#include <cstring>

class BufferInfo {
public:
	BufferInfo(const VkDevice& device);
	BufferInfo(const VkDevice& device, VkDeviceSize size);
	~BufferInfo();
	VkBuffer buffer;
	VkDeviceMemory memory;
	VkDeviceSize size;
private:
	const VkDevice& mVkDevice;
};

class BufferHelper {
public:	

	static void mapMemory(const VulkanState& state, BufferInfo& bufferInfo, const void* src);
	static void mapMemory(const VulkanState& state, VkDeviceMemory& memory, VkDeviceSize size, const void* src); 

	static void mapMemory(
			const VulkanState& state, 
			VkDeviceMemory& memory, 
			VkDeviceSize offset, 
			VkDeviceSize size, 
			const void* src); 

	static void mapMemory(
			const VkDevice& device, 
			VkDeviceMemory& memory, 
			VkDeviceSize offset, 
			VkDeviceSize size, 
			const void* src);

	static uint32_t getMemoryType(
			const VkPhysicalDevice& physicalDevice, 
			uint32_t typeFilter, 
			VkMemoryPropertyFlags& flags);
	
	static void createBuffer(
			const VkPhysicalDevice& physicalDevice,
			const VkDevice& device,
			VkBuffer& buffer, 
			VkDeviceSize size, 
			VkDeviceMemory& memory,  
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags prop);

	static void createBuffer(
			const VulkanState& state,
			VkBuffer& buffer, 
			VkDeviceSize size, 
			VkDeviceMemory& memory,  
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags prop);

	static void createBuffer(
			const VulkanState& state,
			BufferInfo& bufferInfo,  
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags prop);

	static void createCommonBuffer(const VulkanState& state, BufferInfo& bufferInfo);
	static void createVertexBuffer(const VulkanState& state, BufferInfo& bufferInfo);
	static void createIndexBuffer(const VulkanState& state, BufferInfo& bufferInfo);
	static void createUniformBuffer(const VulkanState& state, BufferInfo& bufferInfo);
	static void createStagingBuffer(const VulkanState& state,BufferInfo& bufferInfo);

	static void createStagingBuffer(
			const VulkanState& state,
			VkBuffer& buffer, 
			VkDeviceSize size, 
			VkDeviceMemory& memory);

	static void createStagingBuffer(
			const VkPhysicalDevice& physicalDevice,
			const VkDevice& device,
			VkBuffer& buffer, 
			VkDeviceSize size, 
			VkDeviceMemory& memory);

	static void copyBuffer(
			const VkDevice& device,
			const VkCommandPool& commandPool,
			const VkQueue& queue,	
			VkBuffer src, 
			VkBuffer dst, 
			VkDeviceSize size);

	static void copyBuffer(
			const VkDevice& device,
			const VkCommandPool& commandPool,
			const VkQueue& queue,
			VkBuffer src, 
			VkBuffer dst, 
			VkDeviceSize offset,
			VkDeviceSize size);

	static void copyBuffer(
			const VulkanState& state,
			VkBuffer src, 
			VkBuffer dst, 
			VkDeviceSize size);

};

#endif
