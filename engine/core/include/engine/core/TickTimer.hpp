// (C) 2003 ModuleWorks GmbH
// Owner: ARB Library Structure

#pragma once

namespace engine::core
{

// Ultra precision timer
// Se foloseste de frecventa la cpu si tick-uri
template <typename T>
class TickTimer
{
public:
	TickTimer();
	~TickTimer();

	T GetTimeSinceStart();
	T GetDeltaTime();

	void StartClock();
	void ResetStartTime();
	void UpdateDeltaTime();

private:
	struct Data;
	Data* pData;
};
}  // namespace engine::core