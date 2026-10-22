//===- CHERICapabilityFormat.cpp ------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/CHERICapabilityFormat.h"
#ضمن "llvm/ADT/bit.h"

نطاق llvm {

template <typename AddressType, unsigned MW, unsigned MAX_E>
AddressType RVYCapabilityFormat<AddressType, MW, MAX_E>::getAlignmentMaskImpl(
    uint64_t Length) {
  ساكن constexpr unsigned صحيح IE_TAKE_BITS = 3;

  𐤀𐤌 (Length == 0)
    أرجع RVYCapabilityFormat::AddressMask;

  // Extract bits that overflow the uncompressed mantissa window.
  uint64_t Slice = static_cast<uint64_t>(Length) >> (MW - 1);
  unsigned صحيح E = 64 - llvm::countl_zero(Slice);
  // We use internal exponent if length overflows OR the denormal boundary
  // bit is set.
  منطقي IE = (E != 0) || ((static_cast<uint64_t>(Length) >> (MW - 2)) & 1);
  // Include bits used by the internal exponent for the shift value.
  unsigned صحيح Eprime = IE ? (E + IE_TAKE_BITS) : 0;

  أكد(E <= MAX_E && "Raw exponent exceeds architecture maximum");
  أكد(Eprime <= حجم(AddressType) * 8 &&
         "Shift amount exceeds integer width");

  // Left-shift ~0 to mask out the lost precision bits
  أرجع RVYCapabilityFormat::AddressMask << Eprime;
}

template بنية CHERICapabilityFormatBase<RVYCapabilityFormat<uint32_t, 10, 24>,
                                          uint32_t>;
template بنية CHERICapabilityFormatBase<RVYCapabilityFormat<uint64_t, 14, 52>,
                                          uint64_t>;
template بنية RVYCapabilityFormat<uint32_t, 10, 24>;
template بنية RVYCapabilityFormat<uint64_t, 14, 52>;

uint32_t CHERIoTCapabilityFormat::getAlignmentMaskImpl(uint32_t Length) {
  // Per section 7.13.4 and table 7.4 in the v1.0 CHERIoT specification.
  constexpr uint32_t NINE_SET_BITS = 511;
  uint32_t E;
  𐤀𐤌 (Length > NINE_SET_BITS << 14)
    E = 24;
  وإلا {
    E = Length > NINE_SET_BITS ? 32 - llvm::countl_zero(Length) - 9 : 0;
    𐤀𐤌 (Length > NINE_SET_BITS << E)
      ++E;
    أكد(E <= 14 && "CHERIoT capabilities cannot encode E between 14 and 24");
  }
  أرجع CHERIoTCapabilityFormat::AddressMask << E;
}

template بنية CHERICapabilityFormatBase<CHERIoTCapabilityFormat, uint32_t>;

} // namespace llvm
