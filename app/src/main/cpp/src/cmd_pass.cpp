#include "cmd_pass.h"

CmdPass::CmdPass(const VkDevice& vkDevice, const VkCommandPool& vkCommandPool, const VkQueue& vkQueue): 
	buffer(VK_NULL_HANDLE), 
	mVkDevice(vkDevice), 
	mVkCommandPool(vkCommandPool), 
	mVkQueue(vkQueue)
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = vkCommandPool;
	allocInfo.commandBufferCount = 1;

	vkAllocateCommandBuffers(vkDevice, &allocInfo, &buffer);
	
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(buffer, &beginInfo);
}

CmdPass::~CmdPass()
{
	vkEndCommandBuffer(buffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType =  VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &buffer;

	vkQueueSubmit(mVkQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(mVkQueue);
	vkFreeCommandBuffers(mVkDevice, mVkCommandPool, 1, &buffer);
}

