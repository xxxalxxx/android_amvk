#ifndef AMVK_ENGINE_H
#define AMVK_ENGINE_H

#include <vector>

#include "macro.h"
#include "task_manager.h"
#include "renderer.h"
#include "window.h"
#include "timer.h"
#include "file_manager.h"

#ifdef __ANDROID__
#include <android_native_app_glue.h>
#endif

class Engine {
public:
	Engine();
	virtual ~Engine();

#ifdef __ANDROID__
    static void handleCmd(struct android_app *app, int32_t cmd);
    static int32_t handleInput(android_app *app, AInputEvent *event);

    void init(android_app* state);

    android_app* state;
    bool isReady;
    bool hasFocus;
#else
	void init();
#endif
	void handleMovement(double dt);
	Window& getWindow();
	TaskManager& getTaskManager();
	Renderer& getRenderer();
	Timer& getTimer();
	Camera& getCamera();
private:
	Window mWindow;
	Camera mCamera;
	TaskManager mTaskManager;
	Renderer mRenderer;
	Timer mTimer;


};


#endif
