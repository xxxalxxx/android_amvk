#ifndef AMVK_VULKAN_MANAGER_H
#define AMVK_VULKAN_MANAGER_H

/*
#define VK_THROW_RESULT_ERROR(text, result) \
	do { \
		char str[128]; \
		int resultCode = static_cast<int>(result); \
		sprintf(str, #text " VkResult: %s (code: %d)", VulkanUtils::getVkResultString(resultCode), resultCode); \
		throw std::runtime_error(str); \
	} while (0)

#define VK_CHECK_RESULT(f)  \
	do { \
		VkResult result = f; \
		if (result != VK_SUCCESS) \
			VK_THROW_RESULT_ERROR(f, result); \
	} while (0)

#define VK_CALL_IPROC(instance, func, ...) \
	do { \
		auto __##func = (PFN_##func) vkGetInstanceProcAddr(instance, #func); \
		if (!__##func) \
			throw std::runtime_error("Failed to get Vulkan instance procedure for " #func); \
		if (__##func(__VA_ARGS__) != VK_SUCCESS) \
		   throw std::runtime_error("Failed to call iproc for " #func); \
	} while (0)
*/
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
#include <chrono>
#include "texture_manager.h"
#include "device_manager.h"
#include "swapchain_manager.h"
#include "shader_manager.h"
#include "descriptor_manager.h"
#include "quad.h"
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

	const VkDevice& getVkDevice() const;


private:
	void updateUniformBuffer(const Timer& timer);

	Window& mWindow;
	VulkanState mVulkanState;
	DeviceManager mDeviceManager;
	SwapchainManager mSwapChainManager;
	Quad mQuad;
	Model mSuit;
	Skinned guard;
	Skinned dwarf;
	uint32_t imageIndex;
};

#endif
