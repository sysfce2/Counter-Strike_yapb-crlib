// test_library.cpp — tests for crlib/library.h (SharedLibrary)
#include <crlib/crlib.h>
#include "catch2/catch_amalgamated.hpp"

using namespace cr;

// ---------------------------------------------------------------------------
// Default construction
// ---------------------------------------------------------------------------
TEST_CASE("SharedLibrary default-constructed is not valid", "[library]") {
    SharedLibrary lib;
    REQUIRE(!lib);
    REQUIRE(lib.handle() == nullptr);
}

// ---------------------------------------------------------------------------
// Load failure
// ---------------------------------------------------------------------------
TEST_CASE("SharedLibrary load of non-existent module fails", "[library]") {
    SharedLibrary lib;
    bool ok = lib.load("crlib_nonexistent_xyz_123.dll");
    REQUIRE(!ok);
    REQUIRE(!lib);
}

// ---------------------------------------------------------------------------
// Load from string constructor failure
// ---------------------------------------------------------------------------
TEST_CASE("SharedLibrary string-ctor with bad path leaves it invalid", "[library]") {
    SharedLibrary lib("crlib_nonexistent_module.dll");
    REQUIRE(!lib);
}

// ---------------------------------------------------------------------------
// Empty string constructor
// ---------------------------------------------------------------------------
TEST_CASE("SharedLibrary empty string constructor does not crash", "[library]") {
    SharedLibrary lib("");
    REQUIRE(!lib);
}

// ---------------------------------------------------------------------------
// resolve on invalid library returns nullptr
// ---------------------------------------------------------------------------
TEST_CASE("SharedLibrary resolve on invalid library returns nullptr", "[library]") {
    SharedLibrary lib;
    using FnType = void(*)();
    FnType fn = lib.resolve<FnType>("SomeFunction");
    REQUIRE(fn == nullptr);
}

#if defined(CR_WINDOWS)
// ---------------------------------------------------------------------------
// Load a known Windows system DLL
// ---------------------------------------------------------------------------
TEST_CASE("SharedLibrary loads kernel32.dll successfully on Windows", "[library]") {
    SharedLibrary lib("kernel32.dll");
    REQUIRE(lib);
    REQUIRE(lib.handle() != nullptr);
}

TEST_CASE("SharedLibrary resolves GetProcAddress from kernel32.dll", "[library]") {
    SharedLibrary lib("kernel32.dll");
    REQUIRE(lib);

    using GetCurrentProcessIdFn = unsigned long (__stdcall *)();
    auto fn = lib.resolve<GetCurrentProcessIdFn>("GetCurrentProcessId");
    REQUIRE(fn != nullptr);

    // Call it and verify it returns current PID
    unsigned long pid = fn();
    REQUIRE(pid > 0);
}

TEST_CASE("SharedLibrary getSymbol static method works", "[library]") {
    SharedLibrary lib("kernel32.dll");
    REQUIRE(lib);

    using GetCurrentProcessIdFn = unsigned long (__stdcall *)();
    auto fn = SharedLibrary::getSymbol<GetCurrentProcessIdFn>(lib.handle(), "GetCurrentProcessId");
    REQUIRE(fn != nullptr);
}

TEST_CASE("SharedLibrary path returns non-empty path for loaded module", "[library]") {
    SharedLibrary lib("kernel32.dll");
    REQUIRE(lib);

    // path() takes any address inside a module; use a well-known exported fn
    // We resolve and cast to void* for path lookup
    using GetCurrentProcessIdFn = unsigned long (__stdcall *)();
    auto fn = lib.resolve<GetCurrentProcessIdFn>("GetCurrentProcessId");
    REQUIRE(fn != nullptr);

    String p = SharedLibrary::path(reinterpret_cast<void*>(fn));
    // Should contain "kernel32" (case-insensitive not tested here, just non-empty)
    REQUIRE(!p.empty());
}

      TEST_CASE("SharedLibrary unload invalidates the handle", "[library]") {
         SharedLibrary lib("kernel32.dll");
         REQUIRE(lib);
         lib.unload();
         REQUIRE(!lib);
      }
#endif

#if !defined(CR_WINDOWS)
#include <dlfcn.h>
// ---------------------------------------------------------------------------
// Load a known Unix system shared library
// ---------------------------------------------------------------------------
TEST_CASE("SharedLibrary loads libc.so successfully on Unix", "[library]") {
   SharedLibrary lib("libc.so.6");
   if (!lib) {
      lib = SharedLibrary("libc.so");
   }
   if (lib) {
      REQUIRE(lib.handle() != nullptr);
   }
}

TEST_CASE("SharedLibrary resolves dlsym from loaded module", "[library]") {
   SharedLibrary lib("libc.so.6");
   if (!lib) {
      lib = SharedLibrary("libc.so");
   }
   if (lib) {
      using PrintfFn = int (*)(const char*, ...);
      auto fn = lib.resolve<PrintfFn>("printf");
      REQUIRE(fn != nullptr);
      fn("dlsym test\n");
   } else {
      REQUIRE(true); // skip test if library not found
   }
}

TEST_CASE("SharedLibrary getSymbol static method works on Unix", "[library]") {
   SharedLibrary lib("libc.so.6");
   if (!lib) {
      lib = SharedLibrary("libc.so");
   }
   if (lib) {
      using PrintfFn = int (*)(const char*, ...);
      auto fn = SharedLibrary::getSymbol<PrintfFn>(lib.handle(), "printf");
      REQUIRE(fn != nullptr);
   } else {
      REQUIRE(true); // skip test if library not found
   }
}

TEST_CASE("SharedLibrary path returns non-empty path for loaded module on Unix", "[library]") {
   SharedLibrary lib("libc.so.6");
   if (!lib) {
      lib = SharedLibrary("libc.so");
   }
   if (lib) {
      using PrintfFn = int (*)(const char*, ...);
      auto fn = lib.resolve<PrintfFn>("printf");
      REQUIRE(fn != nullptr);
      String p = SharedLibrary::path(reinterpret_cast<void*>(fn));
      REQUIRE(!p.empty());
   } else {
      REQUIRE(true); // skip test if library not found
   }
}

TEST_CASE("SharedLibrary unload invalidates the handle on Unix", "[library]") {
   SharedLibrary lib("libc.so.6");
   if (!lib) {
      lib = SharedLibrary("libc.so");
   }
   if (lib) {
      REQUIRE(lib.valid());
      lib.unload();
      REQUIRE(!lib);
   } else {
      REQUIRE(true); // skip test if library not found
   }
}

#endif

// ---------------------------------------------------------------------------
// hasModule
// ---------------------------------------------------------------------------
#if defined(CR_WINDOWS)
TEST_CASE("SharedLibrary hasModule returns true for kernel32.dll", "[library]") {
    REQUIRE(SharedLibrary::hasModule("kernel32.dll"));
}
#endif

TEST_CASE("SharedLibrary hasModule returns false for nonexistent module", "[library]") {
    REQUIRE(!SharedLibrary::hasModule("crlib_nonexistent_module_xyz.dll"));
}

// ---------------------------------------------------------------------------
// hasModule for loaded module on Unix
// ---------------------------------------------------------------------------
#if defined(CR_LINUX)
TEST_CASE("SharedLibrary hasModule returns true for loaded libc.so on Linux", "[library]") {
    bool hasLibc6 = SharedLibrary::hasModule("libc.so.6");
    bool hasLibc = SharedLibrary::hasModule("libc.so");
    bool result = hasLibc6 || hasLibc;
    REQUIRE(result);
}
#elif defined(CR_MACOS)
TEST_CASE("SharedLibrary hasModule returns true for loaded libSystem on macOS", "[library]") {
    bool result = SharedLibrary::hasModule("libSystem.B.dylib");
    REQUIRE(result);
}
#endif

// ---------------------------------------------------------------------------
// Move constructor
// ---------------------------------------------------------------------------
#if defined(CR_WINDOWS)
TEST_CASE("SharedLibrary move constructor transfers handle", "[library]") {
    SharedLibrary lib("kernel32.dll");
    REQUIRE(lib);
    void *originalHandle = lib.handle();

    SharedLibrary lib2(cr::move(lib));
    REQUIRE(lib2.handle() == originalHandle);
    REQUIRE(!lib); // NOLINT - intentionally checking moved-from state
}
#endif

// ---------------------------------------------------------------------------
// Move assignment
// ---------------------------------------------------------------------------
#if defined(CR_WINDOWS)
TEST_CASE("SharedLibrary move assignment transfers handle", "[library]") {
    SharedLibrary lib("kernel32.dll");
    REQUIRE(lib);
    void *originalHandle = lib.handle();

    SharedLibrary lib2;
    lib2 = cr::move(lib);
    REQUIRE(lib2.handle() == originalHandle);
    REQUIRE(!lib); // NOLINT - intentionally checking moved-from state
}
#endif

// ---------------------------------------------------------------------------
// locate() - find module by address
// ---------------------------------------------------------------------------
#if defined(CR_WINDOWS)
TEST_CASE("SharedLibrary locate finds kernel32.dll by address", "[library]") {
    SharedLibrary lib;
    HMODULE h = GetModuleHandleA("kernel32.dll");
    REQUIRE(h != nullptr);

    bool ok = lib.locate(reinterpret_cast<void*>(h));
    REQUIRE(ok);
    REQUIRE(lib);
}
#else
TEST_CASE("SharedLibrary locate finds libc.so by address on Unix", "[library]") {
    SharedLibrary lib;
    void *handle = dlopen("libc.so.6", RTLD_LAZY);
    if (!handle) {
       handle = dlopen("libc.so", RTLD_LAZY);
    }
    if (handle) {
       // Get an address from the loaded library
       void *funcAddr = dlsym(handle, "printf");
       REQUIRE(funcAddr != nullptr);

       bool ok = lib.locate(funcAddr);
       REQUIRE(ok);
       REQUIRE(lib);
        dlclose(handle);
     } else {
        REQUIRE(true); // skip if not found
     }
}
#endif

// ---------------------------------------------------------------------------
// Additional tests for missing coverage
// ---------------------------------------------------------------------------
TEST_CASE("SharedLibrary load with unloadable=false prevents unloading", "[library]") {
#if defined(CR_WINDOWS)
    SharedLibrary lib;
    bool ok = lib.load("kernel32.dll", false); // unloadable = false
    REQUIRE(ok);
    REQUIRE(lib);
    
    lib.unload(); // Should not actually unload
    REQUIRE(lib); // Still valid because unloadable=false
#endif
}

TEST_CASE("SharedLibrary locate with invalid address returns false", "[library]") {
    SharedLibrary lib;
    bool ok = lib.locate(reinterpret_cast<void*>(static_cast<uintptr_t>(0xDEADBEEF)));
    REQUIRE(!ok);
    REQUIRE(!lib);
}

TEST_CASE("SharedLibrary path with invalid address returns empty string", "[library]") {
    String p = SharedLibrary::path(reinterpret_cast<void*>(static_cast<uintptr_t>(0xDEADBEEF)));
    REQUIRE(p.empty());
}

TEST_CASE("SharedLibrary double unload is safe", "[library]") {
#if defined(CR_WINDOWS)
    SharedLibrary lib("kernel32.dll");
    REQUIRE(lib);
    lib.unload();
    REQUIRE(!lib);
    lib.unload(); // Second unload should be safe
    REQUIRE(!lib);
#endif
}

TEST_CASE("SharedLibrary resolve with empty function name returns nullptr", "[library]") {
#if defined(CR_WINDOWS)
    SharedLibrary lib("kernel32.dll");
    REQUIRE(lib);
    using FnType = void(*)();
    auto fn = lib.resolve<FnType>("");
    REQUIRE(fn == nullptr);
#endif
}

TEST_CASE("SharedLibrary load unloads previous library", "[library]") {
#if defined(CR_WINDOWS)
    SharedLibrary lib("kernel32.dll");
    REQUIRE(lib);
    void* firstHandle = lib.handle();
    
    // Load different library (should unload kernel32 first)
    bool ok = lib.load("user32.dll");
    REQUIRE(ok);
    REQUIRE(lib);
    REQUIRE(lib.handle() != firstHandle);
#endif
}

TEST_CASE("SharedLibrary valid method returns same as operator bool", "[library]") {
    SharedLibrary lib;
    REQUIRE(!lib.valid());
    REQUIRE(!lib);
    
#if defined(CR_WINDOWS)
    lib.load("kernel32.dll");
    REQUIRE(lib.valid());
    REQUIRE(lib);
#endif
}

TEST_CASE("SharedLibrary getSymbol with nullptr handle returns nullptr", "[library]") {
    using FnType = void(*)();
    auto fn = SharedLibrary::getSymbol<FnType>(nullptr, "SomeFunction");
    REQUIRE(fn == nullptr);
}

TEST_CASE("SharedLibrary resolve after unload returns nullptr", "[library]") {
#if defined(CR_WINDOWS)
    SharedLibrary lib("kernel32.dll");
    REQUIRE(lib);
    
    using GetCurrentProcessIdFn = unsigned long (__stdcall *)();
    auto fn = lib.resolve<GetCurrentProcessIdFn>("GetCurrentProcessId");
    REQUIRE(fn != nullptr);
    
    lib.unload();
    REQUIRE(!lib);
    
    auto fn2 = lib.resolve<GetCurrentProcessIdFn>("GetCurrentProcessId");
    REQUIRE(fn2 == nullptr);
#endif
}
