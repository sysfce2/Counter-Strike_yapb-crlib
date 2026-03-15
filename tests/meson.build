catch2_lib = static_library(
  'catch2_impl',
  'catch2/catch_amalgamated.cpp',
  include_directories : include_directories('catch2'),
  install : false,
)

test_sources = files(
  'test_basic.cpp',
  'test_array.cpp',
  'test_binheap.cpp',
  'test_color.cpp',
  'test_deque.cpp',
  'test_hashmap.cpp',
  'test_twin.cpp',
  'test_uniqueptr.cpp',
  'test_string.cpp',
  'test_mathlib.cpp',
  'test_vector.cpp',
  'test_random.cpp',
  'test_lambda.cpp',
  'test_timers.cpp',
  'test_ulz.cpp',
  'test_traits.cpp',
  'test_movable.cpp',
  'test_memory.cpp',
  'test_wavehelper.cpp',
  'test_cpuflags.cpp',
  'test_platform.cpp',
  'test_files.cpp',
  'test_library.cpp',
  'test_thread.cpp',
  'test_logger.cpp',
  'test_http.cpp',
  'test_detour.cpp',
  'test_simd.cpp',
)

benchmark_sources = files(
  'benchmark_hashmap.cpp',
  'benchmark_binheap.cpp',
  'benchmark_array.cpp',
  'benchmark_string.cpp',
  'benchmark_deque.cpp',
)

# --- Cross-platform configuration ---

cpp = meson.get_compiler('cpp')
host_system = host_machine.system()

# 1. Dependencies (libraries)
# find_library returns an empty object if the library is not needed or not found (depending on required)
deps = []

# Windows Sockets
if host_system == 'windows'
  deps += cpp.find_library('ws2_32', required : true)
endif

# POSIX Threads (Linux, macOS, *BSD)
# Usually not required for Windows (native threads there), but checking
if host_system != 'windows'
  deps += cpp.find_library('pthread', required : true)
endif

# 2. Compiler arguments
# Use add_project_arguments or pass to executable. 
# Passing to executable here to isolate tests.
test_args = [
  '-DCR_COMPAT_STL',
]

# Windows Specific Flags (MSVC)
if host_system == 'windows'
  # Check compiler, as /FI is specific to MSVC/Clang-cl
  if cpp.get_id() == 'msvc' or cpp.get_id() == 'clang-cl'
    test_args += [
      '/FInew', 
      '/FIcrlib/crlib.h'
    ]
  # If using MinGW (gcc on windows), flags will be different (-include)
  elif cpp.get_id() == 'gcc'
    test_args += [
      '-include', 'new',
      '-include', 'crlib/crlib.h'
    ]
  endif
else
  # Unix-like (GCC, Clang)
  test_args += [
    '-include', 'crlib/crlib.h'
  ]
endif

# --- Directories ---
# Global includes for the whole project
global_inc = include_directories('..')
# Includes for tests (current folder)
test_inc = include_directories('.')

crlib_tests = executable(
  'crlib_tests',
  test_sources,
  include_directories : [global_inc, test_inc],
  link_with : catch2_lib,
  dependencies : deps,
  cpp_args : test_args,
  install : false,
)

crlib_benchmarks = executable(
  'crlib_benchmarks',
  benchmark_sources,
  include_directories : [global_inc, test_inc],
  link_with : catch2_lib,
  dependencies : deps,
  cpp_args : test_args,
  install : false,
)

# --- Tests ---
test('crlib_tests', crlib_tests,
  args : ['--reporter', 'console'],
  timeout : 1200
)

test('crlib_benchmarks', crlib_benchmarks,
  args : ['--reporter', 'console'],
  timeout : 300
)