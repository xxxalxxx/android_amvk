#include "engine.h"

Engine::~Engine() 
{

}



#ifdef __ANDROID__

Engine::Engine():
        mRenderer(mWindow),
        isReady(false),
        hasFocus(false)
{

}

void Engine::init(android_app* state)
{
    FileManager::activity = state->activity;
    FileManager::assetManager = state->activity->assetManager;
    FileManager::internalStoragePath = state->activity->internalDataPath;
    mWindow.initWindow(*this);
    LOG("WINDOW ASPECT %f width: %u height: %u", mWindow.mAspect, mWindow.mWidth, mWindow.mHeight);
    mCamera.setAspect(mWindow.mAspect);
    mRenderer.init();
    mRenderer.buildCommandBuffers(mTimer, mCamera);
    mRenderer.buildGBuffers(mTimer, mCamera);

    JNIEnv* jni;
    state->activity->vm->AttachCurrentThread(&jni, NULL);
    jclass clazz = jni->GetObjectClass(state->activity->clazz);
    jmethodID methodID = jni->GetMethodID(clazz, "showUI", "()V");
    jni->CallVoidMethod(state->activity->clazz, methodID);
    state->activity->vm->DetachCurrentThread();
}


int32_t Engine::handleInput(android_app *app, AInputEvent *event)
{
    Engine* eng = (Engine*) app->userData;
    InputManager& inputManager = eng->getWindow().getInputManager();
    Camera& camera = eng->getCamera();

    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {

        int flags =  AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;

        float x = AMotionEvent_getX(event, 0);
        float y = AMotionEvent_getY(event, 0);


        switch (flags) {
            case AMOTION_EVENT_ACTION_DOWN:
                inputManager.touching = true;
                camera.mPrevMouseX = x;
                camera.mPrevMouseY = y;
                break;
            case AMOTION_EVENT_ACTION_UP:
                inputManager.touching = false;
                break;
        }

        camera.updateOrientation(x, y);


        return 1;
    }
    return 0;
}

void Engine::handleCmd(struct android_app *app, int32_t cmd)
{
    Engine* eng = (Engine*) app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            break;
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            if (app->window != NULL) {
                eng->isReady = true;
                //eng->initDisplay();
                //eng->drawFrame();
            }
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            //eng->termDisplay();
            eng->hasFocus = false;
            break;
        case APP_CMD_STOP:
            break;
        case APP_CMD_GAINED_FOCUS:

            // Start animation
            //eng->hasFocus = true;
            break;
        case APP_CMD_LOST_FOCUS:
            // Also stop animating.
            //eng->hasFocus = false;
            //eng->drawFrame();
            break;
        case APP_CMD_LOW_MEMORY:
            // Free up GL resources
            //eng->trimMemory();
            break;
    }
}

#else

Engine::Engine():
	mVulkanManager(mWindow)
{

}

void onWindowResized(GLFWwindow* window, int width, int height) {
    if (width < 0)
        width = 0;
    if (height < 0)
        height = 0;
	if (width * height == 0)
		return;

	Engine* eng = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
	Window& engWindow = eng->getWindow();
	Camera& engCamera = eng->getCamera();
    Renderer& renderer = eng->
	engWindow.setDimens(width, height);
    engCamera.setAspect(engWindow.getAspect());
    engCamera.rebuildPerspective();
    
	eng->getRenderer().onWindowSizeChanged((uint32_t) width, (uint32_t) height);
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
	Engine* eng = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
	Camera& engCamera = eng->getCamera();
    engCamera.updateOrientation(xpos, ypos);
}


void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if(button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        std::cout<< "MBR ";
        switch(action)
        {
            case GLFW_RELEASE:
                  std::cout<< "RELEASE";

                break;
            case GLFW_PRESS:
                std::cout<< "PRESS";

                break;
            case GLFW_REPEAT:
                  std::cout<< "REPEAT";
                break;
        }
        std::cout<<std::endl;
    }
    else if(button == GLFW_MOUSE_BUTTON_LEFT)
    {
        std::cout<<"MBL";
        switch(action)
        {
            case GLFW_RELEASE:
                  std::cout<< "RELEASE";
                break;
            case GLFW_PRESS:
                std::cout<< "PRESS";
                break;
            case GLFW_REPEAT:
                  std::cout<< "REPEAT";
                break;
        }
        std::cout<<std::endl;
    }
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
  	Engine* eng = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
	Camera& engCamera = eng->getCamera();
    engCamera.updateFOV(yoffset);
    engCamera.rebuildPerspective();
    std::cout<< "SCROLL POS " << xoffset << " " << yoffset << std::endl;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, VK_TRUE);
		window = nullptr;
	} else if(key == GLFW_KEY_E && action == GLFW_PRESS) {
		std::cout<< "E PRESSED" <<std::endl;
	} else if(key == GLFW_KEY_E && action ==GLFW_REPEAT) {
		std::cout<< "E REPEAT" <<std::endl;
	}
}

void Engine::init()
{
	mWindow.initWindow(*this);
	mWindow.setWindowSizeCallback(onWindowResized);
	InputManager& inputManager = mWindow.getInputManager();
	inputManager.setCursorPosCallback(cursorPosCallback);
	inputManager.setScrollCallback(scrollCallback);
	inputManager.setMouseButtonCallback(mouseButtonCallback);
	inputManager.setKeyCallback(keyCallback);
	mCamera.setAspect(mWindow.mAspect);
	//glm::perspective(0.0f, 0.0f, 0.0f, 0.0f);
	mCamera.mPrevMouseX = -400.0f;
	mCamera.mPrevMouseY = 200.0f;

	mRenderer.init();
}

#endif


void Engine::handleMovement(double dt)
{
#ifdef __ANDROID__
    InputManager& im = mWindow.getInputManager();
    if (im.movingForward)
        mCamera.moveStraight(im.directionForward, dt);

    if (im.movingSideways)
        mCamera.moveSideways(im.directionSideways, dt);
#else
    InputManager& im = mWindow.getInputManager();
    if (im.keyPressed(GLFW_KEY_W))
        mCamera.moveStraight(1.0f, dt);
    if (im.keyPressed(GLFW_KEY_S))
        mCamera.moveStraight(-1.0f, dt);

    if (im.keyPressed(GLFW_KEY_D))
        mCamera.moveSideways(1.0f, dt);
    if (im.keyPressed(GLFW_KEY_A))
        mCamera.moveSideways(-1.0f, dt);
#endif

}


Window& Engine::getWindow()
{
	return mWindow;
} 

TaskManager& Engine::getTaskManager() 
{
	return mTaskManager;
} 

Renderer& Engine::getRenderer()
{
	return mRenderer;
}

Timer& Engine::getTimer()
{
	return mTimer;
}

Camera& Engine::getCamera()
{
	return mCamera;
}
