#ifndef AMVK_SWAPCHAIN_MANAGER_H
#define AMVK_SWAPCHAIN_MANAGER_H

#include "vulkan.h"

#ifndef __ANDROID__
#include <GLFW/glfw3.h>
#endif

#include <vector>
#include <array>

#include "state.h"
#include "vulkan_utils.h"
#include "image_helper.h"
#include "window.h"
#include "tquad.h"

class SwapchainManager {
public:
	SwapchainManager(State& vulkanState, Window& window);
	~SwapchainManager();
	void createSurface();
	void createSwapChain();
	void createImageViews();
	void createDepthResources();
	void createFramebuffers(VkRenderPass renderPass);
	void createCommandPool();
	void createCommandBuffers();
	void createSemaphores();
	void createRenderPass();
	uint32_t getWidth() const;
	uint32_t getHeight() const;

	VkSurfaceFormatKHR getSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& surfaceFormats) const; 
	VkPresentModeKHR getPresentMode(const std::vector<VkPresentModeKHR>& presentModes) const;
	VkExtent2D getExtent(VkSurfaceCapabilitiesKHR& surfaceCapabilities) const;

	std::vector<VkFramebuffer> framebuffers;
	std::vector<VkCommandBuffer> cmdBuffers;
private:
	State& mState;
	Window& mWindow;

	std::vector<VkImage> mSwapChainImages;
	std::vector<VkImageView> mSwapChainImageViews;

	ImageInfo mDepthImageDesc;

};

#endif
