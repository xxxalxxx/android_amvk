#ifndef AMVK_DEVICE_QUEUE_INDICES
#define AMVK_DEVICE_QUEUE_INDICES

#include "macro.h"

#define NUM_DEVICE_QUEUE_INDICES 4

class DeviceQueueIndicies {
public:
	DeviceQueueIndicies();
	~DeviceQueueIndicies();
	int& operator[](int i);
	int operator[](int i) const;
	void unset();
	bool allIndicesSet();
	bool graphicsIndexSet();
	bool supportedIndexSet();
	bool computeIndexSet();
	bool transferIndexSet();

	int getGraphicsQueueIndex();
	int getSupportedQueueIndex();
	int getComputeQueueIndex();
	int getTransfersQueueIndex();
	
	void setGraphicsIndex(int index);
	void setSupportedIndex(int index);
	void setComputeIndex(int index);
	void setTransferIndex(int index);

	static constexpr int  
		INDEX_GRAPHICS = 0,
		INDEX_SUPPORTED = 1,
		INDEX_COMPUTE = 2,
		INDEX_TRANSFER = 3,
		INDEX_UNSET = -1;

private:
	void setIndex(int& currIndex, int index);
	int mGraphicsIndex, mSupportedIndex, mComputeIndex, mTransferIndex;
	int mNumIndicesFound; 
	int *mDeviceQueueIndiciesArr[NUM_DEVICE_QUEUE_INDICES];
};

#endif
