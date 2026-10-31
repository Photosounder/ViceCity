//+ rouz edit (ChatGPT)
#pragma once
enum {
	ASSOC_RUNNING = 1,
	ASSOC_REPEAT = 2,
	ASSOC_DELETEFADEDOUT = 4,
	ASSOC_FADEOUTWHENDONE = 8,
	ASSOC_PARTIAL = 0x10,
	ASSOC_MOVEMENT = 0x20,	// ???
	ASSOC_HAS_TRANSLATION = 0x40,
	ASSOC_HAS_X_TRANSLATION = 0x80,	// for 2d velocity extraction
	ASSOC_WALK = 0x100,	// for CPed::PlayFootSteps(void)
	ASSOC_IDLE = 0x200,	// only xpress scratch has it by default, but game adds it to player's idle animations later
	ASSOC_NOWALK = 0x400,	// see CPed::PlayFootSteps(void)
	ASSOC_BLOCK = 0x800,	// unused in assoc description, blocks other anims from being played
	ASSOC_FRONTAL = 0x1000, // anims that we fall to front
	ASSOC_DRIVING = 0x2000,	// new in VC
};
//- rouz edit (ChatGPT)
