set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -fno-exceptions -fno-rtti -D VK_USE_PLATFORM_ANDROID_KHR")

add_library(vulkan_wrapper STATIC
            vulkan_wrapper.cpp)