#ifndef AMVK_VULKAN_IMAGE_MANAGER_H
#define AMVK_VULKAN_IMAGE_MANAGER_H

#include "vulkan.h"
#include <stdexcept>
#include "cmd_pass.h"
#include "state.h"
#include "vulkan_utils.h"
#include "buffer_helper.h"
#include "image_info.h"
#include "texture_data.h"
#include "macro.h"

namespace ImageHelper {

inline void createImage(
		State& state,
		ImageInfo& imageDesc, 
		VkFormat format, 
		VkImageTiling tiling,
		VkImageLayout initialLayout,
		VkImageUsageFlags usage, 
		VkMemoryPropertyFlags properties)
{
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = imageDesc.width;
	imageInfo.extent.height = imageDesc.height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = initialLayout;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_CHECK_RESULT(vkCreateImage(state.device, &imageInfo, nullptr, &imageDesc.image));
	VkMemoryRequirements memReqs;
	vkGetImageMemoryRequirements(state.device, imageDesc.image, &memReqs);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReqs.size;
	allocInfo.memoryTypeIndex = BufferHelper::getMemoryType(state.physicalDevice, memReqs.memoryTypeBits, properties);

	VK_CHECK_RESULT(vkAllocateMemory(state.device, &allocInfo, nullptr, &imageDesc.memory));
	vkBindImageMemory(state.device, imageDesc.image, imageDesc.memory, 0);
}

inline void createImageView(
		const VkDevice& device,
		VkImage image, 
		VkFormat format, 
		VkImageAspectFlags aspectFlags, 
		VkImageView& imageView)
{
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;

	viewInfo.subresourceRange.aspectMask = aspectFlags;

	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;
	
	VK_CHECK_RESULT(vkCreateImageView(device, &viewInfo, nullptr, &imageView));
}

inline void createImageView(
		const VkDevice& device,
		ImageInfo& imageDesc, 
		VkFormat format, 
		VkImageAspectFlags aspectFlags)
{
	createImageView(device, imageDesc.image, format, aspectFlags, imageDesc.imageView);
}
/*
void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) const
{
	CmdPass cmdPass(mState.device, mState.commandPool, mState.graphicsQueue);

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT)
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	} else {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED 
	&& newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED 
	&& newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL 
	&& newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED
	&& newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {

		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT
							  | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	} else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	vkCmdPipelineBarrier(
			cmdPass.buffer,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			0, 
			0, 
			nullptr, 
			0, 
			nullptr, 
			1, 
			&barrier);
}*/

inline void transitionLayout(
		VkCommandBuffer& cmdBuffer,
		VkImage image, 
		VkFormat format, 
		VkImageLayout oldLayout, 
		VkImageLayout newLayout,
		VkImageAspectFlags barrierAspectMask,
		VkAccessFlags srcAccessMask,
		VkAccessFlags dstAccessMask)
{

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = barrierAspectMask;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = srcAccessMask;
	barrier.dstAccessMask = dstAccessMask;

	vkCmdPipelineBarrier(
			cmdBuffer,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			0, 
			0, nullptr, 
			0, nullptr, 
			1, &barrier);
}

inline void transitionLayout(
		State& state,
		VkImage image, 
		VkFormat format, 
		VkImageLayout oldLayout, 
		VkImageLayout newLayout,
		VkImageAspectFlags barrierAspectMask,
		VkAccessFlags srcAccessMask,
		VkAccessFlags dstAccessMask)
{
	CmdPass cmdPass(state.device, state.commandPool, state.graphicsQueue);

	transitionLayout(
			cmdPass.buffer, 
			image, 
			format, 
			oldLayout, 
			newLayout, 
			barrierAspectMask, 
			srcAccessMask, 
			dstAccessMask);
}


inline void copyImage(VkCommandBuffer& cmdBuffer, VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height)
{
	VkImageSubresourceLayers subRes = {};
	subRes.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subRes.baseArrayLayer = 0;
	subRes.mipLevel = 0;
	subRes.layerCount = 1;

	VkImageCopy copy = {};
	copy.srcSubresource = subRes;
	copy.dstSubresource = subRes;
	copy.srcOffset = {0, 0, 0};
	copy.dstOffset = {0, 0, 0};
	copy.extent.width = width;
	copy.extent.height = height;
	copy.extent.depth = 1;

	vkCmdCopyImage(
			cmdBuffer, 
			srcImage, 
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			dstImage, 
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, 
			&copy);
}

inline void copyImage(const State& state, VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height)
{
	CmdPass cmd(state.device, state.commandPool, state.graphicsQueue);
	copyImage(cmd.buffer, srcImage, dstImage, width, height);
}


	
inline void copyImage(
			State& state,
			ImageInfo& srcImage, 
			ImageInfo& dstImage) 
{
	copyImage(state, srcImage.image, dstImage.image, srcImage.width, srcImage.height);
}


inline VkFormat findSupportedFormat(
		const State& state,
		const std::vector<VkFormat>& candidates, 
		VkImageTiling tiling, 
		VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(state.physicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR
		&& (props.linearTilingFeatures & features) == features) 
			return format;
		else if (tiling == VK_IMAGE_TILING_OPTIMAL
		&& (props.optimalTilingFeatures & features) == features)
			return format;
	}
	throw std::runtime_error("failed to find supported format");
}

inline VkFormat findDepthStencilFormat(const VkPhysicalDevice& physicalDevice) 
{
	std::vector<VkFormat> depthStencilFormats = {
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		//VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		//VK_FORMAT_D16_UNORM
	};


	for (auto& format : depthStencilFormats) {
		VkFormatProperties formatProps;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);
		if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			return format;
	}
	throw std::runtime_error("Unable to find supported depth stencil format");
}


inline VkFormat findSupportedFormat(
		const VkPhysicalDevice& physicalDevice, 
		const std::vector<VkFormat>& candidates, 
		VkImageTiling tiling, 
		VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR
		&& (props.linearTilingFeatures & features) == features) 
			return format;
		else if (tiling == VK_IMAGE_TILING_OPTIMAL
		&& (props.optimalTilingFeatures & features) == features)
			return format;
	}
	throw std::runtime_error("failed to find supported format");
}



inline VkFormat findDepthFormat(VkPhysicalDevice& physicalDevice)
{
	return findSupportedFormat(
			physicalDevice,
			{VK_FORMAT_D24_UNORM_S8_UINT},
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

inline void createStagedImage(
		ImageInfo& imageInfo, 
		const TextureData& textureData,
		State& state,
		const VkCommandPool& cmdPool, 
		const VkQueue& cmdQueue) 
 
{
	// Create staging image
	ImageInfo stagingDesc(state.device, textureData.width, textureData.height);

	createImage(
			state, 
			stagingDesc, 
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_LINEAR, 
			VK_IMAGE_LAYOUT_PREINITIALIZED,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT); 
	
	BufferHelper::mapMemory(state.device, stagingDesc.memory, 0, textureData.size, textureData.pixels);
	
	createImage(
			state, 
			imageInfo, 
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL, 
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	// Setup memory barriers to transition images
	{
	CmdPass cmd(state.device, cmdPool, cmdQueue);

		transitionLayout(
				cmd.buffer,
				stagingDesc.image,
				VK_FORMAT_R8G8B8A8_UNORM,
				VK_IMAGE_LAYOUT_PREINITIALIZED, 
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VK_IMAGE_ASPECT_COLOR_BIT,
				0,
				VK_ACCESS_TRANSFER_READ_BIT);

		transitionLayout(
				cmd.buffer,
				imageInfo.image,
				VK_FORMAT_R8G8B8A8_UNORM,
				VK_IMAGE_LAYOUT_UNDEFINED, 
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_ASPECT_COLOR_BIT,
				0,
				VK_ACCESS_TRANSFER_WRITE_BIT);

		// copy staging buffer to image

		copyImage(cmd.buffer, stagingDesc.image, imageInfo.image, imageInfo.width, imageInfo.height);

		transitionLayout(
				cmd.buffer,
				imageInfo.image,
				VK_FORMAT_R8G8B8A8_UNORM, 
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_IMAGE_ASPECT_COLOR_BIT,
				VK_ACCESS_TRANSFER_WRITE_BIT,
				VK_ACCESS_SHADER_READ_BIT);
	}
//#endif

	// Create ImageView

	createImageView(
			state.device,
			imageInfo.image, 
			VK_FORMAT_R8G8B8A8_UNORM, 
			VK_IMAGE_ASPECT_COLOR_BIT, 
			imageInfo.imageView);

	// Create Sampler
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = state.deviceInfo.samplerAnisotropy;
	samplerInfo.maxAnisotropy = state.deviceInfo.samplerAnisotropy ? 1.f : 1.f;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	
	VK_CHECK_RESULT(vkCreateSampler(state.device, &samplerInfo, nullptr, &imageInfo.sampler));
}


};

#endif
