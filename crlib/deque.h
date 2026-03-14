// SPDX-License-Identifier: Unlicense

#pragma once

#include <crlib/memory.h>

CR_NAMESPACE_BEGIN

template <typename T> class Deque : public NonCopyable {
private:
   T *contents_ {};
   size_t capacity_ {};
   size_t head_ {};
   size_t length_ {};

private:
   size_t wrap (size_t index) const {
      return index < capacity_ ? index : index - capacity_;
   }

   size_t tail () const {
      return wrap (head_ + length_);
   }

   void grow () {
      const auto capacity = capacity_ > 0 ? capacity_ * 2 : 8;
      auto contents = mem::allocate <T> (capacity);

      if (length_ > 0) {
         if (head_ + length_ <= capacity_) {
            mem::transfer (contents, &contents_[head_], length_);
         }
         else {
            const auto firstPart = capacity_ - head_;

            mem::transfer (contents, &contents_[head_], firstPart);
            mem::transfer (&contents[firstPart], contents_, length_ - firstPart);
         }
      }
      mem::release (contents_);

      contents_ = contents;
      capacity_ = capacity;
      head_ = 0;
   }

   void destructElements () {
      if (length_ == 0) {
         return;
      }

      if (head_ + length_ <= capacity_) {
         mem::destructArray (&contents_[head_], length_);
      }
      else {
         const auto firstPart = capacity_ - head_;

         mem::destructArray (&contents_[head_], firstPart);
         mem::destructArray (contents_, length_ - firstPart);
      }
   }

   void destroy () {
      destructElements ();
      mem::release (contents_);
   }

   void reset () {
      contents_ = nullptr;
      capacity_ = 0;
      head_ = 0;
      length_ = 0;
   }

public:
   explicit Deque () = default;

   Deque (Deque &&rhs) noexcept
      : contents_ (rhs.contents_)
      , capacity_ (rhs.capacity_)
      , head_ (rhs.head_)
      , length_ (rhs.length_) {
      rhs.reset ();
   }

   ~Deque () {
      destroy ();
   }

public:
   bool empty () const {
      return length_ == 0;
   }

   size_t length () const {
      return length_;
   }

   template <typename ...Args> void emplaceFront (Args &&...args) {
      if (length_ == capacity_) {
         grow ();
      }
      head_ = head_ == 0 ? capacity_ - 1 : head_ - 1;

      mem::construct (&contents_[head_], cr::forward <Args> (args)...);
      ++length_;
   }

   template <typename ...Args> void emplaceLast (Args &&...args) {
      if (length_ == capacity_) {
         grow ();
      }
      mem::construct (&contents_[tail ()], cr::forward <Args> (args)...);
      ++length_;
   }

   void discardFront () {
      mem::destruct (&contents_[head_]);

      head_ = wrap (head_ + 1);
      --length_;
   }

   void discardLast () {
      --length_;
      mem::destruct (&contents_[tail ()]);
   }

   T popFront () {
      auto object (cr::move (front ()));
      discardFront ();

      return object;
   }

   T popLast () {
      auto object (cr::move (last ()));
      discardLast ();

      return object;
   }

public:
   const T &front () const {
      return contents_[head_];
   }

   T &front () {
      return contents_[head_];
   }

   const T &last () const {
      return contents_[wrap (head_ + length_ - 1)];
   }

   T &last () {
      return contents_[wrap (head_ + length_ - 1)];
   }

   void clear () {
      destructElements ();

      head_ = 0;
      length_ = 0;
   }

public:
   Deque &operator = (Deque &&rhs) noexcept {
      if (this != &rhs) {
         destroy ();

         contents_ = rhs.contents_;
         capacity_ = rhs.capacity_;
         head_ = rhs.head_;
         length_ = rhs.length_;

         rhs.reset ();
      }
      return *this;
   }
};

CR_NAMESPACE_END
