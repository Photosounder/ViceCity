//+ rouz edit (ChatGPT)
#include "Pager.h"
#include "Pager_backend.h"
#include "Timer.h"
#include <stddef.h>
CPager Pager_state;

void
Pager_Init(CPager *self)
{
    // Preserve the complete original Init operation with explicit queue storage
	Pager_ClearMessages(self);
	self->m_nNumDisplayLetters = 8;
}

void
Pager_Process(CPager *self)
{
    // Preserve the complete original Process operation with explicit queue storage
	if (self->m_messages[0].m_pText != NULL && self->m_messages[0].m_nCurrentPosition >= (int32_t)self->m_messages[0].m_nStringLength) {
		self->m_messages[0].m_pText = NULL;
		uint16_t i = 0;
		while (i < NUMPAGERMESSAGES-1) {
			if (self->m_messages[i + 1].m_pText == NULL) break;
			self->m_messages[i] = self->m_messages[i + 1];
			i++;
		}
		self->m_messages[i].m_pText = NULL;
		if (self->m_messages[0].m_pText != NULL)
			Pager_BackendBrief(
				self->m_messages[0].m_pText,
				self->m_messages[0].m_nNumber[0],
				self->m_messages[0].m_nNumber[1],
				self->m_messages[0].m_nNumber[2],
				self->m_messages[0].m_nNumber[3],
				self->m_messages[0].m_nNumber[4],
				self->m_messages[0].m_nNumber[5],
				0);
	}
	Pager_Display(self);
	if (self->m_messages[0].m_pText != NULL) {
		if (Pager_BackendWidescreen() || !Pager_BackendDrawHud() || Pager_BackendBigMessage0() || Pager_BackendBigMessage2()) {
			Pager_RestartCurrentMessage(self);
		} else {
			if ((*Timer_GetTimeInMilliseconds()) > self->m_messages[0].m_nTimeToChangePosition) {
				self->m_messages[0].m_nCurrentPosition++;
				self->m_messages[0].m_nTimeToChangePosition = (*Timer_GetTimeInMilliseconds()) + self->m_messages[0].m_nSpeedMs;
			}
		}
	}
}

void
Pager_Display(CPager *self)
{
    // Preserve the complete original Display operation with explicit queue storage
	uint16_t outstr1[256];
	uint16_t outstr2[260];

	uint16_t *pText = self->m_messages[0].m_pText;
	uint16_t i = 0;
	if (pText != NULL) {
		Pager_BackendInsertNumbers(
			pText,
			self->m_messages[0].m_nNumber[0],
			self->m_messages[0].m_nNumber[1],
			self->m_messages[0].m_nNumber[2],
			self->m_messages[0].m_nNumber[3],
			self->m_messages[0].m_nNumber[4],
			self->m_messages[0].m_nNumber[5],
			outstr1);
		for (; i < self->m_nNumDisplayLetters; i++) {
			int pos = self->m_messages[0].m_nCurrentPosition + i;
			if (pos >= 0) {
				if (!outstr1[pos]) break;

				outstr2[i] = outstr1[pos];
			} else {
				outstr2[i] = ' ';
			}
		}
	}
	outstr2[i] = '\0';
	Pager_BackendSetMessage(outstr2);
}

void
Pager_AddMessage(CPager *self,uint16_t *str, uint16_t speed, uint16_t priority, uint16_t a5)
{
    // Preserve the complete original AddMessage operation with explicit queue storage
	uint16_t size = Pager_BackendLength(str);
	for (int32_t i = 0; i < NUMPAGERMESSAGES; i++) {
		if (self->m_messages[i].m_pText) {
			if (self->m_messages[i].m_nPriority >= priority)
				continue;

			for (int j = NUMPAGERMESSAGES-1; j > i; j--)
				self->m_messages[j] = self->m_messages[j-1];

		}
		self->m_messages[i].m_pText = str;
		self->m_messages[i].m_nSpeedMs = speed;
		self->m_messages[i].m_nPriority = priority;
		self->m_messages[i].unused = a5;
		self->m_messages[i].m_nCurrentPosition = -(self->m_nNumDisplayLetters + 10);
		self->m_messages[i].m_nTimeToChangePosition = (*Timer_GetTimeInMilliseconds()) + speed;
		self->m_messages[i].m_nStringLength = size;
		self->m_messages[i].m_nNumber[0] = -1;
		self->m_messages[i].m_nNumber[1] = -1;
		self->m_messages[i].m_nNumber[2] = -1;
		self->m_messages[i].m_nNumber[3] = -1;
		self->m_messages[i].m_nNumber[4] = -1;
		self->m_messages[i].m_nNumber[5] = -1;

		if (i == 0)
			Pager_BackendBrief(
				self->m_messages[0].m_pText,
				self->m_messages[0].m_nNumber[0],
				self->m_messages[0].m_nNumber[1],
				self->m_messages[0].m_nNumber[2],
				self->m_messages[0].m_nNumber[3],
				self->m_messages[0].m_nNumber[4],
				self->m_messages[0].m_nNumber[5],
				NULL);
			return;
	}
}

void
Pager_AddMessageWithNumber(CPager *self,uint16_t *str, int32_t n1, int32_t n2, int32_t n3, int32_t n4, int32_t n5, int32_t n6, uint16_t speed, uint16_t priority, uint16_t a11)
{
    // Preserve the complete original AddMessageWithNumber operation with explicit queue storage
	uint16_t nstr[520];

	Pager_BackendInsertNumbers(str, n1, n2, n3, n4, n5, n6, nstr);
	uint16_t size = Pager_BackendLength(nstr);
	for (int32_t i = 0; i < NUMPAGERMESSAGES; i++) {
		if (self->m_messages[i].m_pText) {
			if (self->m_messages[i].m_nPriority >= priority)
				continue;

			for (int j = NUMPAGERMESSAGES-1; j > i; j--)
				self->m_messages[j] = self->m_messages[j - 1];

		}
		self->m_messages[i].m_pText = str;
		self->m_messages[i].m_nSpeedMs = speed;
		self->m_messages[i].m_nPriority = priority;
		self->m_messages[i].unused = a11;
		self->m_messages[i].m_nCurrentPosition = -(self->m_nNumDisplayLetters + 10);
		self->m_messages[i].m_nTimeToChangePosition = (*Timer_GetTimeInMilliseconds()) + speed;
		self->m_messages[i].m_nStringLength = size;
		self->m_messages[i].m_nNumber[0] = n1;
		self->m_messages[i].m_nNumber[1] = n2;
		self->m_messages[i].m_nNumber[2] = n3;
		self->m_messages[i].m_nNumber[3] = n4;
		self->m_messages[i].m_nNumber[4] = n5;
		self->m_messages[i].m_nNumber[5] = n6;

		if (i == 0)
			Pager_BackendBrief(
				self->m_messages[0].m_pText,
				self->m_messages[0].m_nNumber[0],
				self->m_messages[0].m_nNumber[1],
				self->m_messages[0].m_nNumber[2],
				self->m_messages[0].m_nNumber[3],
				self->m_messages[0].m_nNumber[4],
				self->m_messages[0].m_nNumber[5],
				NULL);
		return;
	}
}

void
Pager_ClearMessages(CPager *self)
{
    // Preserve the complete original ClearMessages operation with explicit queue storage
	for (int32_t i = 0; i < NUMPAGERMESSAGES; i++)
		self->m_messages[i].m_pText = NULL;
}

void
Pager_RestartCurrentMessage(CPager *self)
{
    // Preserve the complete original RestartCurrentMessage operation with explicit queue storage
	if (self->m_messages[0].m_pText != NULL) {
		self->m_messages[0].m_nCurrentPosition = -(self->m_nNumDisplayLetters + 10);
		self->m_messages[0].m_nTimeToChangePosition = (*Timer_GetTimeInMilliseconds()) + self->m_messages[0].m_nSpeedMs;
	}
}
//- rouz edit (ChatGPT)
