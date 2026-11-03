//+ rouz edit (ChatGPT)
#define _CRT_SECURE_NO_WARNINGS
#include "Messages.h"
#include "Messages_backend.h"
#include "FontText.h"
#include "Timer.h"
#include "Pager.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
tMessage Messages_BriefMessages[NUMBRIEFMESSAGES];
tPreviousBrief Messages_PreviousBriefs[NUMPREVIOUSBRIEFS];
tBigMessage Messages_BIGMessages[NUMBIGMESSAGES];
char Messages_PreviousMissionTitle[16];

void
Messages_Init(void)
{
    // Preserve the complete original Init operation and ordered live accesses
	Messages_ClearMessages();

	for (int32_t i = 0; i < NUMPREVIOUSBRIEFS; i++) {
		Messages_PreviousBriefs[i].m_pText = NULL;
		Messages_PreviousBriefs[i].m_pString = NULL;
	}
}

uint16_t
Messages_GetWideStringLength(uint16_t *src)
{
    // Preserve the complete original GetWideStringLength operation and ordered live accesses
	uint16_t length = 0;
	while (*(src++)) length++;
	return length;
}

void
Messages_WideStringCopy(uint16_t *dst, uint16_t *src, uint16_t size)
{
    // Preserve the complete original WideStringCopy operation and ordered live accesses
	int32_t i = 0;
	if (src) {
		while (i < size - 1) {
			if (!src[i]) break;
			dst[i] = src[i];
			i++;
		}
	} else {
		while (i < size - 1)
			dst[i++] = '\0';
	}
	dst[i] = '\0';
}

static uint16_t Messages_FixupChar(uint16_t c)
{
    // Preserve the complete original FixupChar operation and ordered live accesses
#ifdef MORE_LANGUAGES
	if (Messages_BackendJapanese())
		return c & 0x7fff;
#endif
	return c;
}

bool
Messages_WideStringCompare(uint16_t *str1, uint16_t *str2, uint16_t size)
{
    // Preserve the complete original WideStringCompare operation and ordered live accesses
	uint16_t len1 = Messages_GetWideStringLength(str1);
	uint16_t len2 = Messages_GetWideStringLength(str2);
	if (len1 != len2 && (len1 < size || len2 < size))
		return false;

	for (int32_t i = 0; i < size && Messages_FixupChar(str1[i]) != '\0'; i++) {
		if (Messages_FixupChar(str1[i]) != Messages_FixupChar(str2[i]))
			return false;
	}
	return true;
}

void
Messages_Process(void)
{
    // Preserve the complete original Process operation and ordered live accesses
	for (int32_t style = 0; style < 6; style++) {
		if (Messages_BIGMessages[style].m_Stack[0].m_pText != NULL && (*Timer_GetTimeInMilliseconds()) > Messages_BIGMessages[style].m_Stack[0].m_nTime + Messages_BIGMessages[style].m_Stack[0].m_nStartTime) {
			Messages_BIGMessages[style].m_Stack[0].m_pText = NULL;

			int32_t i = 0;
			while (i < 3) {
				if (Messages_BIGMessages[style].m_Stack[i + 1].m_pText == NULL) break;
				Messages_BIGMessages[style].m_Stack[i] = Messages_BIGMessages[style].m_Stack[i + 1];
				i++;
			}

			Messages_BIGMessages[style].m_Stack[i].m_pText = NULL;
			Messages_BIGMessages[style].m_Stack[0].m_nStartTime = (*Timer_GetTimeInMilliseconds());
		}
	}

	if (Messages_BriefMessages[0].m_pText != NULL && (*Timer_GetTimeInMilliseconds()) > Messages_BriefMessages[0].m_nTime + Messages_BriefMessages[0].m_nStartTime) {
		Messages_BriefMessages[0].m_pText = NULL;
		int32_t i;
		for (i = 0; i < NUMBRIEFMESSAGES-1 && Messages_BriefMessages[i + 1].m_pText != NULL; i++) {
			Messages_BriefMessages[i] = Messages_BriefMessages[i + 1];
		}
		Messages_BriefMessages[i].m_pText = NULL;
		Messages_BriefMessages[0].m_nStartTime = (*Timer_GetTimeInMilliseconds());
		if (Messages_BriefMessages[0].m_pText != NULL)
			Messages_AddToPreviousBriefArray(
				Messages_BriefMessages[0].m_pText,
				Messages_BriefMessages[0].m_nNumber[0],
				Messages_BriefMessages[0].m_nNumber[1],
				Messages_BriefMessages[0].m_nNumber[2],
				Messages_BriefMessages[0].m_nNumber[3],
				Messages_BriefMessages[0].m_nNumber[4],
				Messages_BriefMessages[0].m_nNumber[5],
				Messages_BriefMessages[0].m_pString);
	}
}

void
Messages_Display(void)
{
    // Preserve the complete original Display operation and ordered live accesses
	uint16_t outstr[256];

	Messages_BackendDefinedState();

	for (int32_t i = 0; i < NUMBIGMESSAGES; i++) {
		Messages_InsertNumberInString(
			Messages_BIGMessages[i].m_Stack[0].m_pText,
			Messages_BIGMessages[i].m_Stack[0].m_nNumber[0],
			Messages_BIGMessages[i].m_Stack[0].m_nNumber[1],
			Messages_BIGMessages[i].m_Stack[0].m_nNumber[2],
			Messages_BIGMessages[i].m_Stack[0].m_nNumber[3],
			Messages_BIGMessages[i].m_Stack[0].m_nNumber[4],
			Messages_BIGMessages[i].m_Stack[0].m_nNumber[5],
			outstr);
		Messages_InsertStringInString(outstr, Messages_BIGMessages[i].m_Stack[0].m_pString);
		Messages_InsertPlayerControlKeysInString(outstr);
		Messages_BackendSetBigMessage(outstr, i);
	}

	Messages_InsertNumberInString(
		Messages_BriefMessages[0].m_pText,
		Messages_BriefMessages[0].m_nNumber[0],
		Messages_BriefMessages[0].m_nNumber[1],
		Messages_BriefMessages[0].m_nNumber[2],
		Messages_BriefMessages[0].m_nNumber[3],
		Messages_BriefMessages[0].m_nNumber[4],
		Messages_BriefMessages[0].m_nNumber[5],
		outstr);
	Messages_InsertStringInString(outstr, Messages_BriefMessages[0].m_pString);
	Messages_InsertPlayerControlKeysInString(outstr);
	Messages_BackendSetMessage(outstr);
}

void
Messages_AddMessage(uint16_t *msg, uint32_t time, uint16_t flag)
{
    // Preserve the complete original AddMessage operation and ordered live accesses
	uint16_t outstr[512]; // unused
	Messages_WideStringCopy(outstr, msg, 256);
	Messages_InsertPlayerControlKeysInString(outstr);
	Messages_GetWideStringLength(outstr);

	int32_t i = 0;
	while (i < NUMBRIEFMESSAGES && Messages_BriefMessages[i].m_pText != NULL)
		i++;
	if (i >= NUMBRIEFMESSAGES) return;

	Messages_BriefMessages[i].m_pText = msg;
	Messages_BriefMessages[i].m_nFlag = flag;
	Messages_BriefMessages[i].m_nTime = time;
	Messages_BriefMessages[i].m_nStartTime = (*Timer_GetTimeInMilliseconds());
	Messages_BriefMessages[i].m_nNumber[0] = -1;
	Messages_BriefMessages[i].m_nNumber[1] = -1;
	Messages_BriefMessages[i].m_nNumber[2] = -1;
	Messages_BriefMessages[i].m_nNumber[3] = -1;
	Messages_BriefMessages[i].m_nNumber[4] = -1;
	Messages_BriefMessages[i].m_nNumber[5] = -1;
	Messages_BriefMessages[i].m_pString = NULL;
	if (i == 0)
		Messages_AddToPreviousBriefArray(
			Messages_BriefMessages[0].m_pText,
			Messages_BriefMessages[0].m_nNumber[0],
			Messages_BriefMessages[0].m_nNumber[1],
			Messages_BriefMessages[0].m_nNumber[2],
			Messages_BriefMessages[0].m_nNumber[3],
			Messages_BriefMessages[0].m_nNumber[4],
			Messages_BriefMessages[0].m_nNumber[5],
			Messages_BriefMessages[0].m_pString);
}

void
Messages_AddMessageJumpQ(uint16_t *msg, uint32_t time, uint16_t flag)
{
    // Preserve the complete original AddMessageJumpQ operation and ordered live accesses
	uint16_t outstr[512]; // unused
	Messages_WideStringCopy(outstr, msg, 256);
	Messages_InsertPlayerControlKeysInString(outstr);
	Messages_GetWideStringLength(outstr);

	Messages_BriefMessages[0].m_pText = msg;
	Messages_BriefMessages[0].m_nFlag = flag;
	Messages_BriefMessages[0].m_nTime = time;
	Messages_BriefMessages[0].m_nStartTime = (*Timer_GetTimeInMilliseconds());
	Messages_BriefMessages[0].m_nNumber[0] = -1;
	Messages_BriefMessages[0].m_nNumber[1] = -1;
	Messages_BriefMessages[0].m_nNumber[2] = -1;
	Messages_BriefMessages[0].m_nNumber[3] = -1;
	Messages_BriefMessages[0].m_nNumber[4] = -1;
	Messages_BriefMessages[0].m_nNumber[5] = -1;
	Messages_BriefMessages[0].m_pString = NULL;
	Messages_AddToPreviousBriefArray(msg, -1, -1, -1, -1, -1, -1, 0);
}

void
Messages_AddMessageSoon(uint16_t *msg, uint32_t time, uint16_t flag)
{
    // Preserve the complete original AddMessageSoon operation and ordered live accesses
	uint16_t outstr[512]; // unused
	Messages_WideStringCopy(outstr, msg, 256);
	Messages_InsertPlayerControlKeysInString(outstr);
	Messages_GetWideStringLength(outstr);

	if (Messages_BriefMessages[0].m_pText != NULL) {
		for (int i = NUMBRIEFMESSAGES-1; i > 1; i--)
			Messages_BriefMessages[i] = Messages_BriefMessages[i-1];

		Messages_BriefMessages[1].m_pText = msg;
		Messages_BriefMessages[1].m_nFlag = flag;
		Messages_BriefMessages[1].m_nTime = time;
		Messages_BriefMessages[1].m_nStartTime = (*Timer_GetTimeInMilliseconds());
		Messages_BriefMessages[1].m_nNumber[0] = -1;
		Messages_BriefMessages[1].m_nNumber[1] = -1;
		Messages_BriefMessages[1].m_nNumber[2] = -1;
		Messages_BriefMessages[1].m_nNumber[3] = -1;
		Messages_BriefMessages[1].m_nNumber[4] = -1;
		Messages_BriefMessages[1].m_nNumber[5] = -1;
		Messages_BriefMessages[1].m_pString = NULL;
	}else{
		Messages_BriefMessages[0].m_pText = msg;
		Messages_BriefMessages[0].m_nFlag = flag;
		Messages_BriefMessages[0].m_nTime = time;
		Messages_BriefMessages[0].m_nStartTime = (*Timer_GetTimeInMilliseconds());
		Messages_BriefMessages[0].m_nNumber[0] = -1;
		Messages_BriefMessages[0].m_nNumber[1] = -1;
		Messages_BriefMessages[0].m_nNumber[2] = -1;
		Messages_BriefMessages[0].m_nNumber[3] = -1;
		Messages_BriefMessages[0].m_nNumber[4] = -1;
		Messages_BriefMessages[0].m_nNumber[5] = -1;
		Messages_BriefMessages[0].m_pString = NULL;
		Messages_AddToPreviousBriefArray(msg, -1, -1, -1, -1, -1, -1, NULL);
	}
}

void
Messages_ClearMessages(void)
{
    // Preserve the complete original ClearMessages operation and ordered live accesses
	for (int32_t i = 0; i < NUMBIGMESSAGES; i++) {
		for (int32_t j = 0; j < 4; j++) {
			Messages_BIGMessages[i].m_Stack[j].m_pText = NULL;
			Messages_BIGMessages[i].m_Stack[j].m_pString = NULL;
		}
	}
	Messages_ClearSmallMessagesOnly();
}

void
Messages_ClearSmallMessagesOnly(void)
{
    // Preserve the complete original ClearSmallMessagesOnly operation and ordered live accesses
	for (int32_t i = 0; i < NUMBRIEFMESSAGES; i++) {
		Messages_BriefMessages[i].m_pText = NULL;
		Messages_BriefMessages[i].m_pString = NULL;
	}
}

void
Messages_AddBigMessage(uint16_t *msg, uint32_t time, uint16_t style)
{
    // Preserve the complete original AddBigMessage operation and ordered live accesses
	uint16_t outstr[512]; // unused
	Messages_WideStringCopy(outstr, msg, 256);
	Messages_InsertPlayerControlKeysInString(outstr);
	Messages_GetWideStringLength(outstr);

	Messages_BIGMessages[style].m_Stack[0].m_pText = msg;
	Messages_BIGMessages[style].m_Stack[0].m_nFlag = 0;
	Messages_BIGMessages[style].m_Stack[0].m_nTime = time;
	Messages_BIGMessages[style].m_Stack[0].m_nStartTime = (*Timer_GetTimeInMilliseconds());
	Messages_BIGMessages[style].m_Stack[0].m_nNumber[0] = -1;
	Messages_BIGMessages[style].m_Stack[0].m_nNumber[1] = -1;
	Messages_BIGMessages[style].m_Stack[0].m_nNumber[2] = -1;
	Messages_BIGMessages[style].m_Stack[0].m_nNumber[3] = -1;
	Messages_BIGMessages[style].m_Stack[0].m_nNumber[4] = -1;
	Messages_BIGMessages[style].m_Stack[0].m_nNumber[5] = -1;
	Messages_BIGMessages[style].m_Stack[0].m_pString = NULL;
}

void
Messages_AddBigMessageQ(uint16_t *msg, uint32_t time, uint16_t style)
{
    // Preserve the complete original AddBigMessageQ operation and ordered live accesses
	uint16_t outstr[512]; // unused
	Messages_WideStringCopy(outstr, msg, 256);
	Messages_InsertPlayerControlKeysInString(outstr);
	Messages_GetWideStringLength(outstr);

	int32_t i = 0;
	while (i < 4 && Messages_BIGMessages[style].m_Stack[i].m_pText != NULL)
		i++;

	if (i >= 4) return;

	Messages_BIGMessages[style].m_Stack[i].m_pText = msg;
	Messages_BIGMessages[style].m_Stack[i].m_nFlag = 0;
	Messages_BIGMessages[style].m_Stack[i].m_nTime = time;
	Messages_BIGMessages[style].m_Stack[i].m_nStartTime = (*Timer_GetTimeInMilliseconds());
	Messages_BIGMessages[style].m_Stack[i].m_nNumber[0] = -1;
	Messages_BIGMessages[style].m_Stack[i].m_nNumber[1] = -1;
	Messages_BIGMessages[style].m_Stack[i].m_nNumber[2] = -1;
	Messages_BIGMessages[style].m_Stack[i].m_nNumber[3] = -1;
	Messages_BIGMessages[style].m_Stack[i].m_nNumber[4] = -1;
	Messages_BIGMessages[style].m_Stack[i].m_nNumber[5] = -1;
	Messages_BIGMessages[style].m_Stack[i].m_pString = NULL;
}

void
Messages_AddToPreviousBriefArray(uint16_t *text, int32_t n1, int32_t n2, int32_t n3, int32_t n4, int32_t n5, int32_t n6, uint16_t *string)
{
    // Preserve the complete original AddToPreviousBriefArray operation and ordered live accesses
	int32_t i;
	for (i = 0; i < NUMPREVIOUSBRIEFS && Messages_PreviousBriefs[i].m_pText != NULL; i++) {
		if (Messages_PreviousBriefs[i].m_nNumber[0] == n1
			&& Messages_PreviousBriefs[i].m_nNumber[1] == n2
			&& Messages_PreviousBriefs[i].m_nNumber[2] == n3
			&& Messages_PreviousBriefs[i].m_nNumber[3] == n4
			&& Messages_PreviousBriefs[i].m_nNumber[4] == n5
			&& Messages_PreviousBriefs[i].m_nNumber[5] == n6
			&& Messages_PreviousBriefs[i].m_pText == text
			&& Messages_PreviousBriefs[i].m_pString == string)
			return;
	}

	if (i != 0) {
		if (i == NUMPREVIOUSBRIEFS) i -= 2;
		else i--;

		while (i >= 0) {
			Messages_PreviousBriefs[i + 1] = Messages_PreviousBriefs[i];
			i--;
		}
	}
	Messages_PreviousBriefs[0].m_pText = text;
	Messages_PreviousBriefs[0].m_nNumber[0] = n1;
	Messages_PreviousBriefs[0].m_nNumber[1] = n2;
	Messages_PreviousBriefs[0].m_nNumber[2] = n3;
	Messages_PreviousBriefs[0].m_nNumber[3] = n4;
	Messages_PreviousBriefs[0].m_nNumber[4] = n5;
	Messages_PreviousBriefs[0].m_nNumber[5] = n6;
	Messages_PreviousBriefs[0].m_pString = string;
}

void
Messages_InsertNumberInString(uint16_t *str, int32_t n1, int32_t n2, int32_t n3, int32_t n4, int32_t n5, int32_t n6, uint16_t *outstr)
{
    // Preserve the complete original InsertNumberInString operation and ordered live accesses
	char numStr[10];
	uint16_t wNumStr[10];

	if (str == NULL) {
		*outstr = '\0';
		return;
	}

	sprintf(numStr, "%d", n1);
	size_t outLen = strlen(numStr);
	AsciiToUnicode(numStr, wNumStr);
	if (str[0] == 0) {
		*outstr = '\0';
		return;
	}

	int32_t size = Messages_GetWideStringLength(str);

	int32_t i = 0;

	for (int32_t c = 0; c < size;) {
#ifdef MORE_LANGUAGES
		if ((Messages_BackendJapanese() && str[c] == (0x8000 | '~') && str[c + 1] == (0x8000 | '1') && str[c + 2] == (0x8000 | '~')) ||
			(!Messages_BackendJapanese() && str[c] == '~' && str[c + 1] == '1' && str[c + 2] == '~')) {
#else
		if (str[c] == '~' && str[c + 1] == '1' && str[c + 2] == '~') {
#endif
			c += 3;
			for (int j = 0; j < outLen; )
				*(outstr++) = wNumStr[j++];

			i++;
			switch (i) {
			case 1: sprintf(numStr, "%d", n2); break;
			case 2: sprintf(numStr, "%d", n3); break;
			case 3: sprintf(numStr, "%d", n4); break;
			case 4: sprintf(numStr, "%d", n5); break;
			case 5: sprintf(numStr, "%d", n6); break;
			}
			outLen = strlen(numStr);
			AsciiToUnicode(numStr, wNumStr);
		} else {
			*(outstr++) = str[c++];
		}
	}
	*outstr = '\0';
}

void
Messages_InsertStringInString(uint16_t *str1, uint16_t *str2)
{
    // Preserve the complete original InsertStringInString operation and ordered live accesses
	uint16_t tempstr[256];

	if (!str1 || !str2) return;

	int32_t str1_size = Messages_GetWideStringLength(str1);
	int32_t str2_size = Messages_GetWideStringLength(str2);
	int32_t total_size = str1_size + str2_size;
	
	uint16_t *_str1 = str1;
	uint16_t i;
	for (i = 0; i < total_size; ) {
#ifdef MORE_LANGUAGES
		if ((Messages_BackendJapanese() && *_str1 == (0x8000 | '~') && *(_str1 + 1) == (0x8000 | 'a') && *(_str1 + 2) == (0x8000 | '~'))
			|| (*_str1 == '~' && *(_str1 + 1) == 'a' && *(_str1 + 2) == '~')) {
#else
		if (*_str1 == '~' && *(_str1 + 1) == 'a' && *(_str1 + 2) == '~') {
#endif
			_str1 += 3;
			for (int j = 0; j < str2_size; j++) {
				tempstr[i++] = str2[j];
			}
		} else {
			tempstr[i++] = *(_str1++);
		}
	}
	tempstr[i] = '\0';

	for (i = 0; i < total_size; i++)
		str1[i] = tempstr[i];

	while (i < 256)
		str1[i++] = '\0';
}

void
Messages_InsertPlayerControlKeysInString(uint16_t *str)
{
    // Preserve the complete original InsertPlayerControlKeysInString operation and ordered live accesses
	uint16_t i;
	uint16_t outstr[256];
	uint16_t keybuf[256];

	if (!str) return;
	uint16_t strSize = Messages_GetWideStringLength(str);
	memset(keybuf, 0, 256*sizeof(uint16_t));

	uint16_t *_outstr = outstr;
	for (i = 0; i < strSize;) {
#ifdef MORE_LANGUAGES
		if ((Messages_BackendJapanese() && str[i] == (0x8000 | '~') && str[i + 1] == (0x8000 | 'k') && str[i + 2] == (0x8000 | '~')) ||
			(!Messages_BackendJapanese() && str[i] == '~' && str[i + 1] == 'k' && str[i + 2] == '~')) {
#else
		if (str[i] == '~' && str[i + 1] == 'k' && str[i + 2] == '~') {
#endif
			i += 4;
			bool done = false;
			for (int32_t cont = 0; cont < MAX_CONTROLLERACTIONS && !done; cont++) {
				uint16_t contSize = Messages_GetWideStringLength((*Messages_BackendActionNames())[cont]);
				if (contSize != 0) {
					if (Messages_WideStringCompare(&str[i], (*Messages_BackendActionNames())[cont], contSize)) {
						done = true;
						Messages_BackendCommandKeys(cont, keybuf, 256);
						uint16_t keybuf_size = Messages_GetWideStringLength(keybuf);
						for (uint16_t j = 0; j < keybuf_size; j++) {
							*(_outstr++) = keybuf[j];
							keybuf[j] = '\0';
						}
						i += contSize + 1;
					}
				}
			}
		} else {
			*(_outstr++) = str[i++];
		}
	}
	*_outstr = '\0';

	for (i = 0; i < Messages_GetWideStringLength(outstr); i++)
		str[i] = outstr[i];

	while (i < 256)
		str[i++] = '\0';
}

void
Messages_AddMessageWithNumber(uint16_t *str, uint32_t time, uint16_t flag, int32_t n1, int32_t n2, int32_t n3, int32_t n4, int32_t n5, int32_t n6)
{
    // Preserve the complete original AddMessageWithNumber operation and ordered live accesses
	uint16_t outstr[512]; // unused
	Messages_InsertNumberInString(str, n1, n2, n3, n4, n5, n6, outstr);
	Messages_InsertPlayerControlKeysInString(outstr);
	Messages_GetWideStringLength(outstr);

	uint16_t i = 0;
	while (i < NUMBRIEFMESSAGES && Messages_BriefMessages[i].m_pText != NULL)
		i++;

	if (i >= NUMBRIEFMESSAGES) return;

	Messages_BriefMessages[i].m_pText = str;
	Messages_BriefMessages[i].m_nFlag = flag;
	Messages_BriefMessages[i].m_nTime = time;
	Messages_BriefMessages[i].m_nStartTime = (*Timer_GetTimeInMilliseconds());
	Messages_BriefMessages[i].m_nNumber[0] = n1;
	Messages_BriefMessages[i].m_nNumber[1] = n2;
	Messages_BriefMessages[i].m_nNumber[2] = n3;
	Messages_BriefMessages[i].m_nNumber[3] = n4;
	Messages_BriefMessages[i].m_nNumber[4] = n5;
	Messages_BriefMessages[i].m_nNumber[5] = n6;
	Messages_BriefMessages[i].m_pString = NULL;
	if (i == 0)
		Messages_AddToPreviousBriefArray(
			Messages_BriefMessages[0].m_pText,
			Messages_BriefMessages[0].m_nNumber[0],
			Messages_BriefMessages[0].m_nNumber[1],
			Messages_BriefMessages[0].m_nNumber[2],
			Messages_BriefMessages[0].m_nNumber[3],
			Messages_BriefMessages[0].m_nNumber[4],
			Messages_BriefMessages[0].m_nNumber[5],
			Messages_BriefMessages[0].m_pString);
}

void 
Messages_AddMessageJumpQWithNumber(uint16_t *str, uint32_t time, uint16_t flag, int32_t n1, int32_t n2, int32_t n3, int32_t n4, int32_t n5, int32_t n6)
{
    // Preserve the complete original AddMessageJumpQWithNumber operation and ordered live accesses
	uint16_t outstr[512]; // unused
	Messages_InsertNumberInString(str, n1, n2, n3, n4, n5, n6, outstr);
	Messages_InsertPlayerControlKeysInString(outstr);
	Messages_GetWideStringLength(outstr);

	Messages_BriefMessages[0].m_pText = str;
	Messages_BriefMessages[0].m_nFlag = flag;
	Messages_BriefMessages[0].m_nTime = time;
	Messages_BriefMessages[0].m_nStartTime = (*Timer_GetTimeInMilliseconds());
	Messages_BriefMessages[0].m_nNumber[0] = n1;
	Messages_BriefMessages[0].m_nNumber[1] = n2;
	Messages_BriefMessages[0].m_nNumber[2] = n3;
	Messages_BriefMessages[0].m_nNumber[3] = n4;
	Messages_BriefMessages[0].m_nNumber[4] = n5;
	Messages_BriefMessages[0].m_nNumber[5] = n6;
	Messages_BriefMessages[0].m_pString = NULL;
	Messages_AddToPreviousBriefArray(str, n1, n2, n3, n4, n5, n6, NULL);
}

void
Messages_AddMessageSoonWithNumber(uint16_t *str, uint32_t time, uint16_t flag, int32_t n1, int32_t n2, int32_t n3, int32_t n4, int32_t n5, int32_t n6)
{
    // Preserve the complete original AddMessageSoonWithNumber operation and ordered live accesses
	uint16_t outstr[512]; // unused
	Messages_InsertNumberInString(str, n1, n2, n3, n4, n5, n6, outstr);
	Messages_InsertPlayerControlKeysInString(outstr);
	Messages_GetWideStringLength(outstr);

	if (Messages_BriefMessages[0].m_pText != NULL) {
		for (int32_t i = NUMBRIEFMESSAGES-1; i > 1; i--)
			Messages_BriefMessages[i] = Messages_BriefMessages[i-1];

		Messages_BriefMessages[1].m_pText = str;
		Messages_BriefMessages[1].m_nFlag = flag;
		Messages_BriefMessages[1].m_nTime = time;
		Messages_BriefMessages[1].m_nStartTime = (*Timer_GetTimeInMilliseconds());
		Messages_BriefMessages[1].m_nNumber[0] = n1;
		Messages_BriefMessages[1].m_nNumber[1] = n2;
		Messages_BriefMessages[1].m_nNumber[2] = n3;
		Messages_BriefMessages[1].m_nNumber[3] = n4;
		Messages_BriefMessages[1].m_nNumber[4] = n5;
		Messages_BriefMessages[1].m_nNumber[5] = n6;
		Messages_BriefMessages[1].m_pString = NULL;
	} else {
		Messages_BriefMessages[0].m_pText = str;
		Messages_BriefMessages[0].m_nFlag = flag;
		Messages_BriefMessages[0].m_nTime = time;
		Messages_BriefMessages[0].m_nStartTime = (*Timer_GetTimeInMilliseconds());
		Messages_BriefMessages[0].m_nNumber[0] = n1;
		Messages_BriefMessages[0].m_nNumber[1] = n2;
		Messages_BriefMessages[0].m_nNumber[2] = n3;
		Messages_BriefMessages[0].m_nNumber[3] = n4;
		Messages_BriefMessages[0].m_nNumber[4] = n5;
		Messages_BriefMessages[0].m_nNumber[5] = n6;
		Messages_BriefMessages[0].m_pString = NULL;
		Messages_AddToPreviousBriefArray(str, n1, n2, n3, n4, n5, n6, NULL);
	}
}

void
Messages_AddBigMessageWithNumber(uint16_t *str, uint32_t time, uint16_t style, int32_t n1, int32_t n2, int32_t n3, int32_t n4, int32_t n5, int32_t n6)
{
    // Preserve the complete original AddBigMessageWithNumber operation and ordered live accesses
	uint16_t outstr[512]; // unused
	Messages_InsertNumberInString(str, n1, n2, n3, n4, n5, n6, outstr);
	Messages_InsertPlayerControlKeysInString(outstr);
	Messages_GetWideStringLength(outstr);

	Messages_BIGMessages[style].m_Stack[0].m_pText = str;
	Messages_BIGMessages[style].m_Stack[0].m_nFlag = 0;
	Messages_BIGMessages[style].m_Stack[0].m_nTime = time;
	Messages_BIGMessages[style].m_Stack[0].m_nStartTime = (*Timer_GetTimeInMilliseconds());
	Messages_BIGMessages[style].m_Stack[0].m_nNumber[0] = n1;
	Messages_BIGMessages[style].m_Stack[0].m_nNumber[1] = n2;
	Messages_BIGMessages[style].m_Stack[0].m_nNumber[2] = n3;
	Messages_BIGMessages[style].m_Stack[0].m_nNumber[3] = n4;
	Messages_BIGMessages[style].m_Stack[0].m_nNumber[4] = n5;
	Messages_BIGMessages[style].m_Stack[0].m_nNumber[5] = n6;
	Messages_BIGMessages[style].m_Stack[0].m_pString = NULL;
}

void
Messages_AddBigMessageWithNumberQ(uint16_t *str, uint32_t time, uint16_t style, int32_t n1, int32_t n2, int32_t n3, int32_t n4, int32_t n5, int32_t n6)
{
    // Preserve the complete original AddBigMessageWithNumberQ operation and ordered live accesses
	uint16_t outstr[512]; // unused
	Messages_InsertNumberInString(str, n1, n2, n3, n4, n5, n6, outstr);
	Messages_InsertPlayerControlKeysInString(outstr);
	Messages_GetWideStringLength(outstr);

	int32_t i = 0;

	while (i < 4 && Messages_BIGMessages[style].m_Stack[i].m_pText != NULL)
		i++;

	if (i >= 4) return;

	Messages_BIGMessages[style].m_Stack[i].m_pText = str;
	Messages_BIGMessages[style].m_Stack[i].m_nFlag = 0;
	Messages_BIGMessages[style].m_Stack[i].m_nTime = time;
	Messages_BIGMessages[style].m_Stack[i].m_nStartTime = (*Timer_GetTimeInMilliseconds());
	Messages_BIGMessages[style].m_Stack[i].m_nNumber[0] = n1;
	Messages_BIGMessages[style].m_Stack[i].m_nNumber[1] = n2;
	Messages_BIGMessages[style].m_Stack[i].m_nNumber[2] = n3;
	Messages_BIGMessages[style].m_Stack[i].m_nNumber[3] = n4;
	Messages_BIGMessages[style].m_Stack[i].m_nNumber[4] = n5;
	Messages_BIGMessages[style].m_Stack[i].m_nNumber[5] = n6;
	Messages_BIGMessages[style].m_Stack[i].m_pString = NULL;
}

void
Messages_AddMessageWithString(uint16_t *text, uint32_t time, uint16_t flag, uint16_t *str)
{
    // Preserve the complete original AddMessageWithString operation and ordered live accesses
	uint16_t outstr[512]; // unused
	Messages_WideStringCopy(outstr, text, 256);
	Messages_InsertStringInString(outstr, str);
	Messages_InsertPlayerControlKeysInString(outstr);
	Messages_GetWideStringLength(outstr);

	int32_t i = 0;
	while (i < NUMBRIEFMESSAGES && Messages_BriefMessages[i].m_pText != NULL)
		i++;

	if (i >= NUMBRIEFMESSAGES) return;

	Messages_BriefMessages[i].m_pText = text;
	Messages_BriefMessages[i].m_nFlag = flag;
	Messages_BriefMessages[i].m_nTime = time;
	Messages_BriefMessages[i].m_nStartTime = (*Timer_GetTimeInMilliseconds());
	Messages_BriefMessages[i].m_nNumber[0] = -1;
	Messages_BriefMessages[i].m_nNumber[1] = -1;
	Messages_BriefMessages[i].m_nNumber[2] = -1;
	Messages_BriefMessages[i].m_nNumber[3] = -1;
	Messages_BriefMessages[i].m_nNumber[4] = -1;
	Messages_BriefMessages[i].m_nNumber[5] = -1;
	Messages_BriefMessages[i].m_pString = str;
	if (i == 0)
		Messages_AddToPreviousBriefArray(
			Messages_BriefMessages[0].m_pText,
			Messages_BriefMessages[0].m_nNumber[0],
			Messages_BriefMessages[0].m_nNumber[1],
			Messages_BriefMessages[0].m_nNumber[2],
			Messages_BriefMessages[0].m_nNumber[3],
			Messages_BriefMessages[0].m_nNumber[4],
			Messages_BriefMessages[0].m_nNumber[5],
			Messages_BriefMessages[0].m_pString);
}

void
Messages_AddMessageJumpQWithString(uint16_t *text, uint32_t time, uint16_t flag, uint16_t *str)
{
    // Preserve the complete original AddMessageJumpQWithString operation and ordered live accesses
	uint16_t outstr[512]; // unused
	Messages_WideStringCopy(outstr, text, 256);
	Messages_InsertStringInString(outstr, str);
	Messages_InsertPlayerControlKeysInString(outstr);
	Messages_GetWideStringLength(outstr);

	Messages_BriefMessages[0].m_pText = text;
	Messages_BriefMessages[0].m_nFlag = flag;
	Messages_BriefMessages[0].m_nTime = time;
	Messages_BriefMessages[0].m_nStartTime = (*Timer_GetTimeInMilliseconds());
	Messages_BriefMessages[0].m_nNumber[0] = -1;
	Messages_BriefMessages[0].m_nNumber[1] = -1;
	Messages_BriefMessages[0].m_nNumber[2] = -1;
	Messages_BriefMessages[0].m_nNumber[3] = -1;
	Messages_BriefMessages[0].m_nNumber[4] = -1;
	Messages_BriefMessages[0].m_nNumber[5] = -1;
	Messages_BriefMessages[0].m_pString = str;
	Messages_AddToPreviousBriefArray(text, -1, -1, -1, -1, -1, -1, str);
}

static bool
Messages_FastWideStringComparison(uint16_t *str1, uint16_t *str2)
{
    // Preserve the complete original FastWideStringComparison operation and ordered live accesses
	while (*str1 == *str2) {
		++str1;
		++str2;
		if (!*str1 && !*str2) return true;
	}
	return false;
}

void
Messages_ClearThisPrint(uint16_t *str)
{
    // Preserve the complete original ClearThisPrint operation and ordered live accesses
	bool equal;

	do {
		equal = false;
		uint16_t i;
		for (i = 0; i < NUMBRIEFMESSAGES && Messages_BriefMessages[i].m_pText != NULL; i++) {
			equal = Messages_FastWideStringComparison(str, Messages_BriefMessages[i].m_pText);

			if (equal) break;
		}

		if (equal) {
			if (i != 0) {
				Messages_BriefMessages[i].m_pText = NULL;
				for (; i < NUMBRIEFMESSAGES-1 && Messages_BriefMessages[i+1].m_pText != NULL; i++) {
					Messages_BriefMessages[i] = Messages_BriefMessages[i + 1];
				}
				Messages_BriefMessages[i].m_pText = NULL;
			} else {
				Messages_BriefMessages[0].m_pText = NULL;
				for (; i < NUMBRIEFMESSAGES-1 && Messages_BriefMessages[i+1].m_pText != NULL; i++) {
					Messages_BriefMessages[i] = Messages_BriefMessages[i + 1];
				}
				Messages_BriefMessages[i].m_pText = NULL;
				Messages_BriefMessages[0].m_nStartTime = (*Timer_GetTimeInMilliseconds());
				if (Messages_BriefMessages[0].m_pText != NULL)
					Messages_AddToPreviousBriefArray(
						Messages_BriefMessages[0].m_pText,
						Messages_BriefMessages[0].m_nNumber[0],
						Messages_BriefMessages[0].m_nNumber[1],
						Messages_BriefMessages[0].m_nNumber[2],
						Messages_BriefMessages[0].m_nNumber[3],
						Messages_BriefMessages[0].m_nNumber[4],
						Messages_BriefMessages[0].m_nNumber[5],
						Messages_BriefMessages[0].m_pString);
			}
		}
	} while (equal);
}

void
Messages_ClearThisBigPrint(uint16_t *str)
{
    // Preserve the complete original ClearThisBigPrint operation and ordered live accesses
	bool equal;

	do {
		uint16_t i = 0;
		equal = false;
		uint16_t style = 0;
		while (style < NUMBIGMESSAGES)
		{
			if (i >= 4)
				break;

			if (Messages_BIGMessages[style].m_Stack[i].m_pText == NULL || equal)
				break;

			equal = Messages_FastWideStringComparison(str, Messages_BIGMessages[style].m_Stack[i].m_pText);

			if (!equal && ++i == 4) {
				i = 0;
				style++;
			}
		}
		if (equal) {
			if (i != 0) {
				Messages_BIGMessages[style].m_Stack[i].m_pText = NULL;
				while (i < 3) {
					if (Messages_BIGMessages[style].m_Stack[i + 1].m_pText == NULL)
						break;
					Messages_BIGMessages[style].m_Stack[i] = Messages_BIGMessages[style].m_Stack[i + 1];
					i++;
				}
				Messages_BIGMessages[style].m_Stack[i].m_pText = NULL;
			} else {
				Messages_BIGMessages[style].m_Stack[0].m_pText = NULL;
				i = 0;
				while (i < 3) {
					if (Messages_BIGMessages[style].m_Stack[i + 1].m_pText == NULL)
						break;
					Messages_BIGMessages[style].m_Stack[i] = Messages_BIGMessages[style].m_Stack[i + 1];
					i++;
				}
				Messages_BIGMessages[style].m_Stack[i].m_pText = NULL;
				Messages_BIGMessages[style].m_Stack[0].m_nStartTime = (*Timer_GetTimeInMilliseconds());
			}
		}
	} while (equal);
}

void
Messages_ClearAllMessagesDisplayedByGame(void)
{
    // Preserve the complete original ClearAllMessagesDisplayedByGame operation and ordered live accesses
	Messages_ClearMessages();
	for (int32_t i = 0; i < NUMPREVIOUSBRIEFS; i++) {
		Messages_PreviousBriefs[i].m_pText = NULL;
		Messages_PreviousBriefs[i].m_pString = NULL;
	}
	Messages_BackendClearHud();
	Pager_ClearMessages(&Pager_state);
}
//- rouz edit (ChatGPT)
