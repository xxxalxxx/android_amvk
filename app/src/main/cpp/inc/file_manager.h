#ifndef AMVK_FILE_MANAGER
#define AMVK_FILE_MANAGER

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <limits.h>
#include <stdio.h>
#include <macro.h>
#include <stdexcept>

#ifdef WINDOWS
#include <direct.h>
#define getCurrentDir _getcwd
#else
#include <unistd.h>
#define getCurrentDir getcwd
#endif

#ifdef __ANDROID__

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <android/asset_manager.h>
#include <assimp/port/AndroidJNI/AndroidJNIIOSystem.h>
#endif


class FileManager {
public:

#ifdef __ANDROID__
    static ANativeActivity* activity;
	static AAssetManager* assetManager;
	static const char* internalStoragePath;
    static Assimp::AndroidJNIIOSystem* newAssimpIOSystem();
#endif


	static FileManager& getInstance();
	static std::string getFilePath(const std::string& filename);
	static std::string getFilePath(const char* filename);
	static std::string getResourcePath(std::string&& path);
	static std::string stripPath(const std::string&& fullPath);
	static std::string getModelsPath(std::string&& path); 
	static std::string getCachePath(std::string&& path); 


	static void readCache(std::vector<char>& out, const std::string& cacheName);
	static std::vector<char> readFile(const std::string& filename); 
	static std::vector<char> readShader(const std::string& shaderName);
	static void writeCache(const char* cacheName, void* data, size_t size);

	std::string getBinPath();

	FileManager(const FileManager& fileManager) = delete;
	void operator=(const FileManager& fileManager) = delete;
private:
	FileManager();
	void initBinPath();

	std::string mBinPath, mEngineRoot, mShaderDir, mResourceDir, mModelsDir, mCacheDir;
#if defined(__ANDROID__)
	static constexpr const char* ENGINE_RELATIVE_ROOT = "";
	static constexpr const char* SHADER_DIR = "shaders/";
	static constexpr const char* RESOURCE_DIR = "";
	static constexpr const char* MODELS_DIR = "model/";
	static constexpr const char* CACHE_DIR = "cache/";
#else
	static constexpr const char* ENGINE_RELATIVE_ROOT = "/../";
	static constexpr const char* SHADER_DIR = "shader/spv/";
	static constexpr const char* RESOURCE_DIR = "res/";
	static constexpr const char* MODELS_DIR = "res/model/";
	static constexpr const char* CACHE_DIR = "cache/";
#endif

};

#endif
