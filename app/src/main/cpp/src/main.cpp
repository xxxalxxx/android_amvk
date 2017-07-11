






/* #include <assimp/port/AndroidJNI/AndroidJNIIOSystem.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
 */
#include "macro.h"
#if defined(__ANDROID__)

#include "vulkan_wrapper.h"
#include <jni.h>
#include <android/looper.h>

#include "NDKHelper.h"
#include "JNIHelper.h"

#include <iostream>
#include <string>
#include "task_manager.h"
#include "engine.h"

extern "C"
JNIEXPORT jstring JNICALL
Java_vulkan_melnichuk_al_amvk_MainNativeActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

//1200 line in base - android motion



void android_main(android_app* state) {

#ifdef USE_NDK_PROFILER
    //monstartup("libTeapotNativeActivity.so");
#endif

    /*Assimp::Importer *importer = new Assimp::Importer();
    Assimp::AndroidJNIIOSystem *ioSystem = new Assimp::AndroidJNIIOSystem(state->activity);
    importer->SetIOHandler(ioSystem);
    const aiScene *scene = importer->ReadFile("nanosuit/nanosuit.obj", aiProcess_FlipWindingOrder | aiProcess_Triangulate | aiProcess_PreTransformVertices);

    if (!scene) {
        LOGI("%s", importer->GetErrorString());
    } else {
        LOGI("NO ERROR, num Meshes: %u, num vertices: %u", scene->mNumMeshes, scene->mMeshes[0]->mNumVertices);

    }*/
    bool initialized = false;
    Engine engine;
    engine.state = state;
   // engine.init(state);

    state->userData = &engine;
    state->onAppCmd = Engine::handleCmd;
    state->onInputEvent = Engine::handleInput;

    Window& window = engine.getWindow();
    InputManager& inputManager = window.getInputManager();
    VulkanManager& vulkanManager = engine.getVulkanManager();
    Timer& timer = engine.getTimer();
    Camera& camera = engine.getCamera();

    //vulkanManager.buildCommandBuffers(timer, camera);

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
                    vulkanManager.waitIdle();
                }
                return;
            }
            LOG("EVENT");
        }


        if (engine.isReady) {
            if (!initialized) {
                if (!InitVulkan())
                    throw std::runtime_error("Unable to initialize Vulkan");
                engine.init(state);
                initialized = true;
            }

            double dt = timer.tick();
            //engine.handleMovement(dt);
            vulkanManager.updateUniformBuffers(timer, camera);
            vulkanManager.draw();
        }

        LOG("LOOP");
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
    VulkanManager& vulkanManager = engine.getVulkanManager();
    Timer& timer = engine.getTimer();
    Camera& camera = engine.getCamera();

    vulkanManager.updateCommandBuffers(timer, camera);

    while (window.isOpen()) {
        inputManager.pollEvents();
        double dt = timer.tick();
        engine.handleMovement(dt);
        vulkanManager.updateUniformBuffers(timer, camera);
        vulkanManager.draw();
    }
    vulkanManager.waitIdle();
    return 0;
}

#endif






