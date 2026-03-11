//
// crlib, simple class library for private needs.
// Copyright © RWSH Solutions LLC <lab@rwsh.ru>.
//
// SPDX-License-Identifier: MIT
//

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

   BinaryHeap (BinaryHeap &&rhs) noexcept {
      data_ = rhs.data_;
      size_ = rhs.size_;
      capacity_ = rhs.capacity_;
      rhs.data_ = nullptr;
      rhs.size_ = 0;
      rhs.capacity_ = 0;
   }

   ~BinaryHeap () {
      if (data_) {
         for (size_t i = 0; i < size_; ++i) {
            Memory::destruct (&data_[i]);
         }
         Memory::release (data_);
      }
   }

public:
   bool push (const T &item) {
      if (!ensure (size_ + 1)) {
         return false;
      }
      Memory::construct (&data_[size_], item);
      ++size_;
      if (size_ > 1) {
         percolateUp (size_ - 1);
      }
      return true;
   }

   bool push (T &&item) {
      if (!ensure (size_ + 1)) {
         return false;
      }
      Memory::construct (&data_[size_], cr::move (item));
      ++size_;
      if (size_ > 1) {
         percolateUp (size_ - 1);
      }
      return true;
   }

   template <typename ...Args> bool emplace (Args &&...args) {
      if (!ensure (size_ + 1)) {
         return false;
      }
      Memory::construct (&data_[size_], cr::forward <Args> (args)...);
      ++size_;
      if (size_ > 1) {
         percolateUp (size_ - 1);
      }
      return true;
   }

   const T &top () const {
      return data_[0];
   }

   T pop () {
      auto key = cr::move (data_[0]);

      if (size_ == 1) {
         Memory::destruct (&data_[0]);
         --size_;
         return key;
      }

      cr::swap (data_[0], data_[size_ - 1]);
      Memory::destruct (&data_[size_ - 1]);
      --size_;

      percolateDown (0);
      return key;
   }

   void pop (T &out) {
      out = cr::move (data_[0]);

      if (size_ == 1) {
         Memory::destruct (&data_[0]);
         --size_;
         return;
      }

      Memory::destruct (&data_[0]);
      --size_;
      data_[0] = cr::move (data_[size_]);
      Memory::destruct (&data_[size_]);

      percolateDown (0);
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
         Memory::destruct (&data_[i]);
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
         this->~BinaryHeap ();
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
   bool ensure (size_t needed) {
      if (needed <= capacity_) {
         return true;
      }
      size_t newCap = capacity_ == 0 ? 8 : capacity_ * 2;
      while (newCap < needed) {
         newCap *= 2;
      }
      T *newData = static_cast <T *> (Memory::get <T> (newCap));
      if (!newData) {
         return false;
      }
      for (size_t i = 0; i < size_; ++i) {
         Memory::construct (&newData[i], cr::move (data_[i]));
         Memory::destruct (&data_[i]);
      }
      Memory::release (data_);
      data_ = newData;
      capacity_ = newCap;
      return true;
   }

   void percolateUp (size_t index) {
      while (index > 0) {
         size_t parent = (index - 1) / 2;
         if (!(data_[parent] > data_[index])) {
            break;
         }
         cr::swap (data_[parent], data_[index]);
         index = parent;
      }
   }

   void percolateDown (size_t index) {
      while (true) {
         size_t left = index * 2 + 1;
         if (left >= size_) {
            break;
         }
         size_t best = left;
         size_t right = left + 1;
         if (right < size_ && data_[right] < data_[best]) {
            best = right;
         }
         if (!(data_[index] > data_[best])) {
            break;
         }
         cr::swap (data_[index], data_[best]);
         index = best;
      }
   }
};

CR_NAMESPACE_END
