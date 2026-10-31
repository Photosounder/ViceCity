//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
enum eVarConsoleEntryType
{
	VCE_TYPE_INT8,
	VCE_TYPE_INT16,
	VCE_TYPE_INT32,
	VCE_TYPE_INT64,
	VCE_TYPE_UINT8,
	VCE_TYPE_UINT16,
	VCE_TYPE_UINT32,
	VCE_TYPE_UINT64,
	VCE_TYPE_FLOAT,
	VCE_TYPE_BOOL,
	VCE_TYPE_BOOL8,
	VCE_TYPE_BOOL16,
	VCE_TYPE_BOOL32,
	VCE_TYPE_FUNCTION,
};

typedef struct VarConsoleEntry
{
	char *text;
	int8_t *pInt8Value;
	int16_t *pInt16Value;
	int32_t *pInt32Value;
	int64_t *pInt64Value;
	uint8_t *pUint8Value;
	uint16_t *pUint16Value;
	uint32_t *pUint32Value;
	uint64_t *pUint64Value;
	float *pFloatValue;
	bool *pBoolValue;
	void (*pCallback)(void);
	int8_t I8_step, I8_max, I8_min;
	int16_t I16_step, I16_max, I16_min;
	int32_t I32_step, I32_max, I32_min;
	int64_t I64_step, I64_max, I64_min;
	float F_step, F_max, F_min;
	uint8_t bAllowExceedBounds;
	uint8_t VarType;
} VarConsoleEntry;

typedef struct VarConsoleState {

	int32_t m_nCountEntries;
	uint8_t m_bIsOpen;
	int32_t m_nCurEntry;
	int32_t m_nFirstEntryOnPage;
	int32_t m_nCurPage;
	int32_t m_nNumPages;
	VarConsoleEntry m_aEntries[91];
} VarConsoleState;
#ifdef __cplusplus
extern "C" {
#endif
extern VarConsoleState VarConsole;
void VarConsole_Construct(VarConsoleState *console);
void VarConsole_Initialise(VarConsoleState *console);
void VarConsole_AddInt8(VarConsoleState *console,char *text, int8_t *pVal, uint8_t step, int8_t min, int8_t max, uint8_t isVar);
void VarConsole_AddInt16(VarConsoleState *console,char *text, int16_t *pVal, uint16_t step, int16_t min, int16_t max, uint8_t isVar);
void VarConsole_AddInt32(VarConsoleState *console,char *text, int32_t *pVal, uint32_t step, int32_t min, int32_t max, uint8_t isVar);
void VarConsole_AddInt64(VarConsoleState *console,char *text, int64_t *pVal, uint64_t step, int64_t min, int64_t max, uint8_t isVar);
void VarConsole_AddUInt8(VarConsoleState *console,char *text, uint8_t *pVal, uint8_t step, int8_t min, int8_t max, uint8_t isVar);
void VarConsole_AddUInt16(VarConsoleState *console,char *text, uint16_t *pVal, uint16_t step, int16_t min, int16_t max, uint8_t isVar);
void VarConsole_AddUInt32(VarConsoleState *console,char *text, uint32_t *pVal, uint32_t step, int32_t min, int32_t max, uint8_t isVar);
void VarConsole_AddUInt64(VarConsoleState *console,char *text, uint64_t *pVal, uint64_t step, int64_t min, int64_t max, uint8_t isVar);
void VarConsole_AddFloat(VarConsoleState *console,char *text, float *pVal, float step, float min, float max, uint8_t isVar);
void VarConsole_AddBool(VarConsoleState *console,char *text, bool *pVal, uint8_t isVar);
void VarConsole_AddBool8(VarConsoleState *console,char *text, uint8_t *pVal, uint8_t isVar);
void VarConsole_AddBool16(VarConsoleState *console,char *text, uint16_t *pVal, uint8_t isVar);
void VarConsole_AddBool32(VarConsoleState *console,char *text, uint32_t *pVal, uint8_t isVar);
void VarConsole_AddFunction(VarConsoleState *console,char *text, void (*pCallback)(void));
void VarConsole_Remove(VarConsoleState *console,char *text);
void VarConsole_SortPages(VarConsoleState *console);
void VarConsole_Display(VarConsoleState *console);
void VarConsole_ModifyLeft(VarConsoleState *console);
void VarConsole_ModifyRight(VarConsoleState *console);
void VarConsole_Enter(VarConsoleState *console);
void VarConsole_Exit(VarConsoleState *console);
void VarConsole_Input(VarConsoleState *console);
void VarConsole_Process(VarConsoleState *console);
uint8_t VarConsole_Open(VarConsoleState *console);
void VarConsole_Check(VarConsoleState *console);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
