#ifndef AMVK_SWAP_CHAIN_DESC_H
#define AMVK_SWAP_CHAIN_DESC_H


#ifdef __ANDROID__
#include "vulkan_wrapper.h"
#else
#include <vulkan/vulkan.h>
#endif

#include <vector>

class SwapChainDesc {
public:
	SwapChainDesc();
	bool supported() const;
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	std::vector<VkSurfaceFormatKHR> surfaceFormats;
	std::vector<VkPresentModeKHR> presentModes;
};

#endif
