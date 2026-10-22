//===- CHERICapabilityFormat.cpp ------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/CHERICapabilityFormat.h"
#समावेश "llvm/ADT/bit.h"

namespace llvm {

ढाँचा <typename AddressType, अचिन्हित MW, अचिन्हित MAX_E>
AddressType RVYCapabilityFormat<AddressType, MW, MAX_E>::getAlignmentMaskImpl(
    uint64_t लंबाई) {
  अटल constexpr अचिन्हित पूर्णांक IE_TAKE_BITS = 3;

  अगर (लंबाई == 0)
    वापस RVYCapabilityFormat::AddressMask;

  // Extract bits that overflow the uncompressed mantissa window.
  uint64_t Slice = static_cast<uint64_t>(लंबाई) >> (MW - 1);
  अचिन्हित पूर्णांक E = 64 - llvm::countl_zero(Slice);
  // We use internal exponent if length overflows OR the denormal boundary
  // bit is set.
  bool IE = (E != 0) || ((static_cast<uint64_t>(लंबाई) >> (MW - 2)) & 1);
  // Include bits used by the internal exponent for the shift value.
  अचिन्हित पूर्णांक Eprime = IE ? (E + IE_TAKE_BITS) : 0;

  निश्चित(E <= MAX_E && "Raw exponent exceeds architecture maximum");
  निश्चित(Eprime <= माप(AddressType) * 8 &&
         "Shift amount exceeds integer width");

  // Left-shift ~0 to mask out the lost precision bits
  वापस RVYCapabilityFormat::AddressMask << Eprime;
}

ढाँचा काष्ठा CHERICapabilityFormatBase<RVYCapabilityFormat<uint32_t, 10, 24>,
                                          uint32_t>;
ढाँचा काष्ठा CHERICapabilityFormatBase<RVYCapabilityFormat<uint64_t, 14, 52>,
                                          uint64_t>;
ढाँचा काष्ठा RVYCapabilityFormat<uint32_t, 10, 24>;
ढाँचा काष्ठा RVYCapabilityFormat<uint64_t, 14, 52>;

uint32_t CHERIoTCapabilityFormat::getAlignmentMaskImpl(uint32_t लंबाई) {
  // Per section 7.13.4 and table 7.4 in the v1.0 CHERIoT specification.
  constexpr uint32_t NINE_SET_BITS = 511;
  uint32_t E;
  अगर (लंबाई > NINE_SET_BITS << 14)
    E = 24;
  अन्यथा {
    E = लंबाई > NINE_SET_BITS ? 32 - llvm::countl_zero(लंबाई) - 9 : 0;
    अगर (लंबाई > NINE_SET_BITS << E)
      ++E;
    निश्चित(E <= 14 && "CHERIoT capabilities cannot encode E between 14 and 24");
  }
  वापस CHERIoTCapabilityFormat::AddressMask << E;
}

ढाँचा काष्ठा CHERICapabilityFormatBase<CHERIoTCapabilityFormat, uint32_t>;

} // namespace llvm
