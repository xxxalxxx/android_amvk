#include "vulkan_utils.h"

const char* VulkanUtils::getVkResultString(int result)
{
		switch(result) {
		case 0:
				return "VK_SUCCESS";
		case 1: 
				return "VK_NOT_READY";
		case 2: 
				return "VK_TIMEOUT";
		case 3: 
				return "VK_EVENT_SET";
		case 4: 
				return "VK_EVENT_RESET";
		case 5: 
				return "VK_INCOMPLETE";
		case -1: 
				return "VK_ERROR_OUT_OF_HOST_MEMORY";
		case -2: 
				return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
		case -3: 
				return "VK_ERROR_INITIALIZATION_FAILED";
		case -4: 
				return "VK_ERROR_DEVICE_LOST";
		case -5: 
				return "VK_ERROR_MEMORY_MAP_FAILED";
		case -6: 
				return "VK_ERROR_LAYER_NOT_PRESET";
		case -7: 
				return "VK_ERROR_EXTENSTION_NOT_PRESENT";
		case -8: 
				return "VK_ERROR_FEATURE_NOT_PRESENT";
		case -9: 
				return "VK_ERROR_INCOMPATIBLE_DRIVER";
		case -10: 
				return "VK_ERROR_TOO_MANY_OBJECTS";
		case -11: 
				return "VK_ERROR_FORMAT_IS_NOT_SUPPORTED";
		case -12:
				return "VK_ERROR_FRAGMENT_POOL";
		case -1000000000: 
				return "VK_ERROR_SURFACE_LOST_KHR";
		case -1000000001: 
				return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
		case -1000001003: 
				return "VK_ERROR_SUBOPTIMAL_KHR";
		case -1000001004: 
				return "VK_ERROR_OUT_OF_DATE_KHR";
		case -1000003001: 
				return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
		case -1000011001: 
				return "VK_ERROR_VALIDATION_FAILED_EXT";
		case -1000012000: 
				return "VK_ERROR_INVALID_SHADER_NV";
		default: 
				return "ERROR_CODE_NOT_FOUND";
		}	
}

const char* VulkanUtils::getVkResultString(VkResult result)
{
	return getVkResultString(static_cast<int>(result));
}

