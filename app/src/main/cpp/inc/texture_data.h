#ifndef AMVK_TEXTURE_DATA_H
#define AMVK_TEXTURE_DATA_H

#include "state.h"
#include "file_manager.h"
#include <stb/stb_image.h>
#include <string>
#include <stdexcept>

class TextureData {
public:
	TextureData();
	~TextureData();
	stbi_uc* load(const char* filename, int reqComp);
	int getWidth() const;
	int getHeight() const;
	int getChannels() const;
	uint64_t getSize() const;
	stbi_uc* getPixels();

	int width, height, channels, size;
	stbi_uc* pixels; 
private:

};

struct TextureDesc {
	TextureDesc();
	TextureDesc(const char* filename, int reqComp = STBI_rgb_alpha);
	TextureDesc(std::string filename, int reqComp = STBI_rgb_alpha);
	bool operator==(const TextureDesc &other) const;
	std::string filename;
	int reqComp;
};

namespace std {
    template<> struct hash<TextureDesc> {
        size_t operator()(const TextureDesc& k) const
        {
     		size_t res = 17;
			res = res * 31 + std::hash<std::string>()(k.filename);
			res = res * 31 + std::hash<int>()(k.reqComp);
            return res;
        }
    };
}

#endif
