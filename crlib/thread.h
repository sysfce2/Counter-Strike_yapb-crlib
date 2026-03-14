// SPDX-License-Identifier: Unlicense

#pragma once

#include <crlib/basic.h>

#if !defined(CR_WINDOWS)
#  include <pthread.h>
#  include <errno.h>
#else
#  include <process.h>
#endif

CR_NAMESPACE_BEGIN

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

// scoped unlock wrapper, unlocks on destruction
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
#if defined(CR_WINDOWS) && defined(CR_HAS_WINXP_SUPPORT)
class Mutex final : public NonCopyable {
private:
   CRITICAL_SECTION cs_;

public:
   Mutex () { InitializeCriticalSectionAndSpinCount (&cs_, 1); }
   ~Mutex () { DeleteCriticalSection (&cs_); }

   void lock () { EnterCriticalSection (&cs_); }
   void unlock () { LeaveCriticalSection (&cs_); }
   bool tryLock () { return !!TryEnterCriticalSection (&cs_); }

   decltype (auto) raw () { return &cs_; }
};

#elif defined(CR_WINDOWS)
class Mutex final : public NonCopyable {
private:
   SRWLOCK cs_ = SRWLOCK_INIT;

public:
   Mutex () = default;
   ~Mutex () = default;

   void lock () { AcquireSRWLockExclusive (&cs_); }
   void unlock () { ReleaseSRWLockExclusive (&cs_); }
   bool tryLock () { return !!TryAcquireSRWLockExclusive (&cs_); }

   decltype (auto) raw () { return &cs_; }
};

#else
class Mutex final : public NonCopyable {
private:
   pthread_mutex_t mutex_;

public:
   Mutex () { pthread_mutex_init (&mutex_, nullptr); }
   ~Mutex () { pthread_mutex_destroy (&mutex_); }

   void lock () { pthread_mutex_lock (&mutex_); }
   void unlock () { pthread_mutex_unlock (&mutex_); }
   bool tryLock () { return pthread_mutex_trylock (&mutex_) == 0; }

   decltype (auto) raw () { return &mutex_; }
};
#endif

// conditional variable (signal)
#if defined(CR_WINDOWS) && defined(CR_HAS_WINXP_SUPPORT)
class Signal final : public NonCopyable {
private:
   Mutex cs_;
   HANDLE event_;

public:
   Signal () { event_ = CreateEvent (nullptr, FALSE, FALSE, nullptr); }
   ~Signal () { CloseHandle (event_); }

   void lock () { cs_.lock (); }
   void unlock () { cs_.unlock (); }

   void notify () { SetEvent (event_); }

   // auto-reset event can only wake one waiter
   void broadcast () { SetEvent (event_); }

   template <typename T> bool wait (T timeout) {
      unlock ();
      auto result = WaitForSingleObject (event_, timeout);
      lock ();

      return result == WAIT_OBJECT_0;
   }

   bool wait () {
      return wait (INFINITE);
   }
};

#elif defined(CR_WINDOWS)
class Signal final : public NonCopyable {
private:
   Mutex cs_;
   CONDITION_VARIABLE cv_;

public:
   Signal () { InitializeConditionVariable (&cv_); }
   ~Signal () = default;

   void lock () { cs_.lock (); }
   void unlock () { cs_.unlock (); }

   void notify () { WakeConditionVariable (&cv_); }
   void broadcast () { WakeAllConditionVariable (&cv_); }

   template <typename T> bool wait (T timeout) {
      return SleepConditionVariableSRW (&cv_, cs_.raw (), timeout, 0) != FALSE;
   }

   bool wait () {
      return wait (INFINITE);
   }
};

#else
class Signal final : public NonCopyable {
private:
   Mutex cs_;
   pthread_cond_t cv_;

public:
   Signal () { pthread_cond_init (&cv_, nullptr); }
   ~Signal () { pthread_cond_destroy (&cv_); }

   void lock () { cs_.lock (); }
   void unlock () { cs_.unlock (); }

   void notify () { pthread_cond_signal (&cv_); }
   void broadcast () { pthread_cond_broadcast (&cv_); }

   template <typename T> bool wait (T timeout) {
      struct timespec ts;

#if defined(CR_POSIX) && !defined(CR_MACOS)
      if (clock_gettime (CLOCK_REALTIME, &ts) == -1) {
         return false;
      }
#else
      struct timeval tv;
      gettimeofday (&tv, nullptr);

      ts.tv_sec = tv.tv_sec;
      ts.tv_nsec = tv.tv_usec * 1000;
#endif

      ts.tv_sec += timeout / 1000;
      ts.tv_nsec += (timeout % 1000) * 1000000;

      if (ts.tv_nsec >= 1000000000) {
         ts.tv_sec++;
         ts.tv_nsec -= 1000000000;
      }
      return pthread_cond_timedwait (&cv_, cs_.raw (), &ts) == 0;
   }

   bool wait () {
      return pthread_cond_wait (&cv_, cs_.raw ()) == 0;
   }
};
#endif

using MutexScopedLock = ScopedLock <Mutex>;
using SignalScopedLock = ScopedLock <Signal>;

// basic thread class
class Thread final : public NonCopyable {
public:
   using Func = Lambda <void ()>;

private:
#if defined(CR_WINDOWS)
   HANDLE thread_ {};
#else
   bool initialized_ {};
   pthread_t thread_ {};
#endif
   UniquePtr <Func> invokable_;

private:
#if defined(CR_WINDOWS)
   static unsigned int __stdcall worker (void *pinvokable) {
      assert (pinvokable);
      (*reinterpret_cast <Func *> (pinvokable)) ();
      return 0;
   }
#else
   static void *worker (void *pinvokable) {
      assert (pinvokable);
      (*reinterpret_cast <Func *> (pinvokable)) ();
      return nullptr;
   }
#endif

public:
   Thread () = default;

   explicit Thread (Func &&callback) {
      start (cr::move (callback));
   }

   Thread (Thread &&rhs) noexcept {
      thread_ = rhs.thread_;
      invokable_ = cr::move (rhs.invokable_);

#if defined(CR_WINDOWS)
      rhs.thread_ = nullptr;
#else
      initialized_ = rhs.initialized_;
      rhs.thread_ = 0;
      rhs.initialized_ = false;
#endif
   }

   Thread &operator = (Thread &&rhs) noexcept {
      if (this != &rhs) {
         join ();

         thread_ = rhs.thread_;
         invokable_ = cr::move (rhs.invokable_);

#if defined(CR_WINDOWS)
         rhs.thread_ = nullptr;
#else
         initialized_ = rhs.initialized_;
         rhs.thread_ = 0;
         rhs.initialized_ = false;
#endif
      }
      return *this;
   }

   ~Thread () noexcept {
      join ();
   }

   void start (Func &&callback) {
      join ();

      invokable_ = makeUnique <Func> (cr::move (callback));

#if defined(CR_WINDOWS)
      thread_ = reinterpret_cast <HANDLE> (_beginthreadex (nullptr, 0, worker, invokable_.get (), 0, nullptr));
#else
      initialized_ = (pthread_create (&thread_, nullptr, worker, invokable_.get ()) == 0);
#endif

      if (!ok ()) {
         invokable_.reset ();
      }
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
#if defined(CR_WINDOWS)
      WaitForSingleObjectEx (thread_, INFINITE, FALSE);
      CloseHandle (thread_);
      thread_ = nullptr;
#else
      pthread_join (thread_, nullptr);
      initialized_ = false;
#endif
   }

   void detach () {
      if (!ok ()) {
         return;
      }
#if defined(CR_WINDOWS)
      CloseHandle (thread_);
      thread_ = nullptr;
#else
      pthread_detach (thread_);
      initialized_ = false;
#endif
      invokable_.release ();
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
         signal_.broadcast ();
      }

      for (auto &thread : threads_) {
         thread.join ();
      }
      threads_.clear ();
   }

   void startup (size_t workers) {
      {
         SignalScopedLock lock (signal_);
         running_ = true;
      }

      for (size_t i = 0; i < workers; ++i) {
         threads_.emplace ([this] () {
            for (;;) {
               Func job {};
               {
                  SignalScopedLock lock (signal_);

                  while (running_ && jobs_.empty ()) {
                     signal_.wait ();
                  }

                  if (!running_ && jobs_.empty ()) {
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
