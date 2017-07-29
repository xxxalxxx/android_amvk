#ifndef AMVK_VULKAN_MANAGER_H
#define AMVK_VULKAN_MANAGER_H

#include <limits>
#include <cstring>
#include <vector>
#include <string>
#include <stdio.h>
#include <unordered_set>
#include <cstddef>
#include <chrono>

#include "macro.h"
#include "window.h"
#include "camera.h"
#include "timer.h"
#include "file_manager.h"
#include "vulkan_utils.h"
#include "pipeline_builder.h"
#include "pipeline_creator.h"
#include "texture_manager.h"
#include "device_manager.h"
#include "swapchain_manager.h"
#include "shader_creator.h"
#include "descriptor_creator.h"
#include "tquad.h"
#include "model.h"
#include "skinned.h"
#include "fullscreen_quad.h"
#include "point_light.h"
#include "scene_lights.h"
#include "g_buffer.h"

class Renderer {
	friend class Engine;
	struct SwapChainDesc;
public:

    Renderer(Window& window);
	virtual ~Renderer();
	void init();

	void buildCommandBuffers(const Timer &timer, Camera &camera);
	void buildGBuffers(const Timer &timer, Camera &camera);
	void buildComputeBuffers(const Timer &timer, Camera &camera); 
	void updateUniformBuffers(const Timer& timer, Camera& camera);
	void draw();
    void onWindowSizeChanged(uint32_t width, uint32_t height);
	
	void waitIdle();
	void recreateSwapChain();
private:
	void updateUniformBuffer(const Timer& timer);
	void createSemaphores();
	void createFences();
	
	Window& mWindow;
	State mState;
	DeviceManager mDeviceManager;
	SwapchainManager mSwapChainManager;
	TQuad tquad;
	Model suit;
	Skinned guard;
	Skinned dwarf;
	FullscreenQuad fullscreenQuad;
	SceneLights sceneLights;
	GBuffer gBuffer;

	uint32_t imageIndex;

	VkSemaphore imageAquiredSemaphore;
	VkSemaphore offscreenSemaphore;
	VkSemaphore renderFinishedSemaphore;
	VkSemaphore tilingFinishedSemaphore;

	VkFence tilingFence;
};

#endif
