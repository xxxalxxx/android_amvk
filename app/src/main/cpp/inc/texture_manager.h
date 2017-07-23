#ifndef AMVK_TEXTURE_MANAGER_H
#define AMVK_TEXTURE_MANAGER_H
#include "file_manager.h"
#include "macro.h"
#include "image_helper.h"
#include "image_info.h"
#include "state.h"
#include "texture_data.h"
#include <stb/stb_image.h>
#include <unordered_map>
#include <mutex> 

class TextureManager {
public:
	static TextureManager& getInstance();
	static ImageInfo* load(
			State& state,
			const VkCommandPool& cmdPool, 
			const VkQueue& cmdQueue,
			const TextureDesc& textureDesc);
	TextureManager(const TextureManager& textureManager) = delete;
	void operator=(const TextureManager& textureManager) = delete;
	virtual ~TextureManager();
private:
	TextureManager();
	std::unordered_map<TextureDesc, ImageInfo*> mPool; 
	std::mutex lock;
};


#endif
