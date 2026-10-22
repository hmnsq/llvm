//===-- DataExtractor.cpp -------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/DataExtractor.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Errc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ErrorHandling.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/LEB128.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/SwapByteOrder.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਤਾਰਕਿਕ ਅੰਕੜੇ_Extractor::prepareRead(uint64_t ਵਿੱਥ, uint64_t ਮਾਪ,
                                Error *E) ਅਚਲ {
  ਜੇ (is_Valid_ਵਿੱਥ_For_ਅੰਕੜੇ_ਦਾ_ਮਾਪ(ਵਿੱਥ, ਮਾਪ))
    ਵਾਪਸ ਸੱਚ;
  ਜੇ (E) {
    ਜੇ (ਵਿੱਥ <= ਅੰਕੜੇ_2.ਮਾਪ_2())
      *E = createStringError(
          errc::illegal_byte_sequence,
          "unexpected end of data at offset 0x%zx while reading [0x%" PRIx64
          ", 0x%" PRIx64 ")",
          ਅੰਕੜੇ_2.ਮਾਪ_2(), ਵਿੱਥ, ਵਿੱਥ + ਮਾਪ);
    ਨਹੀਂਤਾਂ
      *E = createStringError(errc::invalid_ਦਲੀਲ,
                             "offset 0x%" PRIx64
                             " is beyond the end of data at 0x%zx",
                             ਵਿੱਥ, ਅੰਕੜੇ_2.ਮਾਪ_2());
  }
  ਵਾਪਸ ਝੂਠ;
}

ਸਥਿਰ ਤਾਰਕਿਕ isError(Error *E) { ਵਾਪਸ E && *E; }

ਸਾਂਚਾ <ਕਿਸਮ_ਨਾਂ T>
T ਅੰਕੜੇ_Extractor::getU(uint64_t *ਵਿੱਥ_ptr, Error *Err) ਅਚਲ {
  Error_As_ਬਾਹਰਲਾ_Parameter Err_As_ਬਾਹਰਲਾ(Err);
  T val = 0;
  ਜੇ (isError(Err))
    ਵਾਪਸ val;

  uint64_t ਵਿੱਥ_2 = *ਵਿੱਥ_ptr;
  ਜੇ (!prepareRead(ਵਿੱਥ_2, ਆਕਾਰ(T), Err))
    ਵਾਪਸ val;
  std::ਸਮ੍ਰਿਤੀ_ਨਕਲ(&val, &ਅੰਕੜੇ_2.ਅੰਕੜੇ()[ਵਿੱਥ_2], ਆਕਾਰ(val));
  ਜੇ (sys::IsLittleEndianHost != IsLittleEndian)
    sys::ਅਦਲਾ_ਬਦਲੀ_Byte_ਕ੍ਰਮ(val);

  // Advance the offset
  *ਵਿੱਥ_ptr += ਆਕਾਰ(val);
  ਵਾਪਸ val;
}

ਸਾਂਚਾ <ਕਿਸਮ_ਨਾਂ T>
T *ਅੰਕੜੇ_Extractor::getUs(uint64_t *ਵਿੱਥ_ptr, T *dst, uint32_t ਗਿਣਤੀ_2,
                        Error *Err) ਅਚਲ {
  Error_As_ਬਾਹਰਲਾ_Parameter Err_As_ਬਾਹਰਲਾ(Err);
  ਜੇ (isError(Err))
    ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;

  uint64_t ਵਿੱਥ_2 = *ਵਿੱਥ_ptr;

  ਜੇ (!prepareRead(ਵਿੱਥ_2, ਆਕਾਰ(*dst) * ਗਿਣਤੀ_2, Err))
    ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
  ਲਈ (T *ਮੁੱਲ_ptr = dst, *ਸਿਰਾ = dst + ਗਿਣਤੀ_2; ਮੁੱਲ_ptr != ਸਿਰਾ;
       ++ਮੁੱਲ_ptr, ਵਿੱਥ_2 += ਆਕਾਰ(*dst))
    *ਮੁੱਲ_ptr = getU<T>(ਵਿੱਥ_ptr, Err);
  // Advance the offset
  *ਵਿੱਥ_ptr = ਵਿੱਥ_2;
  // Return a non-NULL pointer to the converted data as an indicator of
  // success
  ਵਾਪਸ dst;
}

uint8_t ਅੰਕੜੇ_Extractor::getU8(uint64_t *ਵਿੱਥ_ptr, llvm::Error *Err) ਅਚਲ {
  ਵਾਪਸ getU<uint8_t>(ਵਿੱਥ_ptr, Err);
}

uint8_t *ਅੰਕੜੇ_Extractor::getU8(uint64_t *ਵਿੱਥ_ptr, uint8_t *dst,
                              uint32_t ਗਿਣਤੀ_2) ਅਚਲ {
  ਵਾਪਸ getUs<uint8_t>(ਵਿੱਥ_ptr, dst, ਗਿਣਤੀ_2, ਖਾਲੀ_ਸੂਚਕ);
}

uint8_t *ਅੰਕੜੇ_Extractor::getU8(Cursor &C, uint8_t *Dst, uint32_t ਗਿਣਤੀ_3) ਅਚਲ {
  ਵਾਪਸ getUs<uint8_t>(&C.ਵਿੱਥ, Dst, ਗਿਣਤੀ_3, &C.Err);
}

uint16_t ਅੰਕੜੇ_Extractor::getU16(uint64_t *ਵਿੱਥ_ptr, llvm::Error *Err) ਅਚਲ {
  ਵਾਪਸ getU<uint16_t>(ਵਿੱਥ_ptr, Err);
}

uint16_t *ਅੰਕੜੇ_Extractor::getU16(uint64_t *ਵਿੱਥ_ptr, uint16_t *dst,
                                uint32_t ਗਿਣਤੀ_2) ਅਚਲ {
  ਵਾਪਸ getUs<uint16_t>(ਵਿੱਥ_ptr, dst, ਗਿਣਤੀ_2, ਖਾਲੀ_ਸੂਚਕ);
}

uint32_t ਅੰਕੜੇ_Extractor::getU24(uint64_t *ਵਿੱਥ_Ptr, Error *Err) ਅਚਲ {
  uint24_t ExtractedVal = getU<uint24_t>(ਵਿੱਥ_Ptr, Err);
  // The 3 bytes are in the correct byte order for the host.
  ਵਾਪਸ ExtractedVal.getAsUint32(sys::IsLittleEndianHost);
}

uint32_t ਅੰਕੜੇ_Extractor::getU32(uint64_t *ਵਿੱਥ_ptr, llvm::Error *Err) ਅਚਲ {
  ਵਾਪਸ getU<uint32_t>(ਵਿੱਥ_ptr, Err);
}

uint32_t *ਅੰਕੜੇ_Extractor::getU32(uint64_t *ਵਿੱਥ_ptr, uint32_t *dst,
                                uint32_t ਗਿਣਤੀ_2) ਅਚਲ {
  ਵਾਪਸ getUs<uint32_t>(ਵਿੱਥ_ptr, dst, ਗਿਣਤੀ_2, ਖਾਲੀ_ਸੂਚਕ);
}

uint64_t ਅੰਕੜੇ_Extractor::getU64(uint64_t *ਵਿੱਥ_ptr, llvm::Error *Err) ਅਚਲ {
  ਵਾਪਸ getU<uint64_t>(ਵਿੱਥ_ptr, Err);
}

uint64_t *ਅੰਕੜੇ_Extractor::getU64(uint64_t *ਵਿੱਥ_ptr, uint64_t *dst,
                                uint32_t ਗਿਣਤੀ_2) ਅਚਲ {
  ਵਾਪਸ getUs<uint64_t>(ਵਿੱਥ_ptr, dst, ਗਿਣਤੀ_2, ਖਾਲੀ_ਸੂਚਕ);
}

uint64_t ਅੰਕੜੇ_Extractor::getUnsigned(uint64_t *ਵਿੱਥ_ptr, uint32_t byte_ਮਾਪ,
                                    llvm::Error *Err) ਅਚਲ {
  ਚੋਣ (byte_ਮਾਪ) {
  ਹਾਲਤ 1:
    ਵਾਪਸ getU8(ਵਿੱਥ_ptr, Err);
  ਹਾਲਤ 2:
    ਵਾਪਸ getU16(ਵਿੱਥ_ptr, Err);
  ਹਾਲਤ 4:
    ਵਾਪਸ getU32(ਵਿੱਥ_ptr, Err);
  ਹਾਲਤ 8:
    ਵਾਪਸ getU64(ਵਿੱਥ_ptr, Err);
  }

  // For any other byte size, read the bytes and swap/shift if necessary.
  Error_As_ਬਾਹਰਲਾ_Parameter Err_As_ਬਾਹਰਲਾ(Err);
  uint64_t val = 0;
  ਜੇ (isError(Err))
    ਵਾਪਸ val;
  uint64_t ਵਿੱਥ_2 = *ਵਿੱਥ_ptr;
  ਜੇ (!prepareRead(ਵਿੱਥ_2, byte_ਮਾਪ, Err))
    ਵਾਪਸ val;
  // Copy into the least significant bytes of val regardless of host
  // endianness.
  std::ਸਮ੍ਰਿਤੀ_ਨਕਲ(reinterpret_cast<ਅੱਖਰ *>(&val) +
                  (sys::IsLittleEndianHost ? 0 : 8 - byte_ਮਾਪ),
              &ਅੰਕੜੇ_2.ਅੰਕੜੇ()[ਵਿੱਥ_2], byte_ਮਾਪ);
  // Swap the least significant bytes of val if endianness doesn't match.
  ਜੇ (sys::IsLittleEndianHost != IsLittleEndian)
    val = sys::get_Swapped_ਬਾਈਟਾਂ(val) >> (8 * (8 - byte_ਮਾਪ));

  *ਵਿੱਥ_ptr += byte_ਮਾਪ;
  ਵਾਪਸ val;
}

int64_t
ਅੰਕੜੇ_Extractor::getSigned(uint64_t *ਵਿੱਥ_ptr, uint32_t byte_ਮਾਪ) ਅਚਲ {
  ਚੋਣ (byte_ਮਾਪ) {
  ਹਾਲਤ 1:
    ਵਾਪਸ getS8(ਵਿੱਥ_ptr);
  ਹਾਲਤ 2:
    ਵਾਪਸ getS16(ਵਿੱਥ_ptr);
  ਹਾਲਤ 4:
    ਵਾਪਸ getS32(ਵਿੱਥ_ptr);
  ਹਾਲਤ 8:
    ਵਾਪਸ getS64(ਵਿੱਥ_ptr);
  }
  llvm_unreachable("getSigned unhandled case!");
}

StringRef ਅੰਕੜੇ_Extractor::getCStrRef(uint64_t *ਵਿੱਥ_Ptr, Error *Err) ਅਚਲ {
  Error_As_ਬਾਹਰਲਾ_Parameter Err_As_ਬਾਹਰਲਾ(Err);
  ਜੇ (isError(Err))
    ਵਾਪਸ StringRef();

  uint64_t ਸ਼ੁਰੂ = *ਵਿੱਥ_Ptr;
  StringRef::ਮਾਪ_type Pos = ਅੰਕੜੇ_2.ਲੱਭੋ('\0', ਸ਼ੁਰੂ);
  ਜੇ (Pos != StringRef::npos) {
    *ਵਿੱਥ_Ptr = Pos + 1;
    ਵਾਪਸ StringRef(ਅੰਕੜੇ_2.ਅੰਕੜੇ() + ਸ਼ੁਰੂ, Pos - ਸ਼ੁਰੂ);
  }
  ਜੇ (Err)
    *Err = createStringError(errc::illegal_byte_sequence,
                             "no null terminated string at offset 0x%" PRIx64,
                             ਸ਼ੁਰੂ);
  ਵਾਪਸ StringRef();
}

StringRef ਅੰਕੜੇ_Extractor::get_Fixed_ਲੰਮਾਈ_String(uint64_t *ਵਿੱਥ_Ptr,
                                              uint64_t ਲੰਮਾਈ,
                                              StringRef TrimChars) ਅਚਲ {
  StringRef ਬਾਈਟਾਂ_2(get_ਬਾਈਟਾਂ(ਵਿੱਥ_Ptr, ਲੰਮਾਈ));
  ਵਾਪਸ ਬਾਈਟਾਂ_2.trim(TrimChars);
}

StringRef ਅੰਕੜੇ_Extractor::get_ਬਾਈਟਾਂ(uint64_t *ਵਿੱਥ_Ptr, uint64_t ਲੰਮਾਈ,
                                  Error *Err) ਅਚਲ {
  Error_As_ਬਾਹਰਲਾ_Parameter Err_As_ਬਾਹਰਲਾ(Err);
  ਜੇ (isError(Err))
    ਵਾਪਸ StringRef();

  ਜੇ (!prepareRead(*ਵਿੱਥ_Ptr, ਲੰਮਾਈ, Err))
    ਵਾਪਸ StringRef();

  StringRef ਨਤੀਜਾ = ਅੰਕੜੇ_2.substr(*ਵਿੱਥ_Ptr, ਲੰਮਾਈ);
  *ਵਿੱਥ_Ptr += ਲੰਮਾਈ;
  ਵਾਪਸ ਨਤੀਜਾ;
}

ਸਾਂਚਾ <ਕਿਸਮ_ਨਾਂ T>
ਸਥਿਰ T getLEB128(StringRef ਅੰਕੜੇ_2, uint64_t *ਵਿੱਥ_Ptr, Error *Err,
                   T (&Decoder)(ਅਚਲ uint8_t *p, ਬਿਨਾਨਿਸ਼ਾਨੀ *n,
                                ਅਚਲ uint8_t *ਸਿਰਾ, ਅਚਲ ਅੱਖਰ **error)) {
  ArrayRef<uint8_t> ਬਾਈਟਾਂ_2 = arrayRefFromStringRef(ਅੰਕੜੇ_2);
  ਪ੍ਰਤਿਜ੍ਞਾ(*ਵਿੱਥ_Ptr <= ਬਾਈਟਾਂ_2.ਮਾਪ_2());
  Error_As_ਬਾਹਰਲਾ_Parameter Err_As_ਬਾਹਰਲਾ(Err);
  ਜੇ (isError(Err))
    ਵਾਪਸ T();

  ਅਚਲ ਅੱਖਰ *error = ਖਾਲੀ_ਸੂਚਕ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਾਈਟਾਂ_read;
  T ਨਤੀਜਾ_2 =
      Decoder(ਬਾਈਟਾਂ_2.ਅੰਕੜੇ() + *ਵਿੱਥ_Ptr, &ਬਾਈਟਾਂ_read, ਬਾਈਟਾਂ_2.ਸਿਰਾ(), &error);
  ਜੇ (error) {
    ਜੇ (Err)
      *Err = createStringError(errc::illegal_byte_sequence,
                               "unable to decode LEB128 at offset 0x%8.8" PRIx64
                               ": %s",
                               *ਵਿੱਥ_Ptr, error);
    ਵਾਪਸ T();
  }
  *ਵਿੱਥ_Ptr += ਬਾਈਟਾਂ_read;
  ਵਾਪਸ ਨਤੀਜਾ_2;
}

uint64_t ਅੰਕੜੇ_Extractor::getULEB128(uint64_t *ਵਿੱਥ_ptr, Error *Err) ਅਚਲ {
  ਵਾਪਸ getLEB128(ਅੰਕੜੇ_2, ਵਿੱਥ_ptr, Err, decodeULEB128);
}

int64_t ਅੰਕੜੇ_Extractor::getSLEB128(uint64_t *ਵਿੱਥ_ptr, Error *Err) ਅਚਲ {
  ਵਾਪਸ getLEB128(ਅੰਕੜੇ_2, ਵਿੱਥ_ptr, Err, decodeSLEB128);
}

APSInt ਅੰਕੜੇ_Extractor::getSLEB128APSInt(uint64_t *ਵਿੱਥ_Ptr, Error *Err) ਅਚਲ {
  Error_As_ਬਾਹਰਲਾ_Parameter Err_As_ਬਾਹਰਲਾ(Err);
  ਜੇ (isError(Err))
    ਵਾਪਸ APSInt();

  uint64_t ਵਿੱਥ = *ਵਿੱਥ_Ptr;
  APInt ਮੁੱਲ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਖਿਸਕਾਓ_2 = 0;
  uint8_t Byte;
  ਕਰੋ {
    ਜੇ (!prepareRead(ਵਿੱਥ, 1, Err))
      ਵਾਪਸ APSInt();
    Byte = ਅੰਕੜੇ_2[ਵਿੱਥ++];
    APInt ਟੁਕੜਾ_2(7, Byte & 0x7f);
    ਜੇ (ਖਿਸਕਾਓ_2 == 0)
      ਮੁੱਲ = ਟੁਕੜਾ_2;
    ਨਹੀਂਤਾਂ {
      ਮੁੱਲ = ਮੁੱਲ.zext(ਖਿਸਕਾਓ_2 + 7);
      ਮੁੱਲ.ਪਾਓ_ਬਿੱਟ(ਟੁਕੜਾ_2, ਖਿਸਕਾਓ_2);
    }
    ਖਿਸਕਾਓ_2 += 7;
  } ਜਦੋਂ (Byte & 0x80);

  // Sign extend Value to at least 64 bits to match getSLEB128.
  ਜੇ (ਖਿਸਕਾਓ_2 < 64)
    ਮੁੱਲ = ਮੁੱਲ.sext(64);

  *ਵਿੱਥ_Ptr = ਵਿੱਥ;
  ਵਾਪਸ APSInt(std::move(ਮੁੱਲ), /*isUnsigned=*/ਝੂਠ);
}

ਖਾਲੀ ਅੰਕੜੇ_Extractor::skip(Cursor &C, uint64_t ਲੰਮਾਈ) ਅਚਲ {
  Error_As_ਬਾਹਰਲਾ_Parameter Err_As_ਬਾਹਰਲਾ(C.Err);
  ਜੇ (isError(&C.Err))
    ਵਾਪਸ;

  ਜੇ (prepareRead(C.ਵਿੱਥ, ਲੰਮਾਈ, &C.Err))
    C.ਵਿੱਥ += ਲੰਮਾਈ;
}
