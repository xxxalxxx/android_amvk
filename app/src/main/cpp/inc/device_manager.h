#ifndef AMVK_DEVICE_MANAGER_H
#define AMVK_DEVICE_MANAGER_H

#ifdef __ANDROID__
#include "vulkan_wrapper.h"
#else
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#endif

#include <vector>
#include <cstring>
#include <unordered_set>
#include <stdint.h>

#include "state.h"
#include "vulkan_utils.h"
#include "macro.h"
#include "swapchain_manager.h"

class DeviceManager {
public:
	struct QueueIndices {
		uint32_t graphics = UINT32_MAX;
		uint32_t transfer = UINT32_MAX;
		uint32_t compute = UINT32_MAX;
		uint32_t present = UINT32_MAX;
	};

	DeviceManager(State& vulkanState);
	void createVkInstance();
	void enableDebug();
	void createPhysicalDevice(SwapchainManager& vulkanSwapchainManager);
	void createLogicalDevice();

	bool deviceExtensionsSupported(const VkPhysicalDevice& physicalDevice) const;
	bool deviceQueueIndicesSupported(const VkPhysicalDevice& physicalDevice, QueueIndices& outIndices) const;
	std::vector<const char*> getExtensionNames();
	
	static const std::vector<const char*> sDeviceExtensions;
	static const std::vector<const char*> sValidationLayers;

private:
	VkDebugReportCallbackEXT mDebugReportCallback; 
	State& mState;
};

#endif
