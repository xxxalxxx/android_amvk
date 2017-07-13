#include "file_manager.h"


#ifdef __ANDROID__
ANativeActivity* FileManager::activity = nullptr;
AAssetManager* FileManager::assetManager = nullptr;
const char* FileManager::internalStoragePath = nullptr;
Assimp::AndroidJNIIOSystem* FileManager::newAssimpIOSystem() {
    return new Assimp::AndroidJNIIOSystem(FileManager::activity);
}


#endif

FileManager::FileManager() 
{
    initBinPath();
    mEngineRoot = mBinPath + ENGINE_RELATIVE_ROOT;
    mResourceDir = mEngineRoot + RESOURCE_DIR;
    mShaderDir = mEngineRoot + SHADER_DIR;
    mModelsDir = mEngineRoot + MODELS_DIR;
    mCacheDir = mEngineRoot + CACHE_DIR;
}

FileManager& FileManager::getInstance()
{
	static FileManager fileManager;
	return fileManager;
}

std::string FileManager::stripPath(const std::string&& path)
{
	std::string s = path.substr(path.find_last_of("/\\") + 1);
	return s;
}

std::string FileManager::getFilePath(const char* filename) 
{
	std::string s(filename);
	return FileManager::getFilePath(s);
}

std::string FileManager::getFilePath(const std::string& filename)
{
	return filename.substr(0, filename.find_last_of("\\/"));
}

std::string FileManager::getResourcePath(std::string&& path) 
{
	return FileManager::getInstance().mResourceDir + path;
}

std::string FileManager::getModelsPath(std::string&& path) 
{
	return FileManager::getInstance().mModelsDir + path;
}

std::string FileManager::getCachePath(std::string&& path) 
{
	return FileManager::getInstance().mCacheDir + path;
}

std::vector<char> FileManager::readShader(const std::string& shaderName)
{
	std::string filename = FileManager::getInstance().mShaderDir + shaderName + ".spv";
	LOG("SHADER FILE NAME: %s", filename.c_str());
	return readFile(filename);
}


std::vector<char> FileManager::readFile(const std::string& filename) 
{
#ifdef __ANDROID__
	if (!assetManager)
		throw std::runtime_error("Android AAssetManager pointer is not set");
	AAsset* asset = AAssetManager_open(assetManager, filename.c_str(), AASSET_MODE_STREAMING);
	if (!asset)
		throw std::runtime_error("Unable to load asset!");
	size_t shaderSize = AAsset_getLength(asset);
	if (shaderSize == 0)
		throw std::runtime_error("File is empty!");

	std::vector<char> buffer(shaderSize);
	AAsset_read(asset, buffer.data(), shaderSize);
	AAsset_close(asset);
	return buffer;
#else
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
		throw std::runtime_error("failed to open file!");

	size_t fileSize = (size_t) file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
#endif
}

void FileManager::readCache(std::vector<char>& out, const std::string& cacheName)
{
#ifdef __ANDROID__
	if (!assetManager)
		throw std::runtime_error("Android AAssetManager pointer is not set");
	if (!internalStoragePath)
		throw std::runtime_error("Android internalStoragePath is not set");
	std::string filepath = internalStoragePath;
    filepath += "/";
    filepath +=  FileManager::getInstance().mCacheDir;
	LOG("CACHE PATH: %s", filepath.c_str());

	std::string filename = filepath + cacheName;
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		LOG("CACHE CANNOT BE OPENED");
		out.clear();
		return;
	}


	size_t fileSize = (size_t) file.tellg();
	out.resize(fileSize);

	file.seekg(0);
	file.read(out.data(), fileSize);
	file.close();

	LOG("RETURNING CACHED: %zu", fileSize);
#else
	std::string filename = FileManager::getInstance().mCacheDir + cacheName;
	LOG("CACHE FILE NAME: %s", filename);
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		LOG("CACHE CANNOT BE OPENED");
		out.clear();
		return;
	}


	size_t fileSize = (size_t) file.tellg();
	out.resize(fileSize);

	file.seekg(0);
	file.read(out.data(), fileSize);
	file.close();
	LOG("RETURNING CACHED: %zu", fileSize);
#endif
}


void FileManager::writeCache(const char* cacheName, void* data, size_t size)
{
#ifdef __ANDROID__
	if (!assetManager)
		throw std::runtime_error("Android assetManager is not set");
	if (!internalStoragePath)
		throw std::runtime_error("Android internalStoragePath is not set");
	else
		LOG("INTERNAL STORAGE PATH %s", internalStoragePath);
	std::string filepath = internalStoragePath;// + "/" + cacheName;
	filepath += "/";
	filepath += FileManager::getInstance().mCacheDir;

	struct stat sb;
	int32_t res = stat(filepath.c_str(), &sb);
	if (0 == res && sb.st_mode & S_IFDIR) {
		LOG("'files/' dir already in app's internal data storage.");
	} else if (ENOENT == errno) {
		res = mkdir(filepath.c_str(), 0770);
	}

	if (res == 0) {
		std::string filename = filepath + cacheName;
		LOG("INTERNAL STORAGE FILENAME: %s", filename.c_str());
		//AAsset* configFileAsset = AAssetManager_open(assetManager, filename.c_str(), AASSET_MODE_BUFFER);
		std::ofstream binFile(filename, std::ios::out | std::ios::binary);
		if (!binFile.is_open()) {
            LOG("UNABLE TO OPEN FILENAME %s", filename.c_str());
            throw std::runtime_error("failed to open cache!");
        }
		binFile.write((char*) &data, size);
		binFile.close();
		//AAsset_close(configFileAsset);
		LOG("CACHE WRITTEN");
	}
#else
	std::string filename = FileManager::getInstance().mCacheDir + cacheName;
	LOG("CACHE FILE NAME: %s", filename);

	std::ofstream binFile(filename, std::ios::out | std::ios::binary);
    if (!binFile.is_open())
    	throw std::runtime_error("failed to open cache!");

	binFile.write((char*) &data, size);
    binFile.close();
	LOG("CACHE WRITTEN");
#endif
}


void FileManager::initBinPath()
{
#if defined(__ANDROID__)
	mBinPath = std::string();
	mBinPath = getFilePath(mBinPath);
	return;
#endif

#ifdef WINDOWS
		char buff[MAX_PATH];
#else
		char buff[PATH_MAX];
#endif
    size_t sz = sizeof(buff)-1;
    LOG("IN BIN ABS PATH");
#ifdef WINDOWS
        if (!GetModuleFileName(NULL, buff, sz)) 
			throw std::runtime_error("Could not read read binary file directory");
#else
        ssize_t len;
        if ((len = readlink("/proc/self/exe", buff, sz)) == -1) 
			throw std::runtime_error("Could not read read binary file directory");    
        buff[len] = '\0';
#endif
    LOG("B ABS: %s", std::string(buff).c_str());
    mBinPath = std::string(buff);
	mBinPath = getFilePath(mBinPath);
}

std::string FileManager::getBinPath() 
{
	return mBinPath;
}
