#include "task_manager.h"
#include <iostream>

TaskManager::TaskManager(): mNumThreads(std::thread::hardware_concurrency())
{
	for (size_t i = 0; i < mNumThreads; ++i) {
		auto f = [this] () -> void {
			while (mContinue) {
				Task* task = nullptr;
				
				{
					std::unique_lock<std::mutex> lock(mTasksMutex);
					mCondition.wait(lock, [this] () -> bool { 
						return !mContinue || !mTasks.empty(); 
					});
					
					if (mContinue && !mTasks.empty()) {
						task = mTasks.front();
						mTasks.pop();
					}
				}

				if (task) {
					task->execute();
					delete task;
				}
			}
		};

		mPool.push_back(std::thread(f));
	}
}

TaskManager::~TaskManager() 
{
	mContinue = false;
	mCondition.notify_all();
	while (!mTasks.empty()) {
		Task* t = mTasks.front();
		if (t) {
			mTasks.pop();
			delete t;
		}
	}
	
	for (auto& t : mPool)
		t.join();

	std::cout<< "stop" << std::endl;
}

void TaskManager::submit(Task* task) {
	if (task) 
		submit(*task);
}

void TaskManager::submit(Task& task) {
	{
		std::unique_lock<std::mutex> lock(mTasksMutex);
		mTasks.push(&task);
	}
	
	mCondition.notify_one();
}
