#ifndef AMVK_TIMER_H
#define AMVK_TIMER_H

#include <chrono>
#include "macro.h"

class Timer {
public:
	Timer();
	double tick();
	double total() const;
	double dt() const;
	uint32_t FPS() const;
private:
	double mDt;
	double mFrameTime;
	double mTotalTime;

	std::chrono::high_resolution_clock::time_point mPrevTime;
	std::chrono::high_resolution_clock::time_point mStartTime;
	uint32_t mNumFrames;
	uint32_t mFPS;
};

#endif
