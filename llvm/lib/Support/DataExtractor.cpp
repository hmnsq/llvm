//===-- DataExtractor.cpp -------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/DataExtractor.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Errc.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ErrorHandling.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/LEB128.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/SwapByteOrder.h"

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

ತಾರ್ಕಿಕ ದತ್ತಾಂಶ_Extractor::prepareRead(uint64_t ಸ್ಥಾನಾಂತರ, uint64_t ಗಾತ್ರ_2,
                                Error *E) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (is_Valid_ಸ್ಥಾನಾಂತರ_For_ದತ್ತಾಂಶ_ದ_ಗಾತ್ರ(ಸ್ಥಾನಾಂತರ, ಗಾತ್ರ_2))
    ಹಿಂತಿರುಗಿ ನಿಜ;
  ಆದರೆ (E) {
    ಆದರೆ (ಸ್ಥಾನಾಂತರ <= ದತ್ತಾಂಶ_2.ಗಾತ್ರ_3())
      *E = createStringError(
          errc::illegal_byte_sequence,
          "unexpected end of data at offset 0x%zx while reading [0x%" PRIx64
          ", 0x%" PRIx64 ")",
          ದತ್ತಾಂಶ_2.ಗಾತ್ರ_3(), ಸ್ಥಾನಾಂತರ, ಸ್ಥಾನಾಂತರ + ಗಾತ್ರ_2);
    ಇಲ್ಲದಿದ್ದರೆ
      *E = createStringError(errc::invalid_ವಾದ,
                             "offset 0x%" PRIx64
                             " is beyond the end of data at 0x%zx",
                             ಸ್ಥಾನಾಂತರ, ದತ್ತಾಂಶ_2.ಗಾತ್ರ_3());
  }
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ isError(Error *E) { ಹಿಂತಿರುಗಿ E && *E; }

ಮಾದರಿ <ಬಗೆ_ಹೆಸರು T>
T ದತ್ತಾಂಶ_Extractor::getU(uint64_t *ಸ್ಥಾನಾಂತರ_ptr, Error *Err) ಸ್ಥಿರಾಂಕ {
  Error_As_ಹೊರಹೋಗು_Parameter Err_As_ಹೊರಹೋಗು(Err);
  T val = 0;
  ಆದರೆ (isError(Err))
    ಹಿಂತಿರುಗಿ val;

  uint64_t ಸ್ಥಾನಾಂತರ_2 = *ಸ್ಥಾನಾಂತರ_ptr;
  ಆದರೆ (!prepareRead(ಸ್ಥಾನಾಂತರ_2, ಗಾತ್ರ(T), Err))
    ಹಿಂತಿರುಗಿ val;
  std::ಸ್ಮರಣೆ_ನಕಲು(&val, &ದತ್ತಾಂಶ_2.ದತ್ತಾಂಶ()[ಸ್ಥಾನಾಂತರ_2], ಗಾತ್ರ(val));
  ಆದರೆ (sys::IsLittleEndianHost != IsLittleEndian)
    sys::ಅದಲುಬದಲು_Byte_ಕ್ರಮ(val);

  // Advance the offset
  *ಸ್ಥಾನಾಂತರ_ptr += ಗಾತ್ರ(val);
  ಹಿಂತಿರುಗಿ val;
}

ಮಾದರಿ <ಬಗೆ_ಹೆಸರು T>
T *ದತ್ತಾಂಶ_Extractor::getUs(uint64_t *ಸ್ಥಾನಾಂತರ_ptr, T *dst, uint32_t ಎಣಿಕೆ_2,
                        Error *Err) ಸ್ಥಿರಾಂಕ {
  Error_As_ಹೊರಹೋಗು_Parameter Err_As_ಹೊರಹೋಗು(Err);
  ಆದರೆ (isError(Err))
    ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;

  uint64_t ಸ್ಥಾನಾಂತರ_2 = *ಸ್ಥಾನಾಂತರ_ptr;

  ಆದರೆ (!prepareRead(ಸ್ಥಾನಾಂತರ_2, ಗಾತ್ರ(*dst) * ಎಣಿಕೆ_2, Err))
    ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
  ಗಾಗಿ (T *ಮೌಲ್ಯ_ptr = dst, *ಕೊನೆ = dst + ಎಣಿಕೆ_2; ಮೌಲ್ಯ_ptr != ಕೊನೆ;
       ++ಮೌಲ್ಯ_ptr, ಸ್ಥಾನಾಂತರ_2 += ಗಾತ್ರ(*dst))
    *ಮೌಲ್ಯ_ptr = getU<T>(ಸ್ಥಾನಾಂತರ_ptr, Err);
  // Advance the offset
  *ಸ್ಥಾನಾಂತರ_ptr = ಸ್ಥಾನಾಂತರ_2;
  // Return a non-NULL pointer to the converted data as an indicator of
  // success
  ಹಿಂತಿರುಗಿ dst;
}

uint8_t ದತ್ತಾಂಶ_Extractor::getU8(uint64_t *ಸ್ಥಾನಾಂತರ_ptr, llvm::Error *Err) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ getU<uint8_t>(ಸ್ಥಾನಾಂತರ_ptr, Err);
}

uint8_t *ದತ್ತಾಂಶ_Extractor::getU8(uint64_t *ಸ್ಥಾನಾಂತರ_ptr, uint8_t *dst,
                              uint32_t ಎಣಿಕೆ_2) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ getUs<uint8_t>(ಸ್ಥಾನಾಂತರ_ptr, dst, ಎಣಿಕೆ_2, ಶೂನ್ಯಸೂಚಕ);
}

uint8_t *ದತ್ತಾಂಶ_Extractor::getU8(Cursor &C, uint8_t *Dst, uint32_t ಎಣಿಕೆ_3) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ getUs<uint8_t>(&C.ಸ್ಥಾನಾಂತರ, Dst, ಎಣಿಕೆ_3, &C.Err);
}

uint16_t ದತ್ತಾಂಶ_Extractor::getU16(uint64_t *ಸ್ಥಾನಾಂತರ_ptr, llvm::Error *Err) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ getU<uint16_t>(ಸ್ಥಾನಾಂತರ_ptr, Err);
}

uint16_t *ದತ್ತಾಂಶ_Extractor::getU16(uint64_t *ಸ್ಥಾನಾಂತರ_ptr, uint16_t *dst,
                                uint32_t ಎಣಿಕೆ_2) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ getUs<uint16_t>(ಸ್ಥಾನಾಂತರ_ptr, dst, ಎಣಿಕೆ_2, ಶೂನ್ಯಸೂಚಕ);
}

uint32_t ದತ್ತಾಂಶ_Extractor::getU24(uint64_t *ಸ್ಥಾನಾಂತರ_Ptr, Error *Err) ಸ್ಥಿರಾಂಕ {
  uint24_t ExtractedVal = getU<uint24_t>(ಸ್ಥಾನಾಂತರ_Ptr, Err);
  // The 3 bytes are in the correct byte order for the host.
  ಹಿಂತಿರುಗಿ ExtractedVal.getAsUint32(sys::IsLittleEndianHost);
}

uint32_t ದತ್ತಾಂಶ_Extractor::getU32(uint64_t *ಸ್ಥಾನಾಂತರ_ptr, llvm::Error *Err) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ getU<uint32_t>(ಸ್ಥಾನಾಂತರ_ptr, Err);
}

uint32_t *ದತ್ತಾಂಶ_Extractor::getU32(uint64_t *ಸ್ಥಾನಾಂತರ_ptr, uint32_t *dst,
                                uint32_t ಎಣಿಕೆ_2) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ getUs<uint32_t>(ಸ್ಥಾನಾಂತರ_ptr, dst, ಎಣಿಕೆ_2, ಶೂನ್ಯಸೂಚಕ);
}

uint64_t ದತ್ತಾಂಶ_Extractor::getU64(uint64_t *ಸ್ಥಾನಾಂತರ_ptr, llvm::Error *Err) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ getU<uint64_t>(ಸ್ಥಾನಾಂತರ_ptr, Err);
}

uint64_t *ದತ್ತಾಂಶ_Extractor::getU64(uint64_t *ಸ್ಥಾನಾಂತರ_ptr, uint64_t *dst,
                                uint32_t ಎಣಿಕೆ_2) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ getUs<uint64_t>(ಸ್ಥಾನಾಂತರ_ptr, dst, ಎಣಿಕೆ_2, ಶೂನ್ಯಸೂಚಕ);
}

uint64_t ದತ್ತಾಂಶ_Extractor::getUnsigned(uint64_t *ಸ್ಥಾನಾಂತರ_ptr, uint32_t byte_ಗಾತ್ರ,
                                    llvm::Error *Err) ಸ್ಥಿರಾಂಕ {
  ಆಯ್ಕೆ (byte_ಗಾತ್ರ) {
  ಸ್ಥಿತಿ 1:
    ಹಿಂತಿರುಗಿ getU8(ಸ್ಥಾನಾಂತರ_ptr, Err);
  ಸ್ಥಿತಿ 2:
    ಹಿಂತಿರುಗಿ getU16(ಸ್ಥಾನಾಂತರ_ptr, Err);
  ಸ್ಥಿತಿ 4:
    ಹಿಂತಿರುಗಿ getU32(ಸ್ಥಾನಾಂತರ_ptr, Err);
  ಸ್ಥಿತಿ 8:
    ಹಿಂತಿರುಗಿ getU64(ಸ್ಥಾನಾಂತರ_ptr, Err);
  }

  // For any other byte size, read the bytes and swap/shift if necessary.
  Error_As_ಹೊರಹೋಗು_Parameter Err_As_ಹೊರಹೋಗು(Err);
  uint64_t val = 0;
  ಆದರೆ (isError(Err))
    ಹಿಂತಿರುಗಿ val;
  uint64_t ಸ್ಥಾನಾಂತರ_2 = *ಸ್ಥಾನಾಂತರ_ptr;
  ಆದರೆ (!prepareRead(ಸ್ಥಾನಾಂತರ_2, byte_ಗಾತ್ರ, Err))
    ಹಿಂತಿರುಗಿ val;
  // Copy into the least significant bytes of val regardless of host
  // endianness.
  std::ಸ್ಮರಣೆ_ನಕಲು(reinterpret_cast<ಅಕ್ಷರ *>(&val) +
                  (sys::IsLittleEndianHost ? 0 : 8 - byte_ಗಾತ್ರ),
              &ದತ್ತಾಂಶ_2.ದತ್ತಾಂಶ()[ಸ್ಥಾನಾಂತರ_2], byte_ಗಾತ್ರ);
  // Swap the least significant bytes of val if endianness doesn't match.
  ಆದರೆ (sys::IsLittleEndianHost != IsLittleEndian)
    val = sys::get_Swapped_ಬೈಟುಗಳು(val) >> (8 * (8 - byte_ಗಾತ್ರ));

  *ಸ್ಥಾನಾಂತರ_ptr += byte_ಗಾತ್ರ;
  ಹಿಂತಿರುಗಿ val;
}

int64_t
ದತ್ತಾಂಶ_Extractor::getSigned(uint64_t *ಸ್ಥಾನಾಂತರ_ptr, uint32_t byte_ಗಾತ್ರ) ಸ್ಥಿರಾಂಕ {
  ಆಯ್ಕೆ (byte_ಗಾತ್ರ) {
  ಸ್ಥಿತಿ 1:
    ಹಿಂತಿರುಗಿ getS8(ಸ್ಥಾನಾಂತರ_ptr);
  ಸ್ಥಿತಿ 2:
    ಹಿಂತಿರುಗಿ getS16(ಸ್ಥಾನಾಂತರ_ptr);
  ಸ್ಥಿತಿ 4:
    ಹಿಂತಿರುಗಿ getS32(ಸ್ಥಾನಾಂತರ_ptr);
  ಸ್ಥಿತಿ 8:
    ಹಿಂತಿರುಗಿ getS64(ಸ್ಥಾನಾಂತರ_ptr);
  }
  llvm_unreachable("getSigned unhandled case!");
}

StringRef ದತ್ತಾಂಶ_Extractor::getCStrRef(uint64_t *ಸ್ಥಾನಾಂತರ_Ptr, Error *Err) ಸ್ಥಿರಾಂಕ {
  Error_As_ಹೊರಹೋಗು_Parameter Err_As_ಹೊರಹೋಗು(Err);
  ಆದರೆ (isError(Err))
    ಹಿಂತಿರುಗಿ StringRef();

  uint64_t ಆರಂಭ = *ಸ್ಥಾನಾಂತರ_Ptr;
  StringRef::ಗಾತ್ರ_type Pos = ದತ್ತಾಂಶ_2.ಕಂಡುಹಿಡಿ('\0', ಆರಂಭ);
  ಆದರೆ (Pos != StringRef::npos) {
    *ಸ್ಥಾನಾಂತರ_Ptr = Pos + 1;
    ಹಿಂತಿರುಗಿ StringRef(ದತ್ತಾಂಶ_2.ದತ್ತಾಂಶ() + ಆರಂಭ, Pos - ಆರಂಭ);
  }
  ಆದರೆ (Err)
    *Err = createStringError(errc::illegal_byte_sequence,
                             "no null terminated string at offset 0x%" PRIx64,
                             ಆರಂಭ);
  ಹಿಂತಿರುಗಿ StringRef();
}

StringRef ದತ್ತಾಂಶ_Extractor::get_Fixed_ಉದ್ದಳತೆ_String(uint64_t *ಸ್ಥಾನಾಂತರ_Ptr,
                                              uint64_t ಉದ್ದಳತೆ,
                                              StringRef TrimChars) ಸ್ಥಿರಾಂಕ {
  StringRef ಬೈಟುಗಳು_2(get_ಬೈಟುಗಳು(ಸ್ಥಾನಾಂತರ_Ptr, ಉದ್ದಳತೆ));
  ಹಿಂತಿರುಗಿ ಬೈಟುಗಳು_2.trim(TrimChars);
}

StringRef ದತ್ತಾಂಶ_Extractor::get_ಬೈಟುಗಳು(uint64_t *ಸ್ಥಾನಾಂತರ_Ptr, uint64_t ಉದ್ದಳತೆ,
                                  Error *Err) ಸ್ಥಿರಾಂಕ {
  Error_As_ಹೊರಹೋಗು_Parameter Err_As_ಹೊರಹೋಗು(Err);
  ಆದರೆ (isError(Err))
    ಹಿಂತಿರುಗಿ StringRef();

  ಆದರೆ (!prepareRead(*ಸ್ಥಾನಾಂತರ_Ptr, ಉದ್ದಳತೆ, Err))
    ಹಿಂತಿರುಗಿ StringRef();

  StringRef ಫಲಿತಾಂಶ = ದತ್ತಾಂಶ_2.substr(*ಸ್ಥಾನಾಂತರ_Ptr, ಉದ್ದಳತೆ);
  *ಸ್ಥಾನಾಂತರ_Ptr += ಉದ್ದಳತೆ;
  ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

ಮಾದರಿ <ಬಗೆ_ಹೆಸರು T>
ಸ್ಥಿರ T getLEB128(StringRef ದತ್ತಾಂಶ_2, uint64_t *ಸ್ಥಾನಾಂತರ_Ptr, Error *Err,
                   T (&Decoder)(ಸ್ಥಿರಾಂಕ uint8_t *p, ಚಿಹ್ನೆರಹಿತ *n,
                                ಸ್ಥಿರಾಂಕ uint8_t *ಕೊನೆ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **error)) {
  ArrayRef<uint8_t> ಬೈಟುಗಳು_2 = arrayRefFromStringRef(ದತ್ತಾಂಶ_2);
  ಪ್ರತಿಜ್ಞಾ(*ಸ್ಥಾನಾಂತರ_Ptr <= ಬೈಟುಗಳು_2.ಗಾತ್ರ_3());
  Error_As_ಹೊರಹೋಗು_Parameter Err_As_ಹೊರಹೋಗು(Err);
  ಆದರೆ (isError(Err))
    ಹಿಂತಿರುಗಿ T();

  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *error = ಶೂನ್ಯಸೂಚಕ;
  ಚಿಹ್ನೆರಹಿತ ಬೈಟುಗಳು_read;
  T ಫಲಿತಾಂಶ_2 =
      Decoder(ಬೈಟುಗಳು_2.ದತ್ತಾಂಶ() + *ಸ್ಥಾನಾಂತರ_Ptr, &ಬೈಟುಗಳು_read, ಬೈಟುಗಳು_2.ಕೊನೆ(), &error);
  ಆದರೆ (error) {
    ಆದರೆ (Err)
      *Err = createStringError(errc::illegal_byte_sequence,
                               "unable to decode LEB128 at offset 0x%8.8" PRIx64
                               ": %s",
                               *ಸ್ಥಾನಾಂತರ_Ptr, error);
    ಹಿಂತಿರುಗಿ T();
  }
  *ಸ್ಥಾನಾಂತರ_Ptr += ಬೈಟುಗಳು_read;
  ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ_2;
}

uint64_t ದತ್ತಾಂಶ_Extractor::getULEB128(uint64_t *ಸ್ಥಾನಾಂತರ_ptr, Error *Err) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ getLEB128(ದತ್ತಾಂಶ_2, ಸ್ಥಾನಾಂತರ_ptr, Err, decodeULEB128);
}

int64_t ದತ್ತಾಂಶ_Extractor::getSLEB128(uint64_t *ಸ್ಥಾನಾಂತರ_ptr, Error *Err) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ getLEB128(ದತ್ತಾಂಶ_2, ಸ್ಥಾನಾಂತರ_ptr, Err, decodeSLEB128);
}

APSInt ದತ್ತಾಂಶ_Extractor::getSLEB128APSInt(uint64_t *ಸ್ಥಾನಾಂತರ_Ptr, Error *Err) ಸ್ಥಿರಾಂಕ {
  Error_As_ಹೊರಹೋಗು_Parameter Err_As_ಹೊರಹೋಗು(Err);
  ಆದರೆ (isError(Err))
    ಹಿಂತಿರುಗಿ APSInt();

  uint64_t ಸ್ಥಾನಾಂತರ = *ಸ್ಥಾನಾಂತರ_Ptr;
  APInt ಮೌಲ್ಯ;
  ಚಿಹ್ನೆರಹಿತ ಸರಿಸು_2 = 0;
  uint8_t Byte;
  ಮಾಡು {
    ಆದರೆ (!prepareRead(ಸ್ಥಾನಾಂತರ, 1, Err))
      ಹಿಂತಿರುಗಿ APSInt();
    Byte = ದತ್ತಾಂಶ_2[ಸ್ಥಾನಾಂತರ++];
    APInt ತುಂಡು_2(7, Byte & 0x7f);
    ಆದರೆ (ಸರಿಸು_2 == 0)
      ಮೌಲ್ಯ = ತುಂಡು_2;
    ಇಲ್ಲದಿದ್ದರೆ {
      ಮೌಲ್ಯ = ಮೌಲ್ಯ.zext(ಸರಿಸು_2 + 7);
      ಮೌಲ್ಯ.ಸೇರಿಸು_ಬಿಟ್ಟುಗಳು(ತುಂಡು_2, ಸರಿಸು_2);
    }
    ಸರಿಸು_2 += 7;
  } ಆಗ (Byte & 0x80);

  // Sign extend Value to at least 64 bits to match getSLEB128.
  ಆದರೆ (ಸರಿಸು_2 < 64)
    ಮೌಲ್ಯ = ಮೌಲ್ಯ.sext(64);

  *ಸ್ಥಾನಾಂತರ_Ptr = ಸ್ಥಾನಾಂತರ;
  ಹಿಂತಿರುಗಿ APSInt(std::move(ಮೌಲ್ಯ), /*isUnsigned=*/ಸುಳ್ಳು);
}

ಶೂನ್ಯ ದತ್ತಾಂಶ_Extractor::skip(Cursor &C, uint64_t ಉದ್ದಳತೆ) ಸ್ಥಿರಾಂಕ {
  Error_As_ಹೊರಹೋಗು_Parameter Err_As_ಹೊರಹೋಗು(C.Err);
  ಆದರೆ (isError(&C.Err))
    ಹಿಂತಿರುಗಿ;

  ಆದರೆ (prepareRead(C.ಸ್ಥಾನಾಂತರ, ಉದ್ದಳತೆ, &C.Err))
    C.ಸ್ಥಾನಾಂತರ += ಉದ್ದಳತೆ;
}
