//===- CHERICapabilityFormat.cpp ------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/CHERICapabilityFormat.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/bit.h"

ਨਾਮਾਕਾਸ਼ llvm {

ਸਾਂਚਾ <ਕਿਸਮ_ਨਾਂ ਪਤਾ_Type, ਬਿਨਾਨਿਸ਼ਾਨੀ MW, ਬਿਨਾਨਿਸ਼ਾਨੀ MAX_E>
ਪਤਾ_Type RVYCapabilityFormat<ਪਤਾ_Type, MW, MAX_E>::get_Alignment_ਨਕਾਬ_Impl(
    uint64_t ਲੰਮਾਈ) {
  ਸਥਿਰ constexpr ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ IE_TAKE_ਬਿੱਟ = 3;

  ਜੇ (ਲੰਮਾਈ == 0)
    ਵਾਪਸ RVYCapabilityFormat::ਪਤਾ_ਨਕਾਬ;

  // Extract bits that overflow the uncompressed mantissa window.
  uint64_t ਟੁਕੜਾ_2 = static_cast<uint64_t>(ਲੰਮਾਈ) >> (MW - 1);
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ E = 64 - llvm::countl_zero(ਟੁਕੜਾ_2);
  // We use internal exponent if length overflows OR the denormal boundary
  // bit is set.
  ਤਾਰਕਿਕ IE = (E != 0) || ((static_cast<uint64_t>(ਲੰਮਾਈ) >> (MW - 2)) & 1);
  // Include bits used by the internal exponent for the shift value.
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ Eprime = IE ? (E + IE_TAKE_ਬਿੱਟ) : 0;

  ਪ੍ਰਤਿਜ੍ਞਾ(E <= MAX_E && "Raw exponent exceeds architecture maximum");
  ਪ੍ਰਤਿਜ੍ਞਾ(Eprime <= ਆਕਾਰ(ਪਤਾ_Type) * 8 &&
         "Shift amount exceeds integer width");

  // Left-shift ~0 to mask out the lost precision bits
  ਵਾਪਸ RVYCapabilityFormat::ਪਤਾ_ਨਕਾਬ << Eprime;
}

ਸਾਂਚਾ ਬਣਤਰ CHERICapability_Format_ਅਧਾਰ<RVYCapabilityFormat<uint32_t, 10, 24>,
                                          uint32_t>;
ਸਾਂਚਾ ਬਣਤਰ CHERICapability_Format_ਅਧਾਰ<RVYCapabilityFormat<uint64_t, 14, 52>,
                                          uint64_t>;
ਸਾਂਚਾ ਬਣਤਰ RVYCapabilityFormat<uint32_t, 10, 24>;
ਸਾਂਚਾ ਬਣਤਰ RVYCapabilityFormat<uint64_t, 14, 52>;

uint32_t CHERIoTCapabilityFormat::get_Alignment_ਨਕਾਬ_Impl(uint32_t ਲੰਮਾਈ) {
  // Per section 7.13.4 and table 7.4 in the v1.0 CHERIoT specification.
  constexpr uint32_t NINE_ਸੈੱਟ_ਬਿੱਟ = 511;
  uint32_t E;
  ਜੇ (ਲੰਮਾਈ > NINE_ਸੈੱਟ_ਬਿੱਟ << 14)
    E = 24;
  ਨਹੀਂਤਾਂ {
    E = ਲੰਮਾਈ > NINE_ਸੈੱਟ_ਬਿੱਟ ? 32 - llvm::countl_zero(ਲੰਮਾਈ) - 9 : 0;
    ਜੇ (ਲੰਮਾਈ > NINE_ਸੈੱਟ_ਬਿੱਟ << E)
      ++E;
    ਪ੍ਰਤਿਜ੍ਞਾ(E <= 14 && "CHERIoT capabilities cannot encode E between 14 and 24");
  }
  ਵਾਪਸ CHERIoTCapabilityFormat::ਪਤਾ_ਨਕਾਬ << E;
}

ਸਾਂਚਾ ਬਣਤਰ CHERICapability_Format_ਅਧਾਰ<CHERIoTCapabilityFormat, uint32_t>;

} // namespace llvm
