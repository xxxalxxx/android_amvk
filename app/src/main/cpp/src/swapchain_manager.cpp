#include "swapchain_manager.h"

SwapchainManager::SwapchainManager(State& vulkanState, Window& window):
	mState(vulkanState),
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
	VK_CHECK_RESULT(vkCreateAndroidSurfaceKHR(mState.instance, &surfaceCreateInfo, NULL, &mState.surface));
	LOG("Android surface created");
#else
	VK_CHECK_RESULT(glfwCreateWindowSurface(mState.instance, mWindow.mGlfwWindow, nullptr, &mState.surface));

	if (glfwVulkanSupported() == GLFW_FALSE)
		throw std::runtime_error("Vulkan is not supported by GLFW. Cannot create a surface");
#endif
}

void SwapchainManager::createSwapChain()
{
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mState.physicalDevice, mState.surface, &surfaceCapabilities);

	VkSurfaceFormatKHR surfaceFormat = getSurfaceFormat(mState.deviceInfo.surfaceFormats);
	VkPresentModeKHR presentMode = getPresentMode(mState.deviceInfo.presentModes);
	VkExtent2D extent = getExtent(mState.deviceInfo.surfaceCapabilities);
    LOG("EXTENT width: %u, height: %u", extent.width, extent.height);

	uint32_t numImages = mState.deviceInfo.surfaceCapabilities.minImageCount;
	numImages = std::min(numImages + 1, mState.deviceInfo.surfaceCapabilities.maxImageCount);
	
	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = mState.surface;
	createInfo.minImageCount = numImages;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	
	if (mState.graphicsQueueIndex != mState.presentQueueIndex) {
		uint32_t queueFamilyIndices[] = { 
			mState.graphicsQueueIndex,
			mState.presentQueueIndex
		};
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
		LOG("Swapchain sharing mode concurrent");
	} else {
		LOG("Swapchain sharing mode exclusive");
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform = mState.deviceInfo.surfaceCapabilities.currentTransform;

	VkCompositeAlphaFlagBitsKHR compositeAlpha;
	if (surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
		compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	else
		compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
	createInfo.compositeAlpha = compositeAlpha;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	VkSwapchainKHR oldSwapChain = mState.swapChain;
	createInfo.oldSwapchain = oldSwapChain;
	VkSwapchainKHR swapChain;
	LOG("BEFORE SWAPCHAIN");
	VK_CHECK_RESULT(vkCreateSwapchainKHR(mState.device, &createInfo, nullptr, &swapChain));
	LOG("AFTER SWAPCHAIN");
	mState.swapChain = swapChain;
	vkGetSwapchainImagesKHR(mState.device, mState.swapChain, &numImages, nullptr);
	mSwapChainImages.resize(numImages);
	vkGetSwapchainImagesKHR(mState.device, mState.swapChain, &numImages, mSwapChainImages.data());
	
	mState.swapChainImageFormat = surfaceFormat.format;
	mState.swapChainExtent = extent;

	LOG("SWAP CHAIN CREATED");
}

void SwapchainManager::createImageViews() 
{
	mSwapChainImageViews.resize(mSwapChainImages.size());
	for (size_t i = 0; i < mSwapChainImages.size(); ++i) {
		ImageHelper::createImageView(
				mState.device,
				mSwapChainImages[i], 
				mState.swapChainImageFormat,
				VK_IMAGE_ASPECT_COLOR_BIT,
				mSwapChainImageViews[i]);

		LOG("IMAGE VIEW CREATED");
	}
}

void SwapchainManager::createDepthResources() 
{
	VkFormat depthFormat = ImageHelper::findDepthStencilFormat(mState.physicalDevice);

	mDepthImageDesc.width = mState.swapChainExtent.width;
	mDepthImageDesc.height = mState.swapChainExtent.height;

	ImageHelper::createImage(
			mState,
			mDepthImageDesc, 
			depthFormat, 
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_LAYOUT_PREINITIALIZED,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	ImageHelper::createImageView(
			mState.device,
			mDepthImageDesc,
			depthFormat,
			VK_IMAGE_ASPECT_DEPTH_BIT);

	ImageHelper::transitionLayout(
			mState,
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
	framebuffers.resize(mSwapChainImageViews.size());

	VkFramebufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.renderPass = mState.renderPass;
	createInfo.width = mState.swapChainExtent.width;
	createInfo.height = mState.swapChainExtent.height;
	createInfo.layers = 1;

	for (size_t i = 0; i < framebuffers.size(); ++i) {
		std::array<VkImageView, 2> attachments = { 
			mSwapChainImageViews[i],
			mDepthImageDesc.imageView
		};

		createInfo.attachmentCount = attachments.size();
		createInfo.pAttachments = attachments.data();

		VK_CHECK_RESULT(vkCreateFramebuffer(mState.device, &createInfo, nullptr, &framebuffers[i]));
		LOG("FRAMEBUFFER CREATED");
	}
}

VkSurfaceFormatKHR SwapchainManager::getSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& surfaceFormats) const 
{
	if (surfaceFormats.size() == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED) {
        LOG("FORMAT INITIAL");
        return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    }

    for (const auto &surfaceFormat : surfaceFormats) {
		LOG("FORMAT LOOP format: %u, colorSpace: %u", surfaceFormat.format, surfaceFormat.colorSpace);
        if ((surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM || surfaceFormat.format == VK_FORMAT_R8G8B8A8_UNORM) 
			&& surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            LOG("FORMAT LOOP");
            return surfaceFormat;
        }
	}
    LOG("FORMAT FIRST");
    return surfaceFormats[0];
}

VkPresentModeKHR SwapchainManager::getPresentMode(const std::vector<VkPresentModeKHR>& presentModes) const
{
#ifdef __ANDROID__
    return VK_PRESENT_MODE_FIFO_KHR;
#else
	for (const auto& presentMode : presentModes)
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return presentMode;
	return VK_PRESENT_MODE_FIFO_KHR;
#endif
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

void SwapchainManager::createRenderPass()
{
	VkAttachmentDescription att = {};
	att.format = mState.swapChainImageFormat;
	att.samples = VK_SAMPLE_COUNT_1_BIT;
	att.loadOp = 
		VK_ATTACHMENT_LOAD_OP_LOAD;
	// VK_ATTACHMENT_LOAD_OP_CLEAR;
	att.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	att.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	att.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	att.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	att.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depthAtt = {};
	depthAtt.format = ImageHelper::findDepthFormat(mState.physicalDevice);
	depthAtt.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAtt.loadOp = 
		VK_ATTACHMENT_LOAD_OP_LOAD;
	//VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAtt.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAtt.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAtt.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	//depthAtt.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	//depthAtt.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
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
	createInfo.dependencyCount = dependencies.size();
	createInfo.pDependencies = dependencies.data();

	VK_CHECK_RESULT(vkCreateRenderPass(mState.device, &createInfo, nullptr, &mState.renderPass));
	LOG("RENDER PASS CREATED");
}


void SwapchainManager::createCommandPool()
{
	VkCommandPoolCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.queueFamilyIndex = mState.graphicsQueueIndex;
	createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VK_CHECK_RESULT(vkCreateCommandPool(mState.device, &createInfo, nullptr, &mState.commandPool));
	LOG("COMMAND BUFFER CREATED");
}

void SwapchainManager::createCommandBuffers()
{
	cmdBuffers.resize(framebuffers.size());

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = mState.commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t) cmdBuffers.size();
	
	VK_CHECK_RESULT(vkAllocateCommandBuffers(mState.device, &allocInfo, cmdBuffers.data()));

	LOG("COMMAND POOL ALLOCATED");

//	buildCommandBuffers();
}

uint32_t SwapchainManager::getWidth() const
{
	return mWindow.getWidth();
}

uint32_t SwapchainManager::getHeight() const
{
	return mWindow.getHeight();
}
