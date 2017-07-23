#include "fullscreen_quad.h"

FullscreenQuad::FullscreenQuad(State& state):
	mState(state),
	mDescriptorSet(VK_NULL_HANDLE),
	mImageInfo(nullptr)
{

}

void FullscreenQuad::init() 
{
	TextureDesc textureDesc(FileManager::getResourcePath("texture/statue.jpg"));
	mImageInfo = TextureManager::load(
            mState,
            mState.commandPool,
            mState.graphicsQueue,
			textureDesc);
	createDescriptorSets();
}

void FullscreenQuad::createDescriptorSets() 
{
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = mState.descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &mState.descriptorSetLayouts.sampler;

	VK_CHECK_RESULT(vkAllocateDescriptorSets(mState.device, &allocInfo, &mDescriptorSet));

	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = mImageInfo->imageView;
	imageInfo.sampler = mImageInfo->sampler;

	VkWriteDescriptorSet writeSet = {};
	writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeSet.dstSet = mDescriptorSet;
	writeSet.dstBinding = 0;
	writeSet.dstArrayElement = 0;
	writeSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeSet.descriptorCount = 1;
	writeSet.pImageInfo = &imageInfo;

	vkUpdateDescriptorSets(mState.device, 1, &writeSet, 0, nullptr);
}

void FullscreenQuad::draw(VkCommandBuffer& cmdBuffer) 
{
	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mState.pipelines.fullscreenQuad.pipeline);
	
	vkCmdBindDescriptorSets(
			cmdBuffer, 
			VK_PIPELINE_BIND_POINT_GRAPHICS,
            mState.pipelines.fullscreenQuad.layout,
			0, 
			1, 
			&mDescriptorSet, 
			0, 
			nullptr);

	vkCmdDraw(cmdBuffer, 3, 1, 0, 0);
}

void FullscreenQuad::draw(
		VkCommandBuffer& cmdBuffer, 
		VkPipeline pipeline, 
		VkPipelineLayout layout, 
		VkDescriptorSet* descriptors, 
		uint32_t numDescriptors) 
{
	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	
	vkCmdBindDescriptorSets(
			cmdBuffer, 
			VK_PIPELINE_BIND_POINT_GRAPHICS,
            layout,
			0, 
			numDescriptors, 
			descriptors, 
			0, 
			nullptr);

	vkCmdDraw(cmdBuffer, 3, 1, 0, 0);
}

