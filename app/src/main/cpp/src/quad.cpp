#include "quad.h"

Quad::Quad(VulkanState& vulkanState):
	mVulkanState(vulkanState), 
	mCommonBufferInfo(vulkanState.device),
	mCommonStagingBufferInfo(vulkanState.device),
	mVertexBufferDesc(vulkanState.device),
	mIndexBufferDesc(vulkanState.device),
	mUniformBufferDesc(vulkanState.device),
	mUniformStagingBufferDesc(vulkanState.device)
{
	
}

Quad::~Quad() 
{
	mTextureDesc = nullptr;
}

void Quad::init()
{
	TextureDesc textureDesc(FileManager::getResourcePath("texture/statue.jpg"));//"texture/statue.jpg"));
	mTextureDesc = TextureManager::load(
			mVulkanState, 
			mVulkanState.commandPool, 
			mVulkanState.graphicsQueue, 
			textureDesc);

	createBuffers();
	createDescriptorSet();
}

void Quad::update(VkCommandBuffer& commandBuffer, const Timer& timer, Camera& camera) 
{

	UBO ubo = {};
	ubo.model = glm::mat4();
	ubo.view = camera.view();
	ubo.proj = camera.proj();
	/*
	Buffer update via copy
	BufferHelper::mapMemory(mVulkanState, mCommonStagingBufferInfo.memory, mUniformBufferOffset, sizeof(ubo), &ubo);
	BufferHelper::copyBuffer(
			mVulkanState.device,
			mVulkanState.commandPool,
			mVulkanState.graphicsQueue,
			mCommonStagingBufferInfo.buffer,
			mCommonBufferInfo.buffer,
			mUniformBufferOffset,
			sizeof(ubo));
	*/

	//CmdPass cmdPass(mVulkanState.device, mVulkanState.commandPool, mVulkanState.graphicsQueue);

	vkCmdUpdateBuffer(
			commandBuffer,
			mCommonBufferInfo.buffer,
			mUniformBufferOffset,
			UBO_SIZE,
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


void Quad::draw(VkCommandBuffer& commandBuffer) 
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mVulkanState.pipelines.quad.pipeline);

	VkDeviceSize offset = mVertexBufferOffset;
	VkBuffer& commonBuff = mCommonBufferInfo.buffer;
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &commonBuff, &offset);
	vkCmdBindIndexBuffer(commandBuffer, mCommonBufferInfo.buffer, mIndexBufferOffset, VK_INDEX_TYPE_UINT32);

	//vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertBuf, offsets);
	//vkCmdBindIndexBuffer(commandBuffer, mIndexBufferDesc.buffer, 0, VK_INDEX_TYPE_UINT32);
	
	vkCmdBindDescriptorSets(
			commandBuffer, 
			VK_PIPELINE_BIND_POINT_GRAPHICS, 
			mVulkanState.pipelines.quad.layout, 
			0, 
			1, 
			&mVkDescriptorSet, 
			0, 
			nullptr);

	vkCmdDrawIndexed(commandBuffer, numIndices, 1, 0, 0, 0);
}

void Quad::createBuffers() 
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
	VkDeviceSize uniformBufferSize = UBO_SIZE;
	UBO ubo = {};
	
	mUniformBufferOffset = 0;
	mVertexBufferOffset = uniformBufferSize;
	mIndexBufferOffset = uniformBufferSize + vertexBufferSize;

	//mVertexBufferOffset = 0;
	//mIndexBufferOffset = vertexBufferSize;
	//mUniformBufferOffset = vertexBufferSize + indexBufferSize;
	mCommonBufferInfo.size = vertexBufferSize + indexBufferSize + uniformBufferSize;
	mCommonStagingBufferInfo.size = mCommonBufferInfo.size;
	BufferHelper::createStagingBuffer(mVulkanState, mCommonStagingBufferInfo);
	
	char* data;
	vkMapMemory(mVulkanState.device, mCommonStagingBufferInfo.memory, 0, mCommonStagingBufferInfo.size, 0, (void**) &data);
	memcpy(data + mUniformBufferOffset, &ubo, (size_t) uniformBufferSize);
	memcpy(data + mVertexBufferOffset, vertices.data(), vertexBufferSize);
	memcpy(data + mIndexBufferOffset, indices.data(), indexBufferSize);
	vkUnmapMemory(mVulkanState.device, mCommonStagingBufferInfo.memory);

	BufferHelper::createCommonBuffer(mVulkanState, mCommonBufferInfo);

	BufferHelper::copyBuffer(
			mVulkanState,
			mCommonStagingBufferInfo.buffer, 
			mCommonBufferInfo.buffer, 
			mCommonBufferInfo.size);
}

void Quad::createVertexBuffer() 
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
	BufferInfo stagingDesc(mVulkanState.device, bufferSize);

	BufferHelper::createStagingBuffer(mVulkanState,stagingDesc);
	BufferHelper::mapMemory(mVulkanState, stagingDesc, vertices.data());
	BufferHelper::createVertexBuffer(mVulkanState, mVertexBufferDesc);

	BufferHelper::copyBuffer(
			mVulkanState,
			stagingDesc.buffer, 
			mVertexBufferDesc.buffer, 
			bufferSize);
}

void Quad::createIndexBuffer()
{
	const std::vector<uint32_t> indices = {
	    0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4
	};
	
	numIndices = indices.size(); 
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
	mIndexBufferDesc.size = bufferSize;
	BufferInfo stagingDesc(mVulkanState.device, bufferSize);

	BufferHelper::createStagingBuffer(mVulkanState, stagingDesc);
	BufferHelper::mapMemory(mVulkanState, stagingDesc, indices.data());
	BufferHelper::createIndexBuffer(mVulkanState,mIndexBufferDesc);
	
	BufferHelper::copyBuffer(
			mVulkanState,
			stagingDesc.buffer, 
			mIndexBufferDesc.buffer, 
			bufferSize);
}

void Quad::createUniformBuffer()
{	
	VkDeviceSize bufSize = UBO_SIZE;

	mUniformStagingBufferDesc.size = bufSize;
	mUniformBufferDesc.size = bufSize;

	BufferHelper::createStagingBuffer(mVulkanState, mUniformStagingBufferDesc);
	BufferHelper::createUniformBuffer(mVulkanState, mUniformBufferDesc);
}

void Quad::createDescriptorSet() 
{
	VkDescriptorSetLayout layouts[] = { mVulkanState.descriptorSetLayouts.quad };
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = mVulkanState.descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = layouts;

	VK_CHECK_RESULT(vkAllocateDescriptorSets(mVulkanState.device, &allocInfo, &mVkDescriptorSet));

	VkDescriptorBufferInfo buffInfo = {};
	buffInfo.buffer = mCommonBufferInfo.buffer;
	buffInfo.offset = mUniformBufferOffset;
	buffInfo.range = UBO_SIZE;

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

	vkUpdateDescriptorSets(mVulkanState.device, writeSets.size(), writeSets.data(), 0, nullptr);
}


