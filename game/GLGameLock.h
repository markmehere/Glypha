#ifndef GLGAMELOCK_H
#define GLGAMELOCK_H

#ifdef _WIN32
#else
#include <pthread.h>
#endif

namespace GL {

class Lock {
#ifdef _WIN32
public:
    Lock() {
        InitializeCriticalSection(&lock_);
    }
    ~Lock() {
        DeleteCriticalSection(&lock_);
    }
    void lock() {
        EnterCriticalSection(&lock_);
    }
    void unlock() {
        LeaveCriticalSection(&lock_);
    }
    private:
    CRITICAL_SECTION lock_;
#else
public:
    Lock() {
        pthread_mutex_init(&mutex_, NULL);
    }
    ~Lock() {
        pthread_mutex_destroy(&mutex_);
    }
    void lock() {
        pthread_mutex_lock(&mutex_);
    }
    void unlock() {
        pthread_mutex_unlock(&mutex_);
    }
private:
    pthread_mutex_t mutex_;
#endif
};

class Locker {
public:
    Locker(Lock& lock)
            : lock_(lock)
    {
        lock_.lock();
    }
    ~Locker() {
        lock_.unlock();
    }
private:
    Lock& lock_;
};

}

#endif // GLGAMELOCK_H
