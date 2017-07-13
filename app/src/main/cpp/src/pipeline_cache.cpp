#include "pipeline_cache.h"


void PipelineCacheInfo::getCache(const VkDevice& device)
{
    hasCache = pipelineCache != VK_NULL_HANDLE;
    if (hasCache)
        return;
    FileManager::getInstance().readCache(data, cacheName);
    PipelineCreator::pipelineCache(device, data, createInfo);

    VK_CHECK_RESULT(vkCreatePipelineCache(device, &createInfo, nullptr, &pipelineCache));
}

void PipelineCacheInfo::saveCache(const VkDevice& device)
{
    if (!hasCache && data.size() == 0) {
        LOG("CREATE CACHE: %s", cacheName);
        size_t cacheSize;
        VK_CHECK_RESULT(vkGetPipelineCacheData(device, pipelineCache, &cacheSize, NULL));
        char* cacheData = (char*) malloc(cacheSize);
        VK_CHECK_RESULT(vkGetPipelineCacheData(device, pipelineCache, &cacheSize, cacheData));
        LOG("CACHE SIZE: %zu DATA: %s", cacheSize, (const char*) cacheData);

        FileManager::getInstance().writeCache(cacheName, cacheData, cacheSize);
        free(cacheData);
    }
}


PipelineCacheInfo::PipelineCacheInfo(const char* cacheName, VkPipelineCache& pipelineCache):
        cacheName(cacheName),
        pipelineCache(pipelineCache)
{

}