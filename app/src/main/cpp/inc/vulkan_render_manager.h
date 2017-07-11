#ifndef AMVK_VULKAN_RENDER_MANAGER_H
#define AMVK_VULKAN_RENDER_MANAGER_H


#ifdef __ANDROID__
#include "vulkan_wrapper.h"
#else
#include <vulkan/vulkan.h>
#endif


#include "vulkan_state.h"
#include "vulkan_render_pass_creator.h"

class VulkanRenderManager {
public:
	VulkanRenderManager(VulkanState& vulkanState);
	~VulkanRenderManager();

private:
	VulkanState& mVulkanState;
};


#endif
