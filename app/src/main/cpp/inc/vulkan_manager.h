#ifndef AMVK_VULKAN_MANAGER_H
#define AMVK_VULKAN_MANAGER_H

#include <limits>
#include <cstring>
#include <vector>
#include <string>
#include <stdio.h>
#include <unordered_set>
#include <cstddef>


#include "macro.h"
#include "window.h"
#include "camera.h"
#include "timer.h"
#include "device_queue_indices.h"
#include "file_manager.h"
#include "vulkan_utils.h"
#include "pipeline_creator.h"
#include "pipeline_manager.h"
#include <chrono>
#include "texture_manager.h"
#include "device_manager.h"
#include "swapchain_manager.h"
#include "shader_manager.h"
#include "descriptor_manager.h"
#include "tquad.h"
#include "model.h"
#include "skinned.h"


class VulkanManager { 
	friend class Engine;
	struct SwapChainDesc;
public:

	VulkanManager(Window& window);
	virtual ~VulkanManager();
	void init();

	void buildCommandBuffers(const Timer &timer, Camera &camera);
	void updateUniformBuffers(const Timer& timer, Camera& camera);
	void draw();
	
	void waitIdle();
	void recreateSwapChain();

	//const VkDevice& getVkDevice() const;

private:
	void updateUniformBuffer(const Timer& timer);

	Window& mWindow;
	VulkanState mState;
	DeviceManager mDeviceManager;
	SwapchainManager mSwapChainManager;
	TQuad tquad;
	Model suit;
	Skinned guard;
	Skinned dwarf;
	uint32_t imageIndex;
};

#endif
