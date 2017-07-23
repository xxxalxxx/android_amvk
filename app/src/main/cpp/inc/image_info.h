#ifndef VULKAN_IMAGE_INFO
#define VULKAN_IMAGE_INFO


#include "vulkan.h"

#include "macro.h"

class ImageInfo {
public:
	ImageInfo();
	ImageInfo(VkDevice& vkDevice);
	ImageInfo(VkDevice& vkDevice, uint32_t width, uint32_t height);
	virtual ~ImageInfo();
	
	ImageInfo& operator=(ImageInfo other);

	uint32_t width, height;
	VkImage image;
	VkImageView imageView;
	VkDeviceMemory memory;
	VkSampler sampler;
	VkDevice* mVkDevice;
private:

};

/*
struct VulkanTexture {
	VkSampler sampler;
	VkImage image;
	VkImageLayout imageLayout;
	VkDeviceMemory deviceMemory;
	VkImageView view;
	uint32_t width, height;
	uint32_t mipLevels;
	uint32_t layerCount;
	VkDescriptorImageInfo descriptor;
};
*/

#endif

