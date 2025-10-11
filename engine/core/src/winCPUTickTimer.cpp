#include "winCPUTickTimer.hpp"

#include "WinInclude.hpp"

#define GENERATE_SPECIALIZED_TEMPLATE_DEFINITIONS(type)     \
	template winCpuTickTimer<type>::winCpuTickTimer();      \
	template winCpuTickTimer<type>::~winCpuTickTimer();     \
	template void winCpuTickTimer<type>::StartClock();      \
	template void winCpuTickTimer<type>::ResetStartTime();  \
	template void winCpuTickTimer<type>::UpdateDeltaTime(); \
	template type winCpuTickTimer<type>::GetDeltaTime();   \
	template type winCpuTickTimer<type>::GetTimeSinceStart()

namespace misc
{
template <typename T>
struct winCpuTickTimer<T>::Data
{
	LARGE_INTEGER m_frequency;

	LARGE_INTEGER m_starttime;
	LARGE_INTEGER m_currentTime;
	LARGE_INTEGER m_endtime;

	T m_deltaTime;

	double m_oneticktime;
};

template <typename T>
winCpuTickTimer<T>::winCpuTickTimer() : pData(new Data)
{
	// set frequency
	QueryPerformanceFrequency(&pData->m_frequency);
	pData->m_oneticktime = 1.0 / pData->m_frequency.QuadPart;
}

template <typename T>
winCpuTickTimer<T>::~winCpuTickTimer()
{
	delete pData;
}

template <typename T>
void winCpuTickTimer<T>::StartClock()
{
	pData->m_deltaTime = T(0);

	pData->m_currentTime.QuadPart = 0;
	pData->m_starttime.QuadPart = 0;
	pData->m_endtime.QuadPart = 0;

	QueryPerformanceCounter(&pData->m_starttime);
	pData->m_currentTime = pData->m_starttime;
}

template <typename T>
void winCpuTickTimer<T>::ResetStartTime()
{
	QueryPerformanceCounter(&pData->m_starttime);
}

template <typename T>
T winCpuTickTimer<T>::GetTimeSinceStart()
{
	QueryPerformanceCounter(&pData->m_endtime);
	double difference = (double)(pData->m_endtime.QuadPart - pData->m_starttime.QuadPart);
	double timeinseconds = difference * pData->m_oneticktime;
	return static_cast<T>(timeinseconds);
}

template <typename T>
T winCpuTickTimer<T>::GetDeltaTime()
{
	return pData->m_deltaTime;
}

template <typename T>
void winCpuTickTimer<T>::UpdateDeltaTime()
{
	LARGE_INTEGER aux = pData->m_currentTime;
	QueryPerformanceCounter(&pData->m_currentTime);
	double difference = (double)(pData->m_currentTime.QuadPart - aux.QuadPart);
	pData->m_deltaTime = static_cast<T>(difference * pData->m_oneticktime);
}

GENERATE_SPECIALIZED_TEMPLATE_DEFINITIONS(float);
GENERATE_SPECIALIZED_TEMPLATE_DEFINITIONS(double);

}  // namespace misc