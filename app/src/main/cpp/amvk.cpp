#include <jni.h>
#include <string>
#include <android/looper.h>
#include <android/log.h>
#include "NDKHelper.h"
#include "JNIHelper.h"
#include "vulkan_wrapper.h"
#include <assimp/port/AndroidJNI/AndroidJNIIOSystem.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp-master/include/assimp/scene.h>

extern "C"
JNIEXPORT jstring JNICALL
Java_vulkan_melnichuk_al_amvk_MainNativeActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}




void android_main(android_app* state) {

    if (!InitVulkan()) {
        LOGE("Unable to load Vulkan");
        return;
    }

    LOGI("Vulkan has been successfully initialized");

#ifdef USE_NDK_PROFILER
    //monstartup("libTeapotNativeActivity.so");
#endif

    Assimp::Importer *importer = new Assimp::Importer();
    Assimp::AndroidJNIIOSystem *ioSystem = new Assimp::AndroidJNIIOSystem(state->activity);
    importer->SetIOHandler(ioSystem);

    const aiScene *scene = importer->ReadFile("nanosuit/nanosuit.obj",
                                              aiProcess_FlipWindingOrder | aiProcess_Triangulate |
                                              aiProcess_PreTransformVertices);
    if (!scene) {
        LOGI("%s", importer->GetErrorString());
    } else {
        LOGI("NO ERROR, num Meshes: %u, num vertices: %u", scene->mNumMeshes, scene->mMeshes[0]->mNumVertices);

    }


    // loop waiting for stuff to do.
    while (1) {
        // Read all pending events.

        int events;
        android_poll_source* source;

        // If not animating, we will block forever waiting for events.
        // If animating, we loop until all events are read, then continue
        // to draw the next frame of animation.
        while (ALooper_pollAll(0/*engine.isReady() ? 0 : -1 */, NULL, &events, (void**)&source) >= 0) {
            // Process this event.
            if (source != NULL)
                source->process(state, source);

            LOGI("EVENT");
            // Check if we are exiting.
            if (state->destroyRequested != 0) {

                LOGI("DESTOROY");
                return;
            }
        }

        LOGI("LOOP");

    }


    LOGI("EXIT");
}
