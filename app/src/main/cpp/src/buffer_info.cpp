#include "buffer_info.h"

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

