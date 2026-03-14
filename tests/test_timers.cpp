// test_timers.cpp — tests for crlib/timers.h
// TimerStorage requires a float* pointing to current game time.
// We drive it with a static variable.
#include <crlib/crlib.h>
#include "catch2/catch_amalgamated.hpp"

using namespace cr;

// Global simulated time value and helper
static float g_time = 0.0f;

static void resetTime(float t = 0.0f) {
    g_time = t;
    timerStorage.setTimeAddress(&g_time);
}

// ===========================================================================
// TimerStorage
// ===========================================================================

TEST_CASE("TimerStorage::value returns pointed-to float", "[timers]") {
    resetTime(3.5f);
    REQUIRE(timerStorage.value() == Catch::Approx(3.5f));

    g_time = 7.0f;
    REQUIRE(timerStorage.value() == Catch::Approx(7.0f));
}

// ===========================================================================
// CountdownTimer
// ===========================================================================

TEST_CASE("CountdownTimer default construction is not started", "[timers]") {
    resetTime(0.0f);
    CountdownTimer ct;
    REQUIRE_FALSE(ct.started());
}

TEST_CASE("CountdownTimer start makes it started", "[timers]") {
    resetTime(1.0f);
    CountdownTimer ct;
    ct.start(2.0f);
    REQUIRE(ct.started());
}

TEST_CASE("CountdownTimer elapsed returns false before duration expires", "[timers]") {
    resetTime(0.0f);
    CountdownTimer ct;
    ct.start(5.0f);      // expires at t=5

    g_time = 3.0f;
    REQUIRE_FALSE(ct.elapsed());
}

TEST_CASE("CountdownTimer elapsed returns true after duration expires", "[timers]") {
    resetTime(0.0f);
    CountdownTimer ct;
    ct.start(2.0f);      // expires at t=2

    g_time = 3.0f;
    REQUIRE(ct.elapsed());
}

TEST_CASE("CountdownTimer remainingTime is positive before expiry", "[timers]") {
    resetTime(0.0f);
    CountdownTimer ct;
    ct.start(10.0f);

    g_time = 3.0f;
    REQUIRE(ct.remainingTime() == Catch::Approx(7.0f).epsilon(0.01f));
}

TEST_CASE("CountdownTimer elapsedTime works after expiry", "[timers]") {
    resetTime(0.0f);
    CountdownTimer ct;
    ct.start(5.0f);

    g_time = 8.0f;  // 3 seconds past expiry
    REQUIRE(ct.elapsedTime() == Catch::Approx(8.0f).epsilon(0.01f));
}

TEST_CASE("CountdownTimer countdownDuration returns duration when started", "[timers]") {
    resetTime(0.0f);
    CountdownTimer ct;
    ct.start(3.0f);
    REQUIRE(ct.countdownDuration() == Catch::Approx(3.0f));
}

TEST_CASE("CountdownTimer countdownDuration returns 0 when not started", "[timers]") {
    resetTime(0.0f);
    CountdownTimer ct;
    REQUIRE(ct.countdownDuration() == Catch::Approx(0.0f));
}

TEST_CASE("CountdownTimer reset restarts with same duration", "[timers]") {
    resetTime(0.0f);
    CountdownTimer ct;
    ct.start(5.0f);

    g_time = 3.0f;
    ct.reset();   // new expiry at t=3+5=8

    g_time = 7.0f;
    REQUIRE_FALSE(ct.elapsed());

    g_time = 9.0f;
    REQUIRE(ct.elapsed());
}

TEST_CASE("CountdownTimer invalidate stops the timer", "[timers]") {
    resetTime(0.0f);
    CountdownTimer ct;
    ct.start(5.0f);
    ct.invalidate();
    REQUIRE_FALSE(ct.started());
}

TEST_CASE("CountdownTimer constructed with duration starts immediately", "[timers]") {
    resetTime(0.0f);
    CountdownTimer ct(4.0f);
    REQUIRE(ct.started());
    REQUIRE_FALSE(ct.elapsed());

    g_time = 5.0f;
    REQUIRE(ct.elapsed());
}

// ===========================================================================
// IntervalTimer
// ===========================================================================

TEST_CASE("IntervalTimer default construction is not started", "[timers]") {
    resetTime(0.0f);
    IntervalTimer it;
    REQUIRE_FALSE(it.started());
}

TEST_CASE("IntervalTimer start marks the current time", "[timers]") {
    resetTime(5.0f);
    IntervalTimer it;
    it.start();
    REQUIRE(it.started());
}

TEST_CASE("IntervalTimer elapsedTime measures since start", "[timers]") {
    resetTime(2.0f);
    IntervalTimer it;
    it.start();

    g_time = 7.0f;
    REQUIRE(it.elapsedTime() == Catch::Approx(5.0f).epsilon(0.01f));
}

TEST_CASE("IntervalTimer elapsedTime returns max when not started", "[timers]") {
    resetTime(0.0f);
    IntervalTimer it;
    REQUIRE(it.elapsedTime() == Catch::Approx(detail::kMaxTimerValue).epsilon(1.0f));
}

TEST_CASE("IntervalTimer lessThen returns true when within duration", "[timers]") {
    resetTime(0.0f);
    IntervalTimer it;
    it.start();

    g_time = 3.0f;
    REQUIRE(it.lessThen(5.0f));
    REQUIRE_FALSE(it.lessThen(2.0f));
}

TEST_CASE("IntervalTimer greaterThen returns true after duration", "[timers]") {
    resetTime(0.0f);
    IntervalTimer it;
    it.start();

    g_time = 10.0f;
    REQUIRE(it.greaterThen(5.0f));
    REQUIRE_FALSE(it.greaterThen(15.0f));
}

TEST_CASE("IntervalTimer reset re-anchors to current time", "[timers]") {
    resetTime(0.0f);
    IntervalTimer it;
    it.start();

    g_time = 10.0f;
    it.reset();  // now anchored at t=10

    g_time = 12.0f;
    REQUIRE(it.elapsedTime() == Catch::Approx(2.0f).epsilon(0.01f));
}

TEST_CASE("IntervalTimer invalidate sets timestamp to positive value", "[timers]") {
    resetTime(0.0f);
    IntervalTimer it;
    it.start();
    it.invalidate();
    // invalidate() sets timestamp_ = -kInvalidTimerValue = -(-1) = 1.0f
    // so started() (timestamp_ > 0) still returns true.
    // This differs from CountdownTimer's invalidate() which resets to -1.
    REQUIRE(it.started());
}
