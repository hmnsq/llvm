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

قالب <نام_نوع آدرس_Type, بیعلامت MW, بیعلامت MAX_E>
آدرس_Type RVYCapabilityFormat<آدرس_Type, MW, MAX_E>::get_Alignment_ماسک_Impl(
    uint64_t درازی) {
  ساكن constexpr بیعلامت صحيح IE_TAKE_بیتها = 3;

  إذا (درازی == 0)
    أرجع RVYCapabilityFormat::آدرس_ماسک;

  // Extract bits that overflow the uncompressed mantissa window.
  uint64_t برش_2 = static_cast<uint64_t>(درازی) >> (MW - 1);
  بیعلامت صحيح E = 64 - llvm::countl_zero(برش_2);
  // We use internal exponent if length overflows OR the denormal boundary
  // bit is set.
  منطقي IE = (E != 0) || ((static_cast<uint64_t>(درازی) >> (MW - 2)) & 1);
  // Include bits used by the internal exponent for the shift value.
  بیعلامت صحيح Eprime = IE ? (E + IE_TAKE_بیتها) : 0;

  أكد(E <= MAX_E && "Raw exponent exceeds architecture maximum");
  أكد(Eprime <= حجم(آدرس_Type) * 8 &&
         "Shift amount exceeds integer width");

  // Left-shift ~0 to mask out the lost precision bits
  أرجع RVYCapabilityFormat::آدرس_ماسک << Eprime;
}

قالب بنية CHERICapability_Format_پایه<RVYCapabilityFormat<uint32_t, 10, 24>,
                                          uint32_t>;
قالب بنية CHERICapability_Format_پایه<RVYCapabilityFormat<uint64_t, 14, 52>,
                                          uint64_t>;
قالب بنية RVYCapabilityFormat<uint32_t, 10, 24>;
قالب بنية RVYCapabilityFormat<uint64_t, 14, 52>;

uint32_t CHERIoTCapabilityFormat::get_Alignment_ماسک_Impl(uint32_t درازی) {
  // Per section 7.13.4 and table 7.4 in the v1.0 CHERIoT specification.
  constexpr uint32_t NINE_مجموعه_بیتها = 511;
  uint32_t E;
  إذا (درازی > NINE_مجموعه_بیتها << 14)
    E = 24;
  وإلا {
    E = درازی > NINE_مجموعه_بیتها ? 32 - llvm::countl_zero(درازی) - 9 : 0;
    إذا (درازی > NINE_مجموعه_بیتها << E)
      ++E;
    أكد(E <= 14 && "CHERIoT capabilities cannot encode E between 14 and 24");
  }
  أرجع CHERIoTCapabilityFormat::آدرس_ماسک << E;
}

قالب بنية CHERICapability_Format_پایه<CHERIoTCapabilityFormat, uint32_t>;

} // namespace llvm
