#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "../../src/core/IntStack.h"
#include "../../src/control/Route.h"
#include "../../src/core/Validate.h"

struct CPathNode { int32_t id; };
typedef struct LayoutProbe { int32_t first; int32_t second; } LayoutProbe;

static void test_layout(void)
{
	// Keep the original array lengths, counters and eight-pointer route representation
	REVC_STATIC_ASSERT(sizeof(CIntStack16) == 68, "16-entry stack layout");
	REVC_STATIC_ASSERT(offsetof(CIntStack16, sp) == 64, "16-entry counter offset");
	REVC_STATIC_ASSERT(sizeof(CIntStack4000) == 16004, "4000-entry stack layout");
	REVC_STATIC_ASSERT(offsetof(CIntStack4000, sp) == 16000, "4000-entry counter offset");
	REVC_STATIC_ASSERT(sizeof(CRoute) == 8 * sizeof(struct CPathNode*), "Route layout");
	// Exercise both enabled and disabled size validation and unconditional offset validation
#ifdef UTIL_BAD_SIZE
	VALIDATE_SIZE(LayoutProbe, 9);
#else
	VALIDATE_SIZE(LayoutProbe, 8);
#endif
#ifdef UTIL_BAD_OFFSET
	VALIDATE_OFFSET(LayoutProbe, second, 8);
#else
	VALIDATE_OFFSET(LayoutProbe, second, 4);
#endif
}

static void test_stacks(void)
{
	// Initialize counters without zeroing or constructing the embedded arrays
	CIntStack16 small;
	CIntStack4000 large;
	int32_t smallValues[16];
	int32_t largeValues[4000];
	uint32_t i;
	memset(&small, 0xA5, sizeof(small));
	memset(&large, 0x5A, sizeof(large));
	memcpy(smallValues, small.values, sizeof(smallValues));
	memcpy(largeValues, large.values, sizeof(largeValues));
	CIntStack16_Init(&small);
	CIntStack4000_Init(&large);
	assert(small.sp == 0 && large.sp == 0);
	assert(memcmp(smallValues, small.values, sizeof(smallValues)) == 0);
	assert(memcmp(largeValues, large.values, sizeof(largeValues)) == 0);
	// Fill the complete capacities and read every value back in last-in first-out order
	for(i = 0; i < 16; i++)
		CIntStack16_Push(&small, i == 0 ? INT32_MIN : (int32_t)i);
	for(i = 0; i < 4000; i++)
		CIntStack4000_Push(&large, i == 0 ? INT32_MAX : -(int32_t)i);
	assert(small.sp == 16 && large.sp == 4000);
	for(i = 16; i > 0; i--)
		assert(CIntStack16_Pop(&small) == (i == 1 ? INT32_MIN : (int32_t)(i - 1)));
	for(i = 4000; i > 0; i--)
		assert(CIntStack4000_Pop(&large) == (i == 1 ? INT32_MAX : -(int32_t)(i - 1)));
	assert(small.sp == 0 && large.sp == 0);
	// Reuse slots after popping and verify reset only changes the counter
	CIntStack16_Push(&small, -7);
	CIntStack16_Push(&small, 12);
	assert(CIntStack16_Pop(&small) == 12);
	CIntStack16_Push(&small, -9);
	assert(CIntStack16_Pop(&small) == -9);
	assert(CIntStack16_Pop(&small) == -7);
	CIntStack4000_Push(&large, 37);
	CIntStack4000_Init(&large);
	assert(large.sp == 0 && large.values[0] == 37);
}

static void test_route(void)
{
	// Populate the concrete route through a pointer using its embedded eight-node array
	struct CPathNode nodes[8];
	CRoute route;
	CRoute *output = &route;
	int i;
	for(i = 0; i < 8; i++) {
		// Retain node addresses and route ordering without allocating any storage
		nodes[i].id = i;
		output->m_node[i] = &nodes[i];
	}
	for(i = 0; i < 8; i++)
		assert(route.m_node[i] == &nodes[i] && route.m_node[i]->id == i);
}

int main(void)
{
	// Validate the standalone C-compatible utility headers and exercise their operations
	test_layout();
	test_stacks();
	test_route();
	return 0;
}
