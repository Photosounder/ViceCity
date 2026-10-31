//+ rouz edit (ChatGPT)
#define _CRT_SECURE_NO_WARNINGS
#include "VarConsole_backend.h"
#include "Draw_constants.h"
#include <stdio.h>
#ifdef FIX_BUGS
VarConsoleState VarConsole={.m_nCurPage=1};
#else
VarConsoleState VarConsole;
#endif
void VarConsole_Construct(VarConsoleState *console)
{
    // Retain the original conditional constructor without clearing untouched fields or entry records
#ifdef FIX_BUGS
    VarConsole_Initialise(console);
#else
    (void)console;
#endif
}

void
VarConsole_Initialise(VarConsoleState *console)
{
    // Preserve the complete original field accesses and branch ordering
	console->m_nCountEntries = 0;
	console->m_nCurPage = 1;
	console->m_bIsOpen = false;
	console->m_nCurEntry = 0;
	console->m_nFirstEntryOnPage = 0;
}

void
VarConsole_AddInt8(VarConsoleState *console,char *text, int8_t *pVal, uint8_t step, int8_t min, int8_t max, uint8_t isVar)
{
    // Preserve the complete original field accesses and branch ordering
	int i;
	for (i = 0; i < console->m_nCountEntries; i++) {
		if (console->m_aEntries[i].text == text)
			return;
	}

	console->m_aEntries[i].text = text;
	console->m_aEntries[i].pInt8Value = pVal;
	console->m_aEntries[i].bAllowExceedBounds = isVar;
	console->m_aEntries[i].VarType = VCE_TYPE_INT8;
	console->m_aEntries[i].I8_step = step;
	console->m_aEntries[i].I8_min = min;
	console->m_aEntries[i].I8_max = max;
	console->m_nCountEntries++;
}

void
VarConsole_AddInt16(VarConsoleState *console,char *text, int16_t *pVal, uint16_t step, int16_t min, int16_t max, uint8_t isVar)
{
    // Preserve the complete original field accesses and branch ordering
	int i;
	for (i = 0; i < console->m_nCountEntries; i++) {
		if (console->m_aEntries[i].text == text)
			return;
	}

	console->m_aEntries[i].text = text;
	console->m_aEntries[i].pInt16Value = pVal;
	console->m_aEntries[i].bAllowExceedBounds = isVar;
	console->m_aEntries[i].VarType = VCE_TYPE_INT16;
	console->m_aEntries[i].I16_step = step;
	console->m_aEntries[i].I16_min = min;
	console->m_aEntries[i].I16_max = max;
	console->m_nCountEntries++;
}

void
VarConsole_AddInt32(VarConsoleState *console,char *text, int32_t *pVal, uint32_t step, int32_t min, int32_t max, uint8_t isVar)
{
    // Preserve the complete original field accesses and branch ordering
	int i;
	for (i = 0; i < console->m_nCountEntries; i++) {
		if (console->m_aEntries[i].text == text)
			return;
	}

	console->m_aEntries[i].text = text;
	console->m_aEntries[i].pInt32Value = pVal;
	console->m_aEntries[i].bAllowExceedBounds = isVar;
	console->m_aEntries[i].VarType = VCE_TYPE_INT32;
	console->m_aEntries[i].I32_step = step;
	console->m_aEntries[i].I32_min = min;
	console->m_aEntries[i].I32_max = max;
	console->m_nCountEntries++;
}

void
VarConsole_AddInt64(VarConsoleState *console,char *text, int64_t *pVal, uint64_t step, int64_t min, int64_t max, uint8_t isVar)
{
    // Preserve the complete original field accesses and branch ordering
	int i;
	for (i = 0; i < console->m_nCountEntries; i++) {
		if (console->m_aEntries[i].text == text)
			return;
	}

	console->m_aEntries[i].text = text;
	console->m_aEntries[i].pInt64Value = pVal;
	console->m_aEntries[i].bAllowExceedBounds = isVar;
	console->m_aEntries[i].VarType = VCE_TYPE_INT64;
	console->m_aEntries[i].I64_step = step;
	console->m_aEntries[i].I64_min = min;
	console->m_aEntries[i].I64_max = max;
	console->m_nCountEntries++;
}

void
VarConsole_AddUInt8(VarConsoleState *console,char *text, uint8_t *pVal, uint8_t step, int8_t min, int8_t max, uint8_t isVar)
{
    // Preserve the complete original field accesses and branch ordering
	int i;
	for (i = 0; i < console->m_nCountEntries; i++) {
		if (console->m_aEntries[i].text == text)
			return;
	}

	console->m_aEntries[i].text = text;
	console->m_aEntries[i].pUint8Value = pVal;
	console->m_aEntries[i].bAllowExceedBounds = isVar;
	console->m_aEntries[i].VarType = VCE_TYPE_UINT8;
	console->m_aEntries[i].I8_step = step;
	console->m_aEntries[i].I8_min = min;
	console->m_aEntries[i].I8_max = max;
	console->m_nCountEntries++;
}

void
VarConsole_AddUInt16(VarConsoleState *console,char *text, uint16_t *pVal, uint16_t step, int16_t min, int16_t max, uint8_t isVar)
{
    // Preserve the complete original field accesses and branch ordering
	int i;
	for (i = 0; i < console->m_nCountEntries; i++) {
		if (console->m_aEntries[i].text == text)
			return;
	}

	console->m_aEntries[i].text = text;
	console->m_aEntries[i].pUint16Value = pVal;
	console->m_aEntries[i].bAllowExceedBounds = isVar;
	console->m_aEntries[i].VarType = VCE_TYPE_UINT16;
	console->m_aEntries[i].I16_step = step;
	console->m_aEntries[i].I16_min = min;
	console->m_aEntries[i].I16_max = max;
	console->m_nCountEntries++;
}

void
VarConsole_AddUInt32(VarConsoleState *console,char *text, uint32_t *pVal, uint32_t step, int32_t min, int32_t max, uint8_t isVar)
{
    // Preserve the complete original field accesses and branch ordering
	int i;
	for (i = 0; i < console->m_nCountEntries; i++) {
		if (console->m_aEntries[i].text == text)
			return;
	}

	console->m_aEntries[i].text = text;
	console->m_aEntries[i].pUint32Value = pVal;
	console->m_aEntries[i].bAllowExceedBounds = isVar;
	console->m_aEntries[i].VarType = VCE_TYPE_UINT32;
	console->m_aEntries[i].I32_step = step;
	console->m_aEntries[i].I32_min = min;
	console->m_aEntries[i].I32_max = max;
	console->m_nCountEntries++;
}

void
VarConsole_AddUInt64(VarConsoleState *console,char *text, uint64_t *pVal, uint64_t step, int64_t min, int64_t max, uint8_t isVar)
{
    // Preserve the complete original field accesses and branch ordering
	int i;
	for (i = 0; i < console->m_nCountEntries; i++) {
		if (console->m_aEntries[i].text == text)
			return;
	}

	console->m_aEntries[i].text = text;
	console->m_aEntries[i].pUint64Value = pVal;
	console->m_aEntries[i].bAllowExceedBounds = isVar;
	console->m_aEntries[i].VarType = VCE_TYPE_UINT64;
	console->m_aEntries[i].I64_step = step;
	console->m_aEntries[i].I64_min = min;
	console->m_aEntries[i].I64_max = max;
	console->m_nCountEntries++;
}

void
VarConsole_AddFloat(VarConsoleState *console,char *text, float *pVal, float step, float min, float max, uint8_t isVar)
{
    // Preserve the complete original field accesses and branch ordering
	int i;
	for (i = 0; i < console->m_nCountEntries; i++) {
		if (console->m_aEntries[i].text == text)
			return;
	}

	console->m_aEntries[i].text = text;
	console->m_aEntries[i].pFloatValue = pVal;
	console->m_aEntries[i].bAllowExceedBounds = isVar;
	console->m_aEntries[i].VarType = VCE_TYPE_FLOAT;
	console->m_aEntries[i].F_step = step;
	console->m_aEntries[i].F_min = min;
	console->m_aEntries[i].F_max = max;
	console->m_nCountEntries++;
}

void
VarConsole_AddBool(VarConsoleState *console,char *text, bool *pVal, uint8_t isVar)
{
    // Preserve the complete original field accesses and branch ordering
	int i;
	for (i = 0; i < console->m_nCountEntries; i++) {
		if (console->m_aEntries[i].text == text)
			return;
	}

	console->m_aEntries[i].text = text;
	console->m_aEntries[i].pBoolValue = pVal;
	console->m_aEntries[i].bAllowExceedBounds = isVar;
	console->m_aEntries[i].VarType = VCE_TYPE_BOOL;
	console->m_nCountEntries++;
}

void
VarConsole_AddBool8(VarConsoleState *console,char *text, uint8_t *pVal, uint8_t isVar)
{
    // Preserve the complete original field accesses and branch ordering
	int i;
	for (i = 0; i < console->m_nCountEntries; i++) {
		if (console->m_aEntries[i].text == text)
			return;
	}

	console->m_aEntries[i].text = text;
	console->m_aEntries[i].pUint8Value = pVal;
	console->m_aEntries[i].bAllowExceedBounds = isVar;
	console->m_aEntries[i].VarType = VCE_TYPE_BOOL8;
	console->m_nCountEntries++;
}

void
VarConsole_AddBool16(VarConsoleState *console,char *text, uint16_t *pVal, uint8_t isVar)
{
    // Preserve the complete original field accesses and branch ordering
	int i;
	for (i = 0; i < console->m_nCountEntries; i++) {
		if (console->m_aEntries[i].text == text)
			return;
	}

	console->m_aEntries[i].text = text;
	console->m_aEntries[i].pUint16Value = pVal;
	console->m_aEntries[i].bAllowExceedBounds = isVar;
	console->m_aEntries[i].VarType = VCE_TYPE_BOOL16;
	console->m_nCountEntries++;
}

void
VarConsole_AddBool32(VarConsoleState *console,char *text, uint32_t *pVal, uint8_t isVar)
{
    // Preserve the complete original field accesses and branch ordering
	int i;
	for (i = 0; i < console->m_nCountEntries; i++) {
		if (console->m_aEntries[i].text == text)
			return;
	}

	console->m_aEntries[i].text = text;
	console->m_aEntries[i].pUint32Value = pVal;
	console->m_aEntries[i].bAllowExceedBounds = isVar;
	console->m_aEntries[i].VarType = VCE_TYPE_BOOL32;
	console->m_nCountEntries++;
}

void
VarConsole_AddFunction(VarConsoleState *console,char *text, void (*pCallback)(void))
{
    // Preserve the complete original field accesses and branch ordering
	int i;
	for (i = 0; i < console->m_nCountEntries; i++) {
		if (console->m_aEntries[i].text == text)
			return;
	}

	console->m_aEntries[i].text = text;
	console->m_aEntries[i].pCallback = pCallback;
	console->m_aEntries[i].VarType = VCE_TYPE_FUNCTION;
	console->m_nCountEntries++;
}

void
VarConsole_Remove(VarConsoleState *console,char *text)
{
    // Preserve the complete original field accesses and branch ordering
	int i;
	for (i = 0; i < console->m_nCountEntries; i++) {
		if (console->m_aEntries[i].text == text)
		{
			for (int j = i; j < console->m_nCountEntries-1; j++)
				console->m_aEntries[j] = console->m_aEntries[j+1];
			console->m_nCountEntries--;
			return;
		}
	}
}

void
VarConsole_SortPages(VarConsoleState *console)
{
    // Preserve the complete original field accesses and branch ordering
	console->m_nNumPages = console->m_nCountEntries / 30 + 1;
}

void
VarConsole_Display(VarConsoleState *console)
{
    // Preserve the complete original field accesses and branch ordering
	char s[256];
	uint16_t ws[256];

	VarConsole_BackendColor(200, 200, 200, 255);
	VarConsole_BackendStyle();
	VarConsole_BackendScale(VarConsole_BackendScaleX(0.5f), VarConsole_BackendScaleY(0.6f));
	VarConsole_BackendShadowPosition(2);
	VarConsole_BackendShadowColor(0, 0, 0, 255);
	VarConsole_BackendProp();
	VarConsole_BackendWrap(VarConsole_BackendScaleX(DEFAULT_SCREEN_WIDTH));
	VarConsole_BackendRightWrap(0.0f);
	sprintf(s, "PAGE %d", console->m_nCurPage);
	VarConsole_BackendUnicode(s, ws);
	VarConsole_BackendRightOn();
	VarConsole_BackendPrint(VarConsole_BackendScaleX(310.0f), VarConsole_BackendScaleY(30.0f), ws);
	VarConsole_BackendRightOff();
	int y = 45;
	for (int i = console->m_nFirstEntryOnPage; i < console->m_nCountEntries && i < console->m_nFirstEntryOnPage + 30; i++)
	{
		switch (console->m_aEntries[i].VarType)
		{
		case VCE_TYPE_INT8:
			sprintf(s, "(%d) %s:I8:%d", i + 1, console->m_aEntries[i].text, *console->m_aEntries[i].pInt8Value);
			break;
		case VCE_TYPE_INT16:
			sprintf(s, "(%d) %s:I16:%d", i + 1, console->m_aEntries[i].text, *console->m_aEntries[i].pInt16Value);
			break;
		case VCE_TYPE_INT32:
			sprintf(s, "(%d) %s:I32:%d", i + 1, console->m_aEntries[i].text, *console->m_aEntries[i].pInt32Value);
			break;
		case VCE_TYPE_INT64:
#ifdef FIX_BUGS  
			sprintf(s, "(%d) %s:I64:%lld", i + 1, console->m_aEntries[i].text, *console->m_aEntries[i].pInt64Value);
#else
			sprintf(s, "(%d) %s:I64:%d", i + 1, console->m_aEntries[i].text, (int32_t)*console->m_aEntries[i].pInt64Value);
#endif
			break;
		case VCE_TYPE_UINT8:
			sprintf(s, "(%d) %s:U8:%d", i + 1, console->m_aEntries[i].text, *console->m_aEntries[i].pUint8Value);
			break;
		case VCE_TYPE_UINT16:
			sprintf(s, "(%d) %s:U6:%d", i + 1, console->m_aEntries[i].text, *console->m_aEntries[i].pUint16Value);
			break;
		case VCE_TYPE_UINT32:
			sprintf(s, "(%d) %s:U32:%d", i + 1, console->m_aEntries[i].text, *console->m_aEntries[i].pUint32Value);
			break;
		case VCE_TYPE_UINT64:
#ifdef FIX_BUGS
			sprintf(s, "(%d) %s:U64:%llu", i + 1, console->m_aEntries[i].text, *console->m_aEntries[i].pUint64Value);
#else
			sprintf(s, "(%d) %s:U64:%d", i + 1, console->m_aEntries[i].text, (uint32_t)*console->m_aEntries[i].pUint64Value);
#endif
			break;
		case VCE_TYPE_FLOAT:
			sprintf(s, "(%d) %s:F:%f", i + 1, console->m_aEntries[i].text, *console->m_aEntries[i].pFloatValue);
			break;
		case VCE_TYPE_BOOL:
			if (*console->m_aEntries[i].pBoolValue)
				sprintf(s, "(%d) %s:B:TRUE", i + 1, console->m_aEntries[i].text);
			else
				sprintf(s, "(%d) %s:B : 0", i + 1, console->m_aEntries[i].text);
			break;
		case VCE_TYPE_BOOL8:
			if (*console->m_aEntries[i].pUint8Value == 0)
				sprintf(s, "(%d) %s:B8:0", i + 1, console->m_aEntries[i].text);
			else
				sprintf(s, "(%d) %s:B8:TRUE", i + 1, console->m_aEntries[i].text);
			break;
		case VCE_TYPE_BOOL16:
			if (*console->m_aEntries[i].pUint16Value == 0)
				sprintf(s, "(%d) %s:B16:0", i + 1, console->m_aEntries[i].text);
			else
				sprintf(s, "(%d) %s:B16:TRUE", i + 1, console->m_aEntries[i].text);
			break;
		case VCE_TYPE_BOOL32:
			if (*console->m_aEntries[i].pUint32Value == 0)
				sprintf(s, "(%d) %s:B32:0", i + 1, console->m_aEntries[i].text);
			else
				sprintf(s, "(%d) %s:B32:TRUE", i + 1, console->m_aEntries[i].text);
			break;
		case VCE_TYPE_FUNCTION:
			sprintf(s, "(%d) %s:FUNCTION:call this function?", i + 1, console->m_aEntries[i].text);
			break;
		}
		VarConsole_BackendUnicode(s, ws);
		if (console->m_nCurEntry == i) {
			VarConsole_BackendBackgroundOn();
#ifdef FIX_BUGS
			VarConsole_BackendBackgroundColor(128, 128, 128, 128);
#endif
		}
#ifdef FIX_BUGS
		else
			VarConsole_BackendBackgroundColor(128, 128, 128, 0);
#endif

		VarConsole_BackendColor(200, 200, 200, 255);
		VarConsole_BackendPrint(VarConsole_BackendScaleX(30.0f), VarConsole_BackendScaleY(y), ws);
		if (console->m_nCurEntry == i)
			VarConsole_BackendBackgroundOff();
		y += 12;
	}
}

void
VarConsole_ModifyLeft(VarConsoleState *console)
{
    // Preserve the complete original field accesses and branch ordering
	VarConsoleEntry *entry = &console->m_aEntries[console->m_nCurEntry];
	switch (entry->VarType)
	{
	case VCE_TYPE_INT8:
		*entry->pInt8Value -= entry->I8_step;
		if (entry->bAllowExceedBounds) {
			if (*entry->pInt8Value < entry->I8_min)
				*entry->pInt8Value = entry->I8_max;
		} else {
			if (*entry->pInt8Value < entry->I8_min)
				*entry->pInt8Value = entry->I8_min;
		}
		break;
	case VCE_TYPE_INT16:
		*entry->pInt16Value -= entry->I16_step;
		if (entry->bAllowExceedBounds) {
			if (*entry->pInt16Value < entry->I16_min)
				*entry->pInt16Value = entry->I16_max;
		}
		else {
			if (*entry->pInt16Value < entry->I16_min)
				*entry->pInt16Value = entry->I16_min;
		}
		break;
	case VCE_TYPE_INT32:
		*entry->pInt32Value -= entry->I32_step;
		if (entry->bAllowExceedBounds) {
			if (*entry->pInt32Value < entry->I32_min)
				*entry->pInt32Value = entry->I32_max;
		}
		else {
			if (*entry->pInt32Value < entry->I32_min)
				*entry->pInt32Value = entry->I32_min;
		}
		break;
	case VCE_TYPE_INT64:
		*entry->pInt64Value -= entry->I64_step;
		if (entry->bAllowExceedBounds) {
			if (*entry->pInt64Value < entry->I64_min)
				*entry->pInt64Value = entry->I64_max;
		}
		else {
			if (*entry->pInt64Value < entry->I64_min)
				*entry->pInt64Value = entry->I64_min;
		}
		break;
	case VCE_TYPE_UINT8:
		*entry->pUint8Value -= entry->I8_step;
		if (entry->bAllowExceedBounds) {
			if (*(int8_t*)entry->pUint8Value < entry->I8_min)
				*entry->pUint8Value = entry->I8_max;
		}
		else {
			if (*(int8_t*)entry->pUint8Value < entry->I8_min)
				*entry->pUint8Value = entry->I8_min;
		}
		break;
	case VCE_TYPE_UINT16:
		*entry->pUint16Value -= entry->I16_step;
		if (entry->bAllowExceedBounds) {
			if (*(int16_t*)entry->pUint16Value < entry->I16_min)
				*entry->pUint16Value = entry->I16_max;
		}
		else {
			if (*(int16_t*)entry->pUint16Value < entry->I16_min)
				*entry->pUint16Value = entry->I16_min;
		}
		break;
	case VCE_TYPE_UINT32:
		*entry->pUint32Value -= entry->I32_step;
		if (entry->bAllowExceedBounds) {
			if (*(int32_t*)entry->pUint32Value < entry->I32_min)
				*entry->pUint32Value = entry->I32_max;
		}
		else {
			if (*(int32_t*)entry->pUint32Value < entry->I32_min)
				*entry->pUint32Value = entry->I32_min;
		}
		break;
	case VCE_TYPE_UINT64:
		*entry->pUint64Value -= entry->I64_step;
		if (entry->bAllowExceedBounds) {
			if (*(int64_t*)entry->pUint64Value < entry->I64_min)
				*entry->pUint64Value = entry->I64_max;
		}
		else {
			if (*(int64_t*)entry->pUint64Value < entry->I64_min)
				*entry->pUint64Value = entry->I64_min;
		}
		break;
	case VCE_TYPE_FLOAT:
		*entry->pFloatValue -= entry->F_step;
		if (entry->bAllowExceedBounds) {
			if (*entry->pFloatValue < entry->F_min)
				*entry->pFloatValue = entry->F_max;
		}
		else {
			if (*entry->pFloatValue < entry->F_min)
				*entry->pFloatValue = entry->F_min;
		}
		break;
	case VCE_TYPE_BOOL:
		if (entry->bAllowExceedBounds)
			*entry->pBoolValue ^= true;
		else
			*entry->pBoolValue = false;
		break;
	case VCE_TYPE_BOOL8:
		if (entry->bAllowExceedBounds)
			*entry->pUint8Value = *entry->pUint8Value == false;
		else
			*entry->pUint8Value = false;
		break;
	case VCE_TYPE_BOOL16:
		if (entry->bAllowExceedBounds)
			*entry->pUint16Value = *entry->pUint16Value == false;
		else
			*entry->pUint16Value = false;
		break;
	case VCE_TYPE_BOOL32:
		if (entry->bAllowExceedBounds)
			*entry->pUint32Value = *entry->pUint32Value == false;
		else
			*entry->pUint32Value = false;
		break;
	case VCE_TYPE_FUNCTION:
		entry->pCallback();
		break;
	default:
		return;
	}
}

void
VarConsole_ModifyRight(VarConsoleState *console)
{
    // Preserve the complete original field accesses and branch ordering
	VarConsoleEntry *entry = &console->m_aEntries[console->m_nCurEntry];
	switch (entry->VarType)
	{
	case VCE_TYPE_INT8:
		*entry->pInt8Value += entry->I8_step;
		if (entry->bAllowExceedBounds) {
			if (*entry->pInt8Value > entry->I8_max)
				*entry->pInt8Value = entry->I8_min;
		}
		else {
			if (*entry->pInt8Value > entry->I8_max)
				*entry->pInt8Value = entry->I8_max;
		}
		break;
	case VCE_TYPE_INT16:
		*entry->pInt16Value += entry->I16_step;
		if (entry->bAllowExceedBounds) {
			if (*entry->pInt16Value > entry->I16_max)
				*entry->pInt16Value = entry->I16_min;
		}
		else {
			if (*entry->pInt16Value > entry->I16_max)
				*entry->pInt16Value = entry->I16_max;
		}
		break;
	case VCE_TYPE_INT32:
		*entry->pInt32Value += entry->I32_step;
		if (entry->bAllowExceedBounds) {
			if (*entry->pInt32Value > entry->I32_max)
				*entry->pInt32Value = entry->I32_min;
		}
		else {
			if (*entry->pInt32Value > entry->I32_max)
				*entry->pInt32Value = entry->I32_max;
		}
		break;
	case VCE_TYPE_INT64:
		*entry->pInt64Value += entry->I64_step;
		if (entry->bAllowExceedBounds) {
			if (*entry->pInt64Value > entry->I64_max)
				*entry->pInt64Value = entry->I64_min;
		}
		else {
			if (*entry->pInt64Value > entry->I64_max)
				*entry->pInt64Value = entry->I64_max;
		}
		break;
	case VCE_TYPE_UINT8:
		*entry->pUint8Value += entry->I8_step;
		if (entry->bAllowExceedBounds) {
			if (*entry->pUint8Value > (uint8_t)entry->I8_max)
				*entry->pUint8Value = entry->I8_min;
		}
		else {
			if (*entry->pUint8Value > (uint8_t)entry->I8_max)
				*entry->pUint8Value = entry->I8_max;
		}
		break;
	case VCE_TYPE_UINT16:
		*entry->pUint16Value += entry->I16_step;
		if (entry->bAllowExceedBounds) {
			if (*entry->pUint16Value > (uint16_t)entry->I16_max)
				*entry->pUint16Value = entry->I16_min;
		}
		else {
			if (*entry->pUint16Value > (uint16_t)entry->I16_max)
				*entry->pUint16Value = entry->I16_max;
		}
		break;
	case VCE_TYPE_UINT32:
		*entry->pUint32Value += entry->I32_step;
		if (entry->bAllowExceedBounds) {
			if (*entry->pUint32Value > (uint32_t)entry->I32_max)
				*entry->pUint32Value = entry->I32_min;
		}
		else {
			if (*entry->pUint32Value > (uint32_t)entry->I32_max)
				*entry->pUint32Value = entry->I32_max;
		}
		break;
	case VCE_TYPE_UINT64:
		*entry->pUint64Value += entry->I64_step;
		if (entry->bAllowExceedBounds) {
			if (*entry->pUint64Value > (uint64_t)entry->I64_max)
				*entry->pUint64Value = entry->I64_min;
		}
		else {
			if (*entry->pUint64Value > (uint64_t)entry->I64_max)
				*entry->pUint64Value = entry->I64_max;
		}
		break;
	case VCE_TYPE_FLOAT:
		*entry->pFloatValue += entry->F_step;
		if (entry->bAllowExceedBounds) {
			if (*entry->pFloatValue > entry->F_max)
				*entry->pFloatValue = entry->F_min;
		}
		else {
			if (*entry->pFloatValue > entry->F_max)
				*entry->pFloatValue = entry->F_max;
		}
		break;
	case VCE_TYPE_BOOL:
		if (entry->bAllowExceedBounds)
			*entry->pBoolValue ^= true;
		else
			*entry->pBoolValue = true;
		break;
	case VCE_TYPE_BOOL8:
		if (entry->bAllowExceedBounds)
			*entry->pUint8Value = *entry->pUint8Value == false;
		else
			*entry->pUint8Value = true;
		break;
	case VCE_TYPE_BOOL16:
		if (entry->bAllowExceedBounds)
			*entry->pUint16Value = *entry->pUint16Value == false;
		else
			*entry->pUint16Value = true;
		break;
	case VCE_TYPE_BOOL32:
		if (entry->bAllowExceedBounds)
			*entry->pUint32Value = *entry->pUint32Value == false;
		else
			*entry->pUint32Value = true;
		break;
	case VCE_TYPE_FUNCTION:
		entry->pCallback();
		break;
	default:
		return;
	}
}

void
VarConsole_Enter(VarConsoleState *console)
{
    // Preserve the complete original field accesses and branch ordering
	console->m_bIsOpen = true;
}

void
VarConsole_Exit(VarConsoleState *console)
{
    // Preserve the complete original field accesses and branch ordering
	console->m_bIsOpen = false;
}

void
VarConsole_Input(VarConsoleState *console)
{
    // Preserve the complete original field accesses and branch ordering
	if (VarConsole_BackendDPadDownJustDown() || VarConsole_BackendAnaloguePadDown())
	{
		console->m_nCurEntry++;
		if (console->m_nCurEntry < console->m_nCountEntries)
		{
			if (console->m_nCurEntry > console->m_nFirstEntryOnPage + 29)
			{
				console->m_nFirstEntryOnPage = console->m_nCurEntry;
				++console->m_nCurPage;
			}
		}
		else
		{
			console->m_nCurEntry = console->m_nCountEntries - 1;
		}
	}

	if (VarConsole_BackendDPadUpJustDown() || VarConsole_BackendAnaloguePadUp())
	{
		console->m_nCurEntry--;
		if (console->m_nCurEntry < console->m_nFirstEntryOnPage)
		{
			console->m_nFirstEntryOnPage = console->m_nCurEntry - 29;
			--console->m_nCurPage;
		}
		if (console->m_nFirstEntryOnPage < 0)
		{
			console->m_nCurEntry = 0;
			console->m_nFirstEntryOnPage = 0;
			console->m_nCurPage = 1;
		}
	}
	if (VarConsole_BackendSquare())
		VarConsole_ModifyLeft(console);
	if (VarConsole_BackendTriangle())
		VarConsole_ModifyRight(console);

	if (VarConsole_BackendDPadLeftJustDown() || VarConsole_BackendAnaloguePadLeft())
		VarConsole_ModifyLeft(console);

	if (VarConsole_BackendDPadRightJustDown() || VarConsole_BackendAnaloguePadRight())
		VarConsole_ModifyRight(console);

	if (VarConsole_BackendLeftShoulder2JustDown())
	{
		if (console->m_nCurPage > 1)
		{
			console->m_nCurPage--;
			console->m_nFirstEntryOnPage -= 30;
			console->m_nCurEntry = console->m_nFirstEntryOnPage;
			if (console->m_nFirstEntryOnPage < 0)
			{
				console->m_nFirstEntryOnPage = 0;
				console->m_nCurEntry = console->m_nFirstEntryOnPage;
				console->m_nCurPage = 1;
			}
		}
	}

	if (VarConsole_BackendRightShoulder2JustDown())
	{
		if (console->m_nCurPage < console->m_nNumPages)
		{
			console->m_nCurPage++;
			console->m_nFirstEntryOnPage += 30;
			console->m_nCurEntry = console->m_nFirstEntryOnPage;
			if (console->m_nFirstEntryOnPage >= console->m_nCountEntries)
			{
				console->m_nFirstEntryOnPage -= 30;
				console->m_nCurEntry = console->m_nFirstEntryOnPage;
				console->m_nCurPage--;
			}
		}
	}

	if (VarConsole_BackendRightShoulder1JustDown() && VarConsole_BackendLeftShoulder1JustDown())
		VarConsole_Exit(console);
}

void
VarConsole_Process(VarConsoleState *console)
{
    // Preserve the complete original field accesses and branch ordering
	VarConsole_Input(console);
	VarConsole_SortPages(console);
	VarConsole_Display(console);
}

uint8_t
VarConsole_Open(VarConsoleState *console)
{
    // Preserve the complete original field accesses and branch ordering
	return console->m_bIsOpen;
}

void
VarConsole_Check(VarConsoleState *console)
{
    // Preserve the complete original field accesses and branch ordering
	if (VarConsole_Open(console))
		VarConsole_Process(console);
	else if (VarConsole_BackendRightShoulder1JustDown() && VarConsole_BackendLeftShoulder1JustDown())
		VarConsole_Enter(console);
}
//- rouz edit (ChatGPT)
