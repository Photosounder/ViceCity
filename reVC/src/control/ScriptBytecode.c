//+ rouz edit (Claude)
#include "ScriptBytecode.h"
#include <string.h>
#include "ScriptCommands.h"
#include "config.h"

int32_t ScriptBC_Read4(const uint8_t *space, uint32_t *ip)
{
	int32_t retval = (int32_t)((uint32_t)space[*ip + 3] << 24 | (uint32_t)space[*ip + 2] << 16 | (uint32_t)space[*ip + 1] << 8 | (uint32_t)space[*ip]);
	*ip += 4;
	return retval;
}

int16_t ScriptBC_Read2(const uint8_t *space, uint32_t *ip)
{
	int16_t retval = (int16_t)(space[*ip + 1] << 8 | space[*ip]);
	*ip += 2;
	return retval;
}

int8_t ScriptBC_Read1(const uint8_t *space, uint32_t *ip)
{
	int8_t retval = (int8_t)space[*ip];
	*ip += 1;
	return retval;
}

int ScriptBC_CollectParameters(const uint8_t *space, int32_t varSpaceSize, const int32_t *locals, int32_t numLocals, uint32_t *ip, int16_t total, int32_t *params)
{
	int16_t i;

	for (i = 0; i < total; i++) {
		uint16_t varIndex;

		switch (ScriptBC_Read1(space, ip)) {
		case SCRIPTBC_ARG_INT32:
		case SCRIPTBC_ARG_FLOAT:
			params[i] = ScriptBC_Read4(space, ip);
			break;
		case SCRIPTBC_ARG_GLOBALVAR:
			varIndex = (uint16_t)ScriptBC_Read2(space, ip);
			if (!(varIndex >= 8 && varIndex < varSpaceSize))
				return SCRIPTBC_BAD_GLOBAL_INDEX;
			params[i] = *((const int32_t *)&space[varIndex]);
			break;
		case SCRIPTBC_ARG_LOCALVAR:
			varIndex = (uint16_t)ScriptBC_Read2(space, ip);
			if (!(varIndex < numLocals))
				return SCRIPTBC_BAD_LOCAL_INDEX;
			params[i] = locals[varIndex];
			break;
		case SCRIPTBC_ARG_INT8:
			params[i] = ScriptBC_Read1(space, ip);
			break;
		case SCRIPTBC_ARG_INT16:
			params[i] = ScriptBC_Read2(space, ip);
			break;
		default:
			return SCRIPTBC_BAD_ARGUMENT;
		}
	}

	return SCRIPTBC_OK;
}

int32_t ScriptBC_CollectNextParameterWithoutIncreasingPC(const uint8_t *space, const int32_t *locals, uint32_t ip, int *result)
{
	uint32_t *pIp = &ip;

	*result = SCRIPTBC_OK;

	switch (ScriptBC_Read1(space, pIp)) {
	case SCRIPTBC_ARG_INT32:
		return ScriptBC_Read4(space, pIp);
	case SCRIPTBC_ARG_GLOBALVAR:
		return *((const int32_t *)&space[(uint16_t)ScriptBC_Read2(space, pIp)]);
	case SCRIPTBC_ARG_LOCALVAR:
		return locals[ScriptBC_Read2(space, pIp)];
	case SCRIPTBC_ARG_INT8:
		return ScriptBC_Read1(space, pIp);
	case SCRIPTBC_ARG_INT16:
		return ScriptBC_Read2(space, pIp);
	case SCRIPTBC_ARG_FLOAT:
		return ScriptBC_Read4(space, pIp);
	default:
		*result = SCRIPTBC_BAD_ARGUMENT;
	}

	return -1;
}

int ScriptBC_StoreParameters(uint8_t *space, int32_t *locals, uint32_t *ip, int16_t number, const int32_t *params)
{
	int16_t i;

	for (i = 0; i < number; i++) {
		switch (ScriptBC_Read1(space, ip)) {
		case SCRIPTBC_ARG_GLOBALVAR:
			*(int32_t *)&space[(uint16_t)ScriptBC_Read2(space, ip)] = params[i];
			break;
		case SCRIPTBC_ARG_LOCALVAR:
			locals[ScriptBC_Read2(space, ip)] = params[i];
			break;
		default:
			return SCRIPTBC_BAD_ARGUMENT;
		}
	}

	return SCRIPTBC_OK;
}

int32_t *ScriptBC_GetPointerToVariable(uint8_t *space, int32_t *locals, uint32_t *ip, int16_t type, int *result)
{
	*result = SCRIPTBC_OK;

	switch (ScriptBC_Read1(space, ip)) {
	case SCRIPTBC_ARG_GLOBALVAR:
		if (type != SCRIPTBC_VAR_GLOBAL)
			*result = SCRIPTBC_BAD_VARIABLE_TYPE;
		return (int32_t *)&space[(uint16_t)ScriptBC_Read2(space, ip)];
	case SCRIPTBC_ARG_LOCALVAR:
		if (type != SCRIPTBC_VAR_LOCAL)
			*result = SCRIPTBC_BAD_VARIABLE_TYPE;
		return &locals[ScriptBC_Read2(space, ip)];
	default:
		*result = SCRIPTBC_BAD_ARGUMENT;
	}

	return (int32_t *)0;
}
int32_t ScriptBC_ReadCommand(const uint8_t *space, uint32_t *ip, int *notFlag)
{
	int32_t command = (uint16_t)ScriptBC_Read2(space, ip);

	*notFlag = (command & 0x8000) != 0;
	return command & 0x7FFF;
}

int ScriptBC_CommandGroup(int32_t command)
{
	if (command < 0 || command >= 1500)
		return -1;

	return command / 100;
}

void ScriptBC_UpdateCompareFlag(int flag, int notFlag, uint16_t *andOrState, int *condResult)
{
	flag = flag != 0;
	if (notFlag)
		flag = !flag;
	if (*andOrState == SCRIPTBC_ANDOR_NONE) {
		*condResult = flag;
		return;
	}
	if (*andOrState >= SCRIPTBC_ANDS_1 && *andOrState <= SCRIPTBC_ANDS_8) {
		*condResult = *condResult & flag;
		if (*andOrState == SCRIPTBC_ANDS_1) {
			*andOrState = SCRIPTBC_ANDOR_NONE;
			return;
		}
	}
	else if (*andOrState >= SCRIPTBC_ORS_1 && *andOrState <= SCRIPTBC_ORS_8) {
		*condResult = *condResult | flag;
		if (*andOrState == SCRIPTBC_ORS_1) {
			*andOrState = SCRIPTBC_ANDOR_NONE;
			return;
		}
	}
	else {
		return;
	}
	(*andOrState)--;
}

int ScriptBC_Arithmetic(int32_t command, uint8_t *space, int32_t varSpaceSize, int32_t *locals, int32_t numLocals, uint32_t *ip, int32_t *params)
{
	// Five groups of four commands: set, add, subtract, multiply and divide, each for a global int, global float, local int and local float
	int32_t index = command - COMMAND_SET_VAR_INT;
	int32_t group = index / 4;
	int32_t kind = index % 4;
	int isLocal = kind >= 2;
	int isFloat = kind & 1;
	int result;
	int32_t *ptr;

	ptr = ScriptBC_GetPointerToVariable(space, locals, ip, isLocal ? SCRIPTBC_VAR_LOCAL : SCRIPTBC_VAR_GLOBAL, &result);
	if (result != SCRIPTBC_OK)
		return result;
	result = ScriptBC_CollectParameters(space, varSpaceSize, locals, numLocals, ip, 1, params);
	if (result != SCRIPTBC_OK)
		return result;

	if (!isFloat) {
		switch (group) {
		case 0: *ptr = params[0]; break;
		case 1: *ptr += params[0]; break;
		case 2: *ptr -= params[0]; break;
		case 3: *ptr *= params[0]; break;
		default: *ptr /= params[0]; break;
		}
	} else {
		float var, val;

		memcpy(&var, ptr, sizeof var);
		memcpy(&val, &params[0], sizeof val);
		switch (group) {
		case 0: var = val; break;
		case 1: var += val; break;
		case 2: var -= val; break;
		case 3: var *= val; break;
		default: var /= val; break;
		}
		memcpy(ptr, &var, sizeof var);
	}

	return SCRIPTBC_OK;
}

static float ScriptBC_F(const int32_t *p)
{
	float f;

	memcpy(&f, p, sizeof f);
	return f;
}

int ScriptBC_IsCompareCommand(int32_t command)
{
	switch (command) {
	case COMMAND_IS_INT_VAR_GREATER_THAN_NUMBER:
	case COMMAND_IS_INT_LVAR_GREATER_THAN_NUMBER:
	case COMMAND_IS_NUMBER_GREATER_THAN_INT_VAR:
	case COMMAND_IS_NUMBER_GREATER_THAN_INT_LVAR:
	case COMMAND_IS_INT_VAR_GREATER_THAN_INT_VAR:
	case COMMAND_IS_INT_LVAR_GREATER_THAN_INT_VAR:
	case COMMAND_IS_INT_VAR_GREATER_THAN_INT_LVAR:
	case COMMAND_IS_INT_LVAR_GREATER_THAN_INT_LVAR:
	case COMMAND_IS_FLOAT_VAR_GREATER_THAN_NUMBER:
	case COMMAND_IS_FLOAT_LVAR_GREATER_THAN_NUMBER:
	case COMMAND_IS_NUMBER_GREATER_THAN_FLOAT_VAR:
	case COMMAND_IS_NUMBER_GREATER_THAN_FLOAT_LVAR:
	case COMMAND_IS_FLOAT_VAR_GREATER_THAN_FLOAT_VAR:
	case COMMAND_IS_FLOAT_LVAR_GREATER_THAN_FLOAT_VAR:
	case COMMAND_IS_FLOAT_VAR_GREATER_THAN_FLOAT_LVAR:
	case COMMAND_IS_FLOAT_LVAR_GREATER_THAN_FLOAT_LVAR:
	case COMMAND_IS_INT_VAR_GREATER_OR_EQUAL_TO_NUMBER:
	case COMMAND_IS_INT_LVAR_GREATER_OR_EQUAL_TO_NUMBER:
	case COMMAND_IS_NUMBER_GREATER_OR_EQUAL_TO_INT_VAR:
	case COMMAND_IS_NUMBER_GREATER_OR_EQUAL_TO_INT_LVAR:
	case COMMAND_IS_INT_VAR_GREATER_OR_EQUAL_TO_INT_VAR:
	case COMMAND_IS_INT_LVAR_GREATER_OR_EQUAL_TO_INT_VAR:
	case COMMAND_IS_INT_VAR_GREATER_OR_EQUAL_TO_INT_LVAR:
	case COMMAND_IS_INT_LVAR_GREATER_OR_EQUAL_TO_INT_LVAR:
	case COMMAND_IS_FLOAT_VAR_GREATER_OR_EQUAL_TO_NUMBER:
	case COMMAND_IS_FLOAT_LVAR_GREATER_OR_EQUAL_TO_NUMBER:
	case COMMAND_IS_NUMBER_GREATER_OR_EQUAL_TO_FLOAT_VAR:
	case COMMAND_IS_NUMBER_GREATER_OR_EQUAL_TO_FLOAT_LVAR:
	case COMMAND_IS_FLOAT_VAR_GREATER_OR_EQUAL_TO_FLOAT_VAR:
	case COMMAND_IS_FLOAT_LVAR_GREATER_OR_EQUAL_TO_FLOAT_VAR:
	case COMMAND_IS_FLOAT_VAR_GREATER_OR_EQUAL_TO_FLOAT_LVAR:
	case COMMAND_IS_FLOAT_LVAR_GREATER_OR_EQUAL_TO_FLOAT_LVAR:
	case COMMAND_IS_INT_VAR_EQUAL_TO_NUMBER:
	case COMMAND_IS_INT_LVAR_EQUAL_TO_NUMBER:
	case COMMAND_IS_INT_VAR_EQUAL_TO_INT_VAR:
	case COMMAND_IS_INT_VAR_EQUAL_TO_INT_LVAR:
	case COMMAND_IS_INT_LVAR_EQUAL_TO_INT_LVAR:
	case COMMAND_IS_FLOAT_VAR_EQUAL_TO_NUMBER:
	case COMMAND_IS_FLOAT_LVAR_EQUAL_TO_NUMBER:
	case COMMAND_IS_FLOAT_VAR_EQUAL_TO_FLOAT_VAR:
	case COMMAND_IS_FLOAT_VAR_EQUAL_TO_FLOAT_LVAR:
	case COMMAND_IS_FLOAT_LVAR_EQUAL_TO_FLOAT_LVAR:
		return 1;
	default:
		return 0;
	}
}

int ScriptBC_Compare(int32_t command, uint8_t *space, int32_t varSpaceSize, int32_t *locals, int32_t numLocals, uint32_t *ip, int32_t *params, int *flag)
{
	int result;
	int32_t *ptr = 0, *ptr1 = 0, *ptr2 = 0;

	(void)ptr; (void)ptr1; (void)ptr2;
	*flag = 0;

	switch (command) {
	case COMMAND_IS_INT_VAR_GREATER_THAN_NUMBER:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		result = ScriptBC_CollectParameters(space, varSpaceSize, locals, numLocals, ip, 1, params);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = *ptr > params[0];
		break;
	}
	case COMMAND_IS_INT_LVAR_GREATER_THAN_NUMBER:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		result = ScriptBC_CollectParameters(space, varSpaceSize, locals, numLocals, ip, 1, params);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = *ptr > params[0];
		break;
	}
	case COMMAND_IS_NUMBER_GREATER_THAN_INT_VAR:
	{
		result = ScriptBC_CollectParameters(space, varSpaceSize, locals, numLocals, ip, 1, params);
		if (result != SCRIPTBC_OK)
			return result;
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = params[0] > *ptr;
		break;
	}
	case COMMAND_IS_NUMBER_GREATER_THAN_INT_LVAR:
	{
		result = ScriptBC_CollectParameters(space, varSpaceSize, locals, numLocals, ip, 1, params);
		if (result != SCRIPTBC_OK)
			return result;
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = params[0] > *ptr;
		break;
	}
	case COMMAND_IS_INT_VAR_GREATER_THAN_INT_VAR:
	{
		ptr1 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = *ptr1 > *ptr2;
		break;
	}
	case COMMAND_IS_INT_LVAR_GREATER_THAN_INT_VAR:
	{
		ptr1 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = *ptr1 > *ptr2;
		break;
	}
	case COMMAND_IS_INT_VAR_GREATER_THAN_INT_LVAR:
	{
		ptr1 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = *ptr1 > *ptr2;
		break;
	}
	case COMMAND_IS_INT_LVAR_GREATER_THAN_INT_LVAR:
	{
		ptr1 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = *ptr1 > *ptr2;
		break;
	}
	case COMMAND_IS_FLOAT_VAR_GREATER_THAN_NUMBER:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		result = ScriptBC_CollectParameters(space, varSpaceSize, locals, numLocals, ip, 1, params);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = ScriptBC_F(ptr) > ScriptBC_F(&params[0]);
		break;
	}
	case COMMAND_IS_FLOAT_LVAR_GREATER_THAN_NUMBER:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		result = ScriptBC_CollectParameters(space, varSpaceSize, locals, numLocals, ip, 1, params);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = ScriptBC_F(ptr) > ScriptBC_F(&params[0]);
		break;
	}
	case COMMAND_IS_NUMBER_GREATER_THAN_FLOAT_VAR:
	{
		result = ScriptBC_CollectParameters(space, varSpaceSize, locals, numLocals, ip, 1, params);
		if (result != SCRIPTBC_OK)
			return result;
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = ScriptBC_F(&params[0]) > ScriptBC_F(ptr);
		break;
	}
	case COMMAND_IS_NUMBER_GREATER_THAN_FLOAT_LVAR:
	{
		result = ScriptBC_CollectParameters(space, varSpaceSize, locals, numLocals, ip, 1, params);
		if (result != SCRIPTBC_OK)
			return result;
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = ScriptBC_F(&params[0]) > ScriptBC_F(ptr);
		break;
	}
	case COMMAND_IS_FLOAT_VAR_GREATER_THAN_FLOAT_VAR:
	{
		ptr1 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = ScriptBC_F(ptr1) > ScriptBC_F(ptr2);
		break;
	}
	case COMMAND_IS_FLOAT_LVAR_GREATER_THAN_FLOAT_VAR:
	{
		ptr1 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = ScriptBC_F(ptr1) > ScriptBC_F(ptr2);
		break;
	}
	case COMMAND_IS_FLOAT_VAR_GREATER_THAN_FLOAT_LVAR:
	{
		ptr1 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = ScriptBC_F(ptr1) > ScriptBC_F(ptr2);
		break;
	}
	case COMMAND_IS_FLOAT_LVAR_GREATER_THAN_FLOAT_LVAR:
	{
		ptr1 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = ScriptBC_F(ptr1) > ScriptBC_F(ptr2);
		break;
	}
	case COMMAND_IS_INT_VAR_GREATER_OR_EQUAL_TO_NUMBER:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		result = ScriptBC_CollectParameters(space, varSpaceSize, locals, numLocals, ip, 1, params);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = *ptr >= params[0];
		break;
	}
	case COMMAND_IS_INT_LVAR_GREATER_OR_EQUAL_TO_NUMBER:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		result = ScriptBC_CollectParameters(space, varSpaceSize, locals, numLocals, ip, 1, params);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = *ptr >= params[0];
		break;
	}
	case COMMAND_IS_NUMBER_GREATER_OR_EQUAL_TO_INT_VAR:
	{
		result = ScriptBC_CollectParameters(space, varSpaceSize, locals, numLocals, ip, 1, params);
		if (result != SCRIPTBC_OK)
			return result;
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = params[0] >= *ptr;
		break;
	}
	case COMMAND_IS_NUMBER_GREATER_OR_EQUAL_TO_INT_LVAR:
	{
		result = ScriptBC_CollectParameters(space, varSpaceSize, locals, numLocals, ip, 1, params);
		if (result != SCRIPTBC_OK)
			return result;
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = params[0] >= *ptr;
		break;
	}
	case COMMAND_IS_INT_VAR_GREATER_OR_EQUAL_TO_INT_VAR:
	{
		ptr1 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = *ptr1 >= *ptr2;
		break;
	}
	case COMMAND_IS_INT_LVAR_GREATER_OR_EQUAL_TO_INT_VAR:
	{
		ptr1 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = *ptr1 >= *ptr2;
		break;
	}
	case COMMAND_IS_INT_VAR_GREATER_OR_EQUAL_TO_INT_LVAR:
	{
		ptr1 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = *ptr1 >= *ptr2;
		break;
	}
	case COMMAND_IS_INT_LVAR_GREATER_OR_EQUAL_TO_INT_LVAR:
	{
		ptr1 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = *ptr1 >= *ptr2;
		break;
	}
	case COMMAND_IS_FLOAT_VAR_GREATER_OR_EQUAL_TO_NUMBER:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		result = ScriptBC_CollectParameters(space, varSpaceSize, locals, numLocals, ip, 1, params);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = ScriptBC_F(ptr) >= ScriptBC_F(&params[0]);
		break;
	}
	case COMMAND_IS_FLOAT_LVAR_GREATER_OR_EQUAL_TO_NUMBER:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		result = ScriptBC_CollectParameters(space, varSpaceSize, locals, numLocals, ip, 1, params);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = ScriptBC_F(ptr) >= ScriptBC_F(&params[0]);
		break;
	}
	case COMMAND_IS_NUMBER_GREATER_OR_EQUAL_TO_FLOAT_VAR:
	{
		result = ScriptBC_CollectParameters(space, varSpaceSize, locals, numLocals, ip, 1, params);
		if (result != SCRIPTBC_OK)
			return result;
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = ScriptBC_F(&params[0]) >= ScriptBC_F(ptr);
		break;
	}
	case COMMAND_IS_NUMBER_GREATER_OR_EQUAL_TO_FLOAT_LVAR:
	{
		result = ScriptBC_CollectParameters(space, varSpaceSize, locals, numLocals, ip, 1, params);
		if (result != SCRIPTBC_OK)
			return result;
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = ScriptBC_F(&params[0]) >= ScriptBC_F(ptr);
		break;
	}
	case COMMAND_IS_FLOAT_VAR_GREATER_OR_EQUAL_TO_FLOAT_VAR:
	{
		ptr1 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = ScriptBC_F(ptr1) >= ScriptBC_F(ptr2);
		break;
	}
	case COMMAND_IS_FLOAT_LVAR_GREATER_OR_EQUAL_TO_FLOAT_VAR:
	{
		ptr1 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = ScriptBC_F(ptr1) >= ScriptBC_F(ptr2);
		break;
	}
	case COMMAND_IS_FLOAT_VAR_GREATER_OR_EQUAL_TO_FLOAT_LVAR:
	{
		ptr1 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = ScriptBC_F(ptr1) >= ScriptBC_F(ptr2);
		break;
	}
	case COMMAND_IS_FLOAT_LVAR_GREATER_OR_EQUAL_TO_FLOAT_LVAR:
	{
		ptr1 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = ScriptBC_F(ptr1) >= ScriptBC_F(ptr2);
		break;
	}
	case COMMAND_IS_INT_VAR_EQUAL_TO_NUMBER:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		result = ScriptBC_CollectParameters(space, varSpaceSize, locals, numLocals, ip, 1, params);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = *ptr == params[0];
		break;
	}
	case COMMAND_IS_INT_LVAR_EQUAL_TO_NUMBER:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		result = ScriptBC_CollectParameters(space, varSpaceSize, locals, numLocals, ip, 1, params);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = *ptr == params[0];
		break;
	}
	case COMMAND_IS_INT_VAR_EQUAL_TO_INT_VAR:
	{
		ptr1 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = *ptr1 == *ptr2;
		break;
	}
	case COMMAND_IS_INT_VAR_EQUAL_TO_INT_LVAR:
	{
		ptr1 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = *ptr1 == *ptr2;
		break;
	}
	case COMMAND_IS_INT_LVAR_EQUAL_TO_INT_LVAR:
	{
		ptr1 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = *ptr1 == *ptr2;
		break;
	}
	case COMMAND_IS_FLOAT_VAR_EQUAL_TO_NUMBER:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		result = ScriptBC_CollectParameters(space, varSpaceSize, locals, numLocals, ip, 1, params);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = ScriptBC_F(ptr) == ScriptBC_F(&params[0]);
		break;
	}
	case COMMAND_IS_FLOAT_LVAR_EQUAL_TO_NUMBER:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		result = ScriptBC_CollectParameters(space, varSpaceSize, locals, numLocals, ip, 1, params);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = ScriptBC_F(ptr) == ScriptBC_F(&params[0]);
		break;
	}
	case COMMAND_IS_FLOAT_VAR_EQUAL_TO_FLOAT_VAR:
	{
		ptr1 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = ScriptBC_F(ptr1) == ScriptBC_F(ptr2);
		break;
	}
	case COMMAND_IS_FLOAT_VAR_EQUAL_TO_FLOAT_LVAR:
	{
		ptr1 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = ScriptBC_F(ptr1) == ScriptBC_F(ptr2);
		break;
	}
	case COMMAND_IS_FLOAT_LVAR_EQUAL_TO_FLOAT_LVAR:
	{
		ptr1 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*flag = ScriptBC_F(ptr1) == ScriptBC_F(ptr2);
		break;
	}
	default:
		return SCRIPTBC_BAD_ARGUMENT;
	}

	return SCRIPTBC_OK;
}

static void ScriptBC_SetF(int32_t *p, float f)
{
	memcpy(p, &f, sizeof f);
}

int ScriptBC_IsVariableOpCommand(int32_t command)
{
	switch (command) {
	case COMMAND_ADD_INT_VAR_TO_INT_VAR:
	case COMMAND_ADD_INT_LVAR_TO_INT_VAR:
	case COMMAND_ADD_INT_VAR_TO_INT_LVAR:
	case COMMAND_ADD_INT_LVAR_TO_INT_LVAR:
	case COMMAND_ADD_FLOAT_VAR_TO_FLOAT_VAR:
	case COMMAND_ADD_FLOAT_LVAR_TO_FLOAT_VAR:
	case COMMAND_ADD_FLOAT_VAR_TO_FLOAT_LVAR:
	case COMMAND_ADD_FLOAT_LVAR_TO_FLOAT_LVAR:
	case COMMAND_SUB_INT_VAR_FROM_INT_VAR:
	case COMMAND_SUB_INT_LVAR_FROM_INT_LVAR:
	case COMMAND_SUB_FLOAT_VAR_FROM_FLOAT_VAR:
	case COMMAND_SUB_FLOAT_LVAR_FROM_FLOAT_LVAR:
	case COMMAND_SUB_INT_LVAR_FROM_INT_VAR:
	case COMMAND_SUB_INT_VAR_FROM_INT_LVAR:
	case COMMAND_SUB_FLOAT_LVAR_FROM_FLOAT_VAR:
	case COMMAND_SUB_FLOAT_VAR_FROM_FLOAT_LVAR:
	case COMMAND_MULT_INT_VAR_BY_INT_VAR:
	case COMMAND_MULT_INT_VAR_BY_INT_LVAR:
	case COMMAND_MULT_INT_LVAR_BY_INT_VAR:
	case COMMAND_MULT_INT_LVAR_BY_INT_LVAR:
	case COMMAND_MULT_FLOAT_VAR_BY_FLOAT_VAR:
	case COMMAND_MULT_FLOAT_VAR_BY_FLOAT_LVAR:
	case COMMAND_MULT_FLOAT_LVAR_BY_FLOAT_VAR:
	case COMMAND_MULT_FLOAT_LVAR_BY_FLOAT_LVAR:
	case COMMAND_DIV_INT_VAR_BY_INT_VAR:
	case COMMAND_DIV_INT_VAR_BY_INT_LVAR:
	case COMMAND_DIV_INT_LVAR_BY_INT_VAR:
	case COMMAND_DIV_INT_LVAR_BY_INT_LVAR:
	case COMMAND_DIV_FLOAT_VAR_BY_FLOAT_VAR:
	case COMMAND_DIV_FLOAT_VAR_BY_FLOAT_LVAR:
	case COMMAND_DIV_FLOAT_LVAR_BY_FLOAT_VAR:
	case COMMAND_DIV_FLOAT_LVAR_BY_FLOAT_LVAR:
	case COMMAND_SET_VAR_INT_TO_VAR_INT:
	case COMMAND_SET_VAR_INT_TO_LVAR_INT:
	case COMMAND_SET_LVAR_INT_TO_VAR_INT:
	case COMMAND_SET_LVAR_INT_TO_LVAR_INT:
	case COMMAND_SET_VAR_FLOAT_TO_VAR_FLOAT:
	case COMMAND_SET_VAR_FLOAT_TO_LVAR_FLOAT:
	case COMMAND_SET_LVAR_FLOAT_TO_VAR_FLOAT:
	case COMMAND_SET_LVAR_FLOAT_TO_LVAR_FLOAT:
	case COMMAND_CSET_VAR_INT_TO_VAR_FLOAT:
	case COMMAND_CSET_VAR_INT_TO_LVAR_FLOAT:
	case COMMAND_CSET_LVAR_INT_TO_VAR_FLOAT:
	case COMMAND_CSET_LVAR_INT_TO_LVAR_FLOAT:
	case COMMAND_CSET_VAR_FLOAT_TO_VAR_INT:
	case COMMAND_CSET_VAR_FLOAT_TO_LVAR_INT:
	case COMMAND_CSET_LVAR_FLOAT_TO_VAR_INT:
	case COMMAND_CSET_LVAR_FLOAT_TO_LVAR_INT:
		return 1;
	default:
		return 0;
	}
}

int ScriptBC_VariableOp(int32_t command, uint8_t *space, int32_t *locals, uint32_t *ip)
{
	int result;
	int32_t *ptr = 0, *ptr2 = 0;

	switch (command) {
	case COMMAND_ADD_INT_VAR_TO_INT_VAR:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*ptr += *ptr2;
		break;
	}
	case COMMAND_ADD_INT_LVAR_TO_INT_VAR:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*ptr += *ptr2;
		break;
	}
	case COMMAND_ADD_INT_VAR_TO_INT_LVAR:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*ptr += *ptr2;
		break;
	}
	case COMMAND_ADD_INT_LVAR_TO_INT_LVAR:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*ptr += *ptr2;
		break;
	}
	case COMMAND_ADD_FLOAT_VAR_TO_FLOAT_VAR:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ScriptBC_SetF(ptr, ScriptBC_F(ptr) + ScriptBC_F(ptr2));
		break;
	}
	case COMMAND_ADD_FLOAT_LVAR_TO_FLOAT_VAR:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ScriptBC_SetF(ptr, ScriptBC_F(ptr) + ScriptBC_F(ptr2));
		break;
	}
	case COMMAND_ADD_FLOAT_VAR_TO_FLOAT_LVAR:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ScriptBC_SetF(ptr, ScriptBC_F(ptr) + ScriptBC_F(ptr2));
		break;
	}
	case COMMAND_ADD_FLOAT_LVAR_TO_FLOAT_LVAR:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ScriptBC_SetF(ptr, ScriptBC_F(ptr) + ScriptBC_F(ptr2));
		break;
	}
	case COMMAND_SUB_INT_VAR_FROM_INT_VAR:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*ptr -= *ptr2;
		break;
	}
	case COMMAND_SUB_INT_LVAR_FROM_INT_LVAR:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*ptr -= *ptr2;
		break;
	}
	case COMMAND_SUB_FLOAT_VAR_FROM_FLOAT_VAR:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ScriptBC_SetF(ptr, ScriptBC_F(ptr) - ScriptBC_F(ptr2));
		break;
	}
	case COMMAND_SUB_FLOAT_LVAR_FROM_FLOAT_LVAR:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ScriptBC_SetF(ptr, ScriptBC_F(ptr) - ScriptBC_F(ptr2));
		break;
	}
	case COMMAND_SUB_INT_LVAR_FROM_INT_VAR:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*ptr -= *ptr2;
		break;
	}
	case COMMAND_SUB_INT_VAR_FROM_INT_LVAR:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*ptr -= *ptr2;
		break;
	}
	case COMMAND_SUB_FLOAT_LVAR_FROM_FLOAT_VAR:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ScriptBC_SetF(ptr, ScriptBC_F(ptr) - ScriptBC_F(ptr2));
		break;
	}
	case COMMAND_SUB_FLOAT_VAR_FROM_FLOAT_LVAR:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ScriptBC_SetF(ptr, ScriptBC_F(ptr) - ScriptBC_F(ptr2));
		break;
	}
	case COMMAND_MULT_INT_VAR_BY_INT_VAR:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*ptr *= *ptr2;
		break;
	}
	case COMMAND_MULT_INT_VAR_BY_INT_LVAR:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*ptr *= *ptr2;
		break;
	}
	case COMMAND_MULT_INT_LVAR_BY_INT_VAR:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*ptr *= *ptr2;
		break;
	}
	case COMMAND_MULT_INT_LVAR_BY_INT_LVAR:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*ptr *= *ptr2;
		break;
	}
	case COMMAND_MULT_FLOAT_VAR_BY_FLOAT_VAR:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ScriptBC_SetF(ptr, ScriptBC_F(ptr) * ScriptBC_F(ptr2));
		break;
	}
	case COMMAND_MULT_FLOAT_VAR_BY_FLOAT_LVAR:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ScriptBC_SetF(ptr, ScriptBC_F(ptr) * ScriptBC_F(ptr2));
		break;
	}
	case COMMAND_MULT_FLOAT_LVAR_BY_FLOAT_VAR:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ScriptBC_SetF(ptr, ScriptBC_F(ptr) * ScriptBC_F(ptr2));
		break;
	}
	case COMMAND_MULT_FLOAT_LVAR_BY_FLOAT_LVAR:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ScriptBC_SetF(ptr, ScriptBC_F(ptr) * ScriptBC_F(ptr2));
		break;
	}
	case COMMAND_DIV_INT_VAR_BY_INT_VAR:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*ptr /= *ptr2;
		break;
	}
	case COMMAND_DIV_INT_VAR_BY_INT_LVAR:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*ptr /= *ptr2;
		break;
	}
	case COMMAND_DIV_INT_LVAR_BY_INT_VAR:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*ptr /= *ptr2;
		break;
	}
	case COMMAND_DIV_INT_LVAR_BY_INT_LVAR:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*ptr /= *ptr2;
		break;
	}
	case COMMAND_DIV_FLOAT_VAR_BY_FLOAT_VAR:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ScriptBC_SetF(ptr, ScriptBC_F(ptr) / ScriptBC_F(ptr2));
		break;
	}
	case COMMAND_DIV_FLOAT_VAR_BY_FLOAT_LVAR:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ScriptBC_SetF(ptr, ScriptBC_F(ptr) / ScriptBC_F(ptr2));
		break;
	}
	case COMMAND_DIV_FLOAT_LVAR_BY_FLOAT_VAR:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ScriptBC_SetF(ptr, ScriptBC_F(ptr) / ScriptBC_F(ptr2));
		break;
	}
	case COMMAND_DIV_FLOAT_LVAR_BY_FLOAT_LVAR:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ScriptBC_SetF(ptr, ScriptBC_F(ptr) / ScriptBC_F(ptr2));
		break;
	}
	case COMMAND_SET_VAR_INT_TO_VAR_INT:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*ptr = *ptr2;
		break;
	}
	case COMMAND_SET_VAR_INT_TO_LVAR_INT:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*ptr = *ptr2;
		break;
	}
	case COMMAND_SET_LVAR_INT_TO_VAR_INT:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*ptr = *ptr2;
		break;
	}
	case COMMAND_SET_LVAR_INT_TO_LVAR_INT:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*ptr = *ptr2;
		break;
	}
	case COMMAND_SET_VAR_FLOAT_TO_VAR_FLOAT:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ScriptBC_SetF(ptr, ScriptBC_F(ptr2));
		break;
	}
	case COMMAND_SET_VAR_FLOAT_TO_LVAR_FLOAT:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ScriptBC_SetF(ptr, ScriptBC_F(ptr2));
		break;
	}
	case COMMAND_SET_LVAR_FLOAT_TO_VAR_FLOAT:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ScriptBC_SetF(ptr, ScriptBC_F(ptr2));
		break;
	}
	case COMMAND_SET_LVAR_FLOAT_TO_LVAR_FLOAT:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ScriptBC_SetF(ptr, ScriptBC_F(ptr2));
		break;
	}
	case COMMAND_CSET_VAR_INT_TO_VAR_FLOAT:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*ptr = (int32_t)ScriptBC_F(ptr2);
		break;
	}
	case COMMAND_CSET_VAR_INT_TO_LVAR_FLOAT:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*ptr = (int32_t)ScriptBC_F(ptr2);
		break;
	}
	case COMMAND_CSET_LVAR_INT_TO_VAR_FLOAT:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*ptr = (int32_t)ScriptBC_F(ptr2);
		break;
	}
	case COMMAND_CSET_LVAR_INT_TO_LVAR_FLOAT:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		*ptr = (int32_t)ScriptBC_F(ptr2);
		break;
	}
	case COMMAND_CSET_VAR_FLOAT_TO_VAR_INT:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ScriptBC_SetF(ptr, (float)*ptr2);
		break;
	}
	case COMMAND_CSET_VAR_FLOAT_TO_LVAR_INT:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ScriptBC_SetF(ptr, (float)*ptr2);
		break;
	}
	case COMMAND_CSET_LVAR_FLOAT_TO_VAR_INT:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_GLOBAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ScriptBC_SetF(ptr, (float)*ptr2);
		break;
	}
	case COMMAND_CSET_LVAR_FLOAT_TO_LVAR_INT:
	{
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		ScriptBC_SetF(ptr, (float)*ptr2);
		break;
	}
	default:
		return SCRIPTBC_BAD_ARGUMENT;
	}

	return SCRIPTBC_OK;
}

int ScriptBC_IsTimedOpCommand(int32_t command)
{
	switch (command) {
	case COMMAND_ADD_TIMED_VAL_TO_FLOAT_VAR:
	case COMMAND_ADD_TIMED_VAL_TO_FLOAT_LVAR:
	case COMMAND_ADD_TIMED_FLOAT_VAR_TO_FLOAT_VAR:
	case COMMAND_ADD_TIMED_FLOAT_VAR_TO_FLOAT_LVAR:
	case COMMAND_ADD_TIMED_FLOAT_LVAR_TO_FLOAT_VAR:
	case COMMAND_ADD_TIMED_FLOAT_LVAR_TO_FLOAT_LVAR:
	case COMMAND_SUB_TIMED_VAL_FROM_FLOAT_VAR:
	case COMMAND_SUB_TIMED_VAL_FROM_FLOAT_LVAR:
	case COMMAND_SUB_TIMED_FLOAT_VAR_FROM_FLOAT_VAR:
	case COMMAND_SUB_TIMED_FLOAT_VAR_FROM_FLOAT_LVAR:
	case COMMAND_SUB_TIMED_FLOAT_LVAR_FROM_FLOAT_VAR:
	case COMMAND_SUB_TIMED_FLOAT_LVAR_FROM_FLOAT_LVAR:
	case COMMAND_ABS_VAR_INT:
	case COMMAND_ABS_LVAR_INT:
	case COMMAND_ABS_VAR_FLOAT:
	case COMMAND_ABS_LVAR_FLOAT:
		return 1;
	default:
		return 0;
	}
}

/* Adds or subtracts the time step times a value to the float variable at ptr */
static void ScriptBC_AddTimed(int32_t *ptr, float timeStep, float value, int subtract)
{
	if (subtract)
		ScriptBC_SetF(ptr, ScriptBC_F(ptr) - timeStep * value);
	else
		ScriptBC_SetF(ptr, ScriptBC_F(ptr) + timeStep * value);
}

int ScriptBC_TimedOp(int32_t command, uint8_t *space, int32_t varSpaceSize, int32_t *locals, int32_t numLocals, uint32_t *ip, int32_t *params, float timeStep)
{
	int result;
	int32_t *ptr = 0, *ptr2 = 0;
	int subtract = 0;
	int dstLocal = 0, srcLocal = 0;

	switch (command) {
	case COMMAND_SUB_TIMED_VAL_FROM_FLOAT_VAR:
		subtract = 1;
		/* fall through */
	case COMMAND_ADD_TIMED_VAL_TO_FLOAT_VAR:
		dstLocal = 0;
		goto value_form;
	case COMMAND_SUB_TIMED_VAL_FROM_FLOAT_LVAR:
		subtract = 1;
		/* fall through */
	case COMMAND_ADD_TIMED_VAL_TO_FLOAT_LVAR:
		dstLocal = 1;
		goto value_form;

	case COMMAND_SUB_TIMED_FLOAT_VAR_FROM_FLOAT_VAR:
		subtract = 1;
		/* fall through */
	case COMMAND_ADD_TIMED_FLOAT_VAR_TO_FLOAT_VAR:
		dstLocal = 0; srcLocal = 0;
		goto variable_form;
	case COMMAND_SUB_TIMED_FLOAT_LVAR_FROM_FLOAT_LVAR:
		subtract = 1;
		/* fall through */
	case COMMAND_ADD_TIMED_FLOAT_LVAR_TO_FLOAT_LVAR:
		dstLocal = 1; srcLocal = 1;
		goto variable_form;
#ifdef FIX_BUGS
	case COMMAND_SUB_TIMED_FLOAT_VAR_FROM_FLOAT_LVAR:
		subtract = 1;
		/* fall through */
	case COMMAND_ADD_TIMED_FLOAT_VAR_TO_FLOAT_LVAR:
		dstLocal = 0; srcLocal = 1;
		goto variable_form;
	case COMMAND_SUB_TIMED_FLOAT_LVAR_FROM_FLOAT_VAR:
		subtract = 1;
		/* fall through */
	case COMMAND_ADD_TIMED_FLOAT_LVAR_TO_FLOAT_VAR:
		dstLocal = 1; srcLocal = 0;
		goto variable_form;
#else
	case COMMAND_SUB_TIMED_FLOAT_LVAR_FROM_FLOAT_VAR:
		subtract = 1;
		/* fall through */
	case COMMAND_ADD_TIMED_FLOAT_LVAR_TO_FLOAT_VAR:
		dstLocal = 0; srcLocal = 1;
		goto variable_form;
	case COMMAND_SUB_TIMED_FLOAT_VAR_FROM_FLOAT_LVAR:
		subtract = 1;
		/* fall through */
	case COMMAND_ADD_TIMED_FLOAT_VAR_TO_FLOAT_LVAR:
		dstLocal = 1; srcLocal = 0;
		goto variable_form;
#endif

	case COMMAND_ABS_VAR_INT:
	case COMMAND_ABS_LVAR_INT:
	case COMMAND_ABS_VAR_FLOAT:
	case COMMAND_ABS_LVAR_FLOAT:
		ptr = ScriptBC_GetPointerToVariable(space, locals, ip, (command == COMMAND_ABS_VAR_INT || command == COMMAND_ABS_VAR_FLOAT) ? SCRIPTBC_VAR_GLOBAL : SCRIPTBC_VAR_LOCAL, &result);
		if (result != SCRIPTBC_OK)
			return result;
		if (command == COMMAND_ABS_VAR_INT || command == COMMAND_ABS_LVAR_INT) {
			*ptr = (((*ptr) < 0) ? (-(*ptr)) : (*ptr));
		} else {
			float f = ScriptBC_F(ptr);

			ScriptBC_SetF(ptr, (((f) < 0) ? (-(f)) : (f)));
		}
		return SCRIPTBC_OK;
	default:
		return SCRIPTBC_BAD_ARGUMENT;
	}

value_form:
	/* The variable and then the value are given */
	ptr = ScriptBC_GetPointerToVariable(space, locals, ip, dstLocal ? SCRIPTBC_VAR_LOCAL : SCRIPTBC_VAR_GLOBAL, &result);
	if (result != SCRIPTBC_OK)
		return result;
	result = ScriptBC_CollectParameters(space, varSpaceSize, locals, numLocals, ip, 1, params);
	if (result != SCRIPTBC_OK)
		return result;
	ScriptBC_AddTimed(ptr, timeStep, ScriptBC_F(&params[0]), subtract);
	return SCRIPTBC_OK;

variable_form:
	/* Two variables are given */
	ptr = ScriptBC_GetPointerToVariable(space, locals, ip, dstLocal ? SCRIPTBC_VAR_LOCAL : SCRIPTBC_VAR_GLOBAL, &result);
	if (result != SCRIPTBC_OK)
		return result;
	ptr2 = ScriptBC_GetPointerToVariable(space, locals, ip, srcLocal ? SCRIPTBC_VAR_LOCAL : SCRIPTBC_VAR_GLOBAL, &result);
	if (result != SCRIPTBC_OK)
		return result;
	ScriptBC_AddTimed(ptr, timeStep, ScriptBC_F(ptr2), subtract);
	return SCRIPTBC_OK;
}

void ScriptBC_Jump(uint32_t *ip, int32_t target, int32_t mainScriptSize)
{
	*ip = target >= 0 ? target : mainScriptSize - target;
}

int ScriptBC_Gosub(uint32_t *stack, uint16_t *stackPointer, int32_t maxDepth, uint32_t *ip, int32_t target, int32_t mainScriptSize)
{
	if (!(*stackPointer < maxDepth))
		return SCRIPTBC_STACK_OVERFLOW;

	// Remember where to come back to and jump
	stack[(*stackPointer)++] = *ip;
	ScriptBC_Jump(ip, target, mainScriptSize);
	return SCRIPTBC_OK;
}

int ScriptBC_Return(const uint32_t *stack, uint16_t *stackPointer, uint32_t *ip)
{
	if (!(*stackPointer > 0))
		return SCRIPTBC_STACK_UNDERFLOW;

	*ip = stack[--(*stackPointer)];
	return SCRIPTBC_OK;
}

int ScriptBC_AndOr(int32_t param, uint16_t *andOrState, int *condResult)
{
	*andOrState = (uint16_t)param;
	if (*andOrState == SCRIPTBC_ANDOR_NONE) {
		*condResult = 0; /* pointless */
	} else if (*andOrState >= SCRIPTBC_ANDS_1 && *andOrState <= SCRIPTBC_ANDS_8) {
		*condResult = 1;
		(*andOrState)++;
	} else if (*andOrState >= SCRIPTBC_ORS_1 && *andOrState <= SCRIPTBC_ORS_8) {
		*condResult = 0;
		(*andOrState)++;
	} else {
		return SCRIPTBC_BAD_ANDOR_STATE;
	}

	return SCRIPTBC_OK;
}

void ScriptBC_CopyArgumentsToLocals(const uint8_t *space, const int32_t *locals, int32_t *newLocals, uint32_t *ip)
{
	int i;
	int8_t type = ScriptBC_Read1(space, ip);

	for (i = 0; type != SCRIPTBC_ARG_END; type = ScriptBC_Read1(space, ip), i++) {
		switch (type) {
		case SCRIPTBC_ARG_INT32:
			newLocals[i] = ScriptBC_Read4(space, ip);
			break;
		case SCRIPTBC_ARG_GLOBALVAR:
			newLocals[i] = *(const int32_t *)&space[(uint16_t)ScriptBC_Read2(space, ip)];
			break;
		case SCRIPTBC_ARG_LOCALVAR:
			newLocals[i] = locals[ScriptBC_Read2(space, ip)];
			break;
		case SCRIPTBC_ARG_INT8:
			newLocals[i] = ScriptBC_Read1(space, ip);
			break;
		case SCRIPTBC_ARG_INT16:
			newLocals[i] = ScriptBC_Read2(space, ip);
			break;
		case SCRIPTBC_ARG_FLOAT:
		{
			// Floats are fixed point numbers of 16ths in the script
			float tmp = ScriptBC_Read2(space, ip) / 16.0f;

			memcpy(&newLocals[i], &tmp, sizeof tmp);
			break;
		}
		default:
			break;
		}
	}
}

//- rouz edit (Claude)
