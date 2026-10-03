AudioJobQueue.h replaces the global stream-processing and captured-cleanup STL
queues with two intrusive C FIFOs. Requests remain unbounded and duplicate stream
requests are retained. Each FIFO maintains independent arrival order. The worker
still consumes at most one close request before one processing request on each
scheduling pass.

FlagAsToBeProcessed allocates request storage directly at the owning call site,
captures the stream or decoder/buffer pair under the existing scheduler lock,
and publishes it before notification. The C queue functions do not allocate,
free, lock or dereference stream pointers. Pop transfers node ownership to the
consumer; processing nodes are freed before decoding and cleanup nodes after
their captured resources are released. Allocation failure aborts rather than
silently dropping work.

Shutdown now closes streams, joins the worker, discards pending process nodes,
and releases pending decoder/buffer pairs before shutting down MPG123 or OpenAL.
The previous ordering could destroy the libraries while unlocked decoding was
still running and leave captured close requests unprocessed. Stream objects are
freed only after the worker has joined. Initialise resets the shutdown flag to
allow later subsystem initialization.

Run utils/tests/test_audio_job_queue.ps1 for C11/C++17 under clang and gcc at O0/O2.
Each run checks 4096-node FIFOs with duplicate stream pointers, captured cleanup
pairs, empty transitions and one million node reuse cycles. The worker fixture
from test_audio_buffer_queue.ps1 uses actual production worker and lifecycle
functions. It forces shutdown while decoding is paused, queues 100 duplicate
process requests and two close requests, and verifies library lifetime, exact
decoder destruction, empty queues and zero outstanding explicit allocations.
Existing reset, recycling and synchronous cutscene cases remain covered.

Stream state and functions now use the C API in stream.h. Its thread, mutexes
and wake signal use the C API in AudioThread.h. Real-device audio is not exercised by these tests.
