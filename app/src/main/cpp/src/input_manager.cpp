#include "input_manager.h"





InputManager::~InputManager()
{

}

void InputManager::pollEvents()
{
#ifdef __ANDROID__

#else
	glfwPollEvents();
#endif
}

#ifdef __ANDROID__
InputManager::InputManager():
        touching(false),
        movingSideways(false),
        directionSideways(0.0f),
        movingForward(false),
        directionForward(0.0f)
{

}
#else

InputManager::InputManager()
		: mGlfwWindow(nullptr)
{

}

InputManager::InputManager(GLFWwindow& glfwWindow)
		: mGlfwWindow(&glfwWindow)
{

}

void InputManager::setGlfwWindow(GLFWwindow& glfwWindow)
{
	mGlfwWindow = &glfwWindow;
}

void InputManager::assertGlfwWindowIsValid()
{
	if (!mGlfwWindow)
		throw std::runtime_error("GLFWWindow cannot be NULL");
}

void InputManager::setFramebufferSizeCallback(GLFWframebuffersizefun cbfun)
{
	assertGlfwWindowIsValid();
	glfwSetFramebufferSizeCallback(mGlfwWindow, cbfun);
}

void InputManager::setScrollCallback(GLFWscrollfun cbfun)
{
	assertGlfwWindowIsValid();
	glfwSetScrollCallback(mGlfwWindow, cbfun);
}

void InputManager::setKeyCallback(GLFWkeyfun cbfun)
{
	assertGlfwWindowIsValid();
	glfwSetKeyCallback(mGlfwWindow, cbfun);
}

void InputManager::setCursorPosCallback(GLFWcursorposfun cbfun)
{
	assertGlfwWindowIsValid();
	glfwSetCursorPosCallback(mGlfwWindow, cbfun);
}

void InputManager::setMouseButtonCallback(GLFWmousebuttonfun cbfun)
{
	assertGlfwWindowIsValid();
	glfwSetMouseButtonCallback(mGlfwWindow, cbfun);
}

bool InputManager::keyPressed(int key)
{
	return glfwGetKey(mGlfwWindow, key) == GLFW_PRESS;
}


#endif
