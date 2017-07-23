#ifndef AMVK_G_BUFFER_H
#define AMVK_G_BUFFER_H

#include <array>

#include "macro.h"
#include "vulkan.h"
#include "buffer_helper.h"
#include "image_helper.h"
#include "state.h"
#include "fullscreen_quad.h"

struct FramebufferAttachment {
	VkImage image;
	VkDeviceMemory memory;
	VkImageView view;
	VkFormat format;
};

class GBuffer {
public:
	static const constexpr uint32_t INDEX_POSITION = 0;
	static const constexpr uint32_t INDEX_NORMAL   = 1;
	static const constexpr uint32_t INDEX_ALBEDO   = 2;
	static const constexpr uint32_t INDEX_DEPTH    = 3;

	static const constexpr uint32_t DEPTH_ATTACHMENT_COUNT = 1;
	static const constexpr uint32_t ATTACHMENT_COUNT = 4;
	static const constexpr uint32_t COLOR_ATTACHMENT_COUNT = ATTACHMENT_COUNT - DEPTH_ATTACHMENT_COUNT;

	inline FramebufferAttachment& position() { return attachments[INDEX_POSITION]; }
	inline FramebufferAttachment& normal() { return attachments[INDEX_NORMAL]; }
	inline FramebufferAttachment& albedo() { return attachments[INDEX_ALBEDO]; }
	inline FramebufferAttachment& depth() { return attachments[INDEX_DEPTH]; }

	GBuffer(State& state);
	virtual ~GBuffer();

	void init(const VkPhysicalDevice& physicalDevice, const VkDevice& device, uint32_t width, uint32_t height);
	void createCmdBuffer(const VkDevice& device, const VkCommandPool& cmdPool);
	void drawDeferredQuad(VkCommandBuffer& cmdBuffer);

	std::array<FramebufferAttachment, ATTACHMENT_COUNT> attachments;
	std::array<VkClearValue, ATTACHMENT_COUNT> clearValues;
	int32_t width, height;
	VkFramebuffer frameBuffer;
	VkRenderPass renderPass;
	VkSampler sampler;
	VkCommandBuffer cmdBuffer;

private:
	void createFramebuffers(const VkPhysicalDevice& physicalDevice, const VkDevice& device);
	
	void createAttachment(
		const VkPhysicalDevice& physicalDevice, 
		const VkDevice& device,
		FramebufferAttachment& attachment,
		VkFormat format,  
		VkImageUsageFlagBits usage);

	void createSampler(const VkDevice& device);
	void createDescriptorPool();
	void createDescriptors();
	void createColorAttachmentDesc(VkAttachmentDescription& desc, VkFormat format);
	void createDepthAttachmentDesc(VkAttachmentDescription& desc, VkFormat format);

	const State* mState;
	VkDescriptorPool mDescriptorPool;
	VkDescriptorSet mDescriptorSet;
public:
	FullscreenQuad deferredQuad;
};


#endif
