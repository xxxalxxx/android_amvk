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

#include "vulkan_state.h"
#include "vulkan_utils.h"
#include "device_queue_indices.h"
#include "macro.h"
#include "swapchain_manager.h"

class DeviceManager {
public:
	DeviceManager(VulkanState& vulkanState);
	void createVkInstance();
	void enableDebug();
	void createPhysicalDevice(SwapchainManager& vulkanSwapchainManager);
	void createLogicalDevice();


	DeviceQueueIndicies getDeviceQueueFamilyIndices(const VkPhysicalDevice& physicalDevice) const;
	bool deviceExtensionsSupported(const VkPhysicalDevice& physicalDevice) const;
	std::vector<const char*> getExtensionNames();
	
	static const std::vector<const char*> sDeviceExtensions;
	static const std::vector<const char*> sValidationLayers;

private:
	DeviceQueueIndicies mDeviceQueueIndices;
	VkDebugReportCallbackEXT mDebugReportCallback; 
	VulkanState& mVulkanState;
};

#endif
