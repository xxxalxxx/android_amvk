#ifndef AMVK_QUAD_H
#define AMVK_QUAD_H
#include "macro.h"

#ifdef __ANDROID__
#include "vulkan_wrapper.h"
#else
#include <vulkan/vulkan.h>
#endif

#include <cstring>
#include <cstddef>
#include <string.h>
#include <array>
#include <vector>

#include "buffer_helper.h"
#include "vulkan_image_creator.h"
#include "vulkan_image_info.h"
#include "vulkan_render_pass_creator.h"
#include "texture_manager.h"
#include "pipeline_creator.h"
#include "pipeline_cache.h"
#include "timer.h"
#include "camera.h"

class Quad {
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

	static uint32_t const VERTEX_SIZE = sizeof(Vertex);
	static uint32_t const UBO_SIZE = sizeof(UBO);
	static uint32_t const PUSH_CONST_SIZE = sizeof(PushConstants);


	static void createPipeline(VulkanState& state);

	Quad(VulkanState& vulkanState);
	~Quad();
	void draw(VkCommandBuffer& commandBuffer); 
	void init();
	void update(VkCommandBuffer& commandBuffer, const Timer& timer, Camera& camera);

	uint32_t numIndices;
	
	VkDeviceSize mVertexBufferOffset, 
				 mIndexBufferOffset, 
				 mUniformBufferOffset;

	VulkanState& mVulkanState;
	BufferInfo mCommonBufferInfo;
	BufferInfo mCommonStagingBufferInfo;
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


