//
// Created by Mike Smith on 2021/12/24.
//

#include <EASTL/allocator.h>
#include <EASTL/internal/config.h>

#ifdef EASTL_MIMALLOC_ENABLED
#include <mimalloc.h>
#else
#include <cstdlib>
#endif
namespace eastl
{

	namespace detail
	{
#ifdef EASTL_MIMALLOC_ENABLED
		class MimallocSetOptions
		{
		public:
			MimallocSetOptions() {
				mi_option_set(mi_option_allow_large_os_pages, 1);
			}
		};
		static MimallocSetOptions _mimalloc_set_options;
#endif
		inline static allocator*& GetDefaultAllocatorRef() noexcept
		{
			static allocator a;
			static allocator* pa = &a;
			return pa;
		}
	} // namespace detail

	EASTL_API allocator* GetDefaultAllocator() { return detail::GetDefaultAllocatorRef(); }

	EASTL_API allocator* SetDefaultAllocator(allocator* pAllocator)
	{
		allocator* const pPrevAllocator = GetDefaultAllocator();
		detail::GetDefaultAllocatorRef() = pAllocator;
		return pPrevAllocator;
	}

	void* allocator::reallocate(void* originPtr, size_t n)
	{
#ifdef EASTL_MIMALLOC_ENABLED
		return mi_realloc(originPtr, n);
#else
		return realloc(originPtr, n);
#endif
	}

	void* allocator::allocate(size_t n, int /* flags */)
	{
#ifdef EASTL_MIMALLOC_ENABLED
		return mi_malloc(n);
#else
		return malloc(n);
#endif
	}


	void* allocator::allocate(size_t n, size_t alignment, size_t offset [[maybe_unused]], int flags)
	{
		EASTL_ASSERT(offset == 0u);
		if (alignment <= EASTL_SYSTEM_ALLOCATOR_MIN_ALIGNMENT)
		{
			return allocate(n, flags);
		}

#ifdef EASTL_MIMALLOC_ENABLED
		return mi_aligned_alloc(alignment, n);
#else
		return aligned_alloc(alignment, n);
#endif
	}


	void allocator::deallocate(void* p, size_t)
	{
#ifdef EASTL_MIMALLOC_ENABLED
		mi_free(p);
#else
		free(p);
#endif
	}

} // namespace eastl