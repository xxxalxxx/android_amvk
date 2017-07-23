#include "point_light.h"

PointLight::PointLight(UBO& ubo, LightUBO& lightUbo):
	ubo(&ubo),
	lightUbo(&lightUbo)
{

}

void PointLight::init(State& state, const glm::vec3& color, const glm::vec3& position, float radius, float specPower /* = 1.0f */)
{
	mState = &state;
	
	lightUbo->color = color;
	lightUbo->specPower = specPower;
	setPosition(position);
	setRadius(radius);
}

void PointLight::setRadius(float radius) 
{
	float r2 = radius * radius;
    float maxColor = fmax(fmax(lightUbo->color.x, lightUbo->color.y), lightUbo->color.z);

    lightUbo->quadratic = (256.0f * maxColor - 1.0f) / r2;
	lightUbo->linear = 0.0f;
	lightUbo->constant = 1.0f;

	glm::mat4& model = ubo->model;
	model[0][0] = radius;
	model[1][1] = radius;
	model[2][2] = radius;
}


float PointLight::getRadius() const
{
	return ubo->model[0][0];
}

glm::vec3 PointLight::getPosition() const
{
	glm::mat4& model = ubo->model;
	return glm::vec3(model[3][0], model[3][1], model[3][2]);
}

void PointLight::setPosition(const glm::vec3& position) 
{
	lightUbo->position = position;
	glm::mat4& model = ubo->model;
	model[3][0] = position.x;
	model[3][1] = position.y;
	model[3][2] = position.z;
}




