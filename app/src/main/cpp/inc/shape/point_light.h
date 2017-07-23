#ifndef AMVK_POINT_LIGHT_H
#define AMVK_POINT_LIGHT_H

#include "vulkan.h"
#include "state.h"
#include "sphere.h"
#include "timer.h"
#include "camera.h"

#include <array>
#include <math.h>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

class PointLight {
public:
	struct UBO {
	    glm::mat4 model;
		//glm::mat4 view;
		//glm::mat4 proj;
	};

	struct LightUBO {
		glm::vec3 color;
		glm::vec3 position;
		float specPower;
		float quadratic, linear, constant; 
	};

	PointLight(UBO& ubo, LightUBO& lightUbo);
	
	float getRadius() const;
	glm::vec3 getPosition() const;
	
	void setRadius(float radius);
	void setPosition(const glm::vec3& position);
	
	void update(VkCommandBuffer& commandBuffer, const Timer& timer, Camera& camera);
	void draw(VkCommandBuffer& cmdBuffer);
	void init(
			State& state,
			const glm::vec3& color, 
			const glm::vec3& position, 
			float radius, 
			float specPower = 1.0f);

	UBO* ubo;
	LightUBO* lightUbo;

private:

	State* mState;
};

#endif
