#include "vulkan_manager.h"


VulkanManager::VulkanManager(Window& window):
	mWindow(window),
	mDeviceManager(mVulkanState),
	mSwapChainManager(mVulkanState, mWindow),
	mQuad(mVulkanState),
	mSuit(mVulkanState),
	mGuard(mVulkanState),
	imageIndex(0)
{
	
}

VulkanManager::~VulkanManager()
{
}

void VulkanManager::init() 
{
	mDeviceManager.createVkInstance();
#ifdef AMVK_DEBUG
	mDeviceManager.enableDebug();
#endif
	mSwapChainManager.createSurface();
	mDeviceManager.createPhysicalDevice(mSwapChainManager);
	mDeviceManager.createLogicalDevice();

	mSwapChainManager.createSwapChain();
	mSwapChainManager.createImageViews();
	
	mSwapChainManager.createRenderPass();
	mSwapChainManager.createCommandPool();

	ShaderManager::createShaders(mVulkanState);
	DescriptorManager::createDescriptorSetLayouts(mVulkanState);
	DescriptorManager::createDescriptorPool(mVulkanState);
	Quad::createPipeline(mVulkanState);
	Skinned::createPipeline(mVulkanState);
	mQuad.init();

	mGuard.init(FileManager::getModelsPath(
			"dwarf/dwarf2.ms3d"
						//"guard/boblampclean.md5mesh"
				),
				Skinned::DEFAULT_FLAGS | aiProcess_FlipUVs | aiProcess_FlipWindingOrder,
			//0
				Skinned::ModelFlag_stripFullPath
	);
    mGuard.ubo.model = glm::scale(glm::vec3(0.15f, 0.15f, 0.15f));
	mGuard.ubo.model = glm::rotate(glm::radians(180.f), glm::vec3(1.f, 0.f, 0.f)) * mGuard.ubo.model;
    mGuard.ubo.model = glm::rotate(glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f)) * mGuard.ubo.model;
    mGuard.ubo.model = glm::translate(glm::vec3(2.0f, 4.0f, 8.0f))  * mGuard.ubo.model;
	mGuard.animSpeedScale = 0.5f;

	/*ShaderManager::createShaders(mVulkanState);
	DescriptorManager::createDescriptorSetLayouts(mVulkanState);
	DescriptorManager::createDescriptorPool(mVulkanState);

	Quad::createPipeline(mVulkanState);
	Model::createPipeline(mVulkanState);
	Skinned::createPipeline(mVulkanState);

	mQuad.init();

//	mSuit.init(FileManager::getModelsPath("guard/boblampclean.md5mesh"), Model::DEFAULT_FLAGS | aiProcess_FlipUVs);
//	mSuit.ubo.model = glm::rotate(glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
//	mSuit.ubo.model = glm::rotate(glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f)) * mSuit.ubo.model;

	mGuard.init(FileManager::getModelsPath(
				"dwarf/dwarf2.ms3d"
				//"guard/boblampclean.md5mesh"
				), 
			Skinned::DEFAULT_FLAGS | aiProcess_FlipUVs | aiProcess_FlipWindingOrder,
			//0
			Skinned::ModelFlag_stripFullPath
			);
	mGuard.ubo.model = glm::rotate(glm::radians(180.f), glm::vec3(1.f, 0.f, 0.f));
	mGuard.animSpeedScale = 0.5f;
	//mGuard.ubo.model = glm::rotate(glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f)) * mGuard.ubo.model;
*/
	mSwapChainManager.createDepthResources();
	mSwapChainManager.createFramebuffers(mVulkanState.renderPass);

	mSwapChainManager.createCommandBuffers();
	mSwapChainManager.createSemaphores();
	
	LOG("INIT SUCCESSFUL");
}


void VulkanManager::updateUniformBuffers(const Timer& timer, Camera& camera)
{
	CmdPass cmd(mVulkanState.device, mVulkanState.commandPool, mVulkanState.graphicsQueue);
	mQuad.updateUniformBuffers(cmd.buffer, timer, camera);
	//mSuit.update(cmd.buffer, timer, camera);
	mGuard.update(cmd.buffer, timer, camera);
}

void VulkanManager::buildCommandBuffers(const Timer &timer, Camera &camera)
{

	VkClearValue clearValues[] ={
		{{0.4f, 0.1f, 0.1f, 1.0f}},	// VkClearColorValue color; 
		{{1.0f, 0}} // VkClearDepthStencilValue depthStencil 
	};


	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = mVulkanState.renderPass;
	renderPassBeginInfo.renderArea.offset = {0, 0};
	renderPassBeginInfo.renderArea.extent = mVulkanState.swapChainExtent;
	renderPassBeginInfo.clearValueCount = ARRAY_SIZE(clearValues);
	renderPassBeginInfo.pClearValues = clearValues;
	
	for (size_t i = 0; i < mSwapChainManager.mVkCommandBuffers.size(); ++i) {	
		VK_CHECK_RESULT(vkBeginCommandBuffer(mSwapChainManager.mVkCommandBuffers[i], &beginInfo));
		
		renderPassBeginInfo.framebuffer = mSwapChainManager.mSwapChainFramebuffers[i];
		vkCmdBeginRenderPass( mSwapChainManager.mVkCommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport;
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float) mVulkanState.swapChainExtent.width;
		viewport.height = (float) mVulkanState.swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		
		VkRect2D scissor = {};
		scissor.offset = {0, 0};
		scissor.extent = mVulkanState.swapChainExtent;

		vkCmdSetViewport( mSwapChainManager.mVkCommandBuffers[i], 0, 1, &viewport);
		vkCmdSetScissor( mSwapChainManager.mVkCommandBuffers[i], 0, 1, &scissor);
			
		//mQuad.draw(mSwapChainManager.mVkCommandBuffers[i]);
		//mSuit.draw(mSwapChainManager.mVkCommandBuffers[i], 	mVulkanState.pipelines.model.pipeline, mVulkanState.pipelines.model.layout);

		mGuard.draw(mSwapChainManager.mVkCommandBuffers[i], mVulkanState.pipelines.skinned.pipeline, mVulkanState.pipelines.skinned.layout);

		vkCmdEndRenderPass( mSwapChainManager.mVkCommandBuffers[i]);
		VK_CHECK_RESULT(vkEndCommandBuffer(mSwapChainManager.mVkCommandBuffers[i]));
	}
}

void VulkanManager::draw() 
{
	VkResult result = vkAcquireNextImageKHR(mVulkanState.device, 
										  mVulkanState.swapChain, 
										  std::numeric_limits<uint64_t>::max(), 
										  mSwapChainManager.mImageAvailableSemaphore, 
										  VK_NULL_HANDLE, 
										  &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapChain();
	} else if (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR) {
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		
		VkSemaphore waitSemaphores[] = { mSwapChainManager.mImageAvailableSemaphore };
		VkSemaphore signalSemaphores[] = { mSwapChainManager.mRenderFinishedSemaphore };
		VkSwapchainKHR swapChains[] = { mVulkanState.swapChain };
		VkPipelineStageFlags stageFlags[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = stageFlags;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &mSwapChainManager.mVkCommandBuffers[imageIndex];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		VK_CHECK_RESULT(vkQueueSubmit(mVulkanState.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
		
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		VK_CHECK_RESULT(vkQueuePresentKHR(mVulkanState.presentQueue, &presentInfo));

	} else {
		VK_THROW_RESULT_ERROR("Failed vkAcquireNextImageKHR", result);
	}
}

void VulkanManager::waitIdle() 
{
	vkDeviceWaitIdle(mVulkanState.device);
}

void VulkanManager::recreateSwapChain()
{
/*	vkQueueWaitIdle(mVulkanState.graphicsQueue);
	vkDeviceWaitIdle(mVulkanState.device);

	vkFreeCommandBuffers(mVulkanState.device, mVulkanState.commandPool, (uint32_t) mVkCommandBuffers.size(), mVkCommandBuffers.data());
	
	for (auto& framebuffer : mSwapChainFramebuffers)
		vkDestroyFramebuffer(mVulkanState.device, framebuffer, nullptr);

	vkDestroyImageView(mVulkanState.device, mDepthImageView, nullptr);
	vkDestroyImage(mVulkanState.device, mDepthImage, nullptr);
	vkFreeMemory(mVulkanState.device, mDepthImageMem, nullptr);
	
	vkDestroyPipeline(mVulkanState.device, mVkPipeline, nullptr);
	vkDestroyRenderPass(mVulkanState.device, mVulkanState.renderPass, nullptr);

	for (size_t i = 0; i < mSwapChainImages.size(); ++i) {
		vkDestroyImageView(mVulkanState.device, mSwapC	}

	vkDestroySwapchainKHR(mVulkanState.device, mVulkanState.swapChain, nullptr);

	createSwapChain(mWindow);
    createImageViews();
    createRenderPass();
    createPipeline();
	createDepthResources();
    createFramebuffers();
    createCommandBuffers();*/
}


