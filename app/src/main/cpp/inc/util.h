#ifndef AMVK_UTIL_H
#define AMVK_UTIL_H

#include <vector>
#include <limits>
#include <initializer_list>

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

#endif
