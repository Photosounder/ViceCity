//+ rouz edit (ChatGPT)
#include "AudioPedComments.h"
#include <string.h>

void
PedComments_Init(cPedComments *comments)
{
	// Initialize only the fields written by the former constructor
	for (int i = 0; i < NUM_PED_COMMENTS_SLOTS; i++)
		for (int j = 0; j < NUM_SOUND_QUEUES; j++) {
			comments->m_aPedCommentQueue[j][i].m_nLoadingTimeout = -1;
			comments->m_aPedCommentOrderList[j][i] = NUM_PED_COMMENTS_SLOTS;
		}

	for (int i = 0; i < NUM_SOUND_QUEUES; i++)
		comments->m_nPedCommentCount[i] = 0;
	comments->m_nActiveQueue = 0;
}

void
PedComments_Add(cPedComments *comments, const tPedComment *com)
{
	// Select a free slot or replace the original lowest-priority slot
	uint8_t index;

	// Preserve the original volume priority check

	if (comments->m_nPedCommentCount[comments->m_nActiveQueue] >= NUM_PED_COMMENTS_SLOTS) {
		index = comments->m_aPedCommentOrderList[comments->m_nActiveQueue][NUM_PED_COMMENTS_SLOTS - 1];
		if (comments->m_aPedCommentQueue[comments->m_nActiveQueue][index].m_nVolume > com->m_nVolume)
			return;
	} else
		index = comments->m_nPedCommentCount[comments->m_nActiveQueue]++;

	comments->m_aPedCommentQueue[comments->m_nActiveQueue][index] = *com;

	// Insert the physical slot into the original volume order
	uint8_t i = 0;
	if (index != 0) {
		for (i = 0; i < index; i++) {
			if (comments->m_aPedCommentQueue[comments->m_nActiveQueue][comments->m_aPedCommentOrderList[comments->m_nActiveQueue][i]].m_nVolume < comments->m_aPedCommentQueue[comments->m_nActiveQueue][index].m_nVolume)
				break;
		}

		if (i < index)
			memmove(&comments->m_aPedCommentOrderList[comments->m_nActiveQueue][i + 1], &comments->m_aPedCommentOrderList[comments->m_nActiveQueue][i], NUM_PED_COMMENTS_SLOTS - 1 - i);
	}

	comments->m_aPedCommentOrderList[comments->m_nActiveQueue][i] = index;
}

void
PedComments_Advance(cPedComments *comments)
{
	// Remember which queue will be consumed this frame
	uint8_t queue;
	// Switch the active queue before carrying pending comments forward
	if (comments->m_nActiveQueue == 0) {
		queue = 0;
		comments->m_nActiveQueue = 1;
	} else {
		queue = 1;
		comments->m_nActiveQueue = 0;
	}
	for (uint8_t i = 0; i < comments->m_nPedCommentCount[queue]; i++) {
		if (comments->m_aPedCommentQueue[queue][comments->m_aPedCommentOrderList[queue][i]].m_nLoadingTimeout > 0) {
			comments->m_aPedCommentQueue[queue][comments->m_aPedCommentOrderList[queue][i]].m_nLoadingTimeout--;
			PedComments_Add(comments, &comments->m_aPedCommentQueue[queue][comments->m_aPedCommentOrderList[queue][i]]);
		}
	}

	// Clear the consumed queue while retaining its stored records
	for (uint8_t i = 0; i < NUM_PED_COMMENTS_SLOTS; i++)
		comments->m_aPedCommentOrderList[queue][i] = NUM_PED_COMMENTS_SLOTS;
	comments->m_nPedCommentCount[queue] = 0;
}
//- rouz edit (ChatGPT)
