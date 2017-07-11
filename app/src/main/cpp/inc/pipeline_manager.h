#ifndef AMVK_PIPELINE_MANAGER_H
#define AMVK_PIPELINE_MANAGER_H

#include <cstddef>
#include <vector>
#include "macro.h"
#include "vulkan_state.h"
#include "pipeline_creator.h"

struct PipelineCacheInfo {
	PipelineCacheInfo(const char* cacheName, VkPipelineCache& pipelineCache);
	
	void getCache(const VkDevice& device);
	void saveCache(const VkDevice& device);

	const char* cacheName;
	VkPipelineCacheCreateInfo createInfo;
	VkPipelineCache& pipelineCache;
	std::vector<char> data;
	bool hasCache;
};

class PipelineManager {
public:
	PipelineManager(VulkanState& state);
	static void getCachedPipeline(const char* cacheName, PipelineCacheInfo& out);
	static void saveCachedPipeline();
	void createPipelines();
private:
	void createQuadPipeline();
	VulkanState& mState;
	Pipelines& mPipelines;
};


#endif
