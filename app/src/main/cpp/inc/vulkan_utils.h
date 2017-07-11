#ifndef AMVK_VULKAN_UTILS_H
#define AMVK_VULKAN_UTILS_H


#ifdef __ANDROID__
#include "vulkan_wrapper.h"
#else
#include <vulkan/vulkan.h>
#endif

#include <cstdio>
#include <stdexcept>

#include "macro.h"

#define VK_THROW_RESULT_ERROR(text, result) \
	do { \
		char str[128]; \
		int resultCode = static_cast<int>(result); \
        LOG(str, #text " VkResult: %s (code: %d)", VulkanUtils::getVkResultString(resultCode), resultCode); \
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

class VulkanUtils {
public:
	static const char* getVkResultString(int result);
	static const char* getVkResultString(VkResult result);
private:
};

#endif
