#include "image_info.h"

ImageInfo::ImageInfo(): 
	width(0),
	height(0),
	image(VK_NULL_HANDLE),
	imageView(VK_NULL_HANDLE),
	memory(VK_NULL_HANDLE),
	sampler(VK_NULL_HANDLE),
	mVkDevice(nullptr)
{

}

ImageInfo::ImageInfo(VkDevice& vkDevice):
	width(0),
	height(0),
	image(VK_NULL_HANDLE),
	imageView(VK_NULL_HANDLE),
	memory(VK_NULL_HANDLE),
	sampler(VK_NULL_HANDLE),
	mVkDevice(&vkDevice)
{

}

ImageInfo::ImageInfo(VkDevice& vkDevice, uint32_t width, uint32_t height):
	width(width),
	height(height),
	image(VK_NULL_HANDLE),
	imageView(VK_NULL_HANDLE),
	memory(VK_NULL_HANDLE),
	sampler(VK_NULL_HANDLE),
	mVkDevice(&vkDevice)
{

}

ImageInfo::~ImageInfo() 
{
	if (imageView != VK_NULL_HANDLE)
		vkDestroyImageView(*mVkDevice, imageView, nullptr);
	if (image != VK_NULL_HANDLE)
		vkDestroyImage(*mVkDevice, image, nullptr);
	if (memory != VK_NULL_HANDLE)
		vkFreeMemory(*mVkDevice, memory, nullptr);
}

ImageInfo& ImageInfo::operator=(ImageInfo other) 
{
	width = other.width;
	height = other.height;
	image = other.image;
	imageView = other.imageView;
	memory = other.memory;
	mVkDevice = other.mVkDevice;
	LOG("__EQ");
	return *this;	
}


