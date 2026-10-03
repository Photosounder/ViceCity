#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../src/audio/oal/AudioJobQueue.h"

int main(void)
{
    // Preserve duplicate requests and independent FIFO order across an unbounded batch
    AudioProcessQueue process = {NULL, NULL};
    AudioCloseQueue close = {NULL, NULL};
    AudioProcessJob *processNodes[4096];
    AudioCloseJob *closeNodes[4096];
    int streams[3] = {1, 2, 3};
    AudioDecoder decoders[4];
    assert(AudioProcessQueue_IsEmpty(&process) && AudioCloseQueue_IsEmpty(&close));
    assert(AudioProcessQueue_Pop(&process) == NULL && AudioCloseQueue_Pop(&close) == NULL);
    for(unsigned i = 0; i < 4096; i++) {
        processNodes[i] = (AudioProcessJob*)malloc(sizeof(*processNodes[i]));
        closeNodes[i] = (AudioCloseJob*)malloc(sizeof(*closeNodes[i]));
        assert(processNodes[i] && closeNodes[i]);
        processNodes[i]->stream = &streams[i % 3];
        closeNodes[i]->decoder = &decoders[i % 4];
        closeNodes[i]->buffer = &streams[i % 3];
        AudioProcessQueue_Push(&process, processNodes[i]);
        AudioCloseQueue_Push(&close, closeNodes[i]);
    }
    for(unsigned i = 0; i < 4096; i++) {
        AudioProcessJob *processJob = AudioProcessQueue_Pop(&process);
        AudioCloseJob *closeJob = AudioCloseQueue_Pop(&close);
        assert(processJob == processNodes[i] && closeJob == closeNodes[i]);
        assert(processJob->stream == &streams[i % 3] && processJob->next == NULL);
        assert(closeJob->decoder == &decoders[i % 4]);
        assert(closeJob->buffer == &streams[i % 3] && closeJob->next == NULL);
        free(processJob);
        free(closeJob);
    }
    assert(process.head == NULL && process.tail == NULL);
    assert(close.head == NULL && close.tail == NULL);

    // Exercise node reuse and empty-to-nonempty transitions without hidden queue allocation
    AudioProcessJob reusableProcess;
    AudioCloseJob reusableClose;
    reusableProcess.stream = &streams[0];
    reusableClose.decoder = &decoders[0];
    reusableClose.buffer = NULL;
    for(unsigned i = 0; i < 1000000; i++) {
        AudioProcessQueue_Push(&process, &reusableProcess);
        AudioCloseQueue_Push(&close, &reusableClose);
        assert(AudioProcessQueue_Pop(&process) == &reusableProcess);
        assert(AudioCloseQueue_Pop(&close) == &reusableClose);
        assert(AudioProcessQueue_IsEmpty(&process) && AudioCloseQueue_IsEmpty(&close));
    }
    puts("C job queue FIFO, duplicate ownership and reuse tests passed");
    return 0;
}
