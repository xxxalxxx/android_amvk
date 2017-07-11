#include "camera.h"


const glm::vec3 Camera::UP = glm::vec3(0.0f, 1.0f, 0.0f);
const glm::vec3 Camera::RIGHT = glm::vec3(1.0f, 0.0f, 0.0f);

const float Camera::MAX_FOV_RADIANS = M_PI_2;
const float Camera::MIN_FOV_RADIANS = M_PI_4;


Camera::Camera():
	mNear(0.1f), mFar(1000.0f), 
	mFOV(0.5f * (MAX_FOV_RADIANS + MIN_FOV_RADIANS)), mAspect(1.0f),
	mPitch(0.0f), mYaw(3.0f),
	mForwardMovementScalar(40.0f), mSidewaysMovementScalar(40.0f), mMouseSensitivityScalar(0.002f), mScrollSensitivityScalar(0.05f),
	mEye(0.0f, 0.0f, -4.0f),
	initPrevPos(true)
{
	updateViewAngles();
	rebuildView();
	rebuildPerspective();
}


void Camera::updateViewAngles()
{
	glm::quat orientation = glm::angleAxis(mPitch, RIGHT) * glm::angleAxis(mYaw, UP);
    mView = glm::mat4_cast(orientation);
}

void Camera::updateOrientation(double mouseX, double mouseY)
{
	if (initPrevPos) {
		mPrevMouseX = mouseX;
		mPrevMouseY = mouseY;
		initPrevPos = false;
	}

    double mouseDx = mMouseSensitivityScalar * (mouseX - mPrevMouseX);
    double mouseDy = mMouseSensitivityScalar * (mouseY - mPrevMouseY);

    mYaw -= mouseDx;//invert angle
    mPrevMouseX = mouseX;

    float newPitch = mPitch + mouseDy;
    
    if (fabs(newPitch) < M_PI_2) {
        mPitch = newPitch;
        mPrevMouseY = mouseY;
    }
    
	updateViewAngles();
    rebuildView();
}

void Camera::updateFOV(float scrollDirection)
{
	mFOV += scrollDirection * mScrollSensitivityScalar;
}

void Camera::rebuildView()
{
	float offsetX = mView[0].x * mEye.x + mView[1].x * mEye.y + mView[2].x * mEye.z;
    float offsetY = mView[0].y * mEye.x + mView[1].y * mEye.y + mView[2].y * mEye.z;
    float offsetZ = mView[0].z * mEye.x + mView[1].z * mEye.y + mView[2].z * mEye.z;

    mView[3].x = -offsetX;
    mView[3].y = -offsetY;
    mView[3].z = -offsetZ;  
}

void Camera::rebuildPerspective()
{
	mProj = glm::perspective(mFOV, mAspect, mNear, mFar);
}



void Camera::setAspect(float aspect)
{
    mAspect = aspect;
    rebuildPerspective();
}

void Camera::moveStraight(float direction, float dt)
{
    float c = direction * mForwardMovementScalar * dt;
    //scale in direction of look vector
    mEye -= c * glm::vec3(mView[0].z, mView[1].z, mView[2].z); 
    rebuildView();
}


void Camera::moveSideways(float direction, float dt)
{  
    float c = direction * mSidewaysMovementScalar * dt; 
    //scale in direction of right vector
    mEye += c * glm::vec3(mView[0].x, mView[1].x, mView[2].x);
    rebuildView();
}

glm::mat4& Camera::view()
{
	return mView;
}

glm::mat4& Camera::proj()
{
	return mProj;
}
