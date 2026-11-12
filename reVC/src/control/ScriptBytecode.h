//+ rouz edit (Claude)
#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
// Decoding of the arguments of script commands from the script byte code, the script space is a byte array and ip an offset into it that moves past what is read
// Argument types in the byte code
#define SCRIPTBC_ARG_END 0
#define SCRIPTBC_ARG_INT32 1
#define SCRIPTBC_ARG_GLOBALVAR 2
#define SCRIPTBC_ARG_LOCALVAR 3
#define SCRIPTBC_ARG_INT8 4
#define SCRIPTBC_ARG_INT16 5
#define SCRIPTBC_ARG_FLOAT 6
// Kinds of variables
#define SCRIPTBC_VAR_LOCAL 1
#define SCRIPTBC_VAR_GLOBAL 2
// Results, anything but OK is a corrupt script, the decoding stops at it
#define SCRIPTBC_OK 0
#define SCRIPTBC_BAD_GLOBAL_INDEX 1
#define SCRIPTBC_BAD_LOCAL_INDEX 2
#define SCRIPTBC_BAD_ARGUMENT 3
#define SCRIPTBC_BAD_VARIABLE_TYPE 4
#define SCRIPTBC_STACK_OVERFLOW 5
#define SCRIPTBC_STACK_UNDERFLOW 6
#define SCRIPTBC_BAD_ANDOR_STATE 7

int32_t ScriptBC_Read4(const uint8_t *space, uint32_t *ip);
int16_t ScriptBC_Read2(const uint8_t *space, uint32_t *ip);
int8_t ScriptBC_Read1(const uint8_t *space, uint32_t *ip);

// Reads total arguments into params, variables are replaced by their values, varSpaceSize is the size of the global variable space
int ScriptBC_CollectParameters(const uint8_t *space, int32_t varSpaceSize, const int32_t *locals, int32_t numLocals, uint32_t *ip, int16_t total, int32_t *params);
// Value of the argument at ip without moving the ip, -1 when it is corrupt
int32_t ScriptBC_CollectNextParameterWithoutIncreasingPC(const uint8_t *space, const int32_t *locals, uint32_t ip, int *result);
// Writes number values to the variables the arguments at ip name
int ScriptBC_StoreParameters(uint8_t *space, int32_t *locals, uint32_t *ip, int16_t number, const int32_t *params);
// Pointer to the variable the argument at ip names, NULL when it is not of the wanted kind
int32_t *ScriptBC_GetPointerToVariable(uint8_t *space, int32_t *locals, uint32_t *ip, int16_t type, int *result);
// Reads the next command: the number without the not flag, which is returned separately
int32_t ScriptBC_ReadCommand(const uint8_t *space, uint32_t *ip, int *notFlag);
// Which group of a hundred commands a command is in, 0 for commands 0 to 99, 14 for 1400 to 1499 and -1 for the commands past that
int ScriptBC_CommandGroup(int32_t command);
// Combines the result of a comparison command into the condition result of a script: the and or state is 0 for none, 1 to 8 for a condition made of up to 8 ands and 21 to 28 for up to 8 ors, it counts down as the comparisons come in
#define SCRIPTBC_ANDOR_NONE 0
#define SCRIPTBC_ANDS_1 1
#define SCRIPTBC_ANDS_8 8
#define SCRIPTBC_ORS_1 21
#define SCRIPTBC_ORS_8 28
void ScriptBC_UpdateCompareFlag(int flag, int notFlag, uint16_t *andOrState, int *condResult);
// The commands that set a variable to a value or add, subtract, multiply or divide it by a value, from COMMAND_SET_VAR_INT to COMMAND_DIV_FLOAT_LVAR_BY_VAL: they name the variable and then give the value, the result is a decoding result
int ScriptBC_Arithmetic(int32_t command, uint8_t *space, int32_t varSpaceSize, int32_t *locals, int32_t numLocals, uint32_t *ip, int32_t *params);
// The commands that compare two values, variables or numbers: whether a command is one of them, and the result of the comparison, the arguments are decoded in the order the command gives them
int ScriptBC_IsCompareCommand(int32_t command);
int ScriptBC_Compare(int32_t command, uint8_t *space, int32_t varSpaceSize, int32_t *locals, int32_t numLocals, uint32_t *ip, int32_t *params, int *flag);
// The commands that set a variable to another or add, subtract, multiply or divide it by another, with conversions between int and float for the c set commands: whether a command is one of them and the operation itself on two variables
int ScriptBC_IsVariableOpCommand(int32_t command);
int ScriptBC_VariableOp(int32_t command, uint8_t *space, int32_t *locals, uint32_t *ip);
// The commands that add or subtract a value or a variable multiplied by the time step to a float variable, and those that take the absolute value of a variable
int ScriptBC_IsTimedOpCommand(int32_t command);
int ScriptBC_TimedOp(int32_t command, uint8_t *space, int32_t varSpaceSize, int32_t *locals, int32_t numLocals, uint32_t *ip, int32_t *params, float timeStep);
// Jumps of a script: the target of a jump is an offset in the main script, a negative one counts back from the end of the main script area, the return stack holds the places to return to
void ScriptBC_Jump(uint32_t *ip, int32_t target, int32_t mainScriptSize);
int ScriptBC_Gosub(uint32_t *stack, uint16_t *stackPointer, int32_t maxDepth, uint32_t *ip, int32_t target, int32_t mainScriptSize);
int ScriptBC_Return(const uint32_t *stack, uint16_t *stackPointer, uint32_t *ip);
// Starts a condition of several comparisons: sets the and or state from the command and the condition result to its start value
int ScriptBC_AndOr(int32_t param, uint16_t *andOrState, int *condResult);
// Copies the arguments after a command that starts a script, until the end of the arguments, to the local variables of the new script
void ScriptBC_CopyArgumentsToLocals(const uint8_t *space, const int32_t *locals, int32_t *newLocals, uint32_t *ip);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
