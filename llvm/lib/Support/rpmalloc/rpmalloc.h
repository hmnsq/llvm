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

#आशय once

#समावेश <stddef.h>

#अगर_घोषित __cplusplus
बाह्य "C" {
#पूर्ण_अगर

#अगर defined(__clang__) || defined(__GNUC__)
#घोषणा RPMALLOC_EXPORT __attribute__((visibility("default")))
#घोषणा RPMALLOC_ALLOCATOR
#अगर (defined(__clang_major__) && (__clang_major__ < 4)) ||                     \
    (defined(__GNUC__) && defined(ENABLE_PRELOAD) && ENABLE_PRELOAD)
#घोषणा RPMALLOC_ATTRIB_MALLOC
#घोषणा RPMALLOC_ATTRIB_ALLOC_आकार(आकार_2)
#घोषणा RPMALLOC_ATTRIB_ALLOC_SIZE2(गिनती, आकार_2)
#अन्यथा
#घोषणा RPMALLOC_ATTRIB_MALLOC __attribute__((__malloc__))
#घोषणा RPMALLOC_ATTRIB_ALLOC_आकार(आकार_2) __attribute__((alloc_आकार(आकार_2)))
#घोषणा RPMALLOC_ATTRIB_ALLOC_SIZE2(गिनती, आकार_2)                               \
  __attribute__((alloc_आकार(गिनती, आकार_2)))
#पूर्ण_अगर
#घोषणा RPMALLOC_CDECL
#या_अगर defined(_MSC_VER)
#घोषणा RPMALLOC_EXPORT
#घोषणा RPMALLOC_ALLOCATOR __declspec(allocator) __declspec(restrict)
#घोषणा RPMALLOC_ATTRIB_MALLOC
#घोषणा RPMALLOC_ATTRIB_ALLOC_आकार(आकार_2)
#घोषणा RPMALLOC_ATTRIB_ALLOC_SIZE2(गिनती, आकार_2)
#घोषणा RPMALLOC_CDECL __cdecl
#अन्यथा
#घोषणा RPMALLOC_EXPORT
#घोषणा RPMALLOC_ALLOCATOR
#घोषणा RPMALLOC_ATTRIB_MALLOC
#घोषणा RPMALLOC_ATTRIB_ALLOC_आकार(आकार_2)
#घोषणा RPMALLOC_ATTRIB_ALLOC_SIZE2(गिनती, आकार_2)
#घोषणा RPMALLOC_CDECL
#पूर्ण_अगर

//! Define RPMALLOC_CONFIGURABLE to enable configuring sizes. Will introduce
//  a very small overhead due to some size calculations not being compile time
//  constants
#अगर_अघोषित RPMALLOC_CONFIGURABLE
#घोषणा RPMALLOC_CONFIGURABLE 0
#पूर्ण_अगर

//! Define RPMALLOC_FIRST_CLASS_HEAPS to enable heap based API (rpmalloc_heap_*
//! functions).
//  Will introduce a very small overhead to track fully allocated spans in heaps
#अगर_अघोषित RPMALLOC_FIRST_CLASS_HEAPS
#घोषणा RPMALLOC_FIRST_CLASS_HEAPS 0
#पूर्ण_अगर

//! Flag to rpaligned_realloc to not preserve content in reallocation
#घोषणा RPMALLOC_NO_PRESERVE 1
//! Flag to rpaligned_realloc to fail and return null pointer if grow cannot be
//! done in-place,
//  in which case the original pointer is still valid (just like a call to
//  realloc which failes to allocate a new block).
#घोषणा RPMALLOC_GROW_OR_FAIL 2

प्रकार काष्ठा rpmalloc_global_statistics_t {
  //! Current amount of virtual memory mapped, all of which might not have been
  //! committed (only if ENABLE_STATISTICS=1)
  माप_प्रकार mapped;
  //! Peak amount of virtual memory mapped, all of which might not have been
  //! committed (only if ENABLE_STATISTICS=1)
  माप_प्रकार mapped_peak;
  //! Current amount of memory in global caches for small and medium sizes
  //! (<32KiB)
  माप_प्रकार cached;
  //! Current amount of memory allocated in huge allocations, i.e larger than
  //! LARGE_SIZE_LIMIT which is 2MiB by default (only if ENABLE_STATISTICS=1)
  माप_प्रकार huge_alloc;
  //! Peak amount of memory allocated in huge allocations, i.e larger than
  //! LARGE_SIZE_LIMIT which is 2MiB by default (only if ENABLE_STATISTICS=1)
  माप_प्रकार huge_alloc_peak;
  //! Total amount of memory mapped since initialization (only if
  //! ENABLE_STATISTICS=1)
  माप_प्रकार mapped_योग_2;
  //! Total amount of memory unmapped since initialization  (only if
  //! ENABLE_STATISTICS=1)
  माप_प्रकार unmapped_योग_2;
} rpmalloc_global_statistics_t;

प्रकार काष्ठा rpmalloc_thread_statistics_t {
  //! Current number of bytes available in thread size class caches for small
  //! and medium sizes (<32KiB)
  माप_प्रकार sizecache;
  //! Current number of bytes available in thread span caches for small and
  //! medium sizes (<32KiB)
  माप_प्रकार spancache;
  //! Total number of bytes transitioned from thread cache to global cache (only
  //! if ENABLE_STATISTICS=1)
  माप_प्रकार thread_to_global;
  //! Total number of bytes transitioned from global cache to thread cache (only
  //! if ENABLE_STATISTICS=1)
  माप_प्रकार global_to_thread;
  //! Per span count statistics (only if ENABLE_STATISTICS=1)
  काष्ठा {
    //! Currently used number of spans
    माप_प्रकार current;
    //! High water mark of spans used
    माप_प्रकार peak;
    //! Number of spans transitioned to global cache
    माप_प्रकार to_global;
    //! Number of spans transitioned from global cache
    माप_प्रकार from_global;
    //! Number of spans transitioned to thread cache
    माप_प्रकार to_cache;
    //! Number of spans transitioned from thread cache
    माप_प्रकार from_cache;
    //! Number of spans transitioned to reserved state
    माप_प्रकार to_reserved;
    //! Number of spans transitioned from reserved state
    माप_प्रकार from_reserved;
    //! Number of raw memory map calls (not hitting the reserve spans but
    //! resulting in actual OS mmap calls)
    माप_प्रकार map_calls;
  } span_use[64];
  //! Per size class statistics (only if ENABLE_STATISTICS=1)
  काष्ठा {
    //! Current number of allocations
    माप_प्रकार alloc_current;
    //! Peak number of allocations
    माप_प्रकार alloc_peak;
    //! Total number of allocations
    माप_प्रकार alloc_योग;
    //! Total number of frees
    माप_प्रकार free_योग;
    //! Number of spans transitioned to cache
    माप_प्रकार spans_to_cache;
    //! Number of spans transitioned from cache
    माप_प्रकार spans_from_cache;
    //! Number of spans transitioned from reserved state
    माप_प्रकार spans_from_reserved;
    //! Number of raw memory map calls (not hitting the reserve spans but
    //! resulting in actual OS mmap calls)
    माप_प्रकार map_calls;
  } आकार_use[128];
} rpmalloc_thread_statistics_t;

प्रकार काष्ठा rpmalloc_config_t {
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
  व्योम *(*memory_map)(माप_प्रकार आकार_2, माप_प्रकार *offset);
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
  व्योम (*memory_unmap)(व्योम *address, माप_प्रकार आकार_2, माप_प्रकार offset,
                       माप_प्रकार release);
  //! Called when an assert fails, if asserts are enabled. Will use the standard
  //! assert()
  //  if this is not set.
  व्योम (*error_callback)(स्थिर अक्षर *message);
  //! Called when a call to map memory pages fails (out of memory). If this
  //! callback is
  //  not set or returns zero the library will return a null pointer in the
  //  allocation call. If this callback returns non-zero the map call will be
  //  retried. The argument passed is the number of bytes that was requested in
  //  the map call. Only used if the default system memory map function is used
  //  (memory_map callback is not set).
  पूर्णांक (*map_fail_callback)(माप_प्रकार आकार_2);
  //! Size of memory pages. The page size MUST be a power of two. All memory
  //! mapping
  //  requests to memory_map will be made with size set to a multiple of the
  //  page size. Used if RPMALLOC_CONFIGURABLE is defined to 1, otherwise system
  //  page size is used.
  माप_प्रकार page_आकार;
  //! Size of a span of memory blocks. MUST be a power of two, and in
  //! [4096,262144]
  //  range (unless 0 - set to 0 to use the default span size). Used if
  //  RPMALLOC_CONFIGURABLE is defined to 1.
  माप_प्रकार span_आकार;
  //! Number of spans to map at each request to map new virtual memory blocks.
  //! This can
  //  be used to minimize the system call overhead at the cost of virtual memory
  //  address space. The extra mapped pages will not be written until actually
  //  used, so physical committed memory should not be affected in the default
  //  implementation. Will be aligned to a multiple of spans that match memory
  //  page size in case of huge pages.
  माप_प्रकार span_map_गिनती;
  //! Enable use of large/huge pages. If this flag is set to non-zero and page
  //! size is
  //  zero, the allocator will try to enable huge pages and auto detect the
  //  configuration. If this is set to non-zero and page_size is also non-zero,
  //  the allocator will assume huge pages have been configured and enabled
  //  prior to initializing the allocator. For Windows, see
  //  https://docs.microsoft.com/en-us/windows/desktop/memory/large-page-support
  //  For Linux, see https://www.kernel.org/doc/Documentation/vm/hugetlbpage.txt
  पूर्णांक enable_huge_pages;
  //! Respectively allocated pages and huge allocated pages names for systems
  //  supporting it to be able to distinguish among anonymous regions.
  स्थिर अक्षर *page_name;
  स्थिर अक्षर *huge_page_name;
} rpmalloc_config_t;

//! Initialize allocator with default configuration
RPMALLOC_EXPORT पूर्णांक rpmalloc_initialize(व्योम);

//! Initialize allocator with given configuration
RPMALLOC_EXPORT पूर्णांक rpmalloc_initialize_config(स्थिर rpmalloc_config_t *config);

//! Get allocator configuration
RPMALLOC_EXPORT स्थिर rpmalloc_config_t *rpmalloc_config(व्योम);

//! Finalize allocator
RPMALLOC_EXPORT व्योम rpmalloc_finalize(व्योम);

//! Initialize allocator for calling thread
RPMALLOC_EXPORT व्योम rpmalloc_thread_initialize(व्योम);

//! Finalize allocator for calling thread
RPMALLOC_EXPORT व्योम rpmalloc_thread_finalize(पूर्णांक release_caches);

//! Perform deferred deallocations pending for the calling thread heap
RPMALLOC_EXPORT व्योम rpmalloc_thread_collect(व्योम);

//! Query if allocator is initialized for calling thread
RPMALLOC_EXPORT पूर्णांक rpmalloc_is_thread_initialized(व्योम);

//! Get per-thread statistics
RPMALLOC_EXPORT व्योम
rpmalloc_thread_statistics(rpmalloc_thread_statistics_t *stats);

//! Get global statistics
RPMALLOC_EXPORT व्योम
rpmalloc_global_statistics(rpmalloc_global_statistics_t *stats);

//! Dump all statistics in human readable format to file (should be a FILE*)
RPMALLOC_EXPORT व्योम rpmalloc_dump_statistics(व्योम *file);

//! Allocate a memory block of at least the given size
RPMALLOC_EXPORT RPMALLOC_ALLOCATOR व्योम *
rpmalloc(माप_प्रकार आकार_2) RPMALLOC_ATTRIB_MALLOC RPMALLOC_ATTRIB_ALLOC_आकार(1);

//! Free the given memory block
RPMALLOC_EXPORT व्योम rpfree(व्योम *ptr);

//! Allocate a memory block of at least the given size and zero initialize it
RPMALLOC_EXPORT RPMALLOC_ALLOCATOR व्योम *
rpcalloc(माप_प्रकार अंक_2, माप_प्रकार आकार_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE2(1, 2);

//! Reallocate the given block to at least the given size
RPMALLOC_EXPORT RPMALLOC_ALLOCATOR व्योम *
rprealloc(व्योम *ptr, माप_प्रकार आकार_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_आकार(2);

//! Reallocate the given block to at least the given size and alignment,
//  with optional control flags (see RPMALLOC_NO_PRESERVE).
//  Alignment must be a power of two and a multiple of sizeof(void*),
//  and should ideally be less than memory page size. A caveat of rpmalloc
//  internals is that this must also be strictly less than the span size
//  (default 64KiB)
RPMALLOC_EXPORT RPMALLOC_ALLOCATOR व्योम *
rpaligned_realloc(व्योम *ptr, माप_प्रकार alignment, माप_प्रकार आकार_2, माप_प्रकार oldsize,
                  अचिन्हित पूर्णांक flags) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_आकार(3);

//! Allocate a memory block of at least the given size and alignment.
//  Alignment must be a power of two and a multiple of sizeof(void*),
//  and should ideally be less than memory page size. A caveat of rpmalloc
//  internals is that this must also be strictly less than the span size
//  (default 64KiB)
RPMALLOC_EXPORT RPMALLOC_ALLOCATOR व्योम *
rpaligned_alloc(माप_प्रकार alignment, माप_प्रकार आकार_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_आकार(2);

//! Allocate a memory block of at least the given size and alignment, and zero
//! initialize it.
//  Alignment must be a power of two and a multiple of sizeof(void*),
//  and should ideally be less than memory page size. A caveat of rpmalloc
//  internals is that this must also be strictly less than the span size
//  (default 64KiB)
RPMALLOC_EXPORT RPMALLOC_ALLOCATOR व्योम *
rpaligned_calloc(माप_प्रकार alignment, माप_प्रकार अंक_2,
                 माप_प्रकार आकार_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE2(2, 3);

//! Allocate a memory block of at least the given size and alignment.
//  Alignment must be a power of two and a multiple of sizeof(void*),
//  and should ideally be less than memory page size. A caveat of rpmalloc
//  internals is that this must also be strictly less than the span size
//  (default 64KiB)
RPMALLOC_EXPORT RPMALLOC_ALLOCATOR व्योम *
rpmemalign(माप_प्रकार alignment, माप_प्रकार आकार_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_आकार(2);

//! Allocate a memory block of at least the given size and alignment.
//  Alignment must be a power of two and a multiple of sizeof(void*),
//  and should ideally be less than memory page size. A caveat of rpmalloc
//  internals is that this must also be strictly less than the span size
//  (default 64KiB)
RPMALLOC_EXPORT पूर्णांक rpposix_memalign(व्योम **memptr, माप_प्रकार alignment,
                                     माप_प्रकार आकार_2);

//! Query the usable size of the given memory block (from given pointer to the
//! end of block)
RPMALLOC_EXPORT माप_प्रकार rpmalloc_usable_आकार(व्योम *ptr);

//! Dummy empty function for forcing linker symbol inclusion
RPMALLOC_EXPORT व्योम rpmalloc_linker_reference(व्योम);

#अगर RPMALLOC_FIRST_CLASS_HEAPS

//! Heap type
प्रकार काष्ठा heap_t rpmalloc_heap_t;

//! Acquire a new heap. Will reuse existing released heaps or allocate memory
//! for a new heap
//  if none available. Heap API is implemented with the strict assumption that
//  only one single thread will call heap functions for a given heap at any
//  given time, no functions are thread safe.
RPMALLOC_EXPORT rpmalloc_heap_t *rpmalloc_heap_acquire(व्योम);

//! Release a heap (does NOT free the memory allocated by the heap, use
//! rpmalloc_heap_free_all before destroying the heap).
//  Releasing a heap will enable it to be reused by other threads. Safe to pass
//  a null pointer.
RPMALLOC_EXPORT व्योम rpmalloc_heap_release(rpmalloc_heap_t *heap);

//! Allocate a memory block of at least the given size using the given heap.
RPMALLOC_EXPORT RPMALLOC_ALLOCATOR व्योम *
rpmalloc_heap_alloc(rpmalloc_heap_t *heap, माप_प्रकार आकार_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_आकार(2);

//! Allocate a memory block of at least the given size using the given heap. The
//! returned
//  block will have the requested alignment. Alignment must be a power of two
//  and a multiple of sizeof(void*), and should ideally be less than memory page
//  size. A caveat of rpmalloc internals is that this must also be strictly less
//  than the span size (default 64KiB).
RPMALLOC_EXPORT RPMALLOC_ALLOCATOR व्योम *
rpmalloc_heap_aligned_alloc(rpmalloc_heap_t *heap, माप_प्रकार alignment,
                            माप_प्रकार आकार_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_आकार(3);

//! Allocate a memory block of at least the given size using the given heap and
//! zero initialize it.
RPMALLOC_EXPORT RPMALLOC_ALLOCATOR व्योम *
rpmalloc_heap_calloc(rpmalloc_heap_t *heap, माप_प्रकार अंक_2,
                     माप_प्रकार आकार_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE2(2, 3);

//! Allocate a memory block of at least the given size using the given heap and
//! zero initialize it. The returned
//  block will have the requested alignment. Alignment must either be zero, or a
//  power of two and a multiple of sizeof(void*), and should ideally be less
//  than memory page size. A caveat of rpmalloc internals is that this must also
//  be strictly less than the span size (default 64KiB).
RPMALLOC_EXPORT RPMALLOC_ALLOCATOR व्योम *
rpmalloc_heap_aligned_calloc(rpmalloc_heap_t *heap, माप_प्रकार alignment,
                             माप_प्रकार अंक_2, माप_प्रकार आकार_2) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_SIZE2(2, 3);

//! Reallocate the given block to at least the given size. The memory block MUST
//! be allocated
//  by the same heap given to this function.
RPMALLOC_EXPORT RPMALLOC_ALLOCATOR व्योम *
rpmalloc_heap_realloc(rpmalloc_heap_t *heap, व्योम *ptr, माप_प्रकार आकार_2,
                      अचिन्हित पूर्णांक flags) RPMALLOC_ATTRIB_MALLOC
    RPMALLOC_ATTRIB_ALLOC_आकार(3);

//! Reallocate the given block to at least the given size. The memory block MUST
//! be allocated
//  by the same heap given to this function. The returned block will have the
//  requested alignment. Alignment must be either zero, or a power of two and a
//  multiple of sizeof(void*), and should ideally be less than memory page size.
//  A caveat of rpmalloc internals is that this must also be strictly less than
//  the span size (default 64KiB).
RPMALLOC_EXPORT RPMALLOC_ALLOCATOR व्योम *rpmalloc_heap_aligned_realloc(
    rpmalloc_heap_t *heap, व्योम *ptr, माप_प्रकार alignment, माप_प्रकार आकार_2,
    अचिन्हित पूर्णांक flags) RPMALLOC_ATTRIB_MALLOC RPMALLOC_ATTRIB_ALLOC_आकार(4);

//! Free the given memory block from the given heap. The memory block MUST be
//! allocated
//  by the same heap given to this function.
RPMALLOC_EXPORT व्योम rpmalloc_heap_free(rpmalloc_heap_t *heap, व्योम *ptr);

//! Free all memory allocated by the heap
RPMALLOC_EXPORT व्योम rpmalloc_heap_free_all(rpmalloc_heap_t *heap);

//! Set the given heap as the current heap for the calling thread. A heap MUST
//! only be current heap
//  for a single thread, a heap can never be shared between multiple threads.
//  The previous current heap for the calling thread is released to be reused by
//  other threads.
RPMALLOC_EXPORT व्योम rpmalloc_heap_thread_set_current(rpmalloc_heap_t *heap);

//! Returns which heap the given pointer is allocated on
RPMALLOC_EXPORT rpmalloc_heap_t *rpmalloc_get_heap_for_ptr(व्योम *ptr);

#पूर्ण_अगर

#अगर_घोषित __cplusplus
}
#पूर्ण_अगर
