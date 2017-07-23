#ifndef AMVK_SCENE_LIGHTS_H
#define AMVK_SCENE_LIGHTS_H

#include "point_light.h"
#include "buffer_helper.h"
#include "util.h"
#include "macro.h"
#include <vector>

class SceneLights {
public:
	static const constexpr uint32_t NUM_POINT_LIGHT_DESCRIPTORS = 3;
	static const constexpr uint32_t NUM_DYNAMIC_OFFSETS = 2;

	struct PointLightInfo {
		VkDeviceSize uboOffset;
		VkDeviceSize lightOffset;
	};

	SceneLights(State& state);
	virtual ~SceneLights();
	void init();
	void update(VkCommandBuffer& cmdBuffer, const Timer& timer, Camera& camera);
	void draw(VkCommandBuffer& cmdBuffer);
	void addLight(PointLight& light);
	PointLight& createPointLight(PointLight::UBO& ubo, PointLight::LightUBO& lightUbo);
	PointLight& createPointLight();

private:

	VkDeviceSize calcAlignment(uint32_t structSize);

	void createDescriptorPool();
	void createDescriptorSets();
	void createUniformBuffer();

	std::vector<PointLight> pointLights;
	std::vector<PointLightInfo> pointLightInfos;
	std::vector<PointLight::UBO> pointLightUbos;
	std::vector<PointLight::LightUBO> pointLightLightUbos;

	//std::vector<VkDescriptorSet> pointLightDescriptors;
	
	uint32_t mDynamicAlignmentSize;

	VkDeviceSize sceneAlignment;
	VkDeviceSize pointLightUboAlignment;
	VkDeviceSize pointLightLightAlignment;
	VkDeviceSize offsetUbos;
	VkDeviceSize offsetLightUbos;

	VkDescriptorSet sceneSet;
	VkDescriptorSet modelSet;
	VkDescriptorSet lightSet;

	std::array<VkDescriptorSet, NUM_POINT_LIGHT_DESCRIPTORS> descriptors;


	State* mState;
	VkDescriptorPool mDescriptorPool;
	BufferInfo mUniformBufferInfo;
	Sphere mSphere;

	char* ubo;

};

#endif
