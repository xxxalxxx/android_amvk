#include "buffer_helper.h"

BufferInfo::BufferInfo(const VkDevice& device):
	buffer(VK_NULL_HANDLE),
	memory(VK_NULL_HANDLE),
	size(0),
	mVkDevice(device)

{

}


BufferInfo::BufferInfo(const VkDevice& device, VkDeviceSize size):
	buffer(VK_NULL_HANDLE),
	memory(VK_NULL_HANDLE),
	size(size),
	mVkDevice(device)
{

}

BufferInfo::~BufferInfo() 
{
	if (buffer != VK_NULL_HANDLE)
		vkDestroyBuffer(mVkDevice, buffer, nullptr);
	if (memory != VK_NULL_HANDLE)
		vkFreeMemory(mVkDevice, memory, nullptr);
}

uint32_t BufferHelper::getMemoryType(
		const VkPhysicalDevice& physicalDevice, 
		uint32_t typeFilter, 
		VkMemoryPropertyFlags& flags)
{
	VkPhysicalDeviceMemoryProperties memProps;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);

	for (size_t i = 0; i < memProps.memoryTypeCount; ++i)
		if ((typeFilter & (1 << i)) 
		&& (memProps.memoryTypes[i].propertyFlags & flags))
			return i;
	throw std::runtime_error("Failed to find memory type");
}

void BufferHelper::mapMemory(const VulkanState& state, BufferInfo& bufferInfo, const void* src) 
{
	mapMemory(state, bufferInfo.memory, 0, bufferInfo.size, src);
}

void BufferHelper::mapMemory(const VulkanState& state, VkDeviceMemory& memory, VkDeviceSize size, const void* src) 
{
	mapMemory(state, memory, 0, size, src);
} 

void BufferHelper::mapMemory(
		const VulkanState& state, 
		VkDeviceMemory& memory, 
		VkDeviceSize offset, 
		VkDeviceSize size, 
		const void* src) 
{
	void* data;
	vkMapMemory(state.device, memory, offset, size, 0 , &data);
	memcpy(data, src, (size_t) size);
	vkUnmapMemory(state.device, memory);
}


void BufferHelper::mapMemory(
		const VkDevice& device, 
		VkDeviceMemory& memory, 
		VkDeviceSize offset, 
		VkDeviceSize size, 
		const void* src) 
{
	void* data;
	vkMapMemory(device, memory, offset, size, 0 , &data);
	memcpy(data, src, (size_t) size);
	vkUnmapMemory(device, memory);
}

void BufferHelper::createBuffer(
		const VkPhysicalDevice& physicalDevice,
		const VkDevice& device,
		VkBuffer& buffer, 
		VkDeviceSize size, 
		VkDeviceMemory& memory, 
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags prop) 
{
	VkBufferCreateInfo buffInfo = {};
	buffInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffInfo.size = size;
	buffInfo.usage = usage;
	buffInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_CHECK_RESULT(vkCreateBuffer(device, &buffInfo, nullptr, &buffer));
	
	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(device, buffer, &memReqs);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReqs.size;
	allocInfo.memoryTypeIndex = getMemoryType(physicalDevice, memReqs.memoryTypeBits, prop);
	
	VK_CHECK_RESULT(vkAllocateMemory(device, &allocInfo, nullptr, &memory));
	vkBindBufferMemory(device, buffer, memory, 0);
}

void BufferHelper::createBuffer(
			const VulkanState& state,
			VkBuffer& buffer, 
			VkDeviceSize size, 
			VkDeviceMemory& memory,  
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags prop) 
{
	createBuffer(
			state.physicalDevice,
			state.device,
			buffer,
			size,
			memory,
			usage,
			prop);

}

void BufferHelper::createBuffer(
			const VulkanState& state,
			BufferInfo& bufferInfo,  
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags prop) 
{
	createBuffer(
			state.physicalDevice,
			state.device,
			bufferInfo.buffer,
			bufferInfo.size,
			bufferInfo.memory,
			usage,
			prop);

}

void BufferHelper::createVertexBuffer(const VulkanState& state, BufferInfo& bufferInfo)
{
	createBuffer(
			state.physicalDevice,
			state.device,
			bufferInfo.buffer,
			bufferInfo.size,
			bufferInfo.memory,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

void BufferHelper::createCommonBuffer(const VulkanState& state, BufferInfo& bufferInfo)
{
	createBuffer(
		state.physicalDevice,
		state.device,
		bufferInfo.buffer,
		bufferInfo.size,
		bufferInfo.memory,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT 
		| VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT 
		| VK_BUFFER_USAGE_INDEX_BUFFER_BIT
		| VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

}

void BufferHelper::createIndexBuffer(const VulkanState& state, BufferInfo& bufferInfo)
{
	createBuffer(
			state.physicalDevice,
			state.device,
			bufferInfo.buffer,
			bufferInfo.size,
			bufferInfo.memory,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

void BufferHelper::createUniformBuffer(const VulkanState& state, BufferInfo& bufferInfo)
{
	createBuffer(
			state.physicalDevice,
			state.device,
			bufferInfo.buffer,
			bufferInfo.size,
			bufferInfo.memory,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}


void BufferHelper::createStagingBuffer(
			const VulkanState& state,
			BufferInfo& bufferInfo) 
{
	createStagingBuffer(
			state,
			bufferInfo.buffer,
			bufferInfo.size,
			bufferInfo.memory);
}

void BufferHelper::createStagingBuffer(
			const VulkanState& state,
			VkBuffer& buffer, 
			VkDeviceSize size, 
			VkDeviceMemory& memory) 
{
	createBuffer(
			state.physicalDevice,
			state.device,
			buffer,
			size,
			memory,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}

void BufferHelper::createStagingBuffer(
			const VkPhysicalDevice& physicalDevice,
			const VkDevice& device,
			VkBuffer& buffer, 
			VkDeviceSize size, 
			VkDeviceMemory& memory) 
{
	createBuffer(
			physicalDevice,
			device,
			buffer,
			size,
			memory,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}

void BufferHelper::copyBuffer(
		const VkDevice& device,
		const VkCommandPool& commandPool,
		const VkQueue& queue,
		VkBuffer src, 
		VkBuffer dst, 
		VkDeviceSize size) 
{
	CmdPass cmdPass(device, commandPool, queue);
	VkBufferCopy bufferCopy = {};
	bufferCopy.size = size;
	vkCmdCopyBuffer(cmdPass.buffer, src, dst, 1, &bufferCopy);
}

void BufferHelper::copyBuffer(
		const VkDevice& device,
		const VkCommandPool& commandPool,
		const VkQueue& queue,
		VkBuffer src, 
		VkBuffer dst, 
		VkDeviceSize offset,
		VkDeviceSize size)
{
	CmdPass cmdPass(device, commandPool, queue);
	VkBufferCopy bufferCopy = {};
	bufferCopy.size = size;
	bufferCopy.dstOffset = offset;
	bufferCopy.srcOffset = offset;
	vkCmdCopyBuffer(cmdPass.buffer, src, dst, 1, &bufferCopy);
}

void BufferHelper::copyBuffer(
			const VulkanState& state,
			VkBuffer src, 
			VkBuffer dst, 
			VkDeviceSize size)
{
	copyBuffer(
			state.device, 
			state.commandPool, 
			state.graphicsQueue,
			src,
			dst,
			size);
}
