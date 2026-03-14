// SPDX-License-Identifier: Unlicense

#pragma once

#include <crlib/basic.h>
#include <crlib/memory.h>
#include <crlib/movable.h>

CR_NAMESPACE_BEGIN

template <typename T> class BinaryHeap final : public NonCopyable {
private:
   T *data_ = nullptr;
   size_t size_ = 0;
   size_t capacity_ = 0;

public:
   explicit BinaryHeap () = default;

   BinaryHeap (BinaryHeap &&rhs) noexcept
      : data_ (rhs.data_), size_ (rhs.size_), capacity_ (rhs.capacity_) {
      rhs.data_ = nullptr;
      rhs.size_ = 0;
      rhs.capacity_ = 0;
   }

   ~BinaryHeap () {
      destroy ();
   }

public:
   bool push (const T &item) {
      if (!ensure (size_ + 1)) {
         return false;
      }
      mem::construct (&data_[size_], item);
      siftUp (size_);
      ++size_;
      return true;
   }

   bool push (T &&item) {
      if (!ensure (size_ + 1)) {
         return false;
      }
      mem::construct (&data_[size_], cr::move (item));
      siftUp (size_);
      ++size_;
      return true;
   }

   template <typename ...Args> bool emplace (Args &&...args) {
      if (!ensure (size_ + 1)) {
         return false;
      }
      mem::construct (&data_[size_], cr::forward <Args> (args)...);
      siftUp (size_);
      ++size_;
      return true;
   }

   const T &top () const {
      return data_[0];
   }

   T pop () {
      auto result = cr::move (data_[0]);
      --size_;

      if (size_ > 0) {
         siftDown (0, cr::move (data_[size_]));
      }
      return result;
   }

   void pop (T &out) {
      out = cr::move (data_[0]);
      --size_;

      if (size_ > 0) {
         siftDown (0, cr::move (data_[size_]));
      }
   }

public:
   size_t length () const {
      return size_;
   }

   bool empty () const {
      return size_ == 0;
   }

   void clear () {
      for (size_t i = 0; i < size_; ++i) {
         mem::destruct (&data_[i]);
      }
      size_ = 0;
   }

   void swap (BinaryHeap &other) noexcept {
      cr::swap (data_, other.data_);
      cr::swap (size_, other.size_);
      cr::swap (capacity_, other.capacity_);
   }

   BinaryHeap &operator = (BinaryHeap &&rhs) noexcept {
      if (this != &rhs) {
         destroy ();
         data_ = rhs.data_;
         size_ = rhs.size_;
         capacity_ = rhs.capacity_;
         rhs.data_ = nullptr;
         rhs.size_ = 0;
         rhs.capacity_ = 0;
      }
      return *this;
   }

private:
   void destroy () {
      if (data_) {
         for (size_t i = 0; i < size_; ++i) {
            mem::destruct (&data_[i]);
         }
         mem::release (data_);
         data_ = nullptr;
      }
   }

   bool ensure (size_t needed) {
      if (needed <= capacity_) {
         return true;
      }
      size_t newCap = capacity_ == 0 ? 8 : capacity_ * 2;

      while (newCap < needed) {
         newCap *= 2;
      }
      auto newData = mem::allocate <T> (newCap);

      if (!newData) {
         return false;
      }
      for (size_t i = 0; i < size_; ++i) {
         mem::construct (&newData[i], cr::move (data_[i]));
         mem::destruct (&data_[i]);
      }
      mem::release (data_);
      data_ = newData;
      capacity_ = newCap;
      return true;
   }

   void siftUp (size_t index) {
      if (index == 0) {
         return;
      }
      T *const data = data_;
      T value = cr::move (data[index]);

      while (index > 0) {
         size_t parent = (index - 1) / 2;

         if (!(value < data[parent])) {
            break;
         }
         data[index] = cr::move (data[parent]);
         index = parent;
      }
      data[index] = cr::move (value);
   }

   void siftDown (size_t hole, T &&value) {
      T *const data = data_;
      const size_t size = size_;
      const size_t half = size / 2;

      while (hole < half) {
         size_t left = hole * 2 + 1;
         size_t best = left;
         size_t right = left + 1;

         if (right < size && data[right] < data[left]) {
            best = right;
         }
         if (!(data[best] < value)) {
            break;
         }
         data[hole] = cr::move (data[best]);
         hole = best;
      }
      data[hole] = cr::move (value);
   }
};

CR_NAMESPACE_END
