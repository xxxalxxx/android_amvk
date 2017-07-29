#include "device_manager.h"

const std::vector<const char*> DeviceManager::sDeviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const std::vector<const char*> DeviceManager::sValidationLayers = {
#ifdef __ANDROID__
            "VK_LAYER_GOOGLE_threading",
            "VK_LAYER_LUNARG_parameter_validation",
            "VK_LAYER_LUNARG_object_tracker",
            "VK_LAYER_LUNARG_core_validation",
            "VK_LAYER_LUNARG_image",
            "VK_LAYER_LUNARG_swapchain",
            "VK_LAYER_GOOGLE_unique_objects"
#else
	"VK_LAYER_LUNARG_standard_validation",
#endif
};

VKAPI_ATTR VkBool32 VKAPI_CALL debugCb(
		VkDebugReportFlagsEXT flags, 
		VkDebugReportObjectTypeEXT objType, 
		uint64_t obj, 
		size_t location, 
		int32_t code, 
		const char* layerPrefix, 
		const char* msg, 
		void* userData) 
{
    LOG(">>> VK_VALIDATION: %s", msg);
	return VK_FALSE;
}

DeviceManager::DeviceManager(State& vulkanState):
	mState(vulkanState)
{

}

void DeviceManager::createVkInstance() 
{
#ifdef AMVK_DEBUG

	uint32_t numLayers = 0;
    LOG("BEFORE LAYERS");
	vkEnumerateInstanceLayerProperties(&numLayers, nullptr);
    LOG("NUM LAYERS: %u", numLayers);
	std::vector<VkLayerProperties> props(numLayers);
	vkEnumerateInstanceLayerProperties(&numLayers, props.data());

	for (const char* layerName : sValidationLayers) {
		bool layerFound = false;
		for (const auto& layerProperties : props) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
            LOG("Layer %s not found!", layerName);
			std::string msg = "Unable to found layer:";
			msg += layerName;
			throw std::runtime_error(msg);
		}
	}

#endif

	VkApplicationInfo applicationInfo = {};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; 
	applicationInfo.pApplicationName = "Learning Vulkan";
	applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
	applicationInfo.pEngineName = "AMVK";
	applicationInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
	applicationInfo.apiVersion = VK_API_VERSION_1_0;

	auto extensionNames = getExtensionNames();
	VkInstanceCreateInfo instanceInfo = {};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo = &applicationInfo;
	instanceInfo.enabledExtensionCount = extensionNames.size();
	instanceInfo.ppEnabledExtensionNames = extensionNames.data();

#ifdef AMVK_DEBUG
	LOG("DEBUG EXISTS");
	instanceInfo.enabledLayerCount = sValidationLayers.size();
	instanceInfo.ppEnabledLayerNames = sValidationLayers.data();
#else
	LOG("DEBUG DOES NOT EXIST");
	instanceInfo.enabledLayerCount = 0;
#endif
    LOG("BEFORE LAYERS");
	VK_CHECK_RESULT(vkCreateInstance(&instanceInfo, nullptr, &mState.instance));
	LOG("INSTANCE CREATED");
}

void DeviceManager::enableDebug() 
{
	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.pNext = nullptr;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT 
					 | VK_DEBUG_REPORT_WARNING_BIT_EXT
					 | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
	createInfo.pfnCallback = &debugCb;
	createInfo.pUserData = nullptr;

	VK_CALL_IPROC(mState.instance, vkCreateDebugReportCallbackEXT, mState.instance, &createInfo, nullptr, &mDebugReportCallback);
	LOG("DEBUG ENABLED");
}

void DeviceManager::createPhysicalDevice(SwapchainManager& swapchainManager) 
{
	uint32_t numDevices = 0;
	vkEnumeratePhysicalDevices(mState.instance, &numDevices, nullptr);

	if (!numDevices)
		throw std::runtime_error("No Physical devices found");

	std::vector<VkPhysicalDevice> devices(numDevices);
	vkEnumeratePhysicalDevices(mState.instance, &numDevices, devices.data());

	for (const auto& device : devices) {

		bool extenstionsSupported = deviceExtensionsSupported(device);
		if (!extenstionsSupported)
			continue;
		uint32_t numSurfaceFormats;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, mState.surface, &numSurfaceFormats, nullptr);
		if (numSurfaceFormats == 0)
			continue;
		uint32_t numPresentModes;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, mState.surface, &numPresentModes, nullptr);
		if (numPresentModes == 0)
			continue;
		QueueIndices queueIndices = {};
		if (!deviceQueueIndicesSupported(device, queueIndices))
			continue;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, mState.surface, &mState.deviceInfo.surfaceCapabilities);

		mState.deviceInfo.surfaceFormats.resize(numSurfaceFormats);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, mState.surface, &numSurfaceFormats, mState.deviceInfo.surfaceFormats.data());

		mState.deviceInfo.presentModes.resize(numPresentModes);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, mState.surface, &numPresentModes, mState.deviceInfo.presentModes.data());

		mState.physicalDevice = device;
		mState.graphicsQueueIndex = queueIndices.graphics;
		mState.presentQueueIndex = queueIndices.present;
		mState.computeQueueIndex = queueIndices.compute;
		mState.transferQueueIndex = queueIndices.transfer;
		LOG("DEVICE INITIALIZED");
		break;

	}

	if (mState.physicalDevice == VK_NULL_HANDLE) {
        LOG("DEVICE NOT FOUND");
		throw std::runtime_error("No valid physical device found");
    }
}

void DeviceManager::createLogicalDevice() 
{
	float queuePriority = 1.0f;

	std::unordered_set<uint32_t> uniqueIndices;
	uniqueIndices.insert(mState.graphicsQueueIndex);
	uniqueIndices.insert(mState.presentQueueIndex);
    if (mState.transferQueueIndex != UINT32_MAX)
	    uniqueIndices.insert(mState.transferQueueIndex);
	uniqueIndices.insert(mState.computeQueueIndex);

    mState.uniqueIndices = std::vector<uint32_t>(uniqueIndices.begin(), uniqueIndices.end());

    LOG("Queue indices: graphics: %u, present: %u, compute: %u, transfer: %u",
        mState.graphicsQueueIndex,
        mState.presentQueueIndex,
        mState.computeQueueIndex,
        mState.transferQueueIndex);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	for (auto index : uniqueIndices) {
		LOG("DEVICE INDEX: %d", index);
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = index;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	//TODO: be aware that extra femilies may mess stuff up
	VkPhysicalDeviceFeatures deviceFeatures = {};
	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = (uint32_t) queueCreateInfos.size();
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = sDeviceExtensions.size();
	createInfo.ppEnabledExtensionNames = sDeviceExtensions.data();
	
#ifdef AMVK_DEBUG
	createInfo.enabledLayerCount = sValidationLayers.size();
	createInfo.ppEnabledLayerNames = sValidationLayers.data(); 
#else
	createInfo.enabledLayerCount = 0;
#endif	
	VK_CHECK_RESULT(vkCreateDevice(mState.physicalDevice, &createInfo, nullptr, &mState.device));
	vkGetDeviceQueue(mState.device, mState.graphicsQueueIndex, 0, &mState.graphicsQueue);
	vkGetDeviceQueue(mState.device, mState.presentQueueIndex, 0, &mState.presentQueue);
    if (mState.transferQueueIndex != UINT32_MAX)
	    vkGetDeviceQueue(mState.device, mState.transferQueueIndex, 0, &mState.transferQueue);
	vkGetDeviceQueue(mState.device, mState.computeQueueIndex, 0, &mState.computeQueue);
	LOG("LOGICAL DEVICE CREATED");

	//init device info
	VkPhysicalDeviceFeatures physicalDeviceFeatures;
	VkPhysicalDeviceProperties physicalDeviceProperties;

	vkGetPhysicalDeviceFeatures(mState.physicalDevice, &physicalDeviceFeatures);
	vkGetPhysicalDeviceProperties(mState.physicalDevice, &physicalDeviceProperties);
	
	mState.deviceInfo.samplerAnisotropy = physicalDeviceFeatures.samplerAnisotropy;
	mState.deviceInfo.maxPushConstantsSize = physicalDeviceProperties.limits.maxPushConstantsSize;
	mState.deviceInfo.minUniformBufferOffsetAlignment = physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
	mState.deviceInfo.maxDescriptorSetUniformBuffersDynamic = physicalDeviceProperties.limits.maxDescriptorSetUniformBuffersDynamic;
	LOG("ANISOTROPY %u", physicalDeviceFeatures.samplerAnisotropy);
	LOG("MAX PUSH CONST SIZE max: %u", mState.deviceInfo.maxPushConstantsSize);

	LOG("LOGICAL DEVICE CREATED");
}


std::vector<const char*> DeviceManager::getExtensionNames() 
{
    std::vector<const char*> extensions;
#ifdef __ANDROID__
    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#ifdef AMVK_DEBUG
    extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif
    uint32_t instanceExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr);
    std::vector<VkExtensionProperties> properties(instanceExtensionCount);
    //VkExtensionProperties* inst_exts = (VkExtensionProperties *) malloc(instExtCount * sizeof(VkExtensionProperties));
    vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, properties.data());
    for (const char* extension : extensions) {
        bool extensionFound = false;
        for (VkExtensionProperties& property : properties) {
            if (strcmp(property.extensionName, extension) == 0) {
                extensionFound = true;
                break;
            }
        }

        if (!extensionFound) {
            LOG("Extension %s not found!", extension);
            std::string msg = std::string("Unable to found layer:") + extension;
            throw std::runtime_error(msg);
        }
    }

    LOG("All extenstions are found");

    //throw std::runtime_error("DeviceManager::getExtensionNames is unsupported");
#else
    unsigned numExt = 0;
    const char **ppExtenstions = glfwGetRequiredInstanceExtensions(&numExt);
    extensions.reserve(numExt);
    for (unsigned i = 0; i < numExt; ++i)
        extensions.push_back(ppExtenstions[i]);

#ifdef AMVK_DEBUG
    extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

#endif



    return extensions;
}

bool DeviceManager::deviceQueueIndicesSupported(const VkPhysicalDevice& physicalDevice, QueueIndices& outIndices) const {
	uint32_t numPhysicalDeviceQueueFamilies;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numPhysicalDeviceQueueFamilies, nullptr);
	std::vector<VkQueueFamilyProperties> properties(numPhysicalDeviceQueueFamilies);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numPhysicalDeviceQueueFamilies, properties.data());

	for (uint32_t i = 0; i < numPhysicalDeviceQueueFamilies; ++i) {
		VkQueueFamilyProperties& prop = properties[i];
		if (prop.queueCount == 0)
			continue;

		if (prop.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			outIndices.graphics = i;
		if (prop.queueFlags & VK_QUEUE_TRANSFER_BIT)
			outIndices.transfer = i;
		if (prop.queueFlags & VK_QUEUE_COMPUTE_BIT)
			outIndices.compute = i;

		VkBool32 surfaceSupported;
		VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, mState.surface, &surfaceSupported));

		if (surfaceSupported)
			outIndices.present = i;

        LOG("QUEUE INDICES:\n device: %p\n index: %u\n graphics: %s\n transfer: %s\n compute: %s\n present: %s",
            &physicalDevice,
            i,
            outIndices.graphics != UINT32_MAX ? "true" : "false",
            outIndices.transfer != UINT32_MAX ? "true" : "false",
            outIndices.compute != UINT32_MAX ? "true" : "false",
            outIndices.present != UINT32_MAX ? "true" : "false");

        // transfer queue is optional
		if (outIndices.graphics != UINT32_MAX
			&& outIndices.compute != UINT32_MAX
			&& outIndices.present != UINT32_MAX)
			return true;

	}
	return false;
}

bool DeviceManager::deviceExtensionsSupported(const VkPhysicalDevice& physicalDevice) const 
{
	uint32_t numExt = 0;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &numExt, nullptr);
	std::vector<VkExtensionProperties> props(numExt);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &numExt, props.data());
	std::unordered_set<std::string> propSet(sDeviceExtensions.begin(), sDeviceExtensions.end());
	
	for (const auto& prop : props) {
		propSet.erase(prop.extensionName);
		if (propSet.empty()) 
			return true;
	}

	return propSet.empty();
}

