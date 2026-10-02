#include <stdio.h>
#include <stdlib.h>

#ifdef POOL_PREDEFINED_ASSERT
static unsigned int customAssertCalls;
static inline void
custom_assert(int passed)
{
	// Track calls to the assertion hook supplied by the header's caller
	customAssertCalls++;
	if (!passed) {
		// Fail quietly without invoking a Windows CRT dialog
		exit(71);
	}
}
#define assert(expression) custom_assert(!!(expression))
#endif

#include "../../src/core/Pool.h"
#include "../../src/core/Store.h"

typedef struct LogicalEntry { int32_t value; } LogicalEntry;
typedef struct PhysicalEntry { LogicalEntry base; uint32_t extra[7]; } PhysicalEntry;

static void
require(int passed, const char *message)
{
	// Identify failed invariants without relying on the library's assertions
	if (!passed) {
		// Report the failed check and stop the test
		fprintf(stderr, "%s\n", message);
		exit(1);
	}
}

static void
test_store(void)
{
	// Bind fixed storage and allocate each existing entry exactly once
	PhysicalEntry entries[3];
	CStore store = { 0, entries, sizeof(entries[0]), 3 };
	memset(entries, 0x55, sizeof(entries));
	for (int32_t i = 0; i < 3; i++) {
		// Verify allocation, lookup, and indexing all use the physical stride
		require(CStore_Alloc(&store) == &entries[i], "Store allocation order changed");
		require(CStore_GetItem(&store, i) == &entries[i], "Store lookup stride changed");
		require(CStore_GetIndex(&store, &entries[i]) == i, "Store indexing changed");
	}
	// Clearing a store must preserve all existing entry bytes
	CStore_Clear(&store);
	require(store.allocPtr == 0, "Store clear did not reset its cursor");
	require(((uint8_t*)entries)[0] == 0x55 && ((uint8_t*)entries)[sizeof(entries) - 1] == 0x55, "Store clear modified entries");
	require(CStore_GetItem(&store, 2) == &entries[2], "Store lookup unexpectedly depends on allocation count");
	require(CStore_Alloc(&store) == &entries[0], "Store did not reuse its first entry");
}

static void
test_allocation(void)
{
	// Allocate logical entries from larger physical storage slots
	CPool pool;
	require(CPool_Init(&pool, 3, sizeof(PhysicalEntry), "Test"), "Pool initialization failed");
	require(CPool_GetSize(&pool) == 3 && CPool_GetMaxEntrySize(&pool) == sizeof(PhysicalEntry), "Physical pool capacity changed");
	require(CPool_GetNoOfUsedSpaces(&pool) == 0 && CPool_GetNoOfFreeSpaces(&pool) == 3, "Initial occupancy changed");
	LogicalEntry *entries[3];
	for (int32_t i = 0; i < 3; i++) {
		// Preserve initial generation zero and advance it when reserving a slot
		require(CPool_GetId(&pool, i) == 0 && CPool_GetIsFree(&pool, i), "Initial slot flags changed");
		entries[i] = (LogicalEntry*)CPool_New(&pool);
		require((uint8_t*)entries[i] == pool.m_entries + i * sizeof(PhysicalEntry), "Allocation stride changed");
		require(CPool_GetJustIndex(&pool, entries[i]) == i, "Occupied slot index changed");
		require(CPool_GetIndex(&pool, entries[i]) == (i << 8) + 1, "Handle encoding changed");
		require(CPool_GetSlot(&pool, i) == entries[i], "Slot lookup changed");
	}
	// Reject exhausted pools and invalidate a freed slot's old handle
	require(CPool_New(&pool) == NULL, "Full pool did not report exhaustion");
	require(CPool_GetNoOfUsedSpaces(&pool) == 3 && CPool_GetNoOfFreeSpaces(&pool) == 0, "Full pool count changed");
	int32_t oldHandle = CPool_GetIndex(&pool, entries[0]);
	CPool_Delete(&pool, entries[0]);
	require(CPool_GetSlot(&pool, 0) == NULL && CPool_GetAt(&pool, oldHandle) == NULL, "Freed handle remained active");
	require(CPool_GetId(&pool, 0) == 1, "Delete changed the slot generation");
	require(CPool_New(&pool) == entries[0], "Allocation did not wrap to the freed slot");
	require(CPool_GetIndex(&pool, entries[0]) == 2 && CPool_GetAt(&pool, oldHandle) == NULL, "Reused slot did not invalidate its stale handle");
#ifdef FIX_BUGS
	// Reject the invalid-handle sentinel before inspecting pool flags
	require(CPool_GetAt(&pool, -1) == NULL, "Invalid handle sentinel changed");
#endif
	// Preserve signed probes without dereferencing a foreign entry pointer
	require(CPool_GetJustIndex_NoFreeAssert(&pool, pool.m_entries + 3 * sizeof(PhysicalEntry)) == 3, "One-past slot index changed");
	require(CPool_GetJustIndex_NoFreeAssert(&pool, (void*)((uintptr_t)pool.m_entries - sizeof(PhysicalEntry))) == -1, "Negative slot probe changed");
	require(CPool_GetJustIndex_NoFreeAssert(&pool, (void*)((uintptr_t)pool.m_entries - 1)) < 0, "Foreign pointer aliased the first slot");
	CPool_Flush(&pool);
	require(CPool_New(&pool) == NULL && CPool_GetSize(&pool) == 0, "Flushed pool did not report exhaustion");
	require(CPool_GetMaxEntrySize(&pool) == sizeof(PhysicalEntry), "Flush lost the physical entry size");
	CPool_Flush(&pool);
}

static void
test_handles(void)
{
	// Restore an explicit handle and preserve the legacy cursor search order
	CPool pool;
	require(CPool_Init(&pool, 3, sizeof(PhysicalEntry), "Handles"), "Handle pool initialization failed");
	void *restored = CPool_NewAt(&pool, (1 << 8) | 0x45);
	require(restored == pool.m_entries + sizeof(PhysicalEntry), "Saved-handle slot changed");
	require(CPool_GetAt(&pool, (1 << 8) | 0x45) == restored, "Saved generation was not restored");
	require(pool.m_allocPtr == 0, "Handle restoration changed the first-free cursor");
	require(CPool_New(&pool) == pool.m_entries + 2 * sizeof(PhysicalEntry), "Allocation after handle restoration changed");
	CPool_SetNotFreeAt(&pool, 0x7f);
	require(CPool_GetId(&pool, 0) == 0x7f && !CPool_GetIsFree(&pool, 0), "Explicit handle flags changed");
#ifdef FIX_BUGS
	// Preserve corrected wrapping when saved handles have filled the entire pool
	require(CPool_New(&pool) == NULL, "Full restored pool did not report exhaustion");
#endif
	// Preserve independent generation and free-bit updates
	CPool_SetIsFree(&pool, 0, 1);
	CPool_SetId(&pool, 0, 0x181);
	require(CPool_GetIsFree(&pool, 0) && CPool_GetId(&pool, 0) == 1, "Generation update changed the free bit");
	require(CPool_GetAt(&pool, 0x81) == pool.m_entries, "Full flag-byte lookup behavior changed");
	CPool_Flush(&pool);

	// Exercise the seven-bit generation wrap through repeated slot reuse
	require(CPool_Init(&pool, 1, sizeof(PhysicalEntry), "Generation"), "Generation pool initialization failed");
	for (int i = 1; i <= 256; i++) {
		// Reuse the same slot and verify generation wrap on every allocation
		void *entry = CPool_New(&pool);
		require(entry == pool.m_entries, "Single-slot allocation changed");
		require(CPool_GetId(&pool, 0) == (i & POOLFLAG_ID), "Generation did not wrap at seven bits");
		require(CPool_GetAt(&pool, i & POOLFLAG_ID) == entry, "Wrapped handle did not resolve");
		CPool_Delete(&pool, entry);
	}
	// Release the reused single-slot storage
	CPool_Flush(&pool);
}

static void
test_snapshots(void)
{
	// Fill every slot including unused physical storage before taking a snapshot
	CPool *pool = CPool_Create(3, sizeof(PhysicalEntry), "Snapshot");
	CPoolSnapshot snapshot;
	require(pool != NULL, "Pool manager creation failed");
	CPool_InitSnapshot(&snapshot);
	uint8_t expected[3 * sizeof(PhysicalEntry)];
	for (size_t i = 0; i < sizeof(expected); i++) {
		// Give each stored byte an independent pattern including slot padding
		expected[i] = (uint8_t)(i * 37 + 11);
	}
	// Preserve separate flag and entry allocations in replay storage
	memcpy(pool->m_entries, expected, sizeof(expected));
	void *first = CPool_New(pool);
	void *second = CPool_New(pool);
	CPool_Store(pool, &snapshot);
	require(snapshot.flags != pool->m_flags && snapshot.entries != pool->m_entries, "Snapshot reused live pool memory");
	require(memcmp(snapshot.entries, expected, sizeof(expected)) == 0, "Snapshot omitted physical entry bytes");
	require(((uint8_t*)snapshot.flags)[0] == 1 && ((uint8_t*)snapshot.flags)[1] == 1 && ((uint8_t*)snapshot.flags)[2] == POOLFLAG_ISFREE, "Snapshot flags changed");
	uint8_t *originalEntries = pool->m_entries;
	CPool_Delete(pool, first);
	memset(pool->m_entries, 0, sizeof(expected));
	require(memcmp(snapshot.entries, expected, sizeof(expected)) == 0, "Live writes changed snapshot storage");
	CPool_CopyBack(pool, &snapshot);
	require(pool->m_entries == originalEntries && memcmp(pool->m_entries, expected, sizeof(expected)) == 0, "Snapshot restore replaced or changed live storage");
	require(CPool_GetSlot(pool, 0) == first && CPool_GetSlot(pool, 1) == second, "Snapshot restore changed occupied slots");
	require(CPool_GetNoOfUsedSpaces(pool) == 2 && pool->m_allocPtr == 0, "Snapshot restore changed occupancy or cursor");
	require(snapshot.flags == NULL && snapshot.entries == NULL, "Consumed snapshot was not cleared");
	CPool_ClearStorage(&snapshot);
	CPool_Store(pool, &snapshot);
	CPool_ClearStorage(&snapshot);
	CPool_Destroy(pool);

	// Handle empty snapshots and repeated cleanup without zero-byte null copies
	pool = CPool_Create(0, sizeof(PhysicalEntry), "Empty");
	require(pool != NULL && CPool_New(pool) == NULL, "Empty pool allocation changed");
	CPool_Store(pool, &snapshot);
	CPool_CopyBack(pool, &snapshot);
	require(snapshot.flags == NULL && snapshot.entries == NULL, "Empty snapshot retained storage");
	CPool_Destroy(pool);
	CPool_Destroy(NULL);
}

int
main(void)
{
	// Exercise storage semantics independently of any game or graphics runtime
	test_store();
	test_allocation();
	test_handles();
	test_snapshots();
#ifdef POOL_PREDEFINED_ASSERT
	// Ensure both headers preserve the caller's assertion hook
	(void)custom_assert;
	require(customAssertCalls > 0, "Pool or store replaced the assertion hook");
#endif
	// Report completion after all allocation and snapshot invariants pass
	puts("Pool and store tests passed");
	return 0;
}
