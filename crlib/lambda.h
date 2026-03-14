// SPDX-License-Identifier: Unlicense

#pragma once

#include <crlib/memory.h>

CR_NAMESPACE_BEGIN

template <typename> class Lambda;
template <typename R, typename ...Args> class Lambda <R (Args...)> final {
private:
   static constexpr size_t kSmallBufferSize = sizeof (void *) * 3;

   // type-erased callable interface
   class Holder {
   public:
      Holder () = default;
      virtual ~Holder () = default;

      Holder (const Holder &) = delete;
      Holder &operator = (const Holder &) = delete;

   public:
      virtual R invoke (Args &&... args) const = 0;
      virtual Holder *cloneInto (void *buf) const = 0;
      virtual Holder *moveInto (void *buf) = 0;
   };

   template <typename T> class CallableHolder final : public Holder {
   private:
      T callee_;

   public:
      CallableHolder (const T &callee) : callee_ (callee) {}
      CallableHolder (T &&callee) : callee_ (cr::move (callee)) {}
      CallableHolder (const CallableHolder &rhs) : callee_ (rhs.callee_) {}
      CallableHolder (CallableHolder &&rhs) noexcept : callee_ (cr::move (rhs.callee_)) {}

   public:
      R invoke (Args &&... args) const override {
         return callee_ (cr::forward <Args> (args)...);
      }

      Holder *cloneInto (void *buf) const override {
         if (buf) {
            return mem::construct (static_cast <CallableHolder *> (buf), *this);
         }
         return mem::allocateAndConstruct <CallableHolder> (*this);
      }

      Holder *moveInto (void *buf) override {
         if (buf) {
            return mem::construct (static_cast <CallableHolder *> (buf), cr::move (*this));
         }
         return mem::allocateAndConstruct <CallableHolder> (cr::move (*this));
      }
   };

private:
   Holder *holder_ {};
   bool small_ {};

   union {
      double alignment_;
      uint8_t buffer_[kSmallBufferSize];
   } storage_ {};

private:
   void *smallBuffer () {
      return storage_.buffer_;
   }

   void destroyHolder () noexcept {
      if (!holder_) {
         return;
      }

      if (small_) {
         holder_->~Holder ();
      }
      else {
         mem::destruct (holder_);
         mem::release (holder_);
      }
      holder_ = nullptr;
      small_ = false;
   }

   template <typename U> void assignCallable (U &&fn) {
      using Callable = CallableHolder <typename cr::decay <U>::type>;

      if constexpr (sizeof (Callable) <= sizeof (storage_)) {
         holder_ = mem::construct (static_cast <Callable *> (smallBuffer ()), cr::forward <U> (fn));
         small_ = true;
      }
      else {
         holder_ = mem::allocateAndConstruct <Callable> (cr::forward <U> (fn));
         small_ = false;
      }
   }

   void copyFrom (const Lambda &rhs) {
      if (!rhs) {
         return;
      }

      if (rhs.small_) {
         holder_ = rhs.holder_->cloneInto (smallBuffer ());
         small_ = true;
      }
      else {
         holder_ = rhs.holder_->cloneInto (nullptr);
         small_ = false;
      }
   }

   void moveFrom (Lambda &&rhs) noexcept {
      if (!rhs) {
         return;
      }

      if (rhs.small_) {
         holder_ = rhs.holder_->moveInto (smallBuffer ());
         small_ = true;

         // destruct the moved-from object still alive in rhs small buffer
         rhs.holder_->~Holder ();
         rhs.holder_ = nullptr;
         rhs.small_ = false;
      }
      else {
         // heap-allocated, just steal the pointer
         holder_ = rhs.holder_;
         small_ = false;

         rhs.holder_ = nullptr;
      }
   }

public:
   Lambda () = default;
   Lambda (nullptr_t) {}

   Lambda (const Lambda &rhs) {
      copyFrom (rhs);
   }

   Lambda (Lambda &&rhs) noexcept {
      moveFrom (cr::move (rhs));
   }

   template <typename U, typename = cr::enable_if_t<!cr::is_same <cr::remove_cv_t <cr::remove_reference_t <U>>, Lambda>::value>>
   Lambda (U &&obj) {
      assignCallable (cr::forward <U> (obj));
   }

   ~Lambda () {
      destroyHolder ();
   }

public:
   explicit operator bool () const {
      return !!holder_;
   }

   decltype (auto) operator () (Args... args) const {
      assert (holder_);
      return holder_->invoke (cr::forward <Args> (args)...);
   }

public:
   Lambda &operator = (nullptr_t) {
      destroyHolder ();
      return *this;
   }

   Lambda &operator = (const Lambda &rhs) {
      if (this != &rhs) {
         destroyHolder ();
         copyFrom (rhs);
      }
      return *this;
   }

   Lambda &operator = (Lambda &&rhs) noexcept {
      if (this != &rhs) {
         destroyHolder ();
         moveFrom (cr::move (rhs));
      }
      return *this;
   }

   template <typename U, typename = cr::enable_if_t<!cr::is_same <cr::remove_cv_t <cr::remove_reference_t <U>>, Lambda>::value>>
   Lambda &operator = (U &&rhs) {
      destroyHolder ();
      assignCallable (cr::forward <U> (rhs));
      return *this;
   }
};

CR_NAMESPACE_END
