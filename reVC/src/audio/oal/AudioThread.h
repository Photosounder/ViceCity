#pragma once

//+ rouz edit (ChatGPT)
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#if defined(_WIN32) && !defined(AUDIO_THREAD_USE_POSIX)
#include <windows.h>
#include <process.h>
typedef struct AudioMutex { CRITICAL_SECTION native; } AudioMutex;
typedef struct AudioWakeSignal { HANDLE event; } AudioWakeSignal;
typedef struct AudioThread {
    HANDLE handle;
    void (*entry)(void *);
    void *context;
} AudioThread;
#else
#include <pthread.h>
typedef struct AudioMutex { pthread_mutex_t native; } AudioMutex;
typedef struct AudioWakeSignal { pthread_cond_t native; } AudioWakeSignal;
typedef struct AudioThread {
    pthread_t handle;
    void (*entry)(void *);
    void *context;
} AudioThread;
#endif

static inline bool AudioMutex_Init(AudioMutex *mutex)
{
    // Initialize externally owned mutex storage before publication
#if defined(_WIN32) && !defined(AUDIO_THREAD_USE_POSIX)
    InitializeCriticalSection(&mutex->native);
    return true;
#else
    return pthread_mutex_init(&mutex->native, NULL) == 0;
#endif
}
static inline void AudioMutex_Destroy(AudioMutex *mutex)
{
    // Release native mutex resources after their final use
#if defined(_WIN32) && !defined(AUDIO_THREAD_USE_POSIX)
    DeleteCriticalSection(&mutex->native);
#else
    if(pthread_mutex_destroy(&mutex->native)) abort();
#endif
}
static inline void AudioMutex_Lock(AudioMutex *mutex)
{
    // Serialize access to the protected stream or scheduler state
#if defined(_WIN32) && !defined(AUDIO_THREAD_USE_POSIX)
    EnterCriticalSection(&mutex->native);
#else
    if(pthread_mutex_lock(&mutex->native)) abort();
#endif
}
static inline void AudioMutex_Unlock(AudioMutex *mutex)
{
    // Publish protected changes before another thread acquires the mutex
#if defined(_WIN32) && !defined(AUDIO_THREAD_USE_POSIX)
    LeaveCriticalSection(&mutex->native);
#else
    if(pthread_mutex_unlock(&mutex->native)) abort();
#endif
}
static inline bool AudioWakeSignal_Init(AudioWakeSignal *signal)
{
    // Initialize a signal for the single scheduler worker
#if defined(_WIN32) && !defined(AUDIO_THREAD_USE_POSIX)
    signal->event = CreateEventA(NULL, FALSE, FALSE, NULL);
    return signal->event != NULL;
#else
    return pthread_cond_init(&signal->native, NULL) == 0;
#endif
}
static inline void AudioWakeSignal_Destroy(AudioWakeSignal *signal)
{
    // Release signal resources only after joining the waiting worker
#if defined(_WIN32) && !defined(AUDIO_THREAD_USE_POSIX)
    if(!CloseHandle(signal->event)) abort();
    signal->event = NULL;
#else
    if(pthread_cond_destroy(&signal->native)) abort();
#endif
}
static inline void AudioWakeSignal_Notify(AudioWakeSignal *signal)
{
    // Wake the single worker after publishing work or shutdown
#if defined(_WIN32) && !defined(AUDIO_THREAD_USE_POSIX)
    if(!SetEvent(signal->event)) abort();
#else
    if(pthread_cond_signal(&signal->native)) abort();
#endif
}
static inline void AudioWakeSignal_Wait(AudioWakeSignal *signal, AudioMutex *mutex)
{
    // Release the scheduler lock while sleeping and reacquire it before rechecking work
#if defined(_WIN32) && !defined(AUDIO_THREAD_USE_POSIX)
    AudioMutex_Unlock(mutex);
    if(WaitForSingleObject(signal->event, INFINITE) != WAIT_OBJECT_0) abort();
    AudioMutex_Lock(mutex);
#else
    if(pthread_cond_wait(&signal->native, &mutex->native)) abort();
#endif
}

#if defined(_WIN32) && !defined(AUDIO_THREAD_USE_POSIX)
static unsigned __stdcall AudioThread_Run(void *opaque)
{
    // Run the caller entry with CRT thread initialization and stable caller-owned context
    AudioThread *thread = (AudioThread*)opaque;
    thread->entry(thread->context);
    return 0;
}
#else
static void *AudioThread_Run(void *opaque)
{
    // Run the caller entry with stable caller-owned context
    AudioThread *thread = (AudioThread*)opaque;
    thread->entry(thread->context);
    return NULL;
}
#endif
static inline bool AudioThread_Start(AudioThread *thread, void (*entry)(void *), void *context)
{
    // Store the entry in persistent caller-owned storage without a helper allocation
    thread->entry = entry;
    thread->context = context;
#if defined(_WIN32) && !defined(AUDIO_THREAD_USE_POSIX)
    thread->handle = (HANDLE)(uintptr_t)_beginthreadex(NULL, 0, AudioThread_Run, thread, 0, NULL);
    return thread->handle != NULL;
#else
    return pthread_create(&thread->handle, NULL, AudioThread_Run, thread) == 0;
#endif
}
static inline void AudioThread_Join(AudioThread *thread)
{
    // Join before releasing any context, mutex, signal or decoder resources
#if defined(_WIN32) && !defined(AUDIO_THREAD_USE_POSIX)
    if(WaitForSingleObject(thread->handle, INFINITE) != WAIT_OBJECT_0) abort();
    if(!CloseHandle(thread->handle)) abort();
    thread->handle = NULL;
#else
    if(pthread_join(thread->handle, NULL)) abort();
#endif
}
//- rouz edit (ChatGPT)
