#pragma once
#include "common.h"
#include "config.h"
#include "CarGenData.h"

enum {
	CARGEN_MAXACTUALLIMIT = 100
};

class CCarGenerator : public CCarGeneratorData
{
public:
	void SwitchOff();
	void SwitchOn();
	uint32 CalcNextGen();
	void DoInternalProcessing();
	void Process();
	void Setup(float x, float y, float z, float angle, int32 mi, int16 color1, int16 color2, uint8 force, uint8 alarm, uint8 lock, uint16 min_delay, uint16 max_delay);
	bool CheckForBlockage(int32 mi);
	bool CheckIfWithinRangeOfAnyPlayers();
	void SetUsesRemaining(uint16 uses) { m_nUsesRemaining = uses; }
};

//+ rouz edit (Claude)
// Formerly the static class CTheCarGenerators, now C globals and functions shared with C
#ifdef __cplusplus
extern "C" {
#endif
extern uint8 TheCarGenerators_ProcessCounter;
extern uint32 TheCarGenerators_NumOfCarGenerators;
extern uint8 TheCarGenerators_GenerateEvenIfPlayerIsCloseCounter;
extern uint32 TheCarGenerators_CurrentActiveCount;
void TheCarGenerators_Process();
int32 TheCarGenerators_CreateCarGenerator(float x, float y, float z, float angle, int32 mi, int16 color1, int16 color2, uint8 force, uint8 alarm, uint8 lock, uint16 min_delay, uint16 max_delay);
void TheCarGenerators_Init();
void TheCarGenerators_SaveAllCarGenerators(uint8 *, uint32 *);
void TheCarGenerators_LoadAllCarGenerators(uint8 *, uint32);
#ifdef __cplusplus
}
// Members that C cannot declare yet (references, default arguments, overloads, C++ class values), with C++ linkage
extern CCarGenerator TheCarGenerators_CarGeneratorArray[NUM_CARGENS];
#endif
//- rouz edit (Claude)
