// SPDX-License-Identifier: Unlicense

#pragma once

#include <crlib/memory.h>
#include <crlib/platform.h>

// override all the operators to get rid of linking to stdc++
void *operator new (size_t size) {
   return cr::mem::allocate <void *> (size);
}

void *operator new [] (size_t size) {
   return cr::mem::allocate <void *> (size);
}

void operator delete (void *ptr) noexcept {
   cr::mem::release (ptr);
}

void operator delete [] (void *ptr) noexcept {
   cr::mem::release (ptr);
}

void operator delete (void *ptr, size_t) noexcept {
   cr::mem::release (ptr);
}

void operator delete [] (void *ptr, size_t) noexcept {
   cr::mem::release (ptr);
}

CR_C_LINKAGE void __cxa_pure_virtual () {
   cr::plat.abort ("pure virtual function call");
}
