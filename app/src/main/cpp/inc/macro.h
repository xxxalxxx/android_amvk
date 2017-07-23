#ifndef AMVK_MACROS_H

#define AMVK_MACROS_H

#include <iostream>
#include <exception>
#include <string>
#include <stdio.h>
#define AMVK_DEBUG

#define GLM_FORCE_RADIANS
#define GLM_FORCE_ZERO_TO_ONE
#define GLM_VULKAN_PERSPECTIVE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#if defined(__ANDROID__)

#include <android/log.h>
#define LOG(...) do { ((void)__android_log_print(ANDROID_LOG_INFO, "__AMVK", __VA_ARGS__)); } while (0)

#else

#define LOG(...) do { \
	printf(__VA_ARGS__); \
	printf("\n"); \
} while (0) 

#endif

#define LOG_TITLE(title) do { \
	std::string t = title; \
	unsigned nPad = 6; \
	std::string s(t.length() + nPad * 2, '*'); \
	std::string pad(nPad, ' '); \
	LOG(" "); \
	LOG("%s", s.c_str()); \
	LOG("%s%s", pad.c_str(), t.c_str()); \
	LOG("%s", s.c_str()); \
	LOG(" "); \
} while(0)

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define SAMPLER_LIST_SIZE 16

#endif


