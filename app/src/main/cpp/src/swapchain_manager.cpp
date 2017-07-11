#include "swapchain_manager.h"

SwapchainManager::SwapchainManager(VulkanState& vulkanState, Window& window):
	mVulkanState(vulkanState), 
	mWindow(window),
	mDepthImageDesc(vulkanState.device)
{

}

SwapchainManager::~SwapchainManager() 
{

}

void SwapchainManager::createSurface()
{
#ifdef __ANDROID__
	VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.window =  mWindow.androidNativeWindow;
	VK_CHECK_RESULT(vkCreateAndroidSurfaceKHR(mVulkanState.instance, &surfaceCreateInfo, NULL, &mVulkanState.surface));
	LOG("Android surface created");
#else
	VK_CHECK_RESULT(glfwCreateWindowSurface(mVulkanState.instance, mWindow.mGlfwWindow, nullptr, &mVulkanState.surface));

	if (glfwVulkanSupported() == GLFW_FALSE)
		throw std::runtime_error("Vulkan is not supported by GLFW. Cannot create a surface");
#endif
}

void SwapchainManager::createSwapChain()
{
	//(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* pSurfaceCapabilities);
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	LOG("Before surface capabilities");
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mVulkanState.physicalDevice, mVulkanState.surface, &surfaceCapabilities);
	LOG("After surface capabilities");

	VkSurfaceFormatKHR surfaceFormat = getSurfaceFormat(mVulkanState.swapChainDesc.surfaceFormats);
	VkPresentModeKHR presentMode = getPresentMode(mVulkanState.swapChainDesc.presentModes);
	VkExtent2D extent = getExtent(mVulkanState.swapChainDesc.surfaceCapabilities); 

	uint32_t numImages = mVulkanState.swapChainDesc.surfaceCapabilities.minImageCount;
	numImages = std::min(numImages + 1, mVulkanState.swapChainDesc.surfaceCapabilities.maxImageCount);
	
	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = mVulkanState.surface;
	createInfo.minImageCount = numImages;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	
	if (mVulkanState.graphicsQueueIndex != mVulkanState.presentQueueIndex) {
		uint32_t queueFamilyIndices[] = { 
			(uint32_t) mVulkanState.graphicsQueueIndex, 
			(uint32_t) mVulkanState.presentQueueIndex 
		};
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
		LOG("OTHER");
	} else {
		LOG("SAME");
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform = mVulkanState.swapChainDesc.surfaceCapabilities.currentTransform;

	VkCompositeAlphaFlagBitsKHR compositeAlpha;
	if (surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
		compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	else
		compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
	createInfo.compositeAlpha = compositeAlpha;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	VkSwapchainKHR oldSwapChain = mVulkanState.swapChain;
	createInfo.oldSwapchain = oldSwapChain;
	VkSwapchainKHR swapChain;
	LOG("BEFORE SWAPCHAIN");
	VK_CHECK_RESULT(vkCreateSwapchainKHR(mVulkanState.device, &createInfo, nullptr, &swapChain));
	LOG("AFTER SWAPCHAIN");
	mVulkanState.swapChain = swapChain;
	vkGetSwapchainImagesKHR(mVulkanState.device, mVulkanState.swapChain, &numImages, nullptr);
	mSwapChainImages.resize(numImages);
	vkGetSwapchainImagesKHR(mVulkanState.device, mVulkanState.swapChain, &numImages, mSwapChainImages.data());
	
	mVulkanState.swapChainImageFormat = surfaceFormat.format;
	mVulkanState.swapChainExtent = extent;

	LOG("SWAP CHAIN CREATED");
}

void SwapchainManager::createImageViews() 
{
	mSwapChainImageViews.resize(mSwapChainImages.size());
	for (size_t i = 0; i < mSwapChainImages.size(); ++i) {
		ImageHelper::createImageView(
				mVulkanState.device, 
				mSwapChainImages[i], 
				mVulkanState.swapChainImageFormat, 
				VK_IMAGE_ASPECT_COLOR_BIT,
				mSwapChainImageViews[i]);

		LOG("IMAGE VIEW CREATED");
	}
}

void SwapchainManager::createDepthResources() 
{
	VkFormat depthFormat = ImageHelper::findSupportedFormat(
			mVulkanState.physicalDevice,
			{VK_FORMAT_D24_UNORM_S8_UINT},
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

	mDepthImageDesc.width = mVulkanState.swapChainExtent.width;
	mDepthImageDesc.height = mVulkanState.swapChainExtent.height;

	ImageHelper::createImage(
			mVulkanState, 
			mDepthImageDesc, 
			depthFormat, 
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_LAYOUT_PREINITIALIZED,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	ImageHelper::createImageView(
			mVulkanState.device,
			mDepthImageDesc,
			depthFormat,
			VK_IMAGE_ASPECT_DEPTH_BIT);

	ImageHelper::transitionLayout(
			mVulkanState, 
			mDepthImageDesc.image, 
			depthFormat, 
			VK_IMAGE_LAYOUT_UNDEFINED, 
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 
			VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 
			0, 
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
}



void SwapchainManager::createFramebuffers(VkRenderPass renderPass)
{
	mSwapChainFramebuffers.resize(mSwapChainImageViews.size());

	VkFramebufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.renderPass = mVulkanState.renderPass;
	createInfo.width = mVulkanState.swapChainExtent.width;
	createInfo.height = mVulkanState.swapChainExtent.height;
	createInfo.layers = 1;

	for (size_t i = 0; i < mSwapChainFramebuffers.size(); ++i) {
		std::array<VkImageView, 2> attachments = { 
			mSwapChainImageViews[i],
			mDepthImageDesc.imageView
		};

		createInfo.attachmentCount = attachments.size();
		createInfo.pAttachments = attachments.data();

		VK_CHECK_RESULT(vkCreateFramebuffer(mVulkanState.device, &createInfo, nullptr, &mSwapChainFramebuffers[i]));
		LOG("FRAMEBUFFER CREATED");
	}
}

VkSurfaceFormatKHR SwapchainManager::getSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& surfaceFormats) const 
{
	if (surfaceFormats.size() == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED) {
		return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
	}

	for (const auto& surfaceFormat : surfaceFormats)
		if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return surfaceFormat;

	return surfaceFormats[0];
}

VkPresentModeKHR SwapchainManager::getPresentMode(const std::vector<VkPresentModeKHR>& presentModes) const
{
	for (const auto& presentMode : presentModes)
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return presentMode;
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapchainManager::getExtent(VkSurfaceCapabilitiesKHR& surfaceCapabilities) const
{
	VkExtent2D extent;
	extent.width = std::max(
			surfaceCapabilities.minImageExtent.width, 
			std::min(mWindow.getWidth(), surfaceCapabilities.maxImageExtent.width));
	extent.height = std::max(
			surfaceCapabilities.minImageExtent.height, 
			std::min(mWindow.getHeight(), surfaceCapabilities.maxImageExtent.height));
	
	return extent;
}


SwapChainDesc SwapchainManager::getSwapChainDesc(const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface)
{
	SwapChainDesc swapChainDesc = {};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &swapChainDesc.surfaceCapabilities);
	uint32_t numSurfaceFormats;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &numSurfaceFormats, nullptr);
	
	if (numSurfaceFormats > 0) {
		swapChainDesc.surfaceFormats.resize(numSurfaceFormats);
		vkGetPhysicalDeviceSurfaceFormatsKHR(
				physicalDevice, 
				surface, 
				&numSurfaceFormats, 
				swapChainDesc.surfaceFormats.data());
	}

	uint32_t numPresentModes;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &numPresentModes, nullptr);

	if (numPresentModes > 0) {
		swapChainDesc.presentModes.resize(numPresentModes);
		vkGetPhysicalDeviceSurfacePresentModesKHR(
				physicalDevice, 
				surface, 
				&numPresentModes,
				swapChainDesc.presentModes.data());
	}

	return swapChainDesc;
}

void SwapchainManager::createRenderPass()
{
	VkAttachmentDescription att = {};
	att.format = mVulkanState.swapChainImageFormat;
	att.samples = VK_SAMPLE_COUNT_1_BIT;
	att.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	att.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	att.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	att.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	att.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	att.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depthAtt = {};
	depthAtt.format = ImageHelper::findDepthFormat(mVulkanState.physicalDevice);
	depthAtt.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAtt.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAtt.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAtt.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAtt.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAtt.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depthAtt.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


	// uint32_t      attachment;
    // VkImageLayout layout;

	VkAttachmentReference attRef = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	VkAttachmentReference depthAttRef = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

	VkSubpassDescription sub = {};
	sub.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	sub.colorAttachmentCount = 1;
	sub.pColorAttachments = &attRef;
	sub.pDepthStencilAttachment = &depthAttRef;
	
	VkSubpassDependency dependancy = {};
	dependancy.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependancy.dstSubpass = 0;
	dependancy.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependancy.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	dependancy.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependancy.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT 
							 | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 2> attachments = {
		att, 
		depthAtt
	};
	
	VkRenderPassCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	createInfo.attachmentCount = attachments.size();
	createInfo.pAttachments = attachments.data();
	createInfo.subpassCount = 1;
	createInfo.pSubpasses = &sub;
	createInfo.dependencyCount = 1;
	createInfo.pDependencies = &dependancy;

	VK_CHECK_RESULT(vkCreateRenderPass(mVulkanState.device, &createInfo, nullptr, &mVulkanState.renderPass));
	LOG("RENDER PASS CREATED");
}


void SwapchainManager::createCommandPool()
{
	VkCommandPoolCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.queueFamilyIndex = mVulkanState.graphicsQueueIndex; 
	createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VK_CHECK_RESULT(vkCreateCommandPool(mVulkanState.device, &createInfo, nullptr, &mVulkanState.commandPool));
	LOG("COMMAND BUFFER CREATED");
}

void SwapchainManager::createCommandBuffers()
{
	mVkCommandBuffers.resize(mSwapChainFramebuffers.size());

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = mVulkanState.commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t) mVkCommandBuffers.size();
	
	VK_CHECK_RESULT(vkAllocateCommandBuffers(mVulkanState.device, &allocInfo, mVkCommandBuffers.data()));

	LOG("COMMAND POOL ALLOCATED");

//	buildCommandBuffers();
}

void SwapchainManager::createSemaphores()
{
	VkSemaphoreCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	VK_CHECK_RESULT(vkCreateSemaphore(mVulkanState.device, &createInfo, nullptr, &mImageAvailableSemaphore));
	VK_CHECK_RESULT(vkCreateSemaphore(mVulkanState.device, &createInfo, nullptr, &mRenderFinishedSemaphore));
	LOG("SEMAPHORES CREATED");
}
