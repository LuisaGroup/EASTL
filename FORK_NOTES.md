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

### No Changes Needed

- `include/EASTL/shared_ptr.h` — `make_shared`/`allocate_shared` already use `EASTLAlloc`/`EASTLFree`.
- `include/EASTL/allocator.h` — Interface compatible with single-arg `allocate(size_t)` calls.
- `source/allocator_luisa.cpp` — Allocator implementation unchanged.

### Test

A comprehensive test suite is in `src/tests/unit/core/test_smart_ptr_allocator.cpp` (24 tests,
registered in `src/tests/xmake.lua`). Run with:

```bash
xmake build test_smart_ptr_allocator
xmake run test_smart_ptr_allocator
```
