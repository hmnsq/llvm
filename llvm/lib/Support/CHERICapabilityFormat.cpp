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

سانچا <قسم_ناں پتہ_Type, بنانشانی MW, بنانشانی MAX_E>
پتہ_Type RVYCapabilityFormat<پتہ_Type, MW, MAX_E>::get_Alignment_نقاب_Impl(
    uint64_t لمائی) {
  ساكن constexpr بنانشانی صحيح IE_TAKE_بٹ = 3;

  إذا (لمائی == 0)
    أرجع RVYCapabilityFormat::پتہ_نقاب;

  // Extract bits that overflow the uncompressed mantissa window.
  uint64_t ٹکڑا_2 = static_cast<uint64_t>(لمائی) >> (MW - 1);
  بنانشانی صحيح E = 64 - llvm::countl_zero(ٹکڑا_2);
  // We use internal exponent if length overflows OR the denormal boundary
  // bit is set.
  منطقي IE = (E != 0) || ((static_cast<uint64_t>(لمائی) >> (MW - 2)) & 1);
  // Include bits used by the internal exponent for the shift value.
  بنانشانی صحيح Eprime = IE ? (E + IE_TAKE_بٹ) : 0;

  أكد(E <= MAX_E && "Raw exponent exceeds architecture maximum");
  أكد(Eprime <= حجم(پتہ_Type) * 8 &&
         "Shift amount exceeds integer width");

  // Left-shift ~0 to mask out the lost precision bits
  أرجع RVYCapabilityFormat::پتہ_نقاب << Eprime;
}

سانچا بنية CHERICapability_Format_ادھار<RVYCapabilityFormat<uint32_t, 10, 24>,
                                          uint32_t>;
سانچا بنية CHERICapability_Format_ادھار<RVYCapabilityFormat<uint64_t, 14, 52>,
                                          uint64_t>;
سانچا بنية RVYCapabilityFormat<uint32_t, 10, 24>;
سانچا بنية RVYCapabilityFormat<uint64_t, 14, 52>;

uint32_t CHERIoTCapabilityFormat::get_Alignment_نقاب_Impl(uint32_t لمائی) {
  // Per section 7.13.4 and table 7.4 in the v1.0 CHERIoT specification.
  constexpr uint32_t NINE_سیٹ_بٹ = 511;
  uint32_t E;
  إذا (لمائی > NINE_سیٹ_بٹ << 14)
    E = 24;
  وإلا {
    E = لمائی > NINE_سیٹ_بٹ ? 32 - llvm::countl_zero(لمائی) - 9 : 0;
    إذا (لمائی > NINE_سیٹ_بٹ << E)
      ++E;
    أكد(E <= 14 && "CHERIoT capabilities cannot encode E between 14 and 24");
  }
  أرجع CHERIoTCapabilityFormat::پتہ_نقاب << E;
}

سانچا بنية CHERICapability_Format_ادھار<CHERIoTCapabilityFormat, uint32_t>;

} // namespace llvm
