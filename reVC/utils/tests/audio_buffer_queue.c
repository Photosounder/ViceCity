#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../../src/audio/oal/AudioBufferQueue.h"

static uint32_t random_state = 42;
static uint32_t next_random(void)
{
    // Generate a reproducible operation sequence
    random_state = random_state * 1664525u + 1013904223u;
    return random_state;
}

int main(void)
{
    // Verify empty operations preserve outputs and initialization preserves unused storage
    AudioBufferQueue queue;
    AudioBufferPair output = {99, 100};
    memset(&queue, 0x5a, sizeof(queue));
    AudioBufferQueue_Init(&queue);
    assert(queue.entries[0].left == 0x5a5a5a5au);
    assert(AudioBufferQueue_IsEmpty(&queue));
    assert(!AudioBufferQueue_Peek(&queue, &output));
    assert(!AudioBufferQueue_Pop(&queue, &output));
    assert(output.left == 99 && output.right == 100);

    // Compare a million mixed operations against a separate linear FIFO model
    AudioBufferPair model[4];
    unsigned count = 0;
    for(unsigned i = 0; i < 1000000; i++) {
        uint32_t op = next_random() >> 24;
        if(op < 128) {
            AudioBufferPair pair = {i, i + 1000000};
            AudioBufferQueue before = queue;
            bool pushed = AudioBufferQueue_Push(&queue, pair);
            assert(pushed == (count < 4));
            if(pushed) model[count++] = pair;
            else assert(memcmp(&queue, &before, sizeof(queue)) == 0);
        } else if(op < 240) {
            uint64_t revision = queue.revision;
            bool popped = AudioBufferQueue_Pop(&queue, &output);
            assert(popped == (count > 0));
            if(popped) {
                assert(output.left == model[0].left && output.right == model[0].right);
                memmove(model, model + 1, --count * sizeof(model[0]));
                assert(!AudioBufferQueue_IsCurrent(&queue, revision));
            }
        } else {
            uint64_t revision = queue.revision;
            AudioBufferQueue_Clear(&queue);
            count = 0;
            assert(queue.revision == revision + 1);
            assert(!AudioBufferQueue_IsCurrent(&queue, revision));
        }
        assert(queue.count == count && queue.head < 4);
        assert(AudioBufferQueue_IsEmpty(&queue) == (count == 0));
        if(count) {
            assert(AudioBufferQueue_Peek(&queue, &output));
            assert(output.left == model[0].left && output.right == model[0].right);
        }
    }

    // Reproduce decoding while another thread appends, resets and reuses the same ring slot
    AudioBufferQueue_Clear(&queue);
    AudioBufferPair first = {1, 2}, second = {3, 4};
    assert(AudioBufferQueue_Push(&queue, first));
    uint64_t token = queue.revision;
    assert(AudioBufferQueue_Push(&queue, second));
    assert(AudioBufferQueue_IsCurrent(&queue, token));
    AudioBufferQueue_Clear(&queue);
    assert(AudioBufferQueue_Push(&queue, first));
    assert(!AudioBufferQueue_IsCurrent(&queue, token));
    token = queue.revision;
    assert(AudioBufferQueue_Pop(&queue, &output));
    assert(AudioBufferQueue_Push(&queue, first));
    assert(!AudioBufferQueue_IsCurrent(&queue, token));

    // Verify unsigned revision rollover remains defined and invalidates the preceding token
    queue.revision = UINT64_MAX;
    AudioBufferQueue_Clear(&queue);
    assert(queue.revision == 0);
    assert(AudioBufferQueue_Push(&queue, first));
    assert(!AudioBufferQueue_IsCurrent(&queue, UINT64_MAX));
    puts("Audio queue FIFO, capacity and reset tests passed");
    return 0;
}
