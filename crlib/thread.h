//
// YaPB - Counter-Strike Bot based on PODBot by Markus Klinge.
// Copyright © 2004-2022 YaPB Project <yapb@jeefo.net>.
//
// SPDX-License-Identifier: MIT
//

#pragma once

#include <crlib/basic.h>

#if !defined (CR_WINDOWS)
#  include <sys/mman.h>
#  include <pthread.h>
#else
#  include <process.h>
#endif

CR_NAMESPACE_BEGIN

#if !defined (CR_WINDOWS)
// pthread workaround since glibc 2.34 doesn't provide linkage with libpthread
// and we're need to target binary compiled with latest compiler on ancient distro
class PthreadWrapper final : public Singleton <PthreadWrapper> {
private:
   using MutexTryLock = decltype (&pthread_mutex_trylock);
   using Join = decltype (&pthread_join);
   using Detach = decltype (&pthread_detach);
   using Create = decltype (&pthread_create);

private:
   SharedLibrary libpthread_;

public:
   MutexTryLock mutex_trylock = nullptr;
   Join join = nullptr;
   Detach detach = nullptr;
   Create create = nullptr;

private:
   template <typename T> T resolve (const char *symbol) {
      auto result = reinterpret_cast <T> (dlsym (RTLD_DEFAULT, symbol));

      if (!result) {
         if (!libpthread_) {
            libpthread_.load ("libpthread.so"); // older than glibc 2.34, try load libpthread as workaround
         }
         result = libpthread_.resolve <T> (symbol); // ... and lookup there
      }
      return result;
   }

public:
   explicit PthreadWrapper () {
      mutex_trylock = resolve <MutexTryLock> ("pthread_mutex_trylock");
      join = resolve <Join> ("pthread_join");
      detach = resolve <Detach> ("pthread_detach");
      create = resolve <Create> ("pthread_create");
   }
   ~PthreadWrapper () = default;
};

// expose thread wrapper
CR_EXPOSE_GLOBAL_SINGLETON (PthreadWrapper, pthread);

#endif

// simple scoped lock wrapper
template <typename T> class ScopedLock final : public NonCopyable {
private:
   T &lockable_;

public:
   ScopedLock (T &lock) : lockable_ (lock) {
      lockable_.lock ();
   }

   ~ScopedLock () {
      lockable_.unlock ();
   }
};

template <typename T> class ScopedUnlock final : public NonCopyable {
private:
   T &lockable_;

public:
   ScopedUnlock (T &lock) : lockable_ (lock) { }

   ~ScopedUnlock () {
      lockable_.unlock ();
   }
};

// simple wrapper for critical sections
class Mutex final : public NonCopyable {
private:
#if defined (CR_WINDOWS)
#if defined (CR_HAS_WINXP_SUPPORT)
   CRITICAL_SECTION cs_;
#else
   SRWLOCK cs_ = SRWLOCK_INIT;
#endif
#else
   pthread_mutex_t mutex_;
#endif

public:
   Mutex () {
#if defined (CR_WINDOWS)
#if defined (CR_HAS_WINXP_SUPPORT)
      InitializeCriticalSectionAndSpinCount (&cs_, 1);
#endif
#else
      pthread_mutex_init (&mutex_, nullptr);
#endif
   }
   ~Mutex () {
#if defined (CR_WINDOWS)
#if defined (CR_HAS_WINXP_SUPPORT)
      DeleteCriticalSection (&cs_);
#endif
#else
      pthread_mutex_destroy (&mutex_);
#endif
   }

   void lock () {
#if defined (CR_WINDOWS)
#if defined (CR_HAS_WINXP_SUPPORT)
      EnterCriticalSection (&cs_);
#else
      AcquireSRWLockExclusive (&cs_);
#endif
#else
      pthread_mutex_lock (&mutex_);
#endif
   }

   void unlock () {
#if defined (CR_WINDOWS)
#if defined (CR_HAS_WINXP_SUPPORT)
      LeaveCriticalSection (&cs_);
#else
      ReleaseSRWLockExclusive (&cs_);
#endif
#else
      pthread_mutex_unlock (&mutex_);
#endif
   }

   bool tryLock () {
#if defined (CR_WINDOWS)
#if defined (CR_HAS_WINXP_SUPPORT)
      return !!TryEnterCriticalSection (&cs_);
#else
      return !!TryAcquireSRWLockExclusive (&cs_);
#endif
#else
      return pthread.mutex_trylock (&mutex_) == 0;
#endif
   }

   decltype (auto) raw () {
#if defined (CR_WINDOWS)
      return &cs_;
#else
      return &mutex_;
#endif
   }
};

// conditional variable (signal)
class Signal final : public NonCopyable {
private:
   Mutex cs_;

#if defined (CR_WINDOWS)
#if defined (CR_HAS_WINXP_SUPPORT)
   HANDLE event_;
#else
   CONDITION_VARIABLE cv_;
#endif
#else
   pthread_cond_t cv_;
#endif

public:
   Signal () {
#if defined (CR_WINDOWS)
#if defined (CR_HAS_WINXP_SUPPORT)
      event_ = CreateEvent (nullptr, FALSE, FALSE, nullptr);
#else
      InitializeConditionVariable (&cv_);
#endif
#else
      pthread_cond_init (&cv_, nullptr);
#endif
   }

   ~Signal () {
#if defined (CR_WINDOWS)
#if defined (CR_HAS_WINXP_SUPPORT)
      CloseHandle (event_);
#endif
#else
      pthread_cond_destroy (&cv_);
#endif
   }

   void lock () {
      cs_.lock ();
   }

   void unlock () {
      cs_.unlock ();
   }

   void notify () {
#if defined (CR_WINDOWS)
#if defined (CR_HAS_WINXP_SUPPORT)
      SetEvent (event_);
#else
      WakeConditionVariable (&cv_);
#endif
#else
      pthread_cond_signal (&cv_);
#endif
   }

#if (defined (CR_WINDOWS) && !defined (CR_HAS_WINXP_SUPPORT)) || defined (CR_LINUX) || defined (CR_OSX)
   void broadcast () {
#if defined (CR_WINDOWS)
      WakeAllConditionVariable (&cv_);
#else
      pthread_cond_broadcast (&cv_);
#endif
   }
#define THREAD_SIGNAL_HAS_BROADCAST
#endif

   template <typename T> bool wait (T timeout) {
#if defined (CR_WINDOWS)
#if defined (CR_HAS_WINXP_SUPPORT)
      ResetEvent (cs_.raw ());

      unlock ();
      auto result = WaitForSingleObject (event_, timeout);
      lock ();
#else
      auto result = SleepConditionVariableSRW (&cv_, cs_.raw (), timeout, 0);
#endif

#if defined (CR_HAS_WINXP_SUPPORT)
      if (result == WAIT_TIMEOUT) {
         return false;
      }
      else if (result == WAIT_FAILED) {
         return false;
      }
      return true;
#else
      return result != FALSE;
#endif
#else
#if defined (CR_LINUX)
      struct timespec ts;

      if (clock_gettime (CLOCK_REALTIME, &ts) == -1) {
         return false;
      }
#else
      struct timeval tv;
      gettimeofday (&tv, nullptr);

      struct timespec ts;
      ts.tv_sec = tv.tv_sec;
      ts.tv_nsec = tv.tv_usec * 1000;
#endif

      ts.tv_sec += timeout / 1000;
      ts.tv_nsec += (timeout % 1000) * 1000000;

      if (ts.tv_nsec >= 1000000000) {
         ts.tv_sec++;
         ts.tv_nsec -= 1000000000;
      }
      auto result = pthread_cond_timedwait (&cv_, cs_.raw (), &ts);

      if (result == ETIMEDOUT) {
         return false;
      }
      else if (result == 0) {
         return true;
      }
      return false;
#endif
   }

   bool wait () {
#if defined (CR_WINDOWS)
      return wait (INFINITE);
#else
      auto result = pthread_cond_wait (&cv_, cs_.raw ());

      if (result == 0) {
         return true;
      }
      return false;
#endif
   }
};

using MutexScopedLock = ScopedLock <Mutex>;
using SignalScopedLock = ScopedLock <Signal>;


// basic thread class
class Thread final : public NonCopyable {
public:
   using Func = Lambda <void ()>;

private:
#if defined (CR_WINDOWS)
   HANDLE thread_ {};
#else
   bool initialized_ {};
   pthread_t thread_ {};
#endif
   UniquePtr <Func> m_invokable;

private:
#if defined (CR_WINDOWS)
   static DWORD __stdcall threadWorker (void *pthis) {
#else
   static void *threadWorker (void *pthis) {
#endif
      assert (pthis);
      (*reinterpret_cast <Thread *> (pthis)->m_invokable) ();

#if defined (CR_WINDOWS)
      return 0;
#else
      return nullptr;
#endif
   }

public:
   explicit Thread (Func &&callback) {
      m_invokable = makeUnique <Func> (cr::move (callback));

#if defined(CR_WINDOWS)
      thread_ =  CreateThread (nullptr, 0, threadWorker, this, 0, nullptr);
#else
      initialized_ = (pthread.create (&thread_, nullptr, threadWorker, this) == 0);
#endif

      if (!ok ()) {
         m_invokable.reset ();
      }
   }

   Thread (Thread &&rhs) noexcept {
      thread_ = rhs.thread_;
#if defined (CR_WINDOWS)
      rhs.thread_ = nullptr;
#else
      initialized_ = rhs.initialized_;

      rhs.thread_ = 0;
      rhs.initialized_ = false;
#endif
      cr::swap (m_invokable, rhs.m_invokable);
   }

   ~Thread () noexcept {
      if (!ok ()) {
         return;
      }
#if defined (CR_WINDOWS)
      CloseHandle (thread_);
      thread_ = nullptr;
#else
      pthread.detach (thread_);
#endif
   }

public:
   bool ok () const {
#if defined(CR_WINDOWS)
      return !!thread_;
#else
      return initialized_;
#endif
   }

   void join () {
      if (!ok ()) {
         return;
      }
#if defined (CR_WINDOWS)
      WaitForSingleObjectEx (thread_, INFINITE, FALSE);
      thread_ = nullptr;
#else
      pthread.join (thread_, nullptr);
      initialized_ = false;
#endif
   }

   decltype (auto) handle () const {
      return thread_;
   }
};

// extra simple thread pool
class ThreadPool final : public NonCopyable {
private:
   using Func = Thread::Func;

private:
   bool running_ { false };
   mutable Signal signal_ {};

   Deque <Func> jobs_ {};
   Array <Thread> threads_;

public:
   explicit ThreadPool (size_t workers = 0) noexcept {
      if (workers > 0) {
         startup (workers);
      }
   }

   ~ThreadPool () {
      shutdown ();
   }

public:
   size_t jobs () noexcept {
      SignalScopedLock lock (signal_);
      return jobs_.length ();
   }

   size_t threadCount () noexcept {
      return threads_.length ();
   }

public:
   void enqueue (Func &&task) {
      SignalScopedLock lock (signal_);

      jobs_.emplaceLast (cr::move (task));
      signal_.notify ();
   }

   void shutdown () {
      {
         SignalScopedLock lock (signal_);
         running_ = false;

#if !defined (THREAD_SIGNAL_HAS_BROADCAST)
         signal_.notify ();
#else
         signal_.broadcast ();
#endif
      }

      for (auto &thread : threads_) {
         thread.join ();
      }
      threads_.clear ();
   }

   void startup (size_t workers) {
      {
         SignalScopedLock lock (signal_);

         jobs_.clear ();
         running_ = true;
      }
      
      for (size_t i = 0; i < workers; ++i) {
         threads_.emplace ([this] () {
            for (;;) {
               
               Func job;
               {
                  SignalScopedLock lock (signal_);

                  while (running_ && jobs_.empty ()) {
                     signal_.wait ();
                  }

                  if (!running_ && jobs_.empty ()) {
#if !defined (THREAD_SIGNAL_HAS_BROADCAST)
                     signal_.notify ();
#endif
                     return;
                  }
                  job = cr::move (jobs_.popFront ());
               }
               job ();
            }
         });
      }
   }
};

CR_NAMESPACE_END
