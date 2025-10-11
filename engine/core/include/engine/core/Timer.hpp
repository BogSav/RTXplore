#pragma once

#include <chrono>

template <typename T>
class ChronoBasedTimer
{
public:
	ChronoBasedTimer() : lastMark(std::chrono::steady_clock::now()) {}

	// Seteaza timpul curent si returneaza cat timp a trecut de la ultimul Mark
	T Mark()
	{
		const auto old = lastMark;
		lastMark = std::chrono::steady_clock::now();
		const std::chrono::duration<T> frameTime = lastMark - old;
		return frameTime.count();
	}

	// Returneaza timpul trecut de la ultimul Mark
	T Peek() const { return std::chrono::duration<T>(std::chrono::steady_clock::now() - lastMark).count(); }

private:
	std::chrono::steady_clock::time_point lastMark;
};

typedef ChronoBasedTimer<float> Timer;
