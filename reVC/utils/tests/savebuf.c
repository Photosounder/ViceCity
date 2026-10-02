#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef SAVE_BUFFER_PREDEFINED_ASSERT
static unsigned int customAssertCalls;
static inline void
custom_assert(int passed)
{
	// Count assertions to detect headers that replace the caller's assertion hook
	customAssertCalls++;
	if (!passed) {
		// Reject malformed headers quietly with the expected assertion status
		exit(71);
	}
}
#define assert(expression) custom_assert(!!(expression))
#endif

#include "../../src/save/SaveBuf.h"

#ifdef VALIDATE_SAVE_SIZE
int32_t _saveBufCount;
#endif

static void
require(int condition, const char *message)
{
	// Report a failed expectation without starting a GUI
	if (!condition) {
		// Print the failed invariant and stop the test
		fprintf(stderr, "%s\n", message);
		exit(1);
	}
}

static void
check_count(int32_t expected)
{
#ifdef VALIDATE_SAVE_SIZE
	// Check the optional byte counter independently of cursor arithmetic
	require(_saveBufCount == expected, "Incorrect validation count");
#else
	// Accept the same test sequence when validation is compiled out
	(void)expected;
#endif
}

static void
test_plain(void)
{
	// Serialize known values into a deliberately unaligned buffer
	uint8_t storage[64];
	uint8_t *start = storage + 1;
	uint8_t *buf = start;
	uint8_t byte = 0xa5;
	uint16_t half = 0x1234;
	int32_t word = -12345;
	float real = 1.5f;
	const uint16_t endian = 1;
	const uint8_t expectedLittle[] = {
		'T', 'E', 'S', 'T', 11, 0, 0, 0,
		0xa5, 0x34, 0x12, 0xc7, 0xcf, 0xff, 0xff, 0, 0, 0xc0, 0x3f
	};
	const uint8_t expectedBig[] = {
		'T', 'E', 'S', 'T', 0, 0, 0, 11,
		0xa5, 0x12, 0x34, 0xff, 0xff, 0xcf, 0xc7, 0x3f, 0xc0, 0, 0
	};
	const uint8_t *expected = *(const uint8_t*)&endian ? expectedLittle : expectedBig;
	memset(storage, 0xcc, sizeof(storage));
	INITSAVEBUF
	WriteSaveHeader(&buf, 'T', 'E', 'S', 'T', 11);
	WriteSaveBuf(&buf, &byte, sizeof(byte));
	WriteSaveBuf(&buf, &half, sizeof(half));
	void *saved = WriteSaveBuf(&buf, &word, sizeof(word));
	WriteSaveBuf(&buf, &real, sizeof(real));
	require(buf == start + sizeof(expectedLittle), "Incorrect write cursor");
	require(saved == start + 11, "Incorrect saved-value address");
	require(memcmp(start, expected, sizeof(expectedLittle)) == 0, "Changed serialized bytes");
	require(storage[0] == 0xcc && *buf == 0xcc, "Write crossed buffer boundary");
	check_count(19);

	// Patch through the returned address and read the modified saved value
	word = -12344;
	memcpy(saved, &word, sizeof(word));
	buf = start;
	INITSAVEBUF
	CheckSaveHeader(&buf, 'T', 'E', 'S', 'T', 11);
	byte = 0;
	half = 0;
	word = 0;
	real = 0;
	ReadSaveBuf(&byte, &buf, sizeof(byte));
	ReadSaveBuf(&half, &buf, sizeof(half));
	ReadSaveBuf(&word, &buf, sizeof(word));
	ReadSaveBuf(&real, &buf, sizeof(real));
	require(byte == 0xa5 && half == 0x1234 && word == -12344 && real == 1.5f, "Round trip failed");
	require(buf == start + 19, "Incorrect read cursor");
	check_count(19);

	// Preserve signed skips and validation when rewinding the cursor
	SkipSaveBuf(&buf, -4);
	require(buf == start + 15, "Signed skip failed");
	check_count(15);
}

static void
test_with_length(void)
{
	// Write a header, a record, and a boolean with an existing byte count
	struct Record { uint32_t x, y; } record = { 0x12345678, 0xabcdef01 }, loaded = { 0, 0 };
	uint8_t storage[64];
	uint8_t *start = storage + 1;
	uint8_t *buf = start;
	uint32_t length = 17;
	bool flag = true, loadedFlag = false;
	memset(storage, 0x55, sizeof(storage));
	INITSAVEBUF
	WriteSaveHeaderWithLength(&buf, &length, 'L', 'E', 'N', '\0', 9);
	void *saved = WriteSaveBufWithLength(&buf, &length, &record, sizeof(record));
	WriteSaveBufWithLength(&buf, &length, &flag, sizeof(flag));
	SkipSaveBufWithLength(&buf, &length, 3);
	require(saved == start + 8, "Incorrect length-tracking saved address");
	require(buf == start + 20 && length == 37, "Incorrect accumulated write length");
	require(start[17] == 0x55 && start[19] == 0x55, "Skip modified padding");
	check_count(20);

	// Preserve the original difference between checked and unchecked header lengths
	buf = start;
	length = 17;
	INITSAVEBUF
	CheckSaveHeaderWithLength(&buf, &length, 'L', 'E', 'N', '\0', 9);
#ifdef VALIDATE_SAVE_SIZE
	// Count checked headers in the accumulated length
	require(length == 25, "Checked header length changed");
#else
	// Leave unchecked header length unchanged as in the release macro
	require(length == 17, "Unchecked header length changed");
#endif
	// Read record and boolean bytes and account for trailing padding
	ReadSaveBufWithLength(&loaded, &buf, &length, sizeof(loaded));
	ReadSaveBufWithLength(&loadedFlag, &buf, &length, sizeof(loadedFlag));
	SkipSaveBufWithLength(&buf, &length, 3);
	require(loaded.x == record.x && loaded.y == record.y && loadedFlag, "Record round trip failed");
	require(buf == start + 20, "Incorrect accumulated read cursor");
#ifdef VALIDATE_SAVE_SIZE
	// Verify the final length including the validated header
	require(length == 37, "Incorrect checked read length");
#else
	// Verify the final length without the unchecked header
	require(length == 29, "Incorrect unchecked read length");
#endif
	// Verify all bytes contributed to the validation counter
	check_count(20);
}

#ifdef COMPATIBLE_SAVES
static void
test_padding(void)
{
	// Clear only the selected padding bytes and leave both guards intact
	uint8_t storage[16];
	uint8_t *buf = storage + 1;
	memset(storage, 0x7f, sizeof(storage));
	INITSAVEBUF
	ZeroSaveBuf(&buf, 5);
	require(buf == storage + 6, "Incorrect zero-padding cursor");
	require(storage[0] == 0x7f && storage[6] == 0x7f, "Padding crossed buffer boundary");
	for (size_t i = 1; i < 6; i++) {
		// Verify each compatibility padding byte was cleared
		require(storage[i] == 0, "Padding was not zeroed");
	}
	// Include the cleared bytes in validation
	check_count(5);
}
#endif

int
main(int argc, char **argv)
{
	// Exercise each buffer operation with real cursor and byte expectations
	test_plain();
	test_with_length();
#ifdef SAVE_BUFFER_PREDEFINED_ASSERT
	// Confirm checked headers use the supplied assertion hook
	(void)custom_assert;
#ifdef VALIDATE_SAVE_SIZE
	require(customAssertCalls == 10, "Buffer API replaced the caller's assertion hook");
#else
	require(customAssertCalls == 0, "Unchecked headers unexpectedly asserted");
#endif
#endif
#ifdef COMPATIBLE_SAVES
	// Exercise compatibility padding when enabled
	test_padding();
#endif
	// Allow quiet external checks of malformed header detection
	if (argc > 1) {
		// Corrupt either the magic or payload size before validating the header
		uint8_t storage[16];
		uint8_t *buf = storage;
		WriteSaveHeader(&buf, 'B', 'A', 'D', '\0', 0);
		storage[argv[1][0] == 'm' ? 0 : 4] = 1;
		buf = storage;
		if (strchr(argv[1], '-') != NULL) {
			// Validate a corrupt header through the length-tracking entry point
			uint32_t length = 0;
			CheckSaveHeaderWithLength(&buf, &length, 'B', 'A', 'D', '\0', 0);
		} else {
			// Validate a corrupt header through the plain entry point
			CheckSaveHeader(&buf, 'B', 'A', 'D', '\0', 0);
		}
	}
	// Report completion after all buffer invariants pass
	puts("Save buffer tests passed");
	return 0;
}
