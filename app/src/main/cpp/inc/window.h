#ifndef AMVK_WINDOW_H
#define AMVK_WINDOW_H

#ifdef __ANDROID__
#include <android/native_window.h>
#include <android_native_app_glue.h>
#else
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#endif

#include <exception>
#include <stdexcept>

#include "macro.h"
#include "input_manager.h"
#include "camera.h"

class Engine;

class Window {
	friend class Engine;

public:
	Window();
	Window(uint32_t w, uint32_t h);
	virtual ~Window();
	bool isOpen();
	InputManager& getInputManager();

	void setDimens(uint32_t w, uint32_t h);
	void setWidth(uint32_t w);
	void setHeight(uint32_t h);

	
	unsigned getWidth() const;
	unsigned getHeight() const;
	float getAspect() const;

#ifdef __ANDROID__
	ANativeWindow* androidNativeWindow;
#else
	void setWindowSizeCallback(GLFWwindowsizefun f);
	GLFWwindow* mGlfwWindow;
#endif
private:
	void initWindow(Engine& engine);

	void calcAspect();

	InputManager mInputManager;

	unsigned mWidth, mHeight;
	float mAspect;

};

#endif
