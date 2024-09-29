//
// Created by Mike Smith on 2021/12/24.
//

#include <EASTL/allocator.h>
#include <EASTL/internal/config.h>
#ifdef EASTL_MIMALLOC_ENABLED
#include <mimalloc.h>
#endif
#if !defined(EASTL_MIMALLOC_ENABLED) || !defined(EASTL_CUSTOM_MALLOC_ENABLED)
#include <cstdlib>
#endif
namespace eastl
{
#ifdef _WIN32
#define EASTL_ALIGNED_ALLOC(align, size) _aligned_malloc(size, align)
#else
#define EASTL_ALIGNED_ALLOC(align, size) ::aligned_alloc(align, size)
#endif
	namespace detail
	{
#ifdef EASTL_CUSTOM_MALLOC_ENABLED
		static void* (*_custom_malloc)(size_t /*size*/) = nullptr;
		static void* (*_custom_aligned_malloc)(size_t /*alignment*/, size_t /*size*/) = nullptr;
		static void (*_custom_free)(void* /*ptr*/) = nullptr;
		static void* (*_custom_realloc)(void* /*ptr*/, size_t /*size*/) = nullptr;
		class CustomAllocSetOptions
		{
		public:
			CustomAllocSetOptions() {}
		};
		static CustomAllocSetOptions _custom_malloc_options;
#endif

// #ifdef EASTL_MIMALLOC_ENABLED
// 		class MimallocSetOptions
// 		{
// 		public:
// 			MimallocSetOptions() { mi_option_set(mi_option_allow_large_os_pages, 1); }
// 		};
// 		static MimallocSetOptions _mimalloc_set_options;
// #endif
		inline static allocator*& GetDefaultAllocatorRef() noexcept
		{
			static allocator a;
			static allocator* pa = &a;
			return pa;
		}
	} // namespace detail
	void allocator::set_custom_malloc(void* (*custom_malloc)(size_t /*size*/),
	                                  void* (*custom_aligned_malloc)(size_t /*alignment*/, size_t /*size*/),
	                                  void (*custom_free)(void* /*ptr*/),
	                                  void* (*custom_realloc)(void* /*ptr*/, size_t /*size*/))
	{
#ifndef EASTL_CUSTOM_MALLOC_ENABLED
		// EASTL's custom malloc not enabled.
		std::abort();
#else
		detail::_custom_malloc = custom_malloc;
		detail::_custom_aligned_malloc = custom_aligned_malloc;
		detail::_custom_free = custom_free;
		detail::_custom_realloc = custom_realloc;
#endif
	}
	auto allocator::get_custom_malloc() -> CustomAlloc
	{
#ifndef EASTL_CUSTOM_MALLOC_ENABLED
		// EASTL's custom malloc not enabled.
		std::abort();
#else
		return CustomAlloc{detail::_custom_malloc, detail::_custom_aligned_malloc, detail::_custom_free,
		                   detail::_custom_realloc};
#endif
	}

	EASTL_API allocator* GetDefaultAllocator() { return detail::GetDefaultAllocatorRef(); }

	EASTL_API allocator* SetDefaultAllocator(allocator* pAllocator)
	{
		allocator* const pPrevAllocator = GetDefaultAllocator();
		detail::GetDefaultAllocatorRef() = pAllocator;
		return pPrevAllocator;
	}

	void* allocator::reallocate(void* originPtr, size_t n)
	{
#ifdef EASTL_CUSTOM_MALLOC_ENABLED
		return detail::_custom_realloc(originPtr, n);
#else
#ifdef EASTL_MIMALLOC_ENABLED
		return mi_realloc(originPtr, n);
#else
		return std::realloc(originPtr, n);
#endif
#endif
	}

	void* allocator::allocate(size_t n, int /* flags */)
	{
#ifdef EASTL_CUSTOM_MALLOC_ENABLED
		return detail::_custom_malloc(n);
#else
#ifdef EASTL_MIMALLOC_ENABLED
		return mi_malloc(n);
#else
		return std::malloc(n);
#endif
#endif
	}


	void* allocator::allocate(size_t n, size_t alignment, size_t offset [[maybe_unused]], int flags)
	{
		EASTL_ASSERT(offset == 0u);
		if (alignment <= EASTL_SYSTEM_ALLOCATOR_MIN_ALIGNMENT)
		{
			return allocate(n, flags);
		}
#ifdef EASTL_CUSTOM_MALLOC_ENABLED
		return detail::_custom_aligned_malloc(alignment, n);
#else
#ifdef EASTL_MIMALLOC_ENABLED
		return mi_aligned_alloc(alignment, n);
#else
		return EASTL_ALIGNED_ALLOC(alignment, n);
#endif
#endif
	}


	void allocator::deallocate(void* p, size_t)
	{
#ifdef EASTL_CUSTOM_MALLOC_ENABLED
		detail::_custom_free(p);
#else
#ifdef EASTL_MIMALLOC_ENABLED
		mi_free(p);
#else
		std::free(p);
#endif
#endif
	}

} // namespace eastl