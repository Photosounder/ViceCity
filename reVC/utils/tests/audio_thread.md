# C audio threading

AudioThread.h provides caller-owned mutex, wake signal and thread state with
explicit initialization, start, join and destruction. The Windows implementation
uses CRITICAL_SECTION, an auto-reset event and _beginthreadex; other platforms
use pthreads. AUDIO_THREAD_USE_POSIX selects pthreads on Windows for verification.
There is no application allocation inside these helpers.

The wake signal supports the scheduler's single waiting worker. Callers publish
work or shutdown under the mutex, unlock, then notify. The worker rechecks its
predicate after every wake. The Windows event retains a notification across the
unlock/wait interval; a retained notification can cause one harmless extra wake.
The POSIX condition wait atomically releases the lock while waiting.

Stream locks replace scoped C++ guards with explicit lock/unlock calls. Scheduler
initialization precedes worker start. Shutdown joins the worker, drains captured
cleanup, destroys scheduler synchronization, then shuts down decoder libraries.
Stream mutex destruction occurs after the stream closes and the scheduler joins.

Run `utils/tests/test_audio_thread.ps1` for 16 C11/C++17 combinations of clang/gcc,
O0/O2 and native/POSIX backends. Each run checks 20 complete lifecycle cycles and
600000 publications from three producers to one consumer.

Run `utils/tests/test_audio_buffer_queue.ps1` for the extracted production worker
and stream methods under both backends. It checks resets during unlocked decode,
400 playback recycling cycles, 100 synchronous cutscene cycles, both provider
initialization branches, pending-work playback status, and shutdown with queued
duplicate process requests and captured cleanup. OpenAL is mocked; no window or
real audio device is opened. Separate decoder fixtures compare existing format
behavior against the saved C++ baseline.

CStream now uses a C struct and explicit C functions. Its source now builds as
C with a small C interface to the game. See audio_stream_c.md.
The full project has additional C++ features outside this audio subsystem.
