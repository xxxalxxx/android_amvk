#!/bin/sh

# Path to Android NDK
export ANDROID_NDK=$HOME/Android/Sdk/ndk-bundle

# Points to the Android SDK
# export ANDROID_SDK=$HOME/Android/Sdk
# export PATH=$PATH:$ANDROID_SDK/tools
# export PATH=$PATH:$ANDROID_SDK/platform-tools
# export PATH=$PATH:$ANDROID_SDK/android-toolchain/bin

# from https://github.com/taka-no-me/android-cmake
export ANDROID_STANDALONE_TOOLCHAIN=$ANDROID_NDK/build/cmake/android.toolchain.cmake

# Add additional args here as appropriate
cmake 
	  -DCMAKE_SYSTEM_NAME=Android \
      -DCMAKE_SYSTEM_VERSION=21 \
	  -DCMAKE_TOOLCHAIN_FILE=$ANDROID_STANDALONE_TOOLCHAIN \
      -DANDROID_NDK=$ANDROID_NDK \
      -DCMAKE_BUILD_TYPE=Debug \
      -DANDROID_ABI="arm64-v8a" \
      -DANDROID_NATIVE_API_LEVEL=android-26 \
	  -DANDROID_TOOLCHAIN=clang \

cmake  --build .
#	  -DCMAKE_INSTALL_PREFIX=install \
	#  -DCMAKE_LIBRARY_OUTPUT_DIRECTORY $HOME/x_compiled \
	# 


#make -j4
