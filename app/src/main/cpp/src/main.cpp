


/* #include <assimp/port/AndroidJNI/AndroidJNIIOSystem.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
 */
#include "macro.h"
#ifdef __ANDROID__
#include "vulkan_wrapper.h"
#include <jni.h>
#include <android/looper.h>

#include "NDKHelper.h"
#include "JNIHelper.h"

#include <iostream>
#include <string>
#include "task_manager.h"
#include "engine.h"



Engine engine;

void android_main(android_app* state) {

#ifdef USE_NDK_PROFILER
    //monstartup("libTeapotNativeActivity.so");
#endif

    bool initialized = false;

    engine.state = state;
   // engine.init(state);

    state->userData = &engine;
    state->onAppCmd = Engine::handleCmd;
    state->onInputEvent = Engine::handleInput;

    Window& window = engine.getWindow();
    InputManager& inputManager = window.getInputManager();
    Renderer& renderer = engine.getRenderer();
    Timer& timer = engine.getTimer();
    Camera& camera = engine.getCamera();

    //renderer.buildCommandBuffers(timer, camera);

    while (1) {
        // Read all pending events.
        int events;
        android_poll_source* source;

        // If not animating, we will block forever waiting for events.
        // If animating, we loop until all events are read, then continue
        // to draw the next frame of animation.
        while (ALooper_pollAll(0/*engine.isReady ? 0 : -1*/, NULL, &events, (void**)&source) >= 0) {
            // Process this event.
            if (source != NULL)
                source->process(state, source);
            // Check if we are exiting.
            if (state->destroyRequested != 0) {
                LOG("DESTOROY");
                if (engine.isReady && initialized) {
                    renderer.waitIdle();
                }
                return;
            }
        }


        if (engine.isReady) {
            if (!initialized) {
                if (!InitVulkan())
                    throw std::runtime_error("Unable to initialize Vulkan");
                engine.init(state);
                initialized = true;
            }

            double dt = timer.tick();
            engine.handleMovement(dt);
            renderer.updateUniformBuffers(timer, camera);
            renderer.draw();
        }
    }

}



extern "C" {

JNIEXPORT void JNICALL
Java_vulkan_melnichuk_al_amvk_MainNativeActivity_setSidewaysMovement(JNIEnv *env, jobject instance,
                                                                     jboolean moving,
                                                                     jfloat direction)
{
    InputManager& im = engine.getWindow().getInputManager();
    im.movingSideways = moving;
    im.directionSideways = direction;
}

JNIEXPORT void JNICALL
Java_vulkan_melnichuk_al_amvk_MainNativeActivity_setForwardMovement(JNIEnv *env, jobject instance,
                                                                    jboolean moving,
                                                                    jfloat direction)
{
    InputManager& im = engine.getWindow().getInputManager();
    im.movingForward = moving;
    im.directionForward = direction;
}

}

#else

#include <iostream>
#include <string>
#include "macro.h"
#include "task_manager.h"
#include "engine.h"

#include <GLFW/glfw3.h>

int main() {

    Engine engine;
    engine.init();

    Window& window = engine.getWindow();
    InputManager& inputManager = window.getInputManager();
    Renderer& renderer = engine.getRenderer();
    Timer& timer = engine.getTimer();
    Camera& camera = engine.getCamera();
	renderer.buildComputeBuffers(timer, camera);
	renderer.buildGBuffers(timer, camera);
    renderer.buildCommandBuffers(timer, camera);

    while (window.isOpen()) {
        inputManager.pollEvents();
        double dt = timer.tick();
        engine.handleMovement(dt);
        renderer.updateUniformBuffers(timer, camera);
	//	renderer.buildCommandBuffers(timer, camera);
        renderer.draw();
    }
    renderer.waitIdle();
    return 0;
}

#endif






