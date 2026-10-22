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

#अन्तर्भाव "llvm/ADT/SmallVector.h"
#अन्तर्भाव "llvm/ADT/Twine.h"
#अन्तर्भाव "llvm/Support/MemAlloc.h"
#अन्तर्भाव <cstdint>
#यदि_परिभाषितम् LLVM_ENABLE_EXCEPTIONS
#अन्तर्भाव <stdexcept>
#यदि_अन्तः
उपयुज्य नामाकाश llvm;

// Check that no bytes are wasted and everything is well-aligned.
नामाकाश {
// These structures may cause binary compat warnings on AIX. Suppress the
// warning since we are only using these types for the static assertions below.
#यदि defined(_AIX)
#निर्देशः GCC diagnostic प्रक्षिप
#निर्देशः GCC diagnostic ignored "-Waix-compat"
#यदि_अन्तः
संरचना Struct16B {
  alignas(16) शून्य *X;
};
संरचना Struct32B {
  alignas(32) शून्य *X;
};
#यदि defined(_AIX)
#निर्देशः GCC diagnostic निष्कासय
#यदि_अन्तः
}
static_assert(परिमाण(Small_सदिशम्<शून्य *, 0>) ==
                  परिमाण(अचिह्नित) * 2 + परिमाण(शून्य *),
              "wasted space in SmallVector size 0");
static_assert(alignof(Small_सदिशम्<Struct16B, 0>) >= alignof(Struct16B),
              "wrong alignment for 16-byte aligned T");
static_assert(alignof(Small_सदिशम्<Struct32B, 0>) >= alignof(Struct32B),
              "wrong alignment for 32-byte aligned T");
static_assert(परिमाण(Small_सदिशम्<Struct16B, 0>) >= alignof(Struct16B),
              "missing padding for 16-byte aligned T");
static_assert(परिमाण(Small_सदिशम्<Struct32B, 0>) >= alignof(Struct32B),
              "missing padding for 32-byte aligned T");
static_assert(परिमाण(Small_सदिशम्<शून्य *, 1>) ==
                  परिमाण(अचिह्नित) * 2 + परिमाण(शून्य *) * 2,
              "wasted space in SmallVector size 1");

static_assert(परिमाण(Small_सदिशम्<वर्ण, 0>) ==
                  परिमाण(शून्य *) * 2 + परिमाण(शून्य *),
              "1 byte elements have word-sized type for size and capacity");

/// Report that MinSize doesn't fit into this vector's size type. Throws
/// std::length_error or calls report_fatal_error.
[[noreturn]] स्थिर शून्य report_आकार_अतिप्रवाहः(size_t Min_आकार, size_t Max_आकार);
स्थिर शून्य report_आकार_अतिप्रवाहः(size_t Min_आकार, size_t Max_आकार) {
  std::string Reason = "SmallVector unable to grow. Requested capacity (" +
                       std::गन्तव्यम्_string(Min_आकार) +
                       ") is larger than maximum value for size type (" +
                       std::गन्तव्यम्_string(Max_आकार) + ")";
#यदि_परिभाषितम् LLVM_ENABLE_EXCEPTIONS
  क्षिप std::दीर्घता_error(Reason);
#अन्यथा
  report_fatal_error(Twine(Reason));
#यदि_अन्तः
}

/// Report that this vector is already at maximum capacity. Throws
/// std::length_error or calls report_fatal_error.
[[noreturn]] स्थिर शून्य report_at_अधिकतमम्_क्षमता(size_t Max_आकार);
स्थिर शून्य report_at_अधिकतमम्_क्षमता(size_t Max_आकार) {
  std::string Reason =
      "SmallVector capacity unable to grow. Already at maximum size " +
      std::गन्तव्यम्_string(Max_आकार);
#यदि_परिभाषितम् LLVM_ENABLE_EXCEPTIONS
  क्षिप std::दीर्घता_error(Reason);
#अन्यथा
  report_fatal_error(Twine(Reason));
#यदि_अन्तः
}

// Note: Moving this function into the header may cause performance regression.
प्रतिरूपम् <वर्ग आकार_T>
स्थिर size_t get_New_क्षमता(size_t Min_आकार, size_t TSize, size_t Old_क्षमता) {
  constexpr size_t Max_आकार = std::numeric_limits<आकार_T>::max();

  // Ensure we can fit the new capacity.
  // This is only going to be applicable when the capacity is 32 bit.
  यदि (Min_आकार > Max_आकार)
    report_आकार_अतिप्रवाहः(Min_आकार, Max_आकार);

  // Ensure we can meet the guarantee of space for at least one more element.
  // The above check alone will not catch the case where grow is called with a
  // default MinSize of 0, but the current capacity cannot be increased.
  // This is only going to be applicable when the capacity is 32 bit.
  यदि (Old_क्षमता == Max_आकार)
    report_at_अधिकतमम्_क्षमता(Max_आकार);

  // In theory 2*capacity can overflow if the capacity is 64 bit, but the
  // original capacity would never be large enough for this to be a problem.
  size_t New_क्षमता = 2 * Old_क्षमता + 1; // Always grow.
  प्रत्यागम std::clamp(New_क्षमता, Min_आकार, Max_आकार);
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
स्थिर शून्य *replace_आवण्टनम्(शून्य *NewElts, size_t TSize, size_t New_क्षमता,
                               size_t VSize = 0) {
  शून्य *NewEltsReplace = llvm::safe_malloc(New_क्षमता * TSize);
  यदि (VSize)
    स्मृतिं_प्रतिलिख(NewEltsReplace, NewElts, VSize * TSize);
  मुक्त(NewElts);
  प्रत्यागम NewEltsReplace;
}

// Note: Moving this function into the header may cause performance regression.
प्रतिरूपम् <वर्ग आकार_T>
शून्य *Small_सदिशम्_आधारः<आकार_T>::mallocForGrow(शून्य *FirstEl, size_t Min_आकार,
                                             size_t TSize,
                                             size_t &New_क्षमता) {
  New_क्षमता = get_New_क्षमता<आकार_T>(Min_आकार, TSize, एतत्->क्षमता_2());
  // Even if capacity is not 0 now, if the vector was originally created with
  // capacity 0, it's possible for the malloc to return FirstEl.
  शून्य *NewElts = llvm::safe_malloc(New_क्षमता * TSize);
  यदि (NewElts == FirstEl)
    NewElts = replace_आवण्टनम्(NewElts, TSize, New_क्षमता);
  प्रत्यागम NewElts;
}

// Note: Moving this function into the header may cause performance regression.
प्रतिरूपम् <वर्ग आकार_T>
शून्य Small_सदिशम्_आधारः<आकार_T>::grow_pod(शून्य *FirstEl, size_t Min_आकार,
                                       size_t TSize) {
  size_t New_क्षमता = get_New_क्षमता<आकार_T>(Min_आकार, TSize, एतत्->क्षमता_2());
  शून्य *NewElts;
  यदि (BeginX == FirstEl) {
    NewElts = llvm::safe_malloc(New_क्षमता * TSize);
    यदि (NewElts == FirstEl)
      NewElts = replace_आवण्टनम्(NewElts, TSize, New_क्षमता);

    // Copy the elements over.  No need to run dtors on PODs.
    स्मृतिं_प्रतिलिख(NewElts, एतत्->BeginX, आकार_2() * TSize);
  } अन्यथा {
    // If this wasn't grown from the inline copy, grow the allocated space.
    NewElts = llvm::safe_realloc(एतत्->BeginX, New_क्षमता * TSize);
    यदि (NewElts == FirstEl)
      NewElts = replace_आवण्टनम्(NewElts, TSize, New_क्षमता, आकार_2());
  }

  एतत्->समुच्चयः_आवण्टनम्_range(NewElts, New_क्षमता);
}

प्रतिरूपम् वर्ग llvm::Small_सदिशम्_आधारः<uint32_t>;

// Disable the uint64_t instantiation for 32-bit builds.
// Both uint32_t and uint64_t instantiations are needed for 64-bit builds.
// This instantiation will never be used in 32-bit builds, and will cause
// warnings when sizeof(Size_T) > sizeof(size_t).
#यदि SIZE_MAX > UINT32_MAX
प्रतिरूपम् वर्ग llvm::Small_सदिशम्_आधारः<uint64_t>;

// Assertions to ensure this #if stays in sync with SmallVectorSizeType.
static_assert(परिमाण(Small_सदिशम्_आकार_Type<वर्ण>) == परिमाण(uint64_t),
              "Expected SmallVectorBase<uint64_t> variant to be in use.");
#अन्यथा
static_assert(परिमाण(Small_सदिशम्_आकार_Type<वर्ण>) == परिमाण(uint32_t),
              "Expected SmallVectorBase<uint32_t> variant to be in use.");
#यदि_अन्तः
