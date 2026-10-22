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

سانچو <قسم_نالو پتو_Type, بنانشان MW, بنانشان MAX_E>
پتو_Type RVYCapabilityFormat<پتو_Type, MW, MAX_E>::get_Alignment_نقاب_Impl(
    uint64_t ڊيگهائي) {
  ساكن constexpr بنانشان صحيح IE_TAKE_بٽ = 3;

  إذا (ڊيگهائي == 0)
    أرجع RVYCapabilityFormat::پتو_نقاب;

  // Extract bits that overflow the uncompressed mantissa window.
  uint64_t ٽڪرو_2 = static_cast<uint64_t>(ڊيگهائي) >> (MW - 1);
  بنانشان صحيح E = 64 - llvm::countl_zero(ٽڪرو_2);
  // We use internal exponent if length overflows OR the denormal boundary
  // bit is set.
  منطقي IE = (E != 0) || ((static_cast<uint64_t>(ڊيگهائي) >> (MW - 2)) & 1);
  // Include bits used by the internal exponent for the shift value.
  بنانشان صحيح Eprime = IE ? (E + IE_TAKE_بٽ) : 0;

  أكد(E <= MAX_E && "Raw exponent exceeds architecture maximum");
  أكد(Eprime <= حجم(پتو_Type) * 8 &&
         "Shift amount exceeds integer width");

  // Left-shift ~0 to mask out the lost precision bits
  أرجع RVYCapabilityFormat::پتو_نقاب << Eprime;
}

سانچو بنية CHERICapability_Format_بنياد<RVYCapabilityFormat<uint32_t, 10, 24>,
                                          uint32_t>;
سانچو بنية CHERICapability_Format_بنياد<RVYCapabilityFormat<uint64_t, 14, 52>,
                                          uint64_t>;
سانچو بنية RVYCapabilityFormat<uint32_t, 10, 24>;
سانچو بنية RVYCapabilityFormat<uint64_t, 14, 52>;

uint32_t CHERIoTCapabilityFormat::get_Alignment_نقاب_Impl(uint32_t ڊيگهائي) {
  // Per section 7.13.4 and table 7.4 in the v1.0 CHERIoT specification.
  constexpr uint32_t NINE_سيٽ_بٽ = 511;
  uint32_t E;
  إذا (ڊيگهائي > NINE_سيٽ_بٽ << 14)
    E = 24;
  وإلا {
    E = ڊيگهائي > NINE_سيٽ_بٽ ? 32 - llvm::countl_zero(ڊيگهائي) - 9 : 0;
    إذا (ڊيگهائي > NINE_سيٽ_بٽ << E)
      ++E;
    أكد(E <= 14 && "CHERIoT capabilities cannot encode E between 14 and 24");
  }
  أرجع CHERIoTCapabilityFormat::پتو_نقاب << E;
}

سانچو بنية CHERICapability_Format_بنياد<CHERIoTCapabilityFormat, uint32_t>;

} // namespace llvm
