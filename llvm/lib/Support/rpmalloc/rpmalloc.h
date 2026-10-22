//===---------------------- rpmalloc.h ------------------*- C -*-=============//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This library provides a cross-platform lock free thread caching malloc
// implementation in C11.
//
//===----------------------------------------------------------------------===//

#ਹਦਾਇਤ once

#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __cplusplus
ਬਾਹਰੀ "C" {
#ਜੇ_ਅੰਤ

#ਜੇ defined(__clang__) || defined(__GNUC__)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RPMALLOC_EXPORT __attribute__((visibility("default")))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RPMALLOC_ALLOCATOR
#ਜੇ (defined(__clang_major__) && (__clang_major__ < 4)) ||                     \
    (defined(__GNUC__) && defined(ENABLE_PRELOAD) && ENABLE_PRELOAD)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RPMALLOC_ATTRIB_MALLOC
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RPMALLOC_ATTRIB_ALLOC_ਮਾਪ(ਮਾਪ_2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RPMALLOC_ATTRIB_ALLOC_SIZE2(ਗਿਣਤੀ_2, ਮਾਪ_2)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RPMALLOC_ATTRIB_MALLOC __attribute__((__malloc__))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RPMALLOC_ATTRIB_ALLOC_ਮਾਪ(ਮਾਪ_2) __attribute__((alloc_ਮਾਪ(ਮਾਪ_2)))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RPMALLOC_ATTRIB_ALLOC_SIZE2(ਗਿਣਤੀ_2, ਮਾਪ_2)                               \
  __attribute__((alloc_ਮਾਪ(ਗਿਣਤੀ_2, ਮਾਪ_2)))
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RPMALLOC_CDECL
#ਜਾਂਜੇ defined(_MSC_VER)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RPMALLOC_EXPORT
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RPMALLOC_ALLOCATOR __declspec(allocator) __declspec(ਸੀਮਿਤ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RPMALLOC_ATTRIB_MALLOC
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RPMALLOC_ATTRIB_ALLOC_ਮਾਪ(ਮਾਪ_2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RPMALLOC_ATTRIB_ALLOC_SIZE2(ਗਿਣਤੀ_2, ਮਾਪ_2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RPMALLOC_CDECL __cdecl
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RPMALLOC_EXPORT
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RPMALLOC_ALLOCATOR
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RPMALLOC_ATTRIB_MALLOC
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RPMALLOC_ATTRIB_ALLOC_ਮਾਪ(ਮਾਪ_2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RPMALLOC_ATTRIB_ALLOC_SIZE2(ਗਿਣਤੀ_2, ਮਾਪ_2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RPMALLOC_CDECL
#ਜੇ_ਅੰਤ

//! Define RPMALLOC_CONFIGURABLE to enable configuring sizes. Will introduce
//  a very small overhead due to some size calculations not being compile time
//  constants
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ RPMALLOC_CONFIGURABLE
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RPMALLOC_CONFIGURABLE 0
#ਜੇ_ਅੰਤ

//! Define RPMALLOC_FIRST_CLASS_HEAPS to enable heap based API (rpmalloc_heap_*
//! functions).
//  Will introduce a very small overhead to track fully allocated spans in heaps
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ RPMALLOC_ਪਹਿਲਾ_CLASS_HEAPS
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RPMALLOC_ਪਹਿਲਾ_CLASS_HEAPS 0
#ਜੇ_ਅੰਤ

//! Flag to rpaligned_realloc to not preserve content in reallocation
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RPMALLOC_NO_PRESERVE 1
//! Flag to rpaligned_realloc to fail and return null pointer if grow cannot be
//! done in-place,
//  in which case the original pointer is still valid (just like a call to
//  realloc which failes to allocate a new block).
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RPMALLOC_GROW_OR_FAIL 2

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ rpmalloc_global_statistics_t {
  //! Current amount of virtual memory mapped, all of which might not have been
  //! committed (only if ENABLE_STATISTICS=1)
  size_t mapped;
  //! Peak amount of virtual memory mapped, all of which might not have been
  //! committed (only if ENABLE_STATISTICS=1)
  size_t mapped_peak;
  //! Current amount of memory in global caches for small and medium sizes
  //! (<32KiB)
  size_t cached;
  //! Current amount of memory allocated in huge allocations, i.e larger than
  //! LARGE_SIZE_LIMIT which is 2MiB by default (only if ENABLE_STATISTICS=1)
  size_t huge_alloc;
  //! Peak amount of memory allocated in huge allocations, i.e larger than
  //! LARGE_SIZE_LIMIT which is 2MiB by default (only if ENABLE_STATISTICS=1)
  size_t huge_alloc_peak;
  //! Total amount of memory mapped since initialization (only if
  //! ENABLE_STATISTICS=1)
  size_t mapped_ਕੁੱਲ_2;
  //! Total amount of memory unmapped since initialization  (only if
  //! ENABLE_STATISTICS=1)
  size_t unmapped_ਕੁੱਲ_2;
} rpmalloc_global_statistics_t;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ rpmalloc_thread_statistics_t {
  //! Current number of bytes available in thread size class caches for small
  //! and medium sizes (<32KiB)
  size_t sizecache;
  //! Current number of bytes available in thread span caches for small and
  //! medium sizes (<32KiB)
  size_t spancache;
  //! Total number of bytes transitioned from thread cache to global cache (only
  //! if ENABLE_STATISTICS=1)
  size_t thread_ਮੰਜ਼ਿਲ_global;
  //! Total number of bytes transitioned from global cache to thread cache (only
  //! if ENABLE_STATISTICS=1)
  size_t global_ਮੰਜ਼ਿਲ_thread;
  //! Per span count statistics (only if ENABLE_STATISTICS=1)
  ਬਣਤਰ {
    //! Currently used number of spans
    size_t current;
    //! High water mark of spans used
    size_t peak;
    //! Number of spans transitioned to global cache
    size_t ਮੰਜ਼ਿਲ_global;
    //! Number of spans transitioned from global cache
    size_t from_global;
    //! Number of spans transitioned to thread cache
    size_t ਮੰਜ਼ਿਲ_cache;
    //! Number of spans transitioned from thread cache
    size_t from_cache;
    //! Number of spans transitioned to reserved state
    size_t ਮੰਜ਼ਿਲ_reserved;
    //! Number of spans transitioned from reserved state
    size_t from_reserved;
    //! Number of raw memory map calls (not hitting the reserve spans but
    //! resulting in actual OS mmap calls)
    size_t map_calls;
  } ਵਿਸਤਾਰ_use[64];
  //! Per size class statistics (only if ENABLE_STATISTICS=1)
  ਬਣਤਰ {
    //! Current number of allocations
    size_t alloc_current;
    //! Peak number of allocations
    size_t alloc_peak;
    //! Total number of allocations
    size_t alloc_ਕੁੱਲ;
    //! Total number of frees
    size_t free_ਕੁੱਲ;
    //! Number of spans transitioned to cache
    size_t spans_ਮੰਜ਼ਿਲ_cache;
    //! Number of spans transitioned from cache
    size_t spans_from_cache;
    //! Number of spans transitioned from reserved state
    size_t spans_from_reserved;
    //! Number of raw memory map calls (not hitting the reserve spans but
    //! resulting in actual OS mmap calls)
    size_t map_calls;
  } ਮਾਪ_use[128];
} rpmalloc_thread_statistics_t;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ rpmalloc_config_t {
  //! Map memory pages for the given number of bytes. The returned address MUST
  //! be
  //  aligned to the rpmalloc span size, which will always be a power of two.
  //  Optionally the function can store an alignment offset in the offset
  //  variable in case it performs alignment and the returned pointer is offset
  //  from the actual start of the memory region due to this alignment. The
  //  alignment offset will be passed to the memory unmap function. The
  //  alignment offset MUST NOT be larger than 65535 (storable in an uint16_t),
  //  if it is you must use natural alignment to shift it into 16 bits. If you
  //  set a memory_map function, you must also set a memory_unmap function or
  //  else the default implementation will be used for both. This function must
  //  be thread safe, it can be called by multiple threads simultaneously.
  ਖਾਲੀ *(*ਸਮ੍ਰਿਤੀ_map)(size_t ਮਾਪ_2, size_t *ਵਿੱਥ_2);
  //! Unmap the memory pages starting at address and spanning the given number
  //! of bytes.
  //  If release is set to non-zero, the unmap is for an entire span range as
  //  returned by a previous call to memory_map and that the entire range should
  //  be released. The release argument holds the size of the entire span range.
  //  If release is set to 0, the unmap is a partial decommit of a subset of the
  //  mapped memory range. If you set a memory_unmap function, you must also set
  //  a memory_map function or else the default implementation will be used for
  //  both. This function must be thread safe, it can be called by multiple
  //  threads simultaneously.
  ਖਾਲੀ (*ਸਮ੍ਰਿਤੀ_unmap)(ਖਾਲੀ *ਪਤਾ_2, size_t ਮਾਪ_2, size_t ਵਿੱਥ_2,
                       size_t release);
  //! Called when an assert fails, if asserts are enabled. Will use the standard
  //! assert()
  //  if this is not set.
  ਖਾਲੀ (*error_callback)(ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ_2);
  //! Called when a call to map memory pages fails (out of memory). If this
  //! callback is
  //  not set or returns zero the library will return a null pointer in the
  //  allocation call. If this callback returns non-zero the map call will be
  //  retried. The argument passed is the number of bytes that was requested in
  //  the map call. Only used if the default system memory map function is used
  //  (memory_map callback is not set).
  ਪੂਰਨ (*map_fail_callback)(size_t ਮਾਪ_2);
  //! Size of memory pages. The page size MUST be a power of two. All memory
  //! mapping
  //  requests to memory_map will be made with size set to a multiple of the
  //  page size. Used if RPMALLOC_CONFIGURABLE is defined to 1, otherwise system
  //  page size is used.
  size_t ਪੰਨਾ_ਮਾਪ_2;
  //! Size of a span of memory blocks. MUST be a power of two, and in
  //! [4096,262144]
  //  range (unless 0 - set to 0 to use the default span size). Used if
  //  RPMALLOC_CONFIGURABLE is defined to 1.
  size_t ਵਿਸਤਾਰ_ਮਾਪ;
  //! Number of spans to map at each request to map new virtual memory blocks.
  //! This can
  //  be used to minimize the system call overhead at the cost of virtual memory
  //  address space. The extra mapped pages will not be written until actually
  //  used, so physical committed memory should not be affected in the default
  //  implementation. Will be aligned to a multiple of spans that match memory
  //  page size in case of huge pages.
  size_t ਵਿਸਤਾਰ_map_ਗਿਣਤੀ;
  //! Enable use of large/huge pages. If this flag is set to non-zero and page
  //! size is
  //  zero, the allocator will try to enable huge pages and auto detect the
  //  configuration. If this is set to non-zero and page_size is also non-zero,
  //  the allocator will assume huge pages have been configured and enabled
  //  prior to initializing the allocator. For Windows, see
  //  https://docs.microsoft.com/en-us/windows/desktop/memory/large-page-support
  //  For Linux, see https://www.kernel.org/doc/Documentation/vm/hugetlbpage.txt
  ਪੂਰਨ enable_huge_ਪੰਨੇ;
  //! Respectively allocated pages and huge allocated pages names for systems
  //  supporting it to be able to distinguish among anonymous regions.
  ਅਚਲ ਅੱਖਰ *ਪੰਨਾ_name;
  ਅਚਲ ਅੱਖਰ *huge_ਪੰਨਾ_name;
} rpmalloc_config_t;

//! Initialize allocator with default configuration
RPMALLOC_EXPORT ਪੂਰਨ rpmalloc_initialize(ਖਾਲੀ);

//! Initialize allocator with given configuration
RPMALLOC_EXPORT ਪੂਰਨ rpmalloc_initialize_config(ਅਚਲ rpmalloc_config_t *config);

//! Get allocator configuration
RPMALLOC_EXPORT ਅਚਲ rpmalloc_config_t *rpmalloc_config(ਖਾਲੀ);

//! Finalize allocator
RPMALLOC_EXPORT ਖਾਲੀ rpmalloc_finalize(ਖਾਲੀ);

//! Initialize allocator for calling thread
RPMALLOC_EXPORT ਖਾਲੀ rpmalloc_thread_initialize(ਖਾਲੀ);

//! Finalize allocator for calling thread
RPMALLOC_EXPORT ਖਾਲੀ rpmalloc_thread_finalize(ਪੂਰਨ release_caches);

//! Perform deferred deallocations pending for the calling thread heap
RPMALLOC_EXPORT ਖਾਲੀ rpmalloc_thread_collect(ਖਾਲੀ);

//! Query if allocator is initialized for calling thread
RPMALLOC_EXPORT ਪੂਰਨ rpmalloc_is_thread_initialized(ਖਾਲੀ);

//! Get per-thread statistics
RPMALLOC_EXPORT ਖਾਲੀ
rpmalloc_thread_statistics(rpmalloc_thread_statistics_t *stats);

//! Get global statistics
RPMALLOC_EXPORT ਖਾਲੀ
rpmalloc_global_statistics(rpmalloc_global_statistics_t *stats);

//! Dump all statistics in human readable format to file (should be a FILE*)
RPMALLOC_EXPORT ਖਾਲੀ rpmalloc_dump_statistics(ਖਾਲੀ *file);

//! Allocate a memory block of at least the given size
RPMALLOC_EXPORT RPMALLOC_ALLOCATOR ਖਾਲੀ *
rpmalloc(size_t ਮਾਪ_2) RPMALLOC_ATTRIB_MALLOC RPMALLOC_ATTRIB_ALLOC_ਮਾਪ(1);

//! Free the given memory block
RPMALLOC_EXPORT ਖਾਲੀ rpfree(ਖਾਲੀ *ptr);

//! Allocate a memory block of at least the given size and zero initialize it
RPMALLOC_EXPORT RPMALLOC_ALLOCATOR ਖਾਲੀ *
rpcalloc(size_t ਅੰਕ_2, size_t ਮਾਪ_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE2(1, 2);

//! Reallocate the given block to at least the given size
RPMALLOC_EXPORT RPMALLOC_ALLOCATOR ਖਾਲੀ *
rprealloc(ਖਾਲੀ *ptr, size_t ਮਾਪ_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_ਮਾਪ(2);

//! Reallocate the given block to at least the given size and alignment,
//  with optional control flags (see RPMALLOC_NO_PRESERVE).
//  Alignment must be a power of two and a multiple of sizeof(void*),
//  and should ideally be less than memory page size. A caveat of rpmalloc
//  internals is that this must also be strictly less than the span size
//  (default 64KiB)
RPMALLOC_EXPORT RPMALLOC_ALLOCATOR ਖਾਲੀ *
rpaligned_realloc(ਖਾਲੀ *ptr, size_t alignment, size_t ਮਾਪ_2, size_t oldsize,
                  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ flags) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_ਮਾਪ(3);

//! Allocate a memory block of at least the given size and alignment.
//  Alignment must be a power of two and a multiple of sizeof(void*),
//  and should ideally be less than memory page size. A caveat of rpmalloc
//  internals is that this must also be strictly less than the span size
//  (default 64KiB)
RPMALLOC_EXPORT RPMALLOC_ALLOCATOR ਖਾਲੀ *
rpaligned_alloc(size_t alignment, size_t ਮਾਪ_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_ਮਾਪ(2);

//! Allocate a memory block of at least the given size and alignment, and zero
//! initialize it.
//  Alignment must be a power of two and a multiple of sizeof(void*),
//  and should ideally be less than memory page size. A caveat of rpmalloc
//  internals is that this must also be strictly less than the span size
//  (default 64KiB)
RPMALLOC_EXPORT RPMALLOC_ALLOCATOR ਖਾਲੀ *
rpaligned_calloc(size_t alignment, size_t ਅੰਕ_2,
                 size_t ਮਾਪ_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE2(2, 3);

//! Allocate a memory block of at least the given size and alignment.
//  Alignment must be a power of two and a multiple of sizeof(void*),
//  and should ideally be less than memory page size. A caveat of rpmalloc
//  internals is that this must also be strictly less than the span size
//  (default 64KiB)
RPMALLOC_EXPORT RPMALLOC_ALLOCATOR ਖਾਲੀ *
rpmemalign(size_t alignment, size_t ਮਾਪ_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_ਮਾਪ(2);

//! Allocate a memory block of at least the given size and alignment.
//  Alignment must be a power of two and a multiple of sizeof(void*),
//  and should ideally be less than memory page size. A caveat of rpmalloc
//  internals is that this must also be strictly less than the span size
//  (default 64KiB)
RPMALLOC_EXPORT ਪੂਰਨ rpposix_memalign(ਖਾਲੀ **memptr, size_t alignment,
                                     size_t ਮਾਪ_2);

//! Query the usable size of the given memory block (from given pointer to the
//! end of block)
RPMALLOC_EXPORT size_t rpmalloc_usable_ਮਾਪ(ਖਾਲੀ *ptr);

//! Dummy empty function for forcing linker symbol inclusion
RPMALLOC_EXPORT ਖਾਲੀ rpmalloc_linker_reference(ਖਾਲੀ);

#ਜੇ RPMALLOC_ਪਹਿਲਾ_CLASS_HEAPS

//! Heap type
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਢੇਰ_t rpmalloc_ਢੇਰ_t;

//! Acquire a new heap. Will reuse existing released heaps or allocate memory
//! for a new heap
//  if none available. Heap API is implemented with the strict assumption that
//  only one single thread will call heap functions for a given heap at any
//  given time, no functions are thread safe.
RPMALLOC_EXPORT rpmalloc_ਢੇਰ_t *rpmalloc_ਢੇਰ_acquire(ਖਾਲੀ);

//! Release a heap (does NOT free the memory allocated by the heap, use
//! rpmalloc_heap_free_all before destroying the heap).
//  Releasing a heap will enable it to be reused by other threads. Safe to pass
//  a null pointer.
RPMALLOC_EXPORT ਖਾਲੀ rpmalloc_ਢੇਰ_release_2(rpmalloc_ਢੇਰ_t *ਢੇਰ_2);

//! Allocate a memory block of at least the given size using the given heap.
RPMALLOC_EXPORT RPMALLOC_ALLOCATOR ਖਾਲੀ *
rpmalloc_ਢੇਰ_alloc(rpmalloc_ਢੇਰ_t *ਢੇਰ_2, size_t ਮਾਪ_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_ਮਾਪ(2);

//! Allocate a memory block of at least the given size using the given heap. The
//! returned
//  block will have the requested alignment. Alignment must be a power of two
//  and a multiple of sizeof(void*), and should ideally be less than memory page
//  size. A caveat of rpmalloc internals is that this must also be strictly less
//  than the span size (default 64KiB).
RPMALLOC_EXPORT RPMALLOC_ALLOCATOR ਖਾਲੀ *
rpmalloc_ਢੇਰ_aligned_alloc(rpmalloc_ਢੇਰ_t *ਢੇਰ_2, size_t alignment,
                            size_t ਮਾਪ_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_ਮਾਪ(3);

//! Allocate a memory block of at least the given size using the given heap and
//! zero initialize it.
RPMALLOC_EXPORT RPMALLOC_ALLOCATOR ਖਾਲੀ *
rpmalloc_ਢੇਰ_calloc(rpmalloc_ਢੇਰ_t *ਢੇਰ_2, size_t ਅੰਕ_2,
                     size_t ਮਾਪ_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE2(2, 3);

//! Allocate a memory block of at least the given size using the given heap and
//! zero initialize it. The returned
//  block will have the requested alignment. Alignment must either be zero, or a
//  power of two and a multiple of sizeof(void*), and should ideally be less
//  than memory page size. A caveat of rpmalloc internals is that this must also
//  be strictly less than the span size (default 64KiB).
RPMALLOC_EXPORT RPMALLOC_ALLOCATOR ਖਾਲੀ *
rpmalloc_ਢੇਰ_aligned_calloc(rpmalloc_ਢੇਰ_t *ਢੇਰ_2, size_t alignment,
                             size_t ਅੰਕ_2, size_t ਮਾਪ_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE2(2, 3);

//! Reallocate the given block to at least the given size. The memory block MUST
//! be allocated
//  by the same heap given to this function.
RPMALLOC_EXPORT RPMALLOC_ALLOCATOR ਖਾਲੀ *
rpmalloc_ਢੇਰ_realloc(rpmalloc_ਢੇਰ_t *ਢੇਰ_2, ਖਾਲੀ *ptr, size_t ਮਾਪ_2,
                      ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ flags) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_ਮਾਪ(3);

//! Reallocate the given block to at least the given size. The memory block MUST
//! be allocated
//  by the same heap given to this function. The returned block will have the
//  requested alignment. Alignment must be either zero, or a power of two and a
//  multiple of sizeof(void*), and should ideally be less than memory page size.
//  A caveat of rpmalloc internals is that this must also be strictly less than
//  the span size (default 64KiB).
RPMALLOC_EXPORT RPMALLOC_ALLOCATOR ਖਾਲੀ *rpmalloc_ਢੇਰ_aligned_realloc(
    rpmalloc_ਢੇਰ_t *ਢੇਰ_2, ਖਾਲੀ *ptr, size_t alignment, size_t ਮਾਪ_2,
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ flags) RPMALLOC_ATTRIB_MALLOC RPMALLOC_ATTRIB_ALLOC_ਮਾਪ(4);

//! Free the given memory block from the given heap. The memory block MUST be
//! allocated
//  by the same heap given to this function.
RPMALLOC_EXPORT ਖਾਲੀ rpmalloc_ਢੇਰ_free(rpmalloc_ਢੇਰ_t *ਢੇਰ_2, ਖਾਲੀ *ptr);

//! Free all memory allocated by the heap
RPMALLOC_EXPORT ਖਾਲੀ rpmalloc_ਢੇਰ_free_all(rpmalloc_ਢੇਰ_t *ਢੇਰ_2);

//! Set the given heap as the current heap for the calling thread. A heap MUST
//! only be current heap
//  for a single thread, a heap can never be shared between multiple threads.
//  The previous current heap for the calling thread is released to be reused by
//  other threads.
RPMALLOC_EXPORT ਖਾਲੀ rpmalloc_ਢੇਰ_thread_ਸੈੱਟ_current(rpmalloc_ਢੇਰ_t *ਢੇਰ_2);

//! Returns which heap the given pointer is allocated on
RPMALLOC_EXPORT rpmalloc_ਢੇਰ_t *rpmalloc_get_ਢੇਰ_for_ptr(ਖਾਲੀ *ptr);

#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __cplusplus
}
#ਜੇ_ਅੰਤ
