//+ rouz edit (ChatGPT)
#include "NodeName_backend.h"
#include <stddef.h>
#include <string.h>

#include "NodeName.h"

static int32_t gPluginOffset;

enum
{
	ID_NODENAME = 0x0253F2FE,
};

#define NODENAMEEXT(o) ((char *)(o) + gPluginOffset)

void*
NodeNameConstructor(void *object, int32_t offsetInObject, int32_t sizeInObject)
{
	// Clear the registered slot only when the original global offset is positive
	if(gPluginOffset > 0)
		NODENAMEEXT(object)[0] = '\0';
	return object;
}

void*
NodeNameDestructor(void *object, int32_t offsetInObject, int32_t sizeInObject)
{
	// Retain the original no-op plugin destruction and object identity
	return object;
}

void*
NodeNameCopy(void *dstObject, const void *srcObject, int32_t offsetInObject, int32_t sizeInObject)
{
	// Copy exactly twenty-three bytes with the original padding and null return
	strncpy(NODENAMEEXT(dstObject), NODENAMEEXT(srcObject), 23);
	return NULL;
}

void*
NodeNameStreamRead(void *stream, int32_t binaryLength, void *object, int32_t offsetInObject, int32_t sizeInObject)
{
	// Read the requested bytes before terminating at the current registered offset
	NodeName_HostRead(stream, NODENAMEEXT(object), binaryLength);
	NODENAMEEXT(object)[binaryLength] = '\0';
	return stream;
}

void*
NodeNameStreamWrite(void *stream, int32_t binaryLength, const void *object, int32_t offsetInObject, int32_t sizeInObject)
{
	// Write the requested bytes and return the caller stream independently of SDK success
	NodeName_HostWrite(stream, NODENAMEEXT(object), binaryLength);
	return stream;
}

int32_t
NodeNameStreamGetSize(const void *object, int32_t offsetInObject, int32_t sizeInObject)
{
	// Measure the current registered name without changing the original callback arguments
	char *name = NODENAMEEXT(object);	// can't be NULL
	return name ? (int32_t)strlen(name) : 0;
}

bool
NodeNamePluginAttach(void)
{
	// Register object callbacks before stream callbacks and preserve the original failure result
	gPluginOffset = NodeName_HostRegister(24, ID_NODENAME);
	NodeName_HostRegisterStream(ID_NODENAME);
	return gPluginOffset != -1;
}

char*
GetFrameNodeName(void *frame)
{
	// Borrow the registered name slot unless plugin registration produced a negative offset
	if(gPluginOffset < 0)
		return NULL;
	return NODENAMEEXT(frame);
}
//- rouz edit (ChatGPT)
