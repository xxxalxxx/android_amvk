#include <engine.h>

#include "window.h"

Window::Window():
#ifndef __ANDROID__
	mGlfwWindow(nullptr),
#endif
	mWidth(800), 
	mHeight(600)
{
	calcAspect();
}

Window::Window(uint32_t w, uint32_t h):
#ifndef __ANDROID__
		mGlfwWindow(nullptr),
#endif
	mWidth(w), 
	mHeight(h)
{
	calcAspect();
}

bool Window::isOpen()
{
#ifdef __ANDROID__
	return true;
#else
	return !glfwWindowShouldClose(mGlfwWindow);
#endif
}

Window::~Window() 
{
#ifdef __ANDROID__

#else
	if (mGlfwWindow)
		glfwSetWindowShouldClose(mGlfwWindow, GLFW_TRUE);
	glfwTerminate();
#endif
}



void Window::initWindow(Engine& engine)
{
#ifdef __ANDROID__
    androidNativeWindow = engine.state->window;
	mWidth = ANativeWindow_getWidth(androidNativeWindow);
	mHeight = ANativeWindow_getHeight(androidNativeWindow);
	calcAspect();
	LOG("window: width = %u, height = %u", mWidth, mHeight);
#else
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	mGlfwWindow = glfwCreateWindow(mWidth, mHeight, "Learning Vulkan", nullptr, nullptr);

	if (!mGlfwWindow) {
		glfwTerminate();
		throw std::runtime_error("GLFW window initialization failed");
	}

	glfwSetInputMode(mGlfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetInputMode(mGlfwWindow, GLFW_STICKY_KEYS, 1);
	mInputManager.setGlfwWindow(*mGlfwWindow);
	glfwSetWindowUserPointer(mGlfwWindow, (void*) &engine);
#endif
}


#ifndef __ANDROID__
void Window::setWindowSizeCallback(GLFWwindowsizefun f) 
{
	glfwSetWindowSizeCallback(mGlfwWindow, f);
}
#endif


void Window::calcAspect() 
{
	mAspect = (float) mWidth / mHeight;
}

InputManager& Window::getInputManager()
{
	return mInputManager;
}

unsigned Window::getWidth() const  
{
	return mWidth;
}

unsigned Window::getHeight() const 
{
	return mHeight;
}

float Window::getAspect() const
{
	return mAspect;
}

void Window::setWidth(uint32_t w) 
{
	mWidth = w;
	calcAspect();
}

void Window::setHeight(uint32_t h) 
{
	mHeight = h;
	calcAspect();
}

void Window::setDimens(uint32_t w, uint32_t h) 
{
	mWidth = w;
	mHeight = h;
	calcAspect();
}
