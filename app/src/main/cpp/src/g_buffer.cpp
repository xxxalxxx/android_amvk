#include "g_buffer.h"

GBuffer::GBuffer(State& state):
	mState(&state),
	mUniformBufferInfo(state.device),
	mTilingUniformBufferInfo(state.device),
	mPointLightsBufferInfo(state.device),
	deferredQuad(state)
{

}

GBuffer::~GBuffer() 
{

}



void GBuffer::init(const VkPhysicalDevice& physicalDevice, const VkDevice& device, uint32_t width, uint32_t height)
{
	LOG_TITLE("G-BUFFER");

	pointLights.push_back(PointLight());
	
	this->width = width;
	this->height = height;

	deferredQuad.init();

	createTilingCmdPool();
	createTilingResultImage();
	//throw std::runtime_error("STOP");
	createFramebuffers(physicalDevice, device);
	createSampler(device);
	createCmdBuffer(device, mState->commandPool);
	createBuffers();
	createDescriptorPool();
	createDescriptors();
}


void GBuffer::createFramebuffers(const VkPhysicalDevice& physicalDevice, const VkDevice& device) 
{
	std::array<VkFormat, ATTACHMENT_COUNT> attFormats = {};
	attFormats[INDEX_POSITION] = VK_FORMAT_R16G16B16A16_SFLOAT;
	attFormats[INDEX_NORMAL]   = VK_FORMAT_R16G16B16A16_SFLOAT;
	attFormats[INDEX_ALBEDO]   = VK_FORMAT_R8G8B8A8_UNORM;
	attFormats[INDEX_DEPTH]    = ImageHelper::findDepthStencilFormat(physicalDevice);

	std::array<VkImageUsageFlags, ATTACHMENT_COUNT> attUsages = {};
	attUsages[INDEX_POSITION] = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
	attUsages[INDEX_NORMAL]   = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
	attUsages[INDEX_ALBEDO]   = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
	attUsages[INDEX_DEPTH]    = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT 
							  | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

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

	std::array<VkAttachmentReference, COLOR_ATTACHMENT_COUNT> colorRefs = {};
	colorRefs[INDEX_POSITION] = { INDEX_POSITION, VK_IMAGE_LAYOUT_GENERAL };
	colorRefs[INDEX_NORMAL]   = { INDEX_NORMAL, VK_IMAGE_LAYOUT_GENERAL };
	colorRefs[INDEX_ALBEDO]   = { INDEX_ALBEDO, VK_IMAGE_LAYOUT_GENERAL };
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
	dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	
	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
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

void GBuffer::createTilingResultImage() 
{
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(mState->physicalDevice, VK_FORMAT_R8G8B8A8_UNORM, &formatProperties);
    if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) {
        LOG("PROP TILING SUPPORTED");
    } else {
        LOG("PROP TILING UNSUPPORTED");
    }


	if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT) {
        LOG("PROP BLIT SUPPORTED");
    } else {
        LOG("PROP BLIT UNSUPPORTED");
    }

	VkImageCreateInfo image = {};
	image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image.imageType = VK_IMAGE_TYPE_2D;
	image.format = VK_FORMAT_R8G8B8A8_UNORM;
    image.extent = { (uint32_t) width, (uint32_t) height, 1 };
	image.mipLevels = 1;
	image.arrayLayers = 1;
	image.samples = VK_SAMPLE_COUNT_1_BIT;
	image.tiling = VK_IMAGE_TILING_OPTIMAL;
	image.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

	VkMemoryAllocateInfo memAlloc = {};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	VkMemoryRequirements memReqs;

	VK_CHECK_RESULT(vkCreateImage(mState->device, &image, nullptr, &tilingImage.image));
	vkGetImageMemoryRequirements(mState->device, tilingImage.image, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = BufferHelper::getMemoryType(mState->physicalDevice, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(mState->device, &memAlloc, nullptr, &tilingImage.memory));
	VK_CHECK_RESULT(vkBindImageMemory(mState->device, tilingImage.image, tilingImage.memory, 0));
	
	VkImageViewCreateInfo imageView = {};
	imageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageView.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageView.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	imageView.image = tilingImage.image;
	VK_CHECK_RESULT(vkCreateImageView(mState->device, &imageView, nullptr, &tilingImage.view));

	LOG("Tiling image created");
}

void GBuffer::createColorAttachmentDesc(VkAttachmentDescription& desc, VkFormat format) 
{
	desc.samples = VK_SAMPLE_COUNT_1_BIT;
	desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;//VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
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

void  GBuffer::initColorImageTransition(CmdPass& cmd, FramebufferAttachment& attachment) {
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = attachment.image;
    barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(
            cmd.buffer,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier);
}

void GBuffer::createAttachment(
		const VkPhysicalDevice& physicalDevice, 
		const VkDevice& device,
		FramebufferAttachment& attachment,
		VkFormat format,  
		VkImageUsageFlags usage)
{
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(mState->physicalDevice, format, &formatProperties);
    VkImageTiling tiling;
	VkImageAspectFlags aspectMask = 0;

    if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        aspectMask =
                VK_IMAGE_ASPECT_DEPTH_BIT;
        if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            tiling = VK_IMAGE_TILING_OPTIMAL;
            LOG("Optimal tiling found for depth stencil attachment with format: %u", format);
        } else if (formatProperties.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            tiling = VK_IMAGE_TILING_LINEAR;
            LOG("Linear tiling found for depth stencil attachment with format: %u", format);
        } else {
            LOG("Tiling not found for depth stencil attachment with format: %u", format);
            throw std::runtime_error("Tiling not found");
        }
        //VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    } else if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
		aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        if (formatProperties.linearTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) {
            tiling = VK_IMAGE_TILING_LINEAR;
            LOG("Linear tiling found for color attachment with format: %u", format);
        } else if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) {
            tiling = VK_IMAGE_TILING_OPTIMAL;
            LOG("Optimal tiling found for color attachment with format: %u", format);
        } else {
            LOG("Tiling not found for color attachment with format: %u", format);
            throw std::runtime_error("Tiling not found");
        }
    } else {
        throw std::runtime_error("Invalid usage for aspectMask");
    }

	VkImageCreateInfo image = {};
	image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image.imageType = VK_IMAGE_TYPE_2D;
	image.format = format;
    image.extent = { (uint32_t) width, (uint32_t) height, 1 };
	image.mipLevels = 1;
	image.arrayLayers = 1;
	image.samples = VK_SAMPLE_COUNT_1_BIT;
	image.tiling = tiling;
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
    imageView.subresourceRange = { aspectMask, 0, 1, 0, 1 };
	imageView.image = attachment.image;
	VK_CHECK_RESULT(vkCreateImageView(device, &imageView, nullptr, &attachment.view));
}

void GBuffer::createSampler(const VkDevice& device)
{
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = mState->deviceInfo.samplerAnisotropy;
    samplerInfo.maxAnisotropy = mState->deviceInfo.samplerAnisotropy ? 1.f : 1.f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    VK_CHECK_RESULT(vkCreateSampler(mState->device, &samplerInfo, nullptr, &sampler));

    /*
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
	VK_CHECK_RESULT(vkCreateSampler(device, &info, nullptr, &sampler));*/
}

void GBuffer::createCmdBuffer(const VkDevice& device, const VkCommandPool& cmdPool)
{
	LOG("creating cmd buffers");
	VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
	cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferAllocInfo.commandPool = mState->commandPool;
	cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; 
	cmdBufferAllocInfo.commandBufferCount = 1;

	VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufferAllocInfo, &cmdBuffer));

	VkCommandBufferAllocateInfo tilingCmdBufferAllocInfo = {};
	tilingCmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	tilingCmdBufferAllocInfo.commandPool = tilingCmdPool;
	tilingCmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; 
	tilingCmdBufferAllocInfo.commandBufferCount = 1;

	VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &tilingCmdBufferAllocInfo, &tilingCmdBuffer));

	LOG("G-BUFFER CMD BUFFER CREATED");
}

void GBuffer::createDescriptorPool()
{
	// VkDescriptorType    type;
	// uint32_t            descriptorCount;

	uint32_t descriptorCount = ATTACHMENT_COUNT + TILING_IMAGE_COUNT + UNIFORM_BUFFER_COUNT + STORAGE_BUFFER_COUNT + pointLights.size();
	VkDescriptorPoolSize poolSizes[] = {
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, descriptorCount },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, descriptorCount },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, descriptorCount },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, descriptorCount },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, descriptorCount },
	};

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = ARRAY_SIZE(poolSizes);
	poolInfo.pPoolSizes = poolSizes;
	poolInfo.maxSets = descriptorCount;

	VK_CHECK_RESULT(vkCreateDescriptorPool(mState->device, &poolInfo, nullptr, &mDescriptorPool));
}

void GBuffer::createDescriptors()
{
	VkDescriptorSetLayout layouts[] = {
		mState->descriptorSetLayouts.deferred,
		mState->descriptorSetLayouts.tiling
	};

	std::array<VkDescriptorSet, 2> descriptorSets = {};

	VkDescriptorSetAllocateInfo samplerAllocInfo = {};
	samplerAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	samplerAllocInfo.descriptorPool = mDescriptorPool;
	samplerAllocInfo.descriptorSetCount = ARRAY_SIZE(layouts);
	samplerAllocInfo.pSetLayouts = layouts;

	VK_CHECK_RESULT(vkAllocateDescriptorSets(mState->device, &samplerAllocInfo, descriptorSets.data()));

	mDescriptorSet = descriptorSets[0];
	mTilingDescriptorSet = descriptorSets[1];

	std::vector<VkWriteDescriptorSet> writeSets = {};
	std::array<VkDescriptorImageInfo, ATTACHMENT_COUNT> imageInfos = {};

	for (size_t i = 0; i < ATTACHMENT_COUNT; ++i) {
		VkDescriptorImageInfo& descriptorInfo = imageInfos[i];
		descriptorInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		descriptorInfo.imageView = attachments[i].view;
		descriptorInfo.sampler = sampler;
		
		writeSets.push_back(VkWriteDescriptorSet());
		VkWriteDescriptorSet& writeSet = writeSets.back();
		writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeSet.dstSet = mDescriptorSet;
		writeSet.dstBinding = i;
		writeSet.dstArrayElement = 0;
		writeSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeSet.descriptorCount = 1;
		writeSet.pImageInfo = &descriptorInfo;
	}

	VkDescriptorBufferInfo buffInfo = {};
	buffInfo.buffer = mUniformBufferInfo.buffer;
	buffInfo.offset = 0;
	buffInfo.range = sizeof(State::UBO);

	writeSets.push_back(VkWriteDescriptorSet());
	VkWriteDescriptorSet& uniformSet = writeSets.back();
	uniformSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	uniformSet.dstSet = mDescriptorSet;
	uniformSet.dstBinding = ATTACHMENT_COUNT;
	uniformSet.dstArrayElement = 0;
	uniformSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformSet.descriptorCount = 1;
	uniformSet.pBufferInfo = &buffInfo;

	std::array<VkImageView, ATTACHMENT_COUNT> tilingImageViews = {};
	tilingImageViews[INDEX_POSITION] = tilingImage.view;
	tilingImageViews[INDEX_NORMAL]   =
            //deferredQuad.mImageInfo->imageView;
        attachments[INDEX_NORMAL].view;
            //attachments[INDEX_ALBEDO].view;
	tilingImageViews[INDEX_ALBEDO]   =
        //    deferredQuad.mImageInfo->imageView;
            attachments[INDEX_ALBEDO].view;

	std::array<VkDescriptorImageInfo, TILING_IMAGE_COUNT> tilingImageInfos = {};
	for (size_t i = 0; i < TILING_IMAGE_COUNT; ++i) {
		VkDescriptorImageInfo& descriptorInfo = tilingImageInfos[i];
		descriptorInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		descriptorInfo.imageView = tilingImageViews[i];
		
		writeSets.push_back(VkWriteDescriptorSet());
		VkWriteDescriptorSet& writeSet = writeSets.back();
		writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeSet.dstSet = mTilingDescriptorSet;
		writeSet.dstBinding = i;
		writeSet.dstArrayElement = 0;
		writeSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		writeSet.descriptorCount = 1;
		writeSet.pImageInfo = &descriptorInfo;
	}

	VkDescriptorBufferInfo tilingBuffInfo = {};
	tilingBuffInfo.buffer = mTilingUniformBufferInfo.buffer;
	tilingBuffInfo.offset = 0;
	tilingBuffInfo.range = sizeof(TilingUBO);

	writeSets.push_back(VkWriteDescriptorSet());
	VkWriteDescriptorSet& tilingUniformSet = writeSets.back();
	tilingUniformSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	tilingUniformSet.dstSet = mTilingDescriptorSet;
	tilingUniformSet.dstBinding = 3;
	tilingUniformSet.dstArrayElement = 0;
	tilingUniformSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	tilingUniformSet.descriptorCount = 1;
	tilingUniformSet.pBufferInfo = &tilingBuffInfo;

	VkDescriptorBufferInfo pointLightsBuffInfo = {};
	pointLightsBuffInfo.buffer = mPointLightsBufferInfo.buffer;
	pointLightsBuffInfo.offset = 0;
	pointLightsBuffInfo.range = sizeof(PointLight) * pointLights.size();

	writeSets.push_back(VkWriteDescriptorSet());
	VkWriteDescriptorSet& pointLightsUniformSet = writeSets.back();
	pointLightsUniformSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	pointLightsUniformSet.dstSet = mTilingDescriptorSet;
	pointLightsUniformSet.dstBinding = 4;
	pointLightsUniformSet.dstArrayElement = 0;
	pointLightsUniformSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	pointLightsUniformSet.descriptorCount = 1;
	pointLightsUniformSet.pBufferInfo = &pointLightsBuffInfo;

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

void GBuffer::update(VkCommandBuffer& cmdBuffer, const Timer& timer, Camera& camera)
{
	mState->ubo.view = camera.view();
	mState->ubo.proj = camera.proj();

	vkCmdUpdateBuffer(
			cmdBuffer,
			mUniformBufferInfo.buffer,
			0,
			sizeof(State::UBO),
			&mState->ubo);
	//LOG("G-Buffer update");

}

void GBuffer::updateTiling(VkCommandBuffer& cmdBuffer, const Timer& timer, Camera& camera)
{

}

void GBuffer::createBuffers()
{
	LOG("creating buffers");
	mUniformBufferInfo.size = sizeof(State::UBO);
	BufferHelper::createUniformBuffer(*mState, mUniformBufferInfo);

	mTilingUniformBufferInfo.size = sizeof(TilingUBO);
	BufferHelper::createUniformBuffer(*mState, mTilingUniformBufferInfo);

	mPointLightsBufferInfo.size = sizeof(PointLight) * pointLights.size();
	BufferHelper::createStorageBuffer(*mState, mPointLightsBufferInfo);

	LOG("G-Buffer buffers created");
}

void GBuffer::createTilingCmdPool() 
{
	VkCommandPoolCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.queueFamilyIndex = mState->computeQueueIndex;
	createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VK_CHECK_RESULT(vkCreateCommandPool(mState->device, &createInfo, nullptr, &tilingCmdPool));
	LOG("Deferred command pool created compute index: %u, pool:%d", mState->computeQueueIndex, tilingCmdPool != VK_NULL_HANDLE);
}

void GBuffer::dispatch() 
{
	vkCmdBindPipeline(tilingCmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, mState->pipelines.tiling.pipeline);
	vkCmdBindDescriptorSets(tilingCmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, mState->pipelines.tiling.layout, 0, 1, &mTilingDescriptorSet, 0, 0);
	//vkCmdDispatch(tilingCmdBuffer, 2 * 256, 1, 1);
    vkCmdDispatch(tilingCmdBuffer, (width + WORK_GROUP_SIZE - 1) / WORK_GROUP_SIZE, (height + WORK_GROUP_SIZE - 1) / WORK_GROUP_SIZE, 1);

    //vkCmdDispatch(tilingCmdBuffer, width, height, 1);
//	vkCmdDispatch(tilingCmdBuffer, 1, 1, 1);

}

VkImageMemoryBarrier GBuffer::createTilingDstBarrier(VkImage image) 
{
	VkImageMemoryBarrier loadBarrier = {};
	loadBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	loadBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		//VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	loadBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	loadBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	loadBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	loadBarrier.srcQueueFamilyIndex = mState->graphicsQueueIndex;
	loadBarrier.dstQueueFamilyIndex = mState->computeQueueIndex;
	loadBarrier.image = image;
    loadBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

	return loadBarrier;
}

VkImageMemoryBarrier GBuffer::createTilingSrcBarrier(VkImage image) 
{
	VkImageMemoryBarrier loadBarrier = {};
	loadBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	loadBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	loadBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		//VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	loadBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
	loadBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	loadBarrier.srcQueueFamilyIndex = mState->computeQueueIndex;
	loadBarrier.dstQueueFamilyIndex = mState->graphicsQueueIndex;
	loadBarrier.image = image;
    loadBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

	return loadBarrier;
}

