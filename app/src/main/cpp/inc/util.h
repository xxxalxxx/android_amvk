#ifndef AMVK_UTIL_H
#define AMVK_UTIL_H

#include <vector>
#include <limits>
#include <initializer_list>
#include <stdio.h>
#include <stdlib.h>

static constexpr const int MAX_SIZE_NOT_FOUND = std::numeric_limits<int>::lowest();
static constexpr const int MIN_SIZE_NOT_FOUND = std::numeric_limits<int>::max();

template <class T>
int maxVectorSize(std::initializer_list<std::vector<T>& > vectors)
{
	int max = MAX_SIZE_NOT_FOUND;
	for (const auto& elem : vectors)
		if (elem.size() > max)
			max = elem.size();
	return max;
}

template <class T>
int minVectorSize(std::initializer_list<std::vector<T>& > vectors)
{
	int min = MIN_SIZE_NOT_FOUND;
	for (const auto& elem : vectors)
		if (elem.size() < min)
			min = elem.size();
	return min;
}
/*
void* alignedAlloc(size_t size, size_t alignment)
{
	void *data = nullptr;
#if defined(_MSC_VER) || defined(__MINGW32__)
	data = _aligned_malloc(size, alignment);
#else 
	int res = posix_memalign(&data, alignment, size);
	if (res != 0)
		data = nullptr;
#endif
	return data;
}

void alignedFree(void* data)
{
#if	defined(_MSC_VER) || defined(__MINGW32__)
	_aligned_free(data);
#else 
	free(data);
#endif
}
*/

#endif
