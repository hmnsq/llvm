//===- llvm/ADT/SmallVector.cpp - 'Normally small' vectors ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the SmallVector class.
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/ADT/SmallVector.h"
#అన్తర్భావ "llvm/ADT/Twine.h"
#అన్తర్భావ "llvm/Support/MemAlloc.h"
#అన్తర్భావ <cstdint>
#నిర్వచితమైతే LLVM_ENABLE_EXCEPTIONS
#అన్తర్భావ <stdexcept>
#అయితే_ముగింపు
ఉపయోగించి నామాకాశ llvm;

// Check that no bytes are wasted and everything is well-aligned.
నామాకాశ {
// These structures may cause binary compat warnings on AIX. Suppress the
// warning since we are only using these types for the static assertions below.
#ఒకవేళ defined(_AIX)
#సూచన GCC diagnostic నెట్టు
#సూచన GCC diagnostic ignored "-Waix-compat"
#అయితే_ముగింపు
నిర్మాణం Struct16B {
  alignas(16) శూన్య *X;
};
నిర్మాణం Struct32B {
  alignas(32) శూన్య *X;
};
#ఒకవేళ defined(_AIX)
#సూచన GCC diagnostic తీయి
#అయితే_ముగింపు
}
static_assert(పరిమాణం(Small_సదిశ<శూన్య *, 0>) ==
                  పరిమాణం(గుర్తులేని) * 2 + పరిమాణం(శూన్య *),
              "wasted space in SmallVector size 0");
static_assert(alignof(Small_సదిశ<Struct16B, 0>) >= alignof(Struct16B),
              "wrong alignment for 16-byte aligned T");
static_assert(alignof(Small_సదిశ<Struct32B, 0>) >= alignof(Struct32B),
              "wrong alignment for 32-byte aligned T");
static_assert(పరిమాణం(Small_సదిశ<Struct16B, 0>) >= alignof(Struct16B),
              "missing padding for 16-byte aligned T");
static_assert(పరిమాణం(Small_సదిశ<Struct32B, 0>) >= alignof(Struct32B),
              "missing padding for 32-byte aligned T");
static_assert(పరిమాణం(Small_సదిశ<శూన్య *, 1>) ==
                  పరిమాణం(గుర్తులేని) * 2 + పరిమాణం(శూన్య *) * 2,
              "wasted space in SmallVector size 1");

static_assert(పరిమాణం(Small_సదిశ<అక్షరం, 0>) ==
                  పరిమాణం(శూన్య *) * 2 + పరిమాణం(శూన్య *),
              "1 byte elements have word-sized type for size and capacity");

/// Report that MinSize doesn't fit into this vector's size type. Throws
/// std::length_error or calls report_fatal_error.
[[noreturn]] స్థిర శూన్య report_కొలత_అధికప్రవాహం(size_t Min_కొలత, size_t Max_కొలత);
స్థిర శూన్య report_కొలత_అధికప్రవాహం(size_t Min_కొలత, size_t Max_కొలత) {
  std::string Reason = "SmallVector unable to grow. Requested capacity (" +
                       std::గమ్యం_string(Min_కొలత) +
                       ") is larger than maximum value for size type (" +
                       std::గమ్యం_string(Max_కొలత) + ")";
#నిర్వచితమైతే LLVM_ENABLE_EXCEPTIONS
  విసురు std::నిడివి_error(Reason);
#లేకపోతే
  report_fatal_error(Twine(Reason));
#అయితే_ముగింపు
}

/// Report that this vector is already at maximum capacity. Throws
/// std::length_error or calls report_fatal_error.
[[noreturn]] స్థిర శూన్య report_at_గరిష్ఠ_సామర్థ్యం(size_t Max_కొలత);
స్థిర శూన్య report_at_గరిష్ఠ_సామర్థ్యం(size_t Max_కొలత) {
  std::string Reason =
      "SmallVector capacity unable to grow. Already at maximum size " +
      std::గమ్యం_string(Max_కొలత);
#నిర్వచితమైతే LLVM_ENABLE_EXCEPTIONS
  విసురు std::నిడివి_error(Reason);
#లేకపోతే
  report_fatal_error(Twine(Reason));
#అయితే_ముగింపు
}

// Note: Moving this function into the header may cause performance regression.
మూస <తరగతి కొలత_T>
స్థిర size_t get_New_సామర్థ్యం(size_t Min_కొలత, size_t TSize, size_t Old_సామర్థ్యం) {
  constexpr size_t Max_కొలత = std::numeric_limits<కొలత_T>::max();

  // Ensure we can fit the new capacity.
  // This is only going to be applicable when the capacity is 32 bit.
  ఒకవేళ (Min_కొలత > Max_కొలత)
    report_కొలత_అధికప్రవాహం(Min_కొలత, Max_కొలత);

  // Ensure we can meet the guarantee of space for at least one more element.
  // The above check alone will not catch the case where grow is called with a
  // default MinSize of 0, but the current capacity cannot be increased.
  // This is only going to be applicable when the capacity is 32 bit.
  ఒకవేళ (Old_సామర్థ్యం == Max_కొలత)
    report_at_గరిష్ఠ_సామర్థ్యం(Max_కొలత);

  // In theory 2*capacity can overflow if the capacity is 64 bit, but the
  // original capacity would never be large enough for this to be a problem.
  size_t New_సామర్థ్యం = 2 * Old_సామర్థ్యం + 1; // Always grow.
  తిరిగి std::clamp(New_సామర్థ్యం, Min_కొలత, Max_కొలత);
}

/// If vector was first created with capacity 0, getFirstEl() points to the
/// memory right after, an area unallocated. If a subsequent allocation,
/// that grows the vector, happens to return the same pointer as getFirstEl(),
/// get a new allocation, otherwise isSmall() will falsely return that no
/// allocation was done (true) and the memory will not be freed in the
/// destructor. If a VSize is given (vector size), also copy that many
/// elements to the new allocation - used if realloca fails to increase
/// space, and happens to allocate precisely at BeginX.
/// This is unlikely to be called often, but resolves a memory leak when the
/// situation does occur.
స్థిర శూన్య *replace_కేటాయింపు(శూన్య *NewElts, size_t TSize, size_t New_సామర్థ్యం,
                               size_t VSize = 0) {
  శూన్య *NewEltsReplace = llvm::సురక్షిత_malloc(New_సామర్థ్యం * TSize);
  ఒకవేళ (VSize)
    జ్ఞాపకం_నకలు(NewEltsReplace, NewElts, VSize * TSize);
  విడుదల(NewElts);
  తిరిగి NewEltsReplace;
}

// Note: Moving this function into the header may cause performance regression.
మూస <తరగతి కొలత_T>
శూన్య *Small_సదిశ_పునాది<కొలత_T>::mallocForGrow(శూన్య *మొదటి_El, size_t Min_కొలత,
                                             size_t TSize,
                                             size_t &New_సామర్థ్యం) {
  New_సామర్థ్యం = get_New_సామర్థ్యం<కొలత_T>(Min_కొలత, TSize, ఇది->సామర్థ్యం_2());
  // Even if capacity is not 0 now, if the vector was originally created with
  // capacity 0, it's possible for the malloc to return FirstEl.
  శూన్య *NewElts = llvm::సురక్షిత_malloc(New_సామర్థ్యం * TSize);
  ఒకవేళ (NewElts == మొదటి_El)
    NewElts = replace_కేటాయింపు(NewElts, TSize, New_సామర్థ్యం);
  తిరిగి NewElts;
}

// Note: Moving this function into the header may cause performance regression.
మూస <తరగతి కొలత_T>
శూన్య Small_సదిశ_పునాది<కొలత_T>::grow_pod(శూన్య *మొదటి_El, size_t Min_కొలత,
                                       size_t TSize) {
  size_t New_సామర్థ్యం = get_New_సామర్థ్యం<కొలత_T>(Min_కొలత, TSize, ఇది->సామర్థ్యం_2());
  శూన్య *NewElts;
  ఒకవేళ (BeginX == మొదటి_El) {
    NewElts = llvm::సురక్షిత_malloc(New_సామర్థ్యం * TSize);
    ఒకవేళ (NewElts == మొదటి_El)
      NewElts = replace_కేటాయింపు(NewElts, TSize, New_సామర్థ్యం);

    // Copy the elements over.  No need to run dtors on PODs.
    జ్ఞాపకం_నకలు(NewElts, ఇది->BeginX, కొలత_2() * TSize);
  } లేకపోతే {
    // If this wasn't grown from the inline copy, grow the allocated space.
    NewElts = llvm::సురక్షిత_realloc(ఇది->BeginX, New_సామర్థ్యం * TSize);
    ఒకవేళ (NewElts == మొదటి_El)
      NewElts = replace_కేటాయింపు(NewElts, TSize, New_సామర్థ్యం, కొలత_2());
  }

  ఇది->సమితి_కేటాయింపు_range(NewElts, New_సామర్థ్యం);
}

మూస తరగతి llvm::Small_సదిశ_పునాది<uint32_t>;

// Disable the uint64_t instantiation for 32-bit builds.
// Both uint32_t and uint64_t instantiations are needed for 64-bit builds.
// This instantiation will never be used in 32-bit builds, and will cause
// warnings when sizeof(Size_T) > sizeof(size_t).
#ఒకవేళ SIZE_MAX > UINT32_MAX
మూస తరగతి llvm::Small_సదిశ_పునాది<uint64_t>;

// Assertions to ensure this #if stays in sync with SmallVectorSizeType.
static_assert(పరిమాణం(Small_సదిశ_కొలత_Type<అక్షరం>) == పరిమాణం(uint64_t),
              "Expected SmallVectorBase<uint64_t> variant to be in use.");
#లేకపోతే
static_assert(పరిమాణం(Small_సదిశ_కొలత_Type<అక్షరం>) == పరిమాణం(uint32_t),
              "Expected SmallVectorBase<uint32_t> variant to be in use.");
#అయితే_ముగింపు
