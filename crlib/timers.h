//
// crlib, simple class library for private needs.
// Copyright © RWSH Solutions LLC <lab@rwsh.ru>.
//
// SPDX-License-Identifier: MIT
//

#pragma once

CR_NAMESPACE_BEGIN

// set the storage time function
static class TimerStorage final : public NonCopyable {
private:
   Lambda <float ()> timefn_;

public:
   constexpr TimerStorage () = default;
   ~TimerStorage () = default;

public:
   constexpr void setTimeFunction (Lambda <float ()> &&tfn) {
      timefn_ = cr::move (tfn);
   }

public:
   constexpr float get () const {
      return timefn_ ();
   }
} timerStorage;

// invalid timer value
namespace detail {
   constexpr float kInvalidTimerValue = -1.0f;
   constexpr float kMaxTimerValue = 1.0e9f;
}

// simple class for counting down a short interval of time
class CountdownTimer {
private:
   float duration_ { 0.0f };
   float timestamp_ { detail::kInvalidTimerValue };

public:
   constexpr CountdownTimer () = default;
   explicit constexpr CountdownTimer (const float duration) { start (duration); }

public:
   constexpr void reset () { timestamp_ = timerStorage.get () + duration_; }

   constexpr void start (const float duration) {
      duration_ = duration;
      reset ();
   }

   constexpr void invalidate () {
      timestamp_ = detail::kInvalidTimerValue;
   }

public:
   constexpr bool started () const {
      return timestamp_ > 0.0f;
   }

   constexpr bool elapsed () const {
      return timestamp_ < timerStorage.get ();
   }

   constexpr float elapsedTime () const {
      return timerStorage.get () - timestamp_ + duration_;
   }

   constexpr float timestamp () const {
      return timestamp_;
   }

   constexpr float remainingTime () const {
      return timestamp_ - timerStorage.get ();
   }

   constexpr float countdownDuration () const {
      return started () ? duration_ : 0.0f;
   }
};

// simple class for tracking intervals of time
class IntervalTimer {
private:
   float timestamp_ { detail::kInvalidTimerValue };

public:
   constexpr IntervalTimer () = default;

public:
   constexpr void reset () {
      timestamp_ = timerStorage.get ();
   }

   constexpr void start () {
      timestamp_ = timerStorage.get ();
   }

   constexpr void invalidate () {
      timestamp_ = -detail::kInvalidTimerValue;
   }

public:
   constexpr bool started () const {
      return timestamp_ > 0.0f;
   }

   constexpr float elapsedTime () const {
      return started () ? timerStorage.get () - timestamp_ : detail::kMaxTimerValue;
   }

   constexpr bool lessThen (const float duration) const {
      return timerStorage.get () - timestamp_ < duration;
   }

   constexpr bool greaterThen (const float duration) const {
      return timerStorage.get () - timestamp_ > duration;
   }
};

CR_NAMESPACE_END
