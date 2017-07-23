#ifndef AMVK_TQUAD_H
#define AMVK_TQUAD_H
#include "macro.h"
#include "vulkan.h"

#include <cstring>
#include <cstddef>
#include <string.h>
#include <array>
#include <vector>

#include "buffer_helper.h"
#include "image_helper.h"
#include "image_info.h"
#include "texture_manager.h"
#include "pipeline_builder.h"
#include "pipeline_cache.h"
#include "timer.h"
#include "camera.h"

#include <glm/gtx/string_cast.hpp>

class TQuad {
public:
	struct Vertex {
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 texCoord;
	};

	struct UBO {
	    glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	struct PushConstants {
	    glm::mat4 model;
		//glm::mat4 view;
		//glm::mat4 proj;
	};

	static void createPipeline(State& state);

	TQuad(State& vulkanState);
	~TQuad();
	void draw(VkCommandBuffer& commandBuffer); 
	void init();
	void update(VkCommandBuffer& commandBuffer, const Timer& timer, Camera& camera);

	uint32_t numIndices;
	
	VkDeviceSize mVertexBufferOffset, 
				 mIndexBufferOffset, 
				 mUniformBufferOffset;

	State& mState;
	BufferInfo mCommonBufferInfo;
	BufferInfo mVertexBufferDesc, mIndexBufferDesc, mUniformBufferDesc, mUniformStagingBufferDesc;
	ImageInfo *mTextureDesc;
	VkDescriptorSet mVkDescriptorSet;

private:
	void createBuffers();
	void createVertexBuffer();
	void createIndexBuffer();
	void createUniformBuffer();

	void createDescriptorSetLayout();

	void createDescriptorPool();
	void createDescriptorSet();


	std::array<VkVertexInputAttributeDescription, 3> getAttrDesc() const;
	VkVertexInputBindingDescription getBindingDesc() const;

};

#endif


