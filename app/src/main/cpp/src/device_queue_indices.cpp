#include "device_queue_indices.h"

DeviceQueueIndicies::DeviceQueueIndicies(): 
	mGraphicsIndex(INDEX_UNSET),
	mSupportedIndex(INDEX_UNSET),
	mComputeIndex(INDEX_UNSET),
	mTransferIndex(INDEX_UNSET),
	mNumIndicesFound(0)
{
	mDeviceQueueIndiciesArr[INDEX_GRAPHICS] = &mGraphicsIndex;
	mDeviceQueueIndiciesArr[INDEX_SUPPORTED] = &mSupportedIndex;
	mDeviceQueueIndiciesArr[INDEX_COMPUTE] = &mComputeIndex;
	mDeviceQueueIndiciesArr[INDEX_TRANSFER] = &mTransferIndex;
}

DeviceQueueIndicies::~DeviceQueueIndicies()
{

}

int& DeviceQueueIndicies::operator[](int i) 
{
	return *mDeviceQueueIndiciesArr[i];
}

int DeviceQueueIndicies::operator[](int i) const 
{
	return *mDeviceQueueIndiciesArr[i];
}

void DeviceQueueIndicies::unset() 
{
	mGraphicsIndex = mSupportedIndex = mComputeIndex = mTransferIndex = INDEX_UNSET;
}

bool DeviceQueueIndicies::allIndicesSet() 
{
	LOG("ALL INDICES SET");
	return mNumIndicesFound == NUM_DEVICE_QUEUE_INDICES;
}

bool DeviceQueueIndicies::graphicsIndexSet() 
{
	return mGraphicsIndex != INDEX_UNSET;
}

bool DeviceQueueIndicies::supportedIndexSet() 
{
	return mSupportedIndex != INDEX_UNSET;
}

bool DeviceQueueIndicies::computeIndexSet() 
{
	return mComputeIndex != INDEX_UNSET;
}

bool DeviceQueueIndicies::transferIndexSet() 
{
	return mTransferIndex != INDEX_UNSET;
}

int DeviceQueueIndicies::getGraphicsQueueIndex()
{
	return mGraphicsIndex;
}

int DeviceQueueIndicies::getSupportedQueueIndex()
{
	return mSupportedIndex;
}

int DeviceQueueIndicies::getComputeQueueIndex()
{
	return mComputeIndex;
}

int DeviceQueueIndicies::getTransfersQueueIndex()
{
	return mTransferIndex;
}

void DeviceQueueIndicies::setGraphicsIndex(int index) 
{
	LOG("GRAPHICS INDEX SET TO %d ", index);
	setIndex(mGraphicsIndex, index);
}

void DeviceQueueIndicies::setSupportedIndex(int index)
{
	LOG("SUPPORTED IDNEX SET TO %d", index);
	setIndex(mSupportedIndex, index);
}

void DeviceQueueIndicies::setComputeIndex(int index) 
{
	setIndex(mComputeIndex, index);
}

void DeviceQueueIndicies::setTransferIndex(int index) 
{
	setIndex(mTransferIndex, index);
}

void DeviceQueueIndicies::setIndex(int& currIndex, int index)
{
	LOG("INDEX %d UPDATED", index);
	if (index >= INDEX_UNSET && currIndex != index) {
		if (currIndex == INDEX_UNSET) 
			++mNumIndicesFound;
		 else if (index == INDEX_UNSET)
			 --mNumIndicesFound;
		currIndex = index;
	}
}

