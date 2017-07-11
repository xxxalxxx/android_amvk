#ifndef AMVK_VULKAN_RENDER_PASS_CREATOR_H
#define AMVK_VULKAN_RENDER_PASS_CREATOR_H

#ifdef __ANDROID__
#include "vulkan_wrapper.h"
#else
#include <vulkan/vulkan.h>
#endif


class VulkanRenderPassCreator {
public:
	VulkanRenderPassCreator();
	~VulkanRenderPassCreator();
		
	VkAttachmentDescription attachmentDescColor(VkFormat format) const;
	VkAttachmentDescription attachmentDescDepthNoStencil(VkFormat format) const; 
};

#endif
