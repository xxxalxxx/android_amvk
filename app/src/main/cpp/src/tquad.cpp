#include "tquad.h"

TQuad::TQuad(VulkanState& vulkanState):
        mState(vulkanState),
	mCommonBufferInfo(vulkanState.device),
	mVertexBufferDesc(vulkanState.device),
	mIndexBufferDesc(vulkanState.device),
	mUniformBufferDesc(vulkanState.device),
	mUniformStagingBufferDesc(vulkanState.device)
{
	
}

TQuad::~TQuad()
{
	mTextureDesc = nullptr;
}

void TQuad::init()
{
	TextureDesc textureDesc(FileManager::getResourcePath("texture/statue.jpg"));//"texture/statue.jpg"));
	mTextureDesc = TextureManager::load(
            mState,
            mState.commandPool,
            mState.graphicsQueue,
			textureDesc);

	createBuffers();
	createDescriptorSet();
}

void TQuad::update(VkCommandBuffer& commandBuffer, const Timer& timer, Camera& camera)
{

	UBO ubo = {};
	ubo.model = glm::mat4();
	ubo.view = camera.view();
	ubo.proj = camera.proj();
	/*
	Buffer update via copy
	BufferHelper::mapMemory(mVulkanState, staging.memory, mUniformBufferOffset, sizeof(ubo), &ubo);
	BufferHelper::copyBuffer(
			mVulkanState.device,
			mVulkanState.commandPool,
			mVulkanState.graphicsQueue,
			staging.buffer,
			mCommonBufferInfo.buffer,
			mUniformBufferOffset,
			sizeof(ubo));
	*/

	//CmdPass cmdPass(mVulkanState.device, mVulkanState.commandPool, mVulkanState.graphicsQueue);

	vkCmdUpdateBuffer(
			commandBuffer,
			mCommonBufferInfo.buffer,
			mUniformBufferOffset,
			sizeof(UBO),
			&ubo);
	/*
	void* data;
	vkMapMemory(mVulkanState.device, mUniformStagingBufferDesc.memory, 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(mVulkanState.device, mUniformStagingBufferDesc.memory);

	BufferHelper::copyBuffer(
			mVulkanState,
			mUniformStagingBufferDesc.buffer,
			mUniformBufferDesc.buffer,
			sizeof(ubo));
	*/

	//glm::rotate(glm::mat4(), (float) (10.f * timer.total() * glm::radians(90.0f)), glm::vec3(0.0f, 0.0f, 1.0f));



	// Update push constants
	/*PushConstants pushConstants;
	pushConstants.model = glm::mat4();
	pushConstants.view = camera.view();
	pushConstants.proj = camera.proj();
	vkCmdPushConstants(
			commandBuffer,
			mVulkanState.pipelines.quad.layout,
			VK_SHADER_STAGE_VERTEX_BIT,
			0,
			sizeof(PushConstants),
			&pushConstants);*/

} 


void TQuad::draw(VkCommandBuffer& commandBuffer)
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mState.pipelines.tquad.pipeline);

	VkDeviceSize offset = mVertexBufferOffset;
	VkBuffer& commonBuff = mCommonBufferInfo.buffer;
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &commonBuff, &offset);
	vkCmdBindIndexBuffer(commandBuffer, mCommonBufferInfo.buffer, mIndexBufferOffset, VK_INDEX_TYPE_UINT32);

	//vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertBuf, offsets);
	//vkCmdBindIndexBuffer(commandBuffer, mIndexBufferDesc.buffer, 0, VK_INDEX_TYPE_UINT32);
	
	vkCmdBindDescriptorSets(
			commandBuffer, 
			VK_PIPELINE_BIND_POINT_GRAPHICS,
            mState.pipelines.tquad.layout,
			0, 
			1, 
			&mVkDescriptorSet, 
			0, 
			nullptr);

	vkCmdDrawIndexed(commandBuffer, numIndices, 1, 0, 0, 0);
}

void TQuad::createBuffers()
{
	// Vertex

	const std::vector<Vertex> vertices = {
	    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
		{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

	    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},

		{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
		{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
	};

	VkDeviceSize vertexBufferSize = sizeof(vertices[0]) * vertices.size();

	// Index

	const std::vector<uint32_t> indices = {
	    0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4
	};
	
	numIndices = indices.size(); 
	VkDeviceSize indexBufferSize = sizeof(indices[0]) * indices.size();
	
	// Uniform
	//TODO: check how to add uniform without recopying same buffer
	VkDeviceSize uniformBufferSize = sizeof(UBO);
	UBO ubo = {};
	
	mUniformBufferOffset = 0;
	mVertexBufferOffset = uniformBufferSize;
	mIndexBufferOffset = uniformBufferSize + vertexBufferSize;

	//mVertexBufferOffset = 0;
	//mIndexBufferOffset = vertexBufferSize;
	//mUniformBufferOffset = vertexBufferSize + indexBufferSize;
	mCommonBufferInfo.size = vertexBufferSize + indexBufferSize + uniformBufferSize;
	BufferInfo staging(mState.device);
	staging.size = mCommonBufferInfo.size;
	BufferHelper::createStagingBuffer(mState, staging);
	
	char* data;
	vkMapMemory(mState.device, staging.memory, 0, staging.size, 0, (void**) &data);
	memcpy(data + mUniformBufferOffset, &ubo, (size_t) uniformBufferSize);
	memcpy(data + mVertexBufferOffset, vertices.data(), vertexBufferSize);
	memcpy(data + mIndexBufferOffset, indices.data(), indexBufferSize);
	vkUnmapMemory(mState.device, staging.memory);

	BufferHelper::createCommonBuffer(mState, mCommonBufferInfo);

	BufferHelper::copyBuffer(
            mState,
			staging.buffer, 
			mCommonBufferInfo.buffer, 
			mCommonBufferInfo.size);
}

void TQuad::createVertexBuffer()
{
	const std::vector<Vertex> vertices = {
	    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
		{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

	    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},

		{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
		{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
	};

	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
	mVertexBufferDesc.size = bufferSize;
	BufferInfo stagingDesc(mState.device, bufferSize);

	BufferHelper::createStagingBuffer(mState,stagingDesc);
	BufferHelper::mapMemory(mState, stagingDesc, vertices.data());
	BufferHelper::createVertexBuffer(mState, mVertexBufferDesc);

	BufferHelper::copyBuffer(
            mState,
			stagingDesc.buffer, 
			mVertexBufferDesc.buffer, 
			bufferSize);
}

void TQuad::createIndexBuffer()
{
	const std::vector<uint32_t> indices = {
	    0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4
	};
	
	numIndices = indices.size(); 
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
	mIndexBufferDesc.size = bufferSize;
	BufferInfo stagingDesc(mState.device, bufferSize);

	BufferHelper::createStagingBuffer(mState, stagingDesc);
	BufferHelper::mapMemory(mState, stagingDesc, indices.data());
	BufferHelper::createIndexBuffer(mState,mIndexBufferDesc);
	
	BufferHelper::copyBuffer(
            mState,
			stagingDesc.buffer, 
			mIndexBufferDesc.buffer, 
			bufferSize);
}

void TQuad::createUniformBuffer()
{	
	VkDeviceSize bufSize = sizeof(UBO);

	mUniformStagingBufferDesc.size = bufSize;
	mUniformBufferDesc.size = bufSize;

	BufferHelper::createStagingBuffer(mState, mUniformStagingBufferDesc);
	BufferHelper::createUniformBuffer(mState, mUniformBufferDesc);
}

void TQuad::createDescriptorSet()
{
	VkDescriptorSetLayout layouts[] = { mState.descriptorSetLayouts.tquad };
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = mState.descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = layouts;

	VK_CHECK_RESULT(vkAllocateDescriptorSets(mState.device, &allocInfo, &mVkDescriptorSet));

	VkDescriptorBufferInfo buffInfo = {};
	buffInfo.buffer = mCommonBufferInfo.buffer;
	buffInfo.offset = mUniformBufferOffset;
	buffInfo.range = sizeof(UBO);

	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = mTextureDesc->imageView;
	imageInfo.sampler = mTextureDesc->sampler;

	std::array<VkWriteDescriptorSet, 2> writeSets = {};

	writeSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeSets[0].dstSet = mVkDescriptorSet;
	writeSets[0].dstBinding = 0;
	writeSets[0].dstArrayElement = 0;
	writeSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeSets[0].descriptorCount = 1;
	writeSets[0].pBufferInfo = &buffInfo;

	writeSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeSets[1].dstSet = mVkDescriptorSet;
	writeSets[1].dstBinding = 1;
	writeSets[1].dstArrayElement = 0;
	writeSets[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeSets[1].descriptorCount = 1;
	writeSets[1].pImageInfo = &imageInfo;

	vkUpdateDescriptorSets(mState.device, writeSets.size(), writeSets.data(), 0, nullptr);
}


