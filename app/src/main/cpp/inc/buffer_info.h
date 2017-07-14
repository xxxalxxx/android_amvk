#ifndef AMVK_BUFFER_INFO_H
#define AMVK_BUFFER_INFO_H

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




#endif
