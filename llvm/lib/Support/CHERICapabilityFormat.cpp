//===- CHERICapabilityFormat.cpp ------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাৱ "llvm/Support/CHERICapabilityFormat.h"
#অন্তর্ভাৱ "llvm/ADT/bit.h"

নামাকাশ llvm {

template <typename AddressType, unsigned MW, unsigned MAX_E>
AddressType RVYCapabilityFormat<AddressType, MW, MAX_E>::getAlignmentMaskImpl(
    uint64_t Length) {
  স্থির constexpr unsigned পূর্ণাঙ্ক IE_TAKE_BITS = 3;

  যদি (Length == 0)
    ঘূৰাই RVYCapabilityFormat::AddressMask;

  // Extract bits that overflow the uncompressed mantissa window.
  uint64_t Slice = static_cast<uint64_t>(Length) >> (MW - 1);
  unsigned পূর্ণাঙ্ক E = 64 - llvm::countl_zero(Slice);
  // We use internal exponent if length overflows OR the denormal boundary
  // bit is set.
  তার্কিক IE = (E != 0) || ((static_cast<uint64_t>(Length) >> (MW - 2)) & 1);
  // Include bits used by the internal exponent for the shift value.
  unsigned পূর্ণাঙ্ক Eprime = IE ? (E + IE_TAKE_BITS) : 0;

  প্রতিজ্ঞা(E <= MAX_E && "Raw exponent exceeds architecture maximum");
  প্রতিজ্ঞা(Eprime <= পরিমাণ(AddressType) * 8 &&
         "Shift amount exceeds integer width");

  // Left-shift ~0 to mask out the lost precision bits
  ঘূৰাই RVYCapabilityFormat::AddressMask << Eprime;
}

template সংরচনা CHERICapabilityFormatBase<RVYCapabilityFormat<uint32_t, 10, 24>,
                                          uint32_t>;
template সংরচনা CHERICapabilityFormatBase<RVYCapabilityFormat<uint64_t, 14, 52>,
                                          uint64_t>;
template সংরচনা RVYCapabilityFormat<uint32_t, 10, 24>;
template সংরচনা RVYCapabilityFormat<uint64_t, 14, 52>;

uint32_t CHERIoTCapabilityFormat::getAlignmentMaskImpl(uint32_t Length) {
  // Per section 7.13.4 and table 7.4 in the v1.0 CHERIoT specification.
  constexpr uint32_t NINE_SET_BITS = 511;
  uint32_t E;
  যদি (Length > NINE_SET_BITS << 14)
    E = 24;
  অন্যথা {
    E = Length > NINE_SET_BITS ? 32 - llvm::countl_zero(Length) - 9 : 0;
    যদি (Length > NINE_SET_BITS << E)
      ++E;
    প্রতিজ্ঞা(E <= 14 && "CHERIoT capabilities cannot encode E between 14 and 24");
  }
  ঘূৰাই CHERIoTCapabilityFormat::AddressMask << E;
}

template সংরচনা CHERICapabilityFormatBase<CHERIoTCapabilityFormat, uint32_t>;

} // namespace llvm
