#include "g_buffer.h"

GBuffer::GBuffer(State& state):
	mState(&state),
	deferredQuad(state)
{

}

GBuffer::~GBuffer() 
{

}



void GBuffer::init(const VkPhysicalDevice& physicalDevice, const VkDevice& device, uint32_t width, uint32_t height)
{
	LOG_TITLE("G-BUFFER");
	this->width = width;
	this->height = height;
	createFramebuffers(physicalDevice, device);
	createSampler(device);
	createCmdBuffer(device, mState->commandPool);
	createDescriptorPool();
	createDescriptors();
	deferredQuad.init();
}


void GBuffer::createFramebuffers(const VkPhysicalDevice& physicalDevice, const VkDevice& device) 
{
	std::array<VkFormat, ATTACHMENT_COUNT> attFormats = {};
	attFormats[INDEX_POSITION] = VK_FORMAT_R16G16B16A16_SFLOAT;
	attFormats[INDEX_NORMAL]   = VK_FORMAT_R16G16B16A16_SFLOAT;
	attFormats[INDEX_ALBEDO]   = VK_FORMAT_R8G8B8A8_UNORM;
	attFormats[INDEX_DEPTH]    = ImageHelper::findDepthStencilFormat(physicalDevice);

	std::array<VkImageUsageFlagBits, ATTACHMENT_COUNT> attUsages = {};
	attUsages[INDEX_POSITION] = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	attUsages[INDEX_NORMAL]   = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	attUsages[INDEX_ALBEDO]   = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	attUsages[INDEX_DEPTH]    = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	for (size_t i = 0; i < ATTACHMENT_COUNT; ++i)
		createAttachment(
				physicalDevice,
				device,
				attachments[i],
				attFormats[i],
				attUsages[i]);

	std::array<VkAttachmentDescription, ATTACHMENT_COUNT> attDescs = {};
	createColorAttachmentDesc(attDescs[INDEX_POSITION], attFormats[INDEX_POSITION]);
	createColorAttachmentDesc(attDescs[INDEX_NORMAL], attFormats[INDEX_NORMAL]);
	createColorAttachmentDesc(attDescs[INDEX_ALBEDO], attFormats[INDEX_ALBEDO]);
	createDepthAttachmentDesc(attDescs[INDEX_DEPTH], attFormats[INDEX_DEPTH]);

	std::array<VkAttachmentReference, ATTACHMENT_COUNT> attRefs = {};
	attRefs[INDEX_POSITION] = { INDEX_POSITION, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	attRefs[INDEX_NORMAL]   = { INDEX_NORMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	attRefs[INDEX_ALBEDO]   = { INDEX_ALBEDO, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	attRefs[INDEX_DEPTH]    = { INDEX_DEPTH, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

	std::array<VkAttachmentReference, COLOR_ATTACHMENT_COUNT> colorRefs = {};
	colorRefs[INDEX_POSITION] = { INDEX_POSITION, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	colorRefs[INDEX_NORMAL]   = { INDEX_NORMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	colorRefs[INDEX_ALBEDO]   = { INDEX_ALBEDO, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	VkAttachmentReference depthRef = { INDEX_DEPTH, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.pColorAttachments = colorRefs.data();
	subpass.colorAttachmentCount = colorRefs.size();
	subpass.pDepthStencilAttachment = &depthRef;

	std::array<VkSubpassDependency, 2> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pAttachments = attDescs.data();
	renderPassInfo.attachmentCount = attDescs.size();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = dependencies.size();
	renderPassInfo.pDependencies = dependencies.data();

	VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));


	std::array<VkImageView, ATTACHMENT_COUNT> viewAttachments = {};
	viewAttachments[INDEX_POSITION] = attachments[INDEX_POSITION].view;
	viewAttachments[INDEX_NORMAL]   = attachments[INDEX_NORMAL].view;
	viewAttachments[INDEX_ALBEDO]   = attachments[INDEX_ALBEDO].view;
	viewAttachments[INDEX_DEPTH]    = attachments[INDEX_DEPTH].view;

	VkFramebufferCreateInfo fbufCreateInfo = {};
	fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbufCreateInfo.pNext = NULL;
	fbufCreateInfo.renderPass = renderPass;
	fbufCreateInfo.pAttachments = viewAttachments.data();
	fbufCreateInfo.attachmentCount = viewAttachments.size();
	fbufCreateInfo.width = width;
	fbufCreateInfo.height = height;
	fbufCreateInfo.layers = 1;
	
	VK_CHECK_RESULT(vkCreateFramebuffer(device, &fbufCreateInfo, nullptr, &frameBuffer));

	LOG("G-BUFFER FRAMEBUFFER CREATED");
}

void GBuffer::createColorAttachmentDesc(VkAttachmentDescription& desc, VkFormat format) 
{
	desc.samples = VK_SAMPLE_COUNT_1_BIT;
	desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	desc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	desc.format = format;
}

void GBuffer::createDepthAttachmentDesc(VkAttachmentDescription& desc, VkFormat format)
{
	desc.samples = VK_SAMPLE_COUNT_1_BIT;
	desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	desc.format = format;
}

void GBuffer::createAttachment(
		const VkPhysicalDevice& physicalDevice, 
		const VkDevice& device,
		FramebufferAttachment& attachment,
		VkFormat format,  
		VkImageUsageFlagBits usage)
{
	VkImageAspectFlags aspectMask = 0;
	VkImageLayout imageLayout;

	if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
		aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}
	if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
		aspectMask = 
			VK_IMAGE_ASPECT_DEPTH_BIT;
			//VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

	if (aspectMask == 0)
		throw std::runtime_error("Invalid usage for aspectMask");

	VkImageCreateInfo image = {};
	image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image.imageType = VK_IMAGE_TYPE_2D;
	image.format = format;
	image.extent.width = width;
	image.extent.height = height;
	image.extent.depth = 1;
	image.mipLevels = 1;
	image.arrayLayers = 1;
	image.samples = VK_SAMPLE_COUNT_1_BIT;
	image.tiling = VK_IMAGE_TILING_OPTIMAL;
	image.usage = usage | VK_IMAGE_USAGE_SAMPLED_BIT;

	VkMemoryAllocateInfo memAlloc = {};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	VkMemoryRequirements memReqs;

	VK_CHECK_RESULT(vkCreateImage(device, &image, nullptr, &attachment.image));
	vkGetImageMemoryRequirements(device, attachment.image, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = BufferHelper::getMemoryType(physicalDevice, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, &attachment.memory));
	VK_CHECK_RESULT(vkBindImageMemory(device, attachment.image, attachment.memory, 0));
	
	VkImageViewCreateInfo imageView = {};
	imageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageView.format = format;
	imageView.subresourceRange = {};
	imageView.subresourceRange.aspectMask = aspectMask;
	imageView.subresourceRange.baseMipLevel = 0;
	imageView.subresourceRange.levelCount = 1;
	imageView.subresourceRange.baseArrayLayer = 0;
	imageView.subresourceRange.layerCount = 1;
	imageView.image = attachment.image;
	VK_CHECK_RESULT(vkCreateImageView(device, &imageView, nullptr, &attachment.view));
}

void GBuffer::createSampler(const VkDevice& device)
{
	VkSamplerCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	info.magFilter = VK_FILTER_NEAREST;
	info.minFilter = VK_FILTER_NEAREST;
	info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	info.addressModeV = info.addressModeU;
	info.addressModeW = info.addressModeU;
	info.mipLodBias = 0.0f;
	info.maxAnisotropy = 1.0f;
	info.minLod = 0.0f;
	info.maxLod = 1.0f;
	info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	VK_CHECK_RESULT(vkCreateSampler(device, &info, nullptr, &sampler));
}

void GBuffer::createCmdBuffer(const VkDevice& device, const VkCommandPool& cmdPool)
{
	VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
	cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferAllocInfo.commandPool = cmdPool;
	cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; 
	cmdBufferAllocInfo.commandBufferCount = 1;

	VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufferAllocInfo, &cmdBuffer));
	LOG("G-BUFFER CMD BUFFER CREATED");
}

void GBuffer::createDescriptorPool()
{
	// VkDescriptorType    type;
	// uint32_t            descriptorCount;

	VkDescriptorPoolSize poolSizes[] = {
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, ATTACHMENT_COUNT }
	};
	uint32_t maxSets = ATTACHMENT_COUNT;
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = ARRAY_SIZE(poolSizes);
	poolInfo.pPoolSizes = poolSizes;
	poolInfo.maxSets = maxSets;

	VK_CHECK_RESULT(vkCreateDescriptorPool(mState->device, &poolInfo, nullptr, &mDescriptorPool));
}

void GBuffer::createDescriptors()
{
	VkDescriptorSetLayout layouts[] = {
		mState->descriptorSetLayouts.deferred
	};

	VkDescriptorSetAllocateInfo samplerAllocInfo = {};
	samplerAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	samplerAllocInfo.descriptorPool = mDescriptorPool;
	samplerAllocInfo.descriptorSetCount = ARRAY_SIZE(layouts);
	samplerAllocInfo.pSetLayouts = layouts;

	VK_CHECK_RESULT(vkAllocateDescriptorSets(mState->device, &samplerAllocInfo, &mDescriptorSet));

	std::array<VkWriteDescriptorSet, ATTACHMENT_COUNT> writeSets = {};
	std::array<VkDescriptorImageInfo, ATTACHMENT_COUNT> imageInfos = {};

	for (size_t i = 0; i < ATTACHMENT_COUNT; ++i) {
		VkDescriptorImageInfo& descriptorInfo = imageInfos[i];
		descriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		descriptorInfo.imageView = attachments[i].view;
		descriptorInfo.sampler = sampler;
		
		VkWriteDescriptorSet& writeSet = writeSets[i];
		writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeSet.dstSet = mDescriptorSet;
		writeSet.dstBinding = i;
		writeSet.dstArrayElement = 0;
		writeSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeSet.descriptorCount = 1;
		writeSet.pImageInfo = &descriptorInfo;
	}
	vkUpdateDescriptorSets(mState->device, writeSets.size(), writeSets.data(), 0, nullptr);
	LOG("G-Buffer descriptors created");
}


void GBuffer::drawDeferredQuad(VkCommandBuffer& cmdBuffer)
{
	deferredQuad.draw(
		cmdBuffer, 
		mState->pipelines.deferred.pipeline, 
		mState->pipelines.deferred.layout, 
		&mDescriptorSet, 
		1);
}

