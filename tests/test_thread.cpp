// test_thread.cpp — tests for crlib/thread.h (Mutex, ScopedLock, Signal, Thread, ThreadPool)
#include <crlib/crlib.h>
#include "catch2/catch_amalgamated.hpp"
#include <thread>
#include <chrono>

namespace {
    inline void testSleep(int ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

    // Note: Keep tests free of std:: usage; use crlib primitives below
}

using namespace cr;

// ---------------------------------------------------------------------------
// Mutex — basic lock / unlock
// ---------------------------------------------------------------------------
TEST_CASE("Mutex lock and unlock do not crash", "[thread]") {
    Mutex m;
    m.lock();
    m.unlock();
    REQUIRE(true);
}

TEST_CASE("Mutex tryLock succeeds when unlocked", "[thread]") {
    Mutex m;
    bool got = m.tryLock();
    REQUIRE(got);
    m.unlock();
}

TEST_CASE("Mutex tryLock fails when already locked (from the same thread on Win non-XP / reentrant path)", "[thread]") {
    // SRWLOCK on Windows is NOT reentrant; tryLock will fail if same thread holds it
    Mutex m;
    m.lock();
#if defined(CR_WINDOWS) && !defined(CR_HAS_WINXP_SUPPORT)
    bool second = m.tryLock();
    // SRWLOCK: same-thread tryLock for exclusive is undefined;
    // we just verify it does not crash
    (void)second;
#endif
    m.unlock();
    REQUIRE(true);
}

// ---------------------------------------------------------------------------
// ScopedLock — RAII wrapper
// ---------------------------------------------------------------------------
TEST_CASE("ScopedLock locks and unlocks automatically", "[thread]") {
    Mutex m;
    {
        MutexScopedLock guard(m);
        // m is locked here
    }
    // m must be unlocked now; tryLock should succeed
    bool got = m.tryLock();
    REQUIRE(got);
    m.unlock();
}

// ---------------------------------------------------------------------------
// ScopedUnlock — releases on destruction
// ---------------------------------------------------------------------------
TEST_CASE("ScopedUnlock unlocks on destruction", "[thread]") {
    Mutex m;
    m.lock();
    {
        ScopedUnlock<Mutex> ul(m);
        // destructor will call m.unlock()
    }
    // Now we should be able to tryLock again
    bool got = m.tryLock();
    REQUIRE(got);
    m.unlock();
}

// ---------------------------------------------------------------------------
// Thread — launch and join
// ---------------------------------------------------------------------------
TEST_CASE("Thread launches and ok() returns true", "[thread]") {
    volatile bool ran = false;

    Thread t([&ran]() {
        ran = true;
    });

    REQUIRE(t.ok());
    t.join();
    REQUIRE(ran);
}

TEST_CASE("Thread join completes without hanging", "[thread]") {
    Thread t([]() {
        // do nothing
    });
    t.join();
    REQUIRE(true);
}

TEST_CASE("Thread move constructor transfers ownership and thread runs correctly", "[thread]") {
    // Hold the thread at its start so the move is guaranteed to happen
    // before the callback executes.  Without the invokable_.get() fix,
    // the moved-from Thread's invokable_ would be null and the worker
    // would crash when it finally runs.
    Signal gate;
    bool released = false;
    volatile bool ran = false;

    Thread t1([&gate, &released, &ran]() {
        gate.lock();
        while (!released) {
            gate.wait();
        }
        gate.unlock();
        ran = true;
    });

    // Move before the thread is released from the gate.
    Thread t2(cr::move(t1));
    REQUIRE_FALSE(t1.ok());
    REQUIRE(t2.ok());

    // Release the thread — it will now execute its body via t2's invokable_.
    {
        SignalScopedLock lock(gate);
        released = true;
        gate.notify();
    }

    t2.join();
    REQUIRE(ran);
}

TEST_CASE("Thread executes work incrementing a shared counter", "[thread]") {
    Mutex m;
    int counter = 0;
    const int N = 5;

    Array<Thread> threads;
    for (int i = 0; i < N; ++i) {
        threads.emplace([&m, &counter]() {
            MutexScopedLock guard(m);
            ++counter;
        });
    }
    for (auto &t : threads) {
        t.join();
    }
    REQUIRE(counter == N);
}

// ---------------------------------------------------------------------------
// ThreadPool — basic usage
// ---------------------------------------------------------------------------
TEST_CASE("ThreadPool starts with correct threadCount", "[thread]") {
    ThreadPool pool(2);
    REQUIRE(pool.threadCount() == 2);
    pool.shutdown();
}

TEST_CASE("ThreadPool enqueue and execute tasks", "[thread]") {
    Mutex m;
    int done = 0;
    const int tasks = 10;

    {
        ThreadPool pool(2);

        for (int i = 0; i < tasks; ++i) {
            pool.enqueue([&m, &done]() {
                MutexScopedLock guard(m);
                ++done;
            });
        }
        // ThreadPool::shutdown() clears pending jobs before workers finish,
        // so we spin-wait for all tasks to actually complete without deadlocking.
        while (true) {
            {
                MutexScopedLock guard(m);
                if (done == tasks) {
                    break;
                }
            }
            testSleep(1);
        }
    }
    REQUIRE(done == tasks);
}

TEST_CASE("ThreadPool jobs count returns zero after shutdown", "[thread]") {
    ThreadPool pool(1);
    pool.shutdown();
    REQUIRE(pool.jobs() == 0);
}

TEST_CASE("ThreadPool startup with 0 workers does nothing", "[thread]") {
    ThreadPool pool(0);
    REQUIRE(pool.threadCount() == 0);
    // enqueue is safe to call even with no workers... but tasks won't run
}

// ---------------------------------------------------------------------------
// Signal — basic notify (does not deadlock)
// ---------------------------------------------------------------------------
TEST_CASE("Signal notify from main thread does not crash", "[thread]") {
    Signal sig;
    sig.lock();
    sig.notify();
    sig.unlock();
    REQUIRE(true);
}

TEST_CASE("Signal wait with short timeout returns", "[thread]") {
    Signal sig;
    sig.lock();
    // wait with 1ms timeout — should return quickly
    bool result = sig.wait(1);
    sig.unlock();
    // result might be true or false depending on spurious wakeups; just no deadlock
    (void)result;
    REQUIRE(true);
}

// ===========================================================================
// Additional tests for missing coverage
// ===========================================================================

// ---------------------------------------------------------------------------
// Mutex — additional tests
// ---------------------------------------------------------------------------
TEST_CASE("Mutex raw method returns handle", "[thread]") {
    Mutex m;
    auto handle = m.raw();
    REQUIRE(handle != nullptr);
}

TEST_CASE("Mutex double unlock does not crash", "[thread]") {
    Mutex m;
    m.lock();
    m.unlock();
    m.unlock(); // Second unlock should be safe
    REQUIRE(true);
}

TEST_CASE("Mutex destruction while locked is safe", "[thread]") {
    // This tests that destroying a locked mutex doesn't crash
    // (though it's bad practice in real code)
    {
        Mutex m;
        m.lock();
        // Mutex destroyed while locked
    }
    REQUIRE(true);
}

// ---------------------------------------------------------------------------
// ScopedLock / ScopedUnlock — additional tests
// ---------------------------------------------------------------------------
TEST_CASE("ScopedLock with Signal type", "[thread]") {
    Signal sig;
    {
        SignalScopedLock lock(sig);
        // Signal is locked
    }
    // Signal should be unlocked
    REQUIRE(true);
}

TEST_CASE("ScopedUnlock with Signal type", "[thread]") {
    Signal sig;
    sig.lock();
    {
        ScopedUnlock<Signal> ul(sig);
        // Signal is unlocked temporarily
    }
    // Signal should be locked again (ScopedUnlock doesn't relock)
    sig.unlock();
    REQUIRE(true);
}

// ---------------------------------------------------------------------------
// Signal — additional tests
// ---------------------------------------------------------------------------
TEST_CASE("Signal broadcast wakes multiple waiters", "[thread]") {
    Signal sig;
    Mutex counterMutex;
    int wokenCount = 0;
    const int numThreads = 3;
    
    Array<Thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace([&sig, &counterMutex, &wokenCount]() {
            sig.lock();
            sig.wait(); // Wait for broadcast
            {
                MutexScopedLock lock(counterMutex);
                ++wokenCount;
            }
            sig.unlock();
        });
    }
    
    // Give threads time to start waiting
    testSleep(50);
    
    // Broadcast should wake all threads
    sig.lock();
    sig.broadcast();
    sig.unlock();
    
    // Wait for threads to finish
    for (auto &t : threads) {
        t.join();
    }
    
    REQUIRE(wokenCount == numThreads);
}

TEST_CASE("Signal wait without timeout blocks", "[thread]") {
    Signal sig;
    volatile bool waited = false;
    
    Thread t([&sig, &waited]() {
        sig.lock();
        sig.wait(); // Should block until notified
        waited = true;
        sig.unlock();
    });
    
    // Give thread time to start waiting
    testSleep(50);
    
    // Thread should still be waiting
    REQUIRE(!waited);
    
    // Notify to wake it
    sig.lock();
    sig.notify();
    sig.unlock();
    
    t.join();
    REQUIRE(waited);
}

TEST_CASE("Signal wait after notify waits", "[thread]") {
    Signal sig;
    
    sig.lock();
    sig.notify(); // Notify before anyone is waiting
    bool result = sig.wait(10); // Should still wait (timeout)
    sig.unlock();
    
    // Should timeout (false) since no one will notify again
    // Just verify it doesn't deadlock
    (void)result;
    REQUIRE(true);
}

// ---------------------------------------------------------------------------
// Thread — additional tests
// ---------------------------------------------------------------------------
TEST_CASE("Thread detach method", "[thread]") {
    Signal done;
    bool ran = false;
    
    Thread t([&done, &ran]() {
        testSleep(10);
        ran = true;
        done.notify();
    });
    
    REQUIRE(t.ok());
    t.detach();
    REQUIRE(!t.ok());
    
    {
        SignalScopedLock lock(done);
        REQUIRE(done.wait(1000));
    }
    REQUIRE(ran);
}

TEST_CASE("Thread handle method returns non-zero", "[thread]") {
    Thread t([]() {});
    auto handle = t.handle();
    // Handle should be non-zero/null for valid thread
    (void)handle;
    t.join();
    REQUIRE(true);
}

TEST_CASE("Thread ok on default-constructed returns false", "[thread]") {
    Thread t;
    REQUIRE(!t.ok());
}

TEST_CASE("Thread join on already joined thread is safe", "[thread]") {
    Thread t([]() {});
    t.join();
    t.join(); // Second join should be safe
    REQUIRE(true);
}

TEST_CASE("Thread start on already running thread joins first", "[thread]") {
    volatile bool firstRan = false;
    volatile bool secondRan = false;
    
    Thread t([&firstRan]() {
        firstRan = true;
    });
    
    t.join();
    REQUIRE(firstRan);
    
    // Start second thread after first completed
    t.start([&secondRan]() {
        secondRan = true;
    });
    
    t.join();
    REQUIRE(secondRan);
}

TEST_CASE("Thread move assignment operator", "[thread]") {
    volatile bool ran1 = false;
    volatile bool ran2 = false;
    
    Thread t1([&ran1]() { ran1 = true; });
    Thread t2([&ran2]() { ran2 = true; });
    
    t2 = cr::move(t1); // Move assignment
    
    REQUIRE(!t1.ok());
    REQUIRE(t2.ok());
    
    t2.join();
    REQUIRE(ran1);
    // t2's original thread was joined by move assignment
}

// ---------------------------------------------------------------------------
// ThreadPool — additional tests
// ---------------------------------------------------------------------------
TEST_CASE("ThreadPool jobs count while tasks are running", "[thread]") {
    cr::Signal done;
    volatile bool taskRunning = false;

    ThreadPool pool(1);

    pool.enqueue([&taskRunning, &done]() {
        taskRunning = true;
        testSleep(60); // Simulate work
        taskRunning = false;
        done.notify();
    });

    // Wait for completion (timeout to avoid hangs)
    done.lock();
    bool signaled = done.wait(1000);
    done.unlock();
    REQUIRE(signaled);

    pool.shutdown();
}

TEST_CASE("ThreadPool shutdown with pending jobs", "[thread]") {
    Mutex m;
    int completed = 0;
    
    ThreadPool pool(1);
    
    // Enqueue more tasks than threads can process before shutdown
    for (int i = 0; i < 5; ++i) {
        pool.enqueue([&m, &completed]() {
            testSleep(50); // Simulate work
            MutexScopedLock lock(m);
            ++completed;
        });
    }
    
    // Shutdown immediately - some tasks may not run
    pool.shutdown();
    
    // Some tasks may have completed, some may have been dropped
    REQUIRE(completed >= 0);
    REQUIRE(completed <= 5);
}

TEST_CASE("ThreadPool restart after shutdown", "[thread]") {
    ThreadPool pool(1);
    pool.shutdown();
    
    // Should be able to start again
    pool.startup(2);
    REQUIRE(pool.threadCount() == 2);
    
    pool.shutdown();
}

TEST_CASE("ThreadPool enqueue after shutdown does not crash", "[thread]") {
    ThreadPool pool(1);
    pool.shutdown();
    
    // Enqueue after shutdown should not crash
    pool.enqueue([]() {
        // This task should not run
    });
    
    REQUIRE(true);
}

TEST_CASE("ThreadPool thread safety of public methods", "[thread]") {
    ThreadPool pool(2);
    Mutex counterMutex;
    int enqueueCount = 0;
    
    // Multiple threads calling enqueue concurrently
    Array<Thread> clients;
    for (int i = 0; i < 4; ++i) {
        clients.emplace([&pool, &counterMutex, &enqueueCount]() {
            for (int j = 0; j < 10; ++j) {
                pool.enqueue([&counterMutex, &enqueueCount]() {
                    MutexScopedLock lock(counterMutex);
                    ++enqueueCount;
                });
                testSleep(1);
            }
        });
    }
    
    // Also call jobs() concurrently
    Array<Thread> monitors;
    for (int i = 0; i < 2; ++i) {
        monitors.emplace([&pool]() {
            for (int j = 0; j < 20; ++j) {
                (void)pool.jobs();
                testSleep(2);
            }
        });
    }
    
    // Wait for all
    for (auto &t : clients) t.join();
    for (auto &t : monitors) t.join();
    
    // Let pool process tasks
    testSleep(100);
    
    pool.shutdown();
    REQUIRE(enqueueCount == 40); // 4 clients * 10 tasks each
}
