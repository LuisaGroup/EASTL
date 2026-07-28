# LuisaCompute EASTL Fork Notes

This directory contains a vendored copy of [EASTL](https://github.com/electronicarts/EASTL) with modifications
specific to LuisaCompute. This file documents all deviations from upstream EASTL.

---

## Fix: Smart Pointer Allocator Consistency (2024)

### Problem

When `EASTL_USER_DEFINED_ALLOCATOR` is defined and `EASTL_MIMALLOC_ENABLED` is active,
the default LuisaCompute allocator routes through mimalloc (or a custom allocator set via
`allocator::set_custom_malloc`). However, several smart pointer factory functions and deleters
had allocation/deallocation mismatches:

1. **`make_unique<T>(args...)`** — used `new T` (system CRT `operator new`) to allocate,
   but the default deleter `default_delete<T>` frees via `GetDefaultAllocator()->deallocate()`
   → `mi_free()`. **Heap mismatch**: `mi_free` called on a pointer from `operator new`.

2. **`make_unique<T[]>(n)`** — used `new T[n]` (system CRT) to allocate, but
   `default_delete<T[]>` frees via `GetDefaultAllocator()->deallocate()` → `mi_free()`.
   **Heap mismatch**.

3. **`alloc_size` formula bug** — All array deleters (`default_delete<T[]>`,
   `smart_array_deleter<T>`, `smart_ptr_deleter<void>`, `smart_ptr_deleter<const void>`,
   `smart_array_deleter<void>`) computed `alloc_size = header_size * ele_size * sizeof(T)`
   instead of the correct `header_size + ele_size * sizeof(T)`. The multiply-by-header_size
   would vastly overestimate the allocation size.

### Files Modified

| File | Lines | Change |
|---|---|---|
| `include/EASTL/unique_ptr.h` | 539–544 | `make_unique<T>(args...)`: allocator-based allocation + placement new |
| `include/EASTL/unique_ptr.h` | 546–561 | `make_unique<T[]>(n)`: allocator-based allocation with header + placement new loop |
| `include/EASTL/internal/smart_ptr.h` | 220 | `default_delete<T[]>`: `alloc_size = header_size + ele_size * sizeof(T)` |
| `include/EASTL/internal/smart_ptr.h` | 263 | `smart_ptr_deleter<void>`: `alloc_size = header_size + ele_size` |
| `include/EASTL/internal/smart_ptr.h` | 279 | `smart_ptr_deleter<const void>`: `alloc_size = header_size + ele_size` |
| `include/EASTL/internal/smart_ptr.h` | 301 | `smart_array_deleter<T>`: `alloc_size = header_size + ele_size * sizeof(T)` |
| `include/EASTL/internal/smart_ptr.h` | 325 | `smart_array_deleter<void>`: `alloc_size = header_size + ele_size` |

### Allocation/Deallocation Consistency Matrix

| Operation | Allocates via | Deallocates via | Status |
|---|---|---|---|
| `make_unique<T>(args...)` | `GetDefaultAllocator()->allocate()` | `default_delete<T>::operator()` → `GetDefaultAllocator()->deallocate()` | ✅ Fixed |
| `make_unique<T[]>(n)` | `GetDefaultAllocator()->allocate()` (with header) | `default_delete<T[]>::operator()` → `GetDefaultAllocator()->deallocate()` | ✅ Fixed |
| `make_shared<T>(args...)` | `EASTLAlloc` (mimalloc) | `EASTLFree` (mimalloc) | ✅ Already consistent |
| `shared_ptr<T>(new T)` | `new T` (system CRT) | `default_delete<T>` → `GetDefaultAllocator()->deallocate()` | ⚠️ Known limitation |

### Additional Bug Found: `fixed_vector.h` uses undeclared `construct_at`

In `fixed_vector.h`, the `DoPushBack(false_type)` and `DoPushBackMove(false_type)` specializations
(for `bEnableOverflow=false`) called `construct_at()` without the `eastl::` namespace prefix.
Neither `std::construct_at` (C++20) nor `eastl::construct_at` is available in this configuration,
causing a compilation error when using `push_back()` on a non-overflow `fixed_vector`.

**Fix**: Replaced with `::new((void*)...) value_type` placement new expressions,
consistent with the existing code path under the `#else` branch.

**Root cause**: These functions were added/modernized with C++20 `construct_at` but the
project is not compiled with C++20, and EASTL does not provide its own `construct_at` wrapper.

### No Changes Needed

- `include/EASTL/shared_ptr.h` — `make_shared`/`allocate_shared` already use `EASTLAlloc`/`EASTLFree`.
- `include/EASTL/allocator.h` — Interface compatible with single-arg `allocate(size_t)` calls.
- `source/allocator_luisa.cpp` — Allocator implementation unchanged.
- `include/EASTL/vector.h` — Already consistent; uses `allocate_memory`/`EASTLFree` through the allocator.
- `include/EASTL/internal/fixed_pool.h` — `fixed_vector_allocator` correctly delegates to overflow allocator and skips fixed buffer deallocation.

### Test

A comprehensive test suite is in `src/tests/unit/core/test_eastl_allocation.cpp` (44 tests,
covering `unique_ptr`, `make_unique`, `default_delete`, `smart_ptr_deleter`, `smart_array_deleter`,
`vector`, and `fixed_vector`). Registered in both `src/tests/xmake.lua` and `src/tests/CMakeLists.txt`.
