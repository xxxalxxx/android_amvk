#include "timer.h"

Timer::Timer()
{
	mStartTime = std::chrono::high_resolution_clock::now();
}

double Timer::tick()
{
	auto now = std::chrono::high_resolution_clock::now();
	mDt = std::chrono::duration<double>(now - mPrevTime).count();
	mTotalTime = std::chrono::duration<double>(now - mStartTime).count();
	mPrevTime = now;
	mFrameTime += mDt;
	++mNumFrames;

	if (mFrameTime > 1.0) {
		mFPS = mNumFrames;
		mFrameTime = 0.0;
		mNumFrames = 0;
	}
	//LOG("dt:" << mDt << " total:" << mTotalTime << " fps:" << mFPS << " frame time:" << mFrameTime);
	return mDt;
}


double Timer::dt() const
{
	return mDt;
}

uint32_t Timer::FPS() const
{
	return mFPS;
}

double Timer::total() const 
{
	return mTotalTime;
}

