# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)


# build native_app_glue as a static lib
add_library(app-glue STATIC
             ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

include_directories(lib src inc)



# add all files from sources dir
file(GLOB SOURCES src/*.cpp)

# build the ndk-helper library
set(ndk_helper_dir lib/ndk_helper)
add_subdirectory(${ndk_helper_dir} ndk_helper)

# build vulkan_wrapper
# add_subdirectory(lib/vulkan_wrapper vulkan_wrapper)

# build assimp
# set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")
set(CMAKE_TOOLCHAIN_PATH ${ANDROID_NDK}/build/cmake)
add_definitions(-DASSIMP_ANDROID_JNIIOSYSTEM=ON)
add_definitions(-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_PATH}/android.toolchain.cmake)
# link_directories(prebuilt/assimp)
# link_directories(prebuilt/assimp_io)
# add_subdirectory(lib/assimp-master assimp)

# add_subdirectory(lib/assimp/port/AndroidJNI android_jniiosystem)

set(distribution_DIR ${CMAKE_SOURCE_DIR}/../../../libs)
add_library(assimp SHARED IMPORTED)
set_target_properties(assimp PROPERTIES IMPORTED_LOCATION ${distribution_DIR}/assimp/${ANDROID_ABI}/libassimp.so)

add_library(android_jniiosystem STATIC IMPORTED)
set_target_properties(android_jniiosystem PROPERTIES IMPORTED_LOCATION ${distribution_DIR}/assimp/${ANDROID_ABI}/libandroid_jniiosystem.a)

# add_library(vulkan SHARED IMPORTED)
# set_target_properties(vulkan PROPERTIES IMPORTED_LOCATION ${distribution_DIR}/vulkan/${ANDROID_ABI}/libvulkan.so)


# Export ANativeActivity_onCreate(), 
# Refer to: https://github.com/android-ndk/ndk/issues/381.
set(CMAKE_SHARED_LINKER_FLAGS
    "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11  -Werror -D VK_USE_PLATFORM_ANDROID_KHR")
# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library(amvk SHARED
            ${SOURCES}
             src/main.cpp
             lib/vulkan_wrapper/vulkan_wrapper.cpp)

 target_include_directories(amvk PRIVATE
                            ${ANDROID_NDK}/sources/android/native_app_glue
                            ${ndk_helper_dir}
                            lib/vulkan_wrapper)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )


# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

# assimp android_jniiosystem
target_link_libraries(
                       amvk
                       android
                       ndk_helper
                       app-glue
                       atomic
                       EGL
                       GLESv2
                       assimp
                       android_jniiosystem
                       ${log-lib} )