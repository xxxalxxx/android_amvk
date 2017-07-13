#ifndef AMVK_PIPELINE_CACHE_H
#define AMVK_PIPELINE_CACHE_H

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

#endif //AMVK_PIPELINE_CACHE_H
