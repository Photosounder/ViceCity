#include <assert.h>
#include <stdio.h>
#include "../../src/audio/oal/AudioThread.h"

typedef struct Work {
    AudioMutex mutex;
    AudioWakeSignal wake;
    unsigned pending, processed;
    bool stopping;
} Work;

static void consume(void *opaque)
{
    // Consume published work while rechecking the predicate after every wake
    Work *work = (Work*)opaque;
    AudioMutex_Lock(&work->mutex);
    for(;;) {
        while(!work->pending && !work->stopping)
            AudioWakeSignal_Wait(&work->wake, &work->mutex);
        if(!work->pending && work->stopping) break;
        work->processed += work->pending;
        work->pending = 0;
    }
    AudioMutex_Unlock(&work->mutex);
}
static void produce(void *opaque)
{
    // Publish from independent producers without losing notifications around waits
    Work *work = (Work*)opaque;
    for(unsigned i = 0; i < 10000; i++) {
        AudioMutex_Lock(&work->mutex);
        work->pending++;
        AudioMutex_Unlock(&work->mutex);
        AudioWakeSignal_Notify(&work->wake);
    }
}
int main(void)
{
    // Repeatedly initialize native synchronization, run work and join before destruction
    for(unsigned cycle = 0; cycle < 20; cycle++) {
        Work work;
        work.pending = work.processed = 0;
        work.stopping = false;
        assert(AudioMutex_Init(&work.mutex));
        assert(AudioWakeSignal_Init(&work.wake));
        AudioThread worker, producers[3];
        assert(AudioThread_Start(&worker, consume, &work));
        for(unsigned i = 0; i < 3; i++) assert(AudioThread_Start(&producers[i], produce, &work));
        for(unsigned i = 0; i < 3; i++) AudioThread_Join(&producers[i]);
        AudioMutex_Lock(&work.mutex);
        work.stopping = true;
        AudioMutex_Unlock(&work.mutex);
        AudioWakeSignal_Notify(&work.wake);
        AudioThread_Join(&worker);
        assert(work.processed == 30000 && work.pending == 0);
        AudioWakeSignal_Destroy(&work.wake);
        AudioMutex_Destroy(&work.mutex);
    }
    puts("C threading lifecycle and 600000 concurrent publications passed");
    return 0;
}
