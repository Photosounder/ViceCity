//+ rouz edit (Claude)
#include "HudMessages.h"

void Hud_CopyMessage(uint16_t *dst, const uint16_t *src, int32_t cap)
{
	int32_t i;

	for (i = 0; i < cap; i++) {
		if (src[i] == 0)
			break;

		dst[i] = src[i];
	}
	dst[i] = 0;
}

bool Hud_StoreBigMessage(uint16_t *big, uint16_t *last, const uint16_t *message, bool trackChanges)
{
	int32_t i;
	bool changed = false;

	for (i = 0; i < 128; i++) {
		if (message[i] == 0)
			break;

		if (trackChanges) {
			// A different text than before restarts the odd job message
			if (message[i] != last[i])
				changed = true;

			last[i] = message[i];
		}
		big[i] = message[i];
	}
	last[i] = 0;
	big[i] = 0;

	return changed;
}
//- rouz edit (Claude)
