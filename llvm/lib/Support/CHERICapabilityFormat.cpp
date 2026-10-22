//===- CHERICapabilityFormat.cpp ------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/CHERICapabilityFormat.h"
#अन्तर्भाव "llvm/ADT/bit.h"

नामाकाश llvm {

template <typename AddressType, unsigned MW, unsigned MAX_E>
AddressType RVYCapabilityFormat<AddressType, MW, MAX_E>::getAlignmentMaskImpl(
    uint64_t Length) {
  स्थिर constexpr unsigned पूर्णाङ्क IE_TAKE_BITS = 3;

  यदि (Length == 0)
    प्रत्यागम RVYCapabilityFormat::AddressMask;

  // Extract bits that overflow the uncompressed mantissa window.
  uint64_t Slice = static_cast<uint64_t>(Length) >> (MW - 1);
  unsigned पूर्णाङ्क E = 64 - llvm::countl_zero(Slice);
  // We use internal exponent if length overflows OR the denormal boundary
  // bit is set.
  तार्किक IE = (E != 0) || ((static_cast<uint64_t>(Length) >> (MW - 2)) & 1);
  // Include bits used by the internal exponent for the shift value.
  unsigned पूर्णाङ्क Eprime = IE ? (E + IE_TAKE_BITS) : 0;

  प्रतिज्ञा(E <= MAX_E && "Raw exponent exceeds architecture maximum");
  प्रतिज्ञा(Eprime <= परिमाण(AddressType) * 8 &&
         "Shift amount exceeds integer width");

  // Left-shift ~0 to mask out the lost precision bits
  प्रत्यागम RVYCapabilityFormat::AddressMask << Eprime;
}

template संरचना CHERICapabilityFormatBase<RVYCapabilityFormat<uint32_t, 10, 24>,
                                          uint32_t>;
template संरचना CHERICapabilityFormatBase<RVYCapabilityFormat<uint64_t, 14, 52>,
                                          uint64_t>;
template संरचना RVYCapabilityFormat<uint32_t, 10, 24>;
template संरचना RVYCapabilityFormat<uint64_t, 14, 52>;

uint32_t CHERIoTCapabilityFormat::getAlignmentMaskImpl(uint32_t Length) {
  // Per section 7.13.4 and table 7.4 in the v1.0 CHERIoT specification.
  constexpr uint32_t NINE_SET_BITS = 511;
  uint32_t E;
  यदि (Length > NINE_SET_BITS << 14)
    E = 24;
  अन्यथा {
    E = Length > NINE_SET_BITS ? 32 - llvm::countl_zero(Length) - 9 : 0;
    यदि (Length > NINE_SET_BITS << E)
      ++E;
    प्रतिज्ञा(E <= 14 && "CHERIoT capabilities cannot encode E between 14 and 24");
  }
  प्रत्यागम CHERIoTCapabilityFormat::AddressMask << E;
}

template संरचना CHERICapabilityFormatBase<CHERIoTCapabilityFormat, uint32_t>;

} // namespace llvm
