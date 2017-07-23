#ifndef AMVK_BUFFER_INFO_H
#define AMVK_BUFFER_INFO_H

#include "vulkan.h"

#include "cmd_pass.h"
#include "vulkan_utils.h"
#include "state.h"

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




#endif
