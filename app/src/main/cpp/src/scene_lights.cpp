#include "scene_lights.h"

SceneLights::SceneLights(State& state):
	mState(&state),
	mUniformBufferInfo(state.device),
	mSphere(state),
	ubo(nullptr)
{
	descriptors = {};
}

SceneLights::~SceneLights() 
{
	if (ubo) {
		vkUnmapMemory(mState->device, mUniformBufferInfo.memory);
		ubo = nullptr;
	}
}

void SceneLights::init() 
{
	mSphere.init(8, 16);

	PointLight& l1 = createPointLight();
	l1.init(*mState, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), 2.0f);

	PointLight& l2 = createPointLight();
	l2.init(*mState, glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(2.0f, -1.0f, 0.0f), 1.0f);
	//l2.ubo->model = glm::translate(glm::vec3(2.0f, -1.0, 0.0f));
	//PointLight l2 = createPointLight();

	createUniformBuffer();
	createDescriptorPool();
	createDescriptorSets();
}

VkDeviceSize SceneLights::calcAlignment(uint32_t structSize) 
{
	// (num of alignments in struct [0, n]) * (size of alignment) + (aligment if struct is not multiple of alignment)
	// for alignment of 256
	// if struct size = 264 -> (264 / 256) * 256 + ((264 % 256) > 0 ? 256 : 0 -> 1 * 256 + 256
	// if struct size = 64 -> (64 / 256) * 256 + ((64 % 256) > 0 ? 256 : 0 -> 0 + 256
	// if struct size = 512 -> (512 / 256) * 256 + ((512 % 256) > 0 ? 256 : 0 -> 512 + 0

	VkDeviceSize uboAlignment = mState->deviceInfo.minUniformBufferOffsetAlignment;
	return (structSize / uboAlignment) * uboAlignment + ((structSize % uboAlignment) > 0 ? uboAlignment : 0);
}

void SceneLights::draw(VkCommandBuffer& cmdBuffer)
{
	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mState->pipelines.pointLight.pipeline);
	vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &(mSphere.mCommonBufferInfo.buffer), &(mSphere.vertexBufferOffset));
	vkCmdBindIndexBuffer(cmdBuffer, mSphere.mCommonBufferInfo.buffer, mSphere.indexBufferOffset, VK_INDEX_TYPE_UINT32);

	for (size_t i = 0; i < pointLights.size(); ++i) {
		uint32_t uboOffset = sceneAlignment + i * mDynamicAlignmentSize;
		uint32_t lightOffset = uboOffset + pointLightUboAlignment;
		uint32_t dynamicOffsets[] = {
			uboOffset,
			lightOffset
		};

		vkCmdBindDescriptorSets(
			cmdBuffer, 
			VK_PIPELINE_BIND_POINT_GRAPHICS,
            mState->pipelines.pointLight.layout,
			0, 
			descriptors.size(), 
			descriptors.data(), 
			NUM_DYNAMIC_OFFSETS, 
			dynamicOffsets);

		vkCmdDrawIndexed(cmdBuffer, mSphere.numIndices, 1, 0, 0, 0);
	}
}

void SceneLights::update(VkCommandBuffer& cmdBuffer, const Timer& timer, Camera& camera)
{
	mState->ubo.view = camera.view();
	mState->ubo.proj = camera.proj();
	*((State::UBO*) ubo) = mState->ubo;

	//memcpy(ubo, &(mState->ubo), sizeof(State::UBO));

	for (size_t i = 0; i < pointLights.size(); ++i) {
		uint32_t uboOffset = sceneAlignment + i * mDynamicAlignmentSize;
		uint32_t lightOffset = uboOffset + pointLightUboAlignment;
		PointLight::UBO& model = pointLightUbos[i];
		PointLight::LightUBO& lightUbo = pointLightLightUbos[i];
		//PointLight::UBO* modelPtr = reinterpret_cast<PointLight::UBO*>(ubo + uboOffset);
		//*modelPtr = model;
		*((PointLight::UBO*) (ubo + uboOffset)) = model;
		*((PointLight::LightUBO*) (ubo + lightOffset)) = lightUbo;
	}
	
	VkMappedMemoryRange range = {};
	range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	range.memory = mUniformBufferInfo.memory;
	range.size = mUniformBufferInfo.size;
	vkFlushMappedMemoryRanges(mState->device, 1, &range);
	/*vkCmdUpdateBuffer(
			cmdBuffer,
			mUniformBufferInfo.buffer,
			0,
			mUniformBufferInfo.size,
			ubo);*/
}

void SceneLights::createUniformBuffer() 
{
	sceneAlignment = calcAlignment(sizeof(State::UBO));
	pointLightUboAlignment = calcAlignment(sizeof(PointLight::UBO));
	pointLightLightAlignment = calcAlignment(sizeof(PointLight::LightUBO));

	offsetUbos = sceneAlignment;
	offsetLightUbos = offsetUbos;

	mDynamicAlignmentSize = pointLightUboAlignment + pointLightLightAlignment;
	
	VkDeviceSize bufSize = sceneAlignment + pointLights.size() * mDynamicAlignmentSize;

	mUniformBufferInfo.size = bufSize;
	BufferHelper::createDynamicUniformBuffer(*mState, mUniformBufferInfo);

	vkMapMemory(mState->device, mUniformBufferInfo.memory, 0, mUniformBufferInfo.size, 0, (void**) &ubo);

	LOG("SCENE LIGHTS UNIFORM BUFFER");
}

void SceneLights::createDescriptorSets() 
{
	VkDescriptorSetLayout layouts[] = {
		mState->descriptorSetLayouts.uniformVertex,
		mState->descriptorSetLayouts.dynamicUniformVertex,
		mState->descriptorSetLayouts.dynamicUniformFragment
	};

	/*VkDescriptorSet descriptors[] = {
		sceneSet,
		modelSet,
		lightSet
	};*/

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = mDescriptorPool;
	allocInfo.descriptorSetCount = NUM_POINT_LIGHT_DESCRIPTORS;
	allocInfo.pSetLayouts = layouts;

	VK_CHECK_RESULT(vkAllocateDescriptorSets(mState->device, &allocInfo, descriptors.data()));


	VkDescriptorBufferInfo sceneInfo = {};
	sceneInfo.buffer = mUniformBufferInfo.buffer;
	sceneInfo.offset = 0;
	sceneInfo.range = sizeof(State::UBO);

	VkDescriptorBufferInfo buffInfo = {};
	buffInfo.buffer = mUniformBufferInfo.buffer;
	buffInfo.offset = 0;
	buffInfo.range = sizeof(PointLight::UBO);

	VkDescriptorBufferInfo lightBuffInfo = {};
	lightBuffInfo.buffer = mUniformBufferInfo.buffer;
	lightBuffInfo.offset = 0;
	lightBuffInfo.range = sizeof(PointLight::LightUBO);

	std::array<VkWriteDescriptorSet, 3> writeSets = {};

	writeSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeSets[0].dstSet = descriptors[0];
	writeSets[0].dstBinding = 0;
	writeSets[0].dstArrayElement = 0;
	writeSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeSets[0].descriptorCount = 1;
	writeSets[0].pBufferInfo = &sceneInfo;

	writeSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeSets[1].dstSet = descriptors[1];
	writeSets[1].dstBinding = 0;
	writeSets[1].dstArrayElement = 0;
	writeSets[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	writeSets[1].descriptorCount = 1;
	writeSets[1].pBufferInfo = &buffInfo;

	writeSets[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeSets[2].dstSet = descriptors[2];
	writeSets[2].dstBinding = 0;
	writeSets[2].dstArrayElement = 0;
	writeSets[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	writeSets[2].descriptorCount = 1;
	writeSets[2].pBufferInfo = &lightBuffInfo;

	vkUpdateDescriptorSets(mState->device, writeSets.size(), writeSets.data(), 0, nullptr);

	LOG("SCENE LIGHTS DESCRIPTORS");

/* List of descriptors example
   uint32_t numDescriptors = numPointLightDescriptors * pointLights.size();
	std::vector<VkWriteDescriptorSet> writeSets(numDescriptors);
	std::vector<VkDescriptorSetLayout> layouts(pointLights.size(), mState->descriptorSetLayouts.pointLight);
	pointLightDescriptors.resize(pointLights.size());
	pointLightInfos.resize(pointLights.size());

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = mDescriptorPool;
	allocInfo.descriptorSetCount = pointLightDescriptors.size();
	allocInfo.pSetLayouts = layouts.data();
	
	VK_CHECK_RESULT(vkAllocateDescriptorSets(mState->device, &allocInfo, pointLightDescriptors.data()));

	VkDescriptorBufferInfo sceneInfo = {};
	sceneInfo.buffer = mUniformBufferInfo.buffer;
	sceneInfo.offset = 0;
	sceneInfo.range = sizeof(State::UBO);

	VkDeviceSize offset = sceneAlignment;

	for (size_t i = 0; i < pointLights.size(); ++i) {
		PointLightInfo& info = pointLightInfos[i];
		//PointLight& light = *(pointLights[i]);
		
		VkDescriptorSet& descSet = pointLightDescriptors[i];
		//VkDescriptorSet& lightSet = pointLightDescriptors[setsPerLight * i + 1];
			
		VkWriteDescriptorSet& sceneWriteSet = writeSets[numPointLightDescriptors * i];
		VkWriteDescriptorSet& uboWriteSet = writeSets[numPointLightDescriptors * i + 1];
		VkWriteDescriptorSet& lightWriteSet = writeSets[numPointLightDescriptors * i + 2];
	
		info.uboOffset = offset;
		VkDescriptorBufferInfo buffInfo = {};
		buffInfo.buffer = mUniformBufferInfo.buffer;
		buffInfo.offset = offset;
		buffInfo.range = sizeof(PointLight::UBO);	
		offset += pointLightUboAlignment;

		info.lightOffset = offset;
		VkDescriptorBufferInfo lightBuffInfo = {};
		lightBuffInfo.buffer = mUniformBufferInfo.buffer;
		lightBuffInfo.offset = offset;
		lightBuffInfo.range = sizeof(PointLight::LightUBO);
		offset += pointLightLightAlignment;

		sceneWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		sceneWriteSet.dstSet = descSet;
		sceneWriteSet.dstBinding = 0;
		sceneWriteSet.dstArrayElement = 0;
		sceneWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		sceneWriteSet.descriptorCount = 1;
		sceneWriteSet.pBufferInfo = &sceneInfo;

		uboWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		uboWriteSet.dstSet = descSet;
		uboWriteSet.dstBinding = 1;
		uboWriteSet.dstArrayElement = 0;
		uboWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		uboWriteSet.descriptorCount = 1;
		uboWriteSet.pBufferInfo = &buffInfo;

		lightWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		lightWriteSet.dstSet = descSet;
		lightWriteSet.dstBinding = 2;
		lightWriteSet.dstArrayElement = 0;
		lightWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		lightWriteSet.descriptorCount = 1;
		lightWriteSet.pBufferInfo = &lightBuffInfo;
	}
	vkUpdateDescriptorSets(mState->device, writeSets.size(), writeSets.data(), 0, nullptr);
	*/
}

void SceneLights::createDescriptorPool() 
{
	/*
	   VkDescriptorPoolSize uboSize = {};
	uboSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	uboSize.descriptorCount = pointLights.size() + 1;
	*/

	VkDescriptorPoolSize uboSize = {};
	uboSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	uboSize.descriptorCount = NUM_POINT_LIGHT_DESCRIPTORS;
	
	VkDescriptorPoolSize uniformSize = {};
	uniformSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformSize.descriptorCount = NUM_POINT_LIGHT_DESCRIPTORS;
	
	VkDescriptorPoolSize poolSizes[] = {
		uboSize,
		uniformSize
	};
	
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = ARRAY_SIZE(poolSizes);
	poolInfo.pPoolSizes = poolSizes;
	poolInfo.maxSets = uboSize.descriptorCount;

	VK_CHECK_RESULT(vkCreateDescriptorPool(mState->device, &poolInfo, nullptr, &mDescriptorPool));
	LOG("SCENE LIGHTS DESCRIPTOR POOL");
}

void SceneLights::addLight(PointLight& light) 
{
//	pointLights.push_back(&light);
}

PointLight& SceneLights::createPointLight(PointLight::UBO& ubo, PointLight::LightUBO& lightUbo) 
{
	pointLightUbos.push_back(ubo);
	pointLightLightUbos.push_back(lightUbo);
	pointLights.push_back(PointLight(pointLightUbos.back(), pointLightLightUbos.back()));
	return pointLights.back();
}

PointLight& SceneLights::createPointLight() 
{
	PointLight::UBO ubo = {};
	PointLight::LightUBO lightUbo = {};
	return createPointLight(ubo, lightUbo);
}
