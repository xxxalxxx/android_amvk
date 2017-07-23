#ifndef AMVK_CMD_PASS_H
#define AMVK_CMD_PASS_H

#include "vulkan.h"


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
