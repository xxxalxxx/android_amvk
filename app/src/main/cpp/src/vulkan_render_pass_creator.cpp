#include "vulkan_render_pass_creator.h"

VulkanRenderPassCreator::VulkanRenderPassCreator() 
{

}

VulkanRenderPassCreator::~VulkanRenderPassCreator() 
{

}

VkAttachmentDescription VulkanRenderPassCreator::attachmentDescColor(VkFormat format) const 
{
	VkAttachmentDescription att = {};
	att.format = format;
	att.samples = VK_SAMPLE_COUNT_1_BIT;
	att.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	att.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	att.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	att.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	att.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	att.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	
	return att;
}

VkAttachmentDescription VulkanRenderPassCreator::attachmentDescDepthNoStencil(VkFormat format) const 
{
	VkAttachmentDescription att = {};
	att.format = format;
	att.samples = VK_SAMPLE_COUNT_1_BIT;
	att.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	att.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	att.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	att.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	att.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	att.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	
	return att;
}
