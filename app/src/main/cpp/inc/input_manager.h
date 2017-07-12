#ifndef AMVK_INPUT_MANAGER_H
#define AMVK_INPUT_MANAGER_H


#ifdef __ANDROID__
#include <android/looper.h>
#else
#include <GLFW/glfw3.h>
#endif

#include <exception>
#include "camera.h"

class InputManager {
public:
	InputManager();
	virtual ~InputManager();

	void pollEvents();
	void handleMovement(double dt);
	bool keyPressed(int key);

#ifdef __ANDROID__
	bool touching;
    bool movingSideways;
    float directionSideways;
    bool movingForward;
    float directionForward;
#else
	InputManager(GLFWwindow& window);

	void setGlfwWindow(GLFWwindow& window);
	void assertGlfwWindowIsValid();
	void setFramebufferSizeCallback(GLFWframebuffersizefun cbfun);
	void setScrollCallback(GLFWscrollfun cbfun);
	void setKeyCallback(GLFWkeyfun cbfun);
	void setCursorPosCallback(GLFWcursorposfun cbfun);
	void setMouseButtonCallback(GLFWmousebuttonfun cbfun);

	GLFWwindow* mGlfwWindow;
#endif
};



#endif


