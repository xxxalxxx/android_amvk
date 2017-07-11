#include "macro.h"

#ifndef AMVK_TASK_MANAGER_H
#define AMVK_TASK_MANAGER_H

#include <thread>
#include <queue>
#include <vector>
#include <mutex>
#include <functional>
#include <atomic>
#include <condition_variable>
#include <chrono>
class Task {
public:
	virtual ~Task() {}
	virtual void execute() = 0;
};

class MessageTask : public Task {
public:
	int mId;

	MessageTask(int id): mId(id) {}

	void execute() {
		std::thread::id pid = std::this_thread::get_id();
		std::cout << "task #" << mId << " pid:" << pid << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
};

class TaskManager {
public:
	TaskManager();
	virtual ~TaskManager();
	void submit(Task* task);
	void submit(Task& task);
private:
	size_t mNumThreads;
	std::atomic_bool mContinue;
	std::condition_variable mCondition;
	std::vector<std::thread> mPool;
	std::queue<Task*> mTasks;
	std::mutex mTasksMutex;
};

#endif
