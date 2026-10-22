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

کينډۍ <ډول_نوم پته_Type, بینښه MW, بینښه MAX_E>
پته_Type RVYCapabilityFormat<پته_Type, MW, MAX_E>::get_Alignment_ماسک_Impl(
    uint64_t اوږدتیا) {
  ساكن constexpr بینښه صحيح IE_TAKE_بټونه = 3;

  إذا (اوږدتیا == 0)
    أرجع RVYCapabilityFormat::پته_ماسک;

  // Extract bits that overflow the uncompressed mantissa window.
  uint64_t ټوټه_2 = static_cast<uint64_t>(اوږدتیا) >> (MW - 1);
  بینښه صحيح E = 64 - llvm::countl_zero(ټوټه_2);
  // We use internal exponent if length overflows OR the denormal boundary
  // bit is set.
  منطقي IE = (E != 0) || ((static_cast<uint64_t>(اوږدتیا) >> (MW - 2)) & 1);
  // Include bits used by the internal exponent for the shift value.
  بینښه صحيح Eprime = IE ? (E + IE_TAKE_بټونه) : 0;

  أكد(E <= MAX_E && "Raw exponent exceeds architecture maximum");
  أكد(Eprime <= حجم(پته_Type) * 8 &&
         "Shift amount exceeds integer width");

  // Left-shift ~0 to mask out the lost precision bits
  أرجع RVYCapabilityFormat::پته_ماسک << Eprime;
}

کينډۍ بنية CHERICapability_Format_بنسټ<RVYCapabilityFormat<uint32_t, 10, 24>,
                                          uint32_t>;
کينډۍ بنية CHERICapability_Format_بنسټ<RVYCapabilityFormat<uint64_t, 14, 52>,
                                          uint64_t>;
کينډۍ بنية RVYCapabilityFormat<uint32_t, 10, 24>;
کينډۍ بنية RVYCapabilityFormat<uint64_t, 14, 52>;

uint32_t CHERIoTCapabilityFormat::get_Alignment_ماسک_Impl(uint32_t اوږدتیا) {
  // Per section 7.13.4 and table 7.4 in the v1.0 CHERIoT specification.
  constexpr uint32_t NINE_سېټ_بټونه = 511;
  uint32_t E;
  إذا (اوږدتیا > NINE_سېټ_بټونه << 14)
    E = 24;
  وإلا {
    E = اوږدتیا > NINE_سېټ_بټونه ? 32 - llvm::countl_zero(اوږدتیا) - 9 : 0;
    إذا (اوږدتیا > NINE_سېټ_بټونه << E)
      ++E;
    أكد(E <= 14 && "CHERIoT capabilities cannot encode E between 14 and 24");
  }
  أرجع CHERIoTCapabilityFormat::پته_ماسک << E;
}

کينډۍ بنية CHERICapability_Format_بنسټ<CHERIoTCapabilityFormat, uint32_t>;

} // namespace llvm
