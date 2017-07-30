#include "renderer.h"


Renderer::Renderer(Window& window):
	mWindow(window),
	mDeviceManager(mState),
	mSwapChainManager(mState, mWindow),
	tquad(mState),
	suit(mState),
    guard(mState),
	dwarf(mState),
	fullscreenQuad(mState),
	sceneLights(mState),
	gBuffer(mState),
	imageIndex(0)
{
	
}

Renderer::~Renderer()
{
}

void Renderer::init()
{
	LOG_TITLE("Device Manager");
	mDeviceManager.createVkInstance();
#ifdef AMVK_DEBUG
	mDeviceManager.enableDebug();
#endif
	mSwapChainManager.createSurface();
	mDeviceManager.createPhysicalDevice(mSwapChainManager);
	mDeviceManager.createLogicalDevice();

	LOG_TITLE("Swapchain");

	mSwapChainManager.createSwapChain();
	mSwapChainManager.createImageViews();
	
	mSwapChainManager.createRenderPass();
	mSwapChainManager.createCommandPool();

	ShaderCreator::createShaders(mState);
	DescriptorCreator::createDescriptorSetLayouts(mState);
	DescriptorCreator::createDescriptorPool(mState);

	gBuffer.init(mState.physicalDevice, mState.device, mSwapChainManager.getWidth(), mSwapChainManager.getHeight());
    PipelineCreator::createPipelines(mState, gBuffer);
	tquad.init();
	//fullscreenQuad.init();

    suit.init(FileManager::getModelsPath("nanosuit/nanosuit.obj"),
              Model::DEFAULT_FLAGS | aiProcess_FlipUVs);

	dwarf.init(FileManager::getModelsPath("dwarf/dwarf2.ms3d"),
               Skinned::DEFAULT_FLAGS | aiProcess_FlipUVs | aiProcess_FlipWindingOrder,
               Skinned::ModelFlag_stripFullPath);

    dwarf.ubo.model = glm::scale(glm::vec3(0.15f, 0.15f, 0.15f));
    dwarf.ubo.model = glm::rotate(glm::radians(180.f), glm::vec3(1.f, 0.f, 0.f)) * dwarf.ubo.model;
    dwarf.ubo.model = glm::rotate(glm::radians(0.f), glm::vec3(0.f, 1.f, 0.f)) * dwarf.ubo.model;
    dwarf.ubo.model = glm::translate(glm::vec3(8.0f, 0.0f, -4.0f))  * dwarf.ubo.model;
    dwarf.animSpeedScale = 0.5f;

	guard.init(FileManager::getModelsPath("guard/boblampclean.md5mesh"),
               Skinned::DEFAULT_FLAGS | aiProcess_FlipUVs | aiProcess_FlipWindingOrder | aiProcess_FixInfacingNormals,
               Skinned::ModelFlag_flipNormals);
    guard.ubo.model = glm::scale(glm::vec3(0.18f, 0.18f, 0.18f));
    guard.ubo.model = glm::rotate(glm::radians(180.f), glm::vec3(1.f, 0.f, 0.f)) * guard.ubo.model;
    guard.ubo.model = glm::rotate(glm::radians(-180.f), glm::vec3(0.f, 1.f, 0.f)) * guard.ubo.model;
    guard.ubo.model = glm::translate(glm::vec3(-9.0f, 0.0f, -8.0f))  * guard.ubo.model;

	sceneLights.init();

	mSwapChainManager.createDepthResources();
	mSwapChainManager.createFramebuffers(mState.renderPass);
	mSwapChainManager.createCommandBuffers();

	createFences();
	createSemaphores();
	
	LOG("INIT SUCCESSFUL");
}


void Renderer::updateUniformBuffers(const Timer& timer, Camera& camera)
{
	CmdPass cmd(mState.device, mState.commandPool, mState.graphicsQueue);
	tquad.update(cmd.buffer, timer, camera);
    suit.update(cmd.buffer, timer, camera);
	dwarf.update(cmd.buffer, timer, camera);
	guard.update(cmd.buffer, timer, camera);
	sceneLights.update(cmd.buffer, timer, camera);
	//gBuffer.update(cmd.buffer, timer, camera);

	//CmdPass tilingCmd(mState.device, tiledRenderer.cmdPool, mState.computeQueue);
	//gBuffer.updateTiling(tilingCmd.buffer, timer, camera);
}

void Renderer::buildGBuffers(const Timer &timer, Camera &camera)
{
	std::array<VkClearValue, GBuffer::ATTACHMENT_COUNT> clearValues;
	clearValues[GBuffer::INDEX_POSITION].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
	clearValues[GBuffer::INDEX_NORMAL].color   = { { 0.0f, 0.0f, 0.0f, 1.0f } };
	clearValues[GBuffer::INDEX_ALBEDO].color   = { { 0.0f, 0.0f, 0.0f, 0.0f } };
	clearValues[GBuffer::INDEX_DEPTH].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.framebuffer = gBuffer.frameBuffer;
	renderPassBeginInfo.renderPass = gBuffer.renderPass;
	renderPassBeginInfo.renderArea.offset = {0, 0};
	renderPassBeginInfo.renderArea.extent.width = gBuffer.width;
	renderPassBeginInfo.renderArea.extent.height = gBuffer.height;
	renderPassBeginInfo.clearValueCount = clearValues.size();
	renderPassBeginInfo.pClearValues = clearValues.data();



	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	//beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	VkCommandBuffer& cmdBuffer = gBuffer.cmdBuffer;

	VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &beginInfo));


    VkImageMemoryBarrier albedoBarrier = {};
    albedoBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    albedoBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    albedoBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    albedoBarrier.image = gBuffer.albedo().image;
    albedoBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    albedoBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    albedoBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    albedoBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    albedoBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    vkCmdPipelineBarrier(
            cmdBuffer,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &albedoBarrier);

	vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);


	VkViewport viewport = {};
	viewport.width = (float) gBuffer.width;
	viewport.height = (float) gBuffer.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	
	VkRect2D scissor = {};
	scissor.offset = {0, 0};
	scissor.extent.width = gBuffer.width;
	scissor.extent.height = gBuffer.height;

	vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
	vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

    guard.draw(cmdBuffer, mState.pipelines.skinned.pipeline, mState.pipelines.skinned.layout);
	dwarf.draw(cmdBuffer, mState.pipelines.skinned.pipeline, mState.pipelines.skinned.layout);
	suit.draw(cmdBuffer, mState.pipelines.model.pipeline, mState.pipelines.model.layout);

	vkCmdEndRenderPass(cmdBuffer);
	VK_CHECK_RESULT(vkEndCommandBuffer(cmdBuffer));
}

void Renderer::buildComputeBuffers(const Timer &timer, Camera &camera)
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	//beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	VK_CHECK_RESULT(vkBeginCommandBuffer(gBuffer.tilingCmdBuffer, &beginInfo));

	VkImageMemoryBarrier imageMemoryBarrier = {};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	imageMemoryBarrier.image = gBuffer.tilingImage.image;
	imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	imageMemoryBarrier.srcAccessMask = 0;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;


    VkImageMemoryBarrier albedoBarrier = {};
    albedoBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    albedoBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    albedoBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    albedoBarrier.image = gBuffer.albedo().image;
    albedoBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    albedoBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    albedoBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    albedoBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    albedoBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    vkCmdPipelineBarrier(
            gBuffer.tilingCmdBuffer,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &albedoBarrier);

	//imageMemoryBarrier.srcQueueFamilyIndex = mState.computeQueueIndex;
	//imageMemoryBarrier.dstQueueFamilyIndex = mState.computeQueueIndex;

	//imageMemoryBarrier.dstQueueFamilyIndex = mState.graphicsQueueIndex;

	gBuffer.dispatch();

    /*
	VkImageMemoryBarrier afterDispatchBarriers[] = {
		gBuffer.createTilingSrcBarrier(gBuffer.normal().image),
		gBuffer.createTilingSrcBarrier(gBuffer.albedo().image)
	};

	vkCmdPipelineBarrier(
		gBuffer.tilingCmdBuffer,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		0,
		0, 
		nullptr,
		0, 
		nullptr,
		ARRAY_SIZE(afterDispatchBarriers),
		afterDispatchBarriers);
*/
	VK_CHECK_RESULT(vkEndCommandBuffer(gBuffer.tilingCmdBuffer));

}

void Renderer::buildCommandBuffers(const Timer &timer, Camera &camera)
{

	VkClearValue clearValues[] ={
		{{0.4f, 0.1f, 0.1f, 1.0f}},	// VkClearColorValue color; 
		{{1.0f, 0}} // VkClearDepthStencilValue depthStencil 
	};


	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	//beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = mState.renderPass;
	renderPassBeginInfo.renderArea.offset = {0, 0};
	renderPassBeginInfo.renderArea.extent = mState.swapChainExtent;
	renderPassBeginInfo.clearValueCount = ARRAY_SIZE(clearValues);
	renderPassBeginInfo.pClearValues = clearValues;
	
	for (size_t i = 0; i < mSwapChainManager.cmdBuffers.size(); ++i) {
        VkCommandBuffer& cmdBuffer = mSwapChainManager.cmdBuffers[i];
		VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &beginInfo));

		
		/*VkImageMemoryBarrier imageMemoryBarrier = {};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imageMemoryBarrier.image = gBuffer.tilingImage.image;
		imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		imageMemoryBarrier.srcQueueFamilyIndex = mState.computeQueueIndex;
		imageMemoryBarrier.dstQueueFamilyIndex = mState.graphicsQueueIndex;

		vkCmdPipelineBarrier(
			cmdBuffer,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageMemoryBarrier);
		*/
		
		VkImageSubresourceLayers subres = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };

		VkImageBlit blit = {};
		blit.srcSubresource = subres;
		blit.srcOffsets[0] = {0, 0, 0};
		blit.srcOffsets[1] = {gBuffer.width, gBuffer.height, 1};
		blit.dstSubresource = subres;
		blit.dstOffsets[0] = {0, 0, 0};
		blit.dstOffsets[1] = {gBuffer.width, gBuffer.height, 1};

		vkCmdBlitImage(
				cmdBuffer, 
				gBuffer.tilingImage.image, 
				VK_IMAGE_LAYOUT_GENERAL,
				mSwapChainManager.mSwapChainImages[i],
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				1,
				&blit,
				VK_FILTER_NEAREST);

	/*	
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imageMemoryBarrier.image = gBuffer.tilingImage.image;
		imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		imageMemoryBarrier.srcQueueFamilyIndex = mState.computeQueueIndex;
		imageMemoryBarrier.dstQueueFamilyIndex = mState.graphicsQueueIndex;

		vkCmdPipelineBarrier(
			cmdBuffer,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageMemoryBarrier);

*/
        /*

		vkCmdBlitImage(
				cmdBuffer, 
				gBuffer.albedo().image, 
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				mSwapChainManager.mSwapChainImages[i],
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				1,
				&blit,
				VK_FILTER_NEAREST);
        */
        /*
		subres.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	
		vkCmdBlitImage(
				cmdBuffer, 
				gBuffer.depth().image, 
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				mSwapChainManager.mDepthImageDesc.image,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				1,
				&blit,
				VK_FILTER_NEAREST);
           */

		
		renderPassBeginInfo.framebuffer = mSwapChainManager.framebuffers[i];
		vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport;
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float) mState.swapChainExtent.width;
		viewport.height = (float) mState.swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		
		VkRect2D scissor = {};
		scissor.offset = {0, 0};
		scissor.extent = mState.swapChainExtent;

		vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
		vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);



		//tquad.draw(cmdBuffer);
		 //fullscreenQuad.draw(cmdBuffer);
		//sceneLights.draw(cmdBuffer);
		//dwarf.draw(cmdBuffer, mState.pipelines.skinned.pipeline, mState.pipelines.skinned.layout);
		//suit.draw(cmdBuffer, mState.pipelines.model.pipeline, mState.pipelines.model.layout);

        //guard.draw(cmdBuffer, mState.pipelines.skinned.pipeline, mState.pipelines.skinned.layout);
		/*
		   gBuffer.deferredQuad.draw(
				cmdBuffer, 
				mState.pipelines.fullscreenQuad.pipeline, 
				mState.pipelines.fullscreenQuad.layout, 
				&gBuffer.deferredQuad.mDescriptorSet, 
				1);
		*/
		//gBuffer.drawDeferredQuad(cmdBuffer);



		vkCmdEndRenderPass(cmdBuffer);

		VK_CHECK_RESULT(vkEndCommandBuffer(cmdBuffer));
	}
}

void Renderer::draw()
{
	VkResult result = vkAcquireNextImageKHR(mState.device,
                                            mState.swapChain,
                                            UINT64_MAX,
                                            imageAquiredSemaphore,
                                            VK_NULL_HANDLE,
                                            &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapChain();
		return;
	} 

	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		VK_THROW_RESULT_ERROR("Failed vkAcquireNextImageKHR", result);
		return;
	}

	//LOG("CNT %u", cnt++);

	vkWaitForFences(mState.device, 1, &tilingFence, VK_TRUE, UINT64_MAX);
	vkResetFences(mState.device, 1, &tilingFence);

	VkPipelineStageFlags stageFlags[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkPipelineStageFlags tilingFlags[] = { VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT };


	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &imageAquiredSemaphore;
	submitInfo.pWaitDstStageMask = stageFlags;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &gBuffer.cmdBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &offscreenSemaphore;

	VK_CHECK_RESULT(vkQueueSubmit(mState.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));

	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &offscreenSemaphore;
	submitInfo.pWaitDstStageMask = stageFlags;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &gBuffer.tilingCmdBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &tilingFinishedSemaphore;

	VK_CHECK_RESULT(vkQueueSubmit(mState.computeQueue, 1, &submitInfo, tilingFence));

	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &tilingFinishedSemaphore;
	submitInfo.pWaitDstStageMask = tilingFlags;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &mSwapChainManager.cmdBuffers[imageIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderFinishedSemaphore;

	VK_CHECK_RESULT(vkQueueSubmit(mState.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
	
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &mState.swapChain;
	presentInfo.pImageIndices = &imageIndex;

	VK_CHECK_RESULT(vkQueuePresentKHR(mState.presentQueue, &presentInfo));
}

void Renderer::createSemaphores()
{
	VkSemaphoreCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	VK_CHECK_RESULT(vkCreateSemaphore(mState.device, &createInfo, nullptr, &imageAquiredSemaphore));
	VK_CHECK_RESULT(vkCreateSemaphore(mState.device, &createInfo, nullptr, &offscreenSemaphore));
	VK_CHECK_RESULT(vkCreateSemaphore(mState.device, &createInfo, nullptr, &renderFinishedSemaphore));
	VK_CHECK_RESULT(vkCreateSemaphore(mState.device, &createInfo, nullptr, &tilingFinishedSemaphore));

	LOG("SEMAPHORES CREATED");
}

void Renderer::createFences() 
{
	VkFenceCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	VK_CHECK_RESULT(vkCreateFence(mState.device, &info, nullptr, &tilingFence));
}

void Renderer::waitIdle()
{
	vkDeviceWaitIdle(mState.device);
}

void Renderer::onWindowSizeChanged(uint32_t width, uint32_t height)
{
    recreateSwapChain();
}

void Renderer::recreateSwapChain()
{
/*	vkQueueWaitIdle(mState.graphicsQueue);
	vkDeviceWaitIdle(mState.device);

	vkFreeCommandBuffers(mState.device, mState.commandPool, (uint32_t) mVkCommandBuffers.size(), mVkCommandBuffers.data());
	
	for (auto& framebuffer : mSwapChainFramebuffers)
		vkDestroyFramebuffer(mState.device, framebuffer, nullptr);

	vkDestroyImageView(mState.device, mDepthImageView, nullptr);
	vkDestroyImage(mState.device, mDepthImage, nullptr);
	vkFreeMemory(mState.device, mDepthImageMem, nullptr);
	
	vkDestroyPipeline(mState.device, mVkPipeline, nullptr);
	vkDestroyRenderPass(mState.device, mState.renderPass, nullptr);

	for (size_t i = 0; i < mSwapChainImages.size(); ++i) {
		vkDestroyImageView(mState.device, mSwapC	}

	vkDestroySwapchainKHR(mState.device, mState.swapChain, nullptr);

	createSwapChain(mWindow);
    createImageViews();
    createRenderPass();
    createPipeline();
	createDepthResources();
    createFramebuffers();
    createCommandBuffers();*/
}


