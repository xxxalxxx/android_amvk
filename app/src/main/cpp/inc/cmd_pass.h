#ifndef AMVK_CMD_PASS_H
#define AMVK_CMD_PASS_H

#ifdef __ANDROID__
#include "vulkan_wrapper.h"
#else
#include <vulkan/vulkan.h>
#endif


class CmdPass {
public:
	CmdPass(const VkDevice& vkDevice, const VkCommandPool& vkCommandPool, const VkQueue& vkQueue);
	~CmdPass();
	VkCommandBuffer buffer;
private:
	const VkDevice& mVkDevice;
	const VkCommandPool& mVkCommandPool;
	const VkQueue& mVkQueue;
};

#endif
