#ifndef AMVK_CAMERA_H
#define AMVK_CAMERA_H

#include <cstdint>
#include <cmath>
#include "macro.h"
#include <glm/glm.hpp>



class Camera {
public:
	Camera();
	void updateOrientation(double mouseX, double mouseY);
	void updateFOV(float scrollDirection);
	void updateViewAngles();
	void rebuildView();
	void rebuildPerspective();
	void setAspect(float aspect);
    void moveStraight(float direction, float dt);
    void moveSideways(float direction, float dt);

	glm::mat4& proj();
	glm::mat4& view();

    double mPrevMouseX, mPrevMouseY;
private:
	float mNear, mFar, mFOV, mAspect;
    float mPitch, mYaw;
    float mForwardMovementScalar, mSidewaysMovementScalar, mMouseSensitivityScalar, mScrollSensitivityScalar;

    glm::vec3 mEye; 
    const static glm::vec3 UP, RIGHT;
    const static float MAX_FOV_RADIANS, MIN_FOV_RADIANS;

    glm::mat4 mProj, mView;
	bool initPrevPos;	
};

#endif
