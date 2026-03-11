//
// crlib, simple class library for private needs.
// Copyright © RWSH Solutions LLC <lab@rwsh.ru>.
//
// SPDX-License-Identifier: MIT
//

#pragma once

#include <crlib/memory.h>

CR_NAMESPACE_BEGIN

template <typename> class Lambda;
template <typename R, typename ...Args> class Lambda <R (Args...)> final {
private:
   static constexpr uint32_t kSmallBufferSize = sizeof (void *) * 3;

private:
   class LambdaWrapper : public NonCopyable {
   public:
      explicit LambdaWrapper () = default;
      virtual ~LambdaWrapper () = default;

   public:
      virtual R invoke (Args &&... args) const = 0;
      virtual LambdaWrapper *clone (void *obj) const = 0;
      virtual LambdaWrapper *move (void *obj) = 0;
   };

   template <typename T> class LambdaFunctor final : public LambdaWrapper {
   private:
      T callee_;

   public:
      constexpr LambdaFunctor (const LambdaFunctor &rhs) : callee_ { rhs.callee_ } {}
      constexpr LambdaFunctor (LambdaFunctor &&rhs) noexcept : callee_ { cr::move (rhs.callee_) } {}
      constexpr LambdaFunctor (const T &callee) : callee_ { callee } {}
      constexpr LambdaFunctor (T &&callee) : callee_ { cr::move (callee) } {}

      virtual ~LambdaFunctor () = default;

   public:
      virtual R invoke (Args &&... args) const override {
         return callee_ (cr::forward <Args> (args)...);
      }

      virtual LambdaWrapper *clone (void *obj) const override {
         if (!obj) {
            return Memory::getAndConstruct <LambdaFunctor> (*this);
         }
         return Memory::construct (reinterpret_cast <LambdaFunctor *> (obj), *this);
      }

      virtual LambdaWrapper *move (void *obj) override {
         return Memory::construct (reinterpret_cast <LambdaFunctor *> (obj), cr::move (*this));
      }
   };

private:
   LambdaWrapper *lambda_ {};
   bool owns_ {};

   union {
      double alignment_;
      uint8_t alias_[kSmallBufferSize];
   } storage_ {};

private:
   constexpr void *small () {
      return storage_.alias_;
   }

   constexpr bool isSmall () const {
      return lambda_ == reinterpret_cast <LambdaWrapper *> (storage_.alias_);
   }

public:
   void destroy () noexcept {
      if (!lambda_) {
         return;
      }

      if (owns_) {
         lambda_->~LambdaWrapper ();
      }
      else {
         Memory::release (lambda_);
      }
      lambda_ = nullptr;
      owns_ = false;
   }

   void moveApply (Lambda &&rhs) noexcept {
      if (!rhs) {
         lambda_ = nullptr;
         owns_ = false;
      }
      else if (rhs.owns_) {
         lambda_ = rhs.lambda_->move (small ());
         owns_ = true;
         rhs.lambda_ = nullptr;
         rhs.owns_ = false;
      }
      else {
         lambda_ = rhs.lambda_;
         owns_ = false;
         rhs.lambda_ = nullptr;
         rhs.owns_ = false;
      }
   }

   template <typename U> void assign (U &&fn) {
      destroy ();
      using Type = LambdaFunctor <typename cr::decay <U>::type>;

      if constexpr (sizeof (Type) > sizeof (storage_)) {
         lambda_ = Memory::getAndConstruct <Type> (cr::forward <U> (fn));
         owns_ = false;
      }
      else {
         lambda_ = Memory::construct (reinterpret_cast <Type *> (small ()), cr::forward <U> (fn));
         owns_ = true;
      }
   }

public:
   Lambda () : lambda_ { nullptr }, owns_ { false } {}
   Lambda (nullptr_t) : lambda_ { nullptr }, owns_ { false } {}

public:
   Lambda (const Lambda &rhs) : lambda_ { nullptr }, owns_ { false } {
      if (!rhs) {
         return;
      }
      if (rhs.owns_) {
         lambda_ = rhs.lambda_->clone (small ());
         owns_ = true;
      }
      else {
         lambda_ = rhs.lambda_->clone (nullptr);
         owns_ = false;
      }
   }

   Lambda (Lambda &&rhs) noexcept : lambda_ { nullptr }, owns_ { false } {
      moveApply (cr::forward <Lambda> (rhs));
   }

   template <typename U> Lambda (U &&obj) : lambda_ { nullptr }, owns_ { false } {
      assign (cr::forward <U> (obj));
   }

   ~Lambda () {
      destroy ();
   }

public:
   explicit operator bool () const {
      return !!lambda_;
   }

   decltype (auto) operator () (Args... args) const {
      assert (lambda_);
      return lambda_->invoke (cr::forward <Args> (args)...);
   }

public:
   Lambda &operator = (nullptr_t) {
      destroy ();
      return *this;
   }

   Lambda &operator = (const Lambda &rhs) noexcept {
      if (this == &rhs) {
         return *this;
      }
      destroy ();
      if (!rhs) {
         return *this;
      }
      if (rhs.owns_) {
         lambda_ = rhs.lambda_->clone (small ());
         owns_ = true;
      }
      else {
         lambda_ = rhs.lambda_->clone (nullptr);
         owns_ = false;
      }
      return *this;
   }

   Lambda &operator = (Lambda &&rhs) noexcept {
      if (this == &rhs) {
         return *this;
      }
      destroy ();
      moveApply (cr::move (rhs));
      return *this;
   }

   template <typename U, typename = cr::enable_if_t<!cr::is_same <cr::remove_cv_t <cr::remove_reference_t <U>>, Lambda>::value>> Lambda &operator = (U &&rhs) noexcept {
      destroy ();
      assign (cr::forward <U> (rhs));
      return *this;
   }
};

CR_NAMESPACE_END
