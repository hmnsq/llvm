//===-- DataExtractor.cpp -------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/DataExtractor.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/Support/Errc.h"
#شامل "llvm/Support/ErrorHandling.h"
#شامل "llvm/Support/LEB128.h"
#شامل "llvm/Support/SwapByteOrder.h"

استعمال نامخلا llvm;

منطقی ڈیٹا_Extractor::prepareRead(uint64_t انحراف, uint64_t سائز,
                                Error *E) ثابت {
  اگر (is_Valid_انحراف_For_ڈیٹا_کا_سائز(انحراف, سائز))
    واپس سچ;
  اگر (E) {
    اگر (انحراف <= ڈیٹا_2.سائز_2())
      *E = createStringError(
          errc::illegal_byte_sequence,
          "unexpected end of data at offset 0x%zx while reading [0x%" PRIx64
          ", 0x%" PRIx64 ")",
          ڈیٹا_2.سائز_2(), انحراف, انحراف + سائز);
    ورنہ
      *E = createStringError(errc::invalid_دلیل,
                             "offset 0x%" PRIx64
                             " is beyond the end of data at 0x%zx",
                             انحراف, ڈیٹا_2.سائز_2());
  }
  واپس جھوٹ;
}

ساکن منطقی isError(Error *E) { واپس E && *E; }

سانچہ <قسم_نام T>
T ڈیٹا_Extractor::getU(uint64_t *انحراف_ptr, Error *Err) ثابت {
  Error_As_برآمد_Parameter Err_As_برآمد(Err);
  T val = 0;
  اگر (isError(Err))
    واپس val;

  uint64_t انحراف_2 = *انحراف_ptr;
  اگر (!prepareRead(انحراف_2, حجم(T), Err))
    واپس val;
  std::حافظہ_نقل(&val, &ڈیٹا_2.ڈیٹا()[انحراف_2], حجم(val));
  اگر (sys::IsLittleEndianHost != IsLittleEndian)
    sys::تبادلہ_Byte_ترتیب(val);

  // Advance the offset
  *انحراف_ptr += حجم(val);
  واپس val;
}

سانچہ <قسم_نام T>
T *ڈیٹا_Extractor::getUs(uint64_t *انحراف_ptr, T *dst, uint32_t گنتی,
                        Error *Err) ثابت {
  Error_As_برآمد_Parameter Err_As_برآمد(Err);
  اگر (isError(Err))
    واپس خالی_اشاریہ;

  uint64_t انحراف_2 = *انحراف_ptr;

  اگر (!prepareRead(انحراف_2, حجم(*dst) * گنتی, Err))
    واپس خالی_اشاریہ;
  برائے (T *قدر_ptr = dst, *انتہا = dst + گنتی; قدر_ptr != انتہا;
       ++قدر_ptr, انحراف_2 += حجم(*dst))
    *قدر_ptr = getU<T>(انحراف_ptr, Err);
  // Advance the offset
  *انحراف_ptr = انحراف_2;
  // Return a non-NULL pointer to the converted data as an indicator of
  // success
  واپس dst;
}

uint8_t ڈیٹا_Extractor::getU8(uint64_t *انحراف_ptr, llvm::Error *Err) ثابت {
  واپس getU<uint8_t>(انحراف_ptr, Err);
}

uint8_t *ڈیٹا_Extractor::getU8(uint64_t *انحراف_ptr, uint8_t *dst,
                              uint32_t گنتی) ثابت {
  واپس getUs<uint8_t>(انحراف_ptr, dst, گنتی, خالی_اشاریہ);
}

uint8_t *ڈیٹا_Extractor::getU8(Cursor &C, uint8_t *Dst, uint32_t گنتی_2) ثابت {
  واپس getUs<uint8_t>(&C.انحراف, Dst, گنتی_2, &C.Err);
}

uint16_t ڈیٹا_Extractor::getU16(uint64_t *انحراف_ptr, llvm::Error *Err) ثابت {
  واپس getU<uint16_t>(انحراف_ptr, Err);
}

uint16_t *ڈیٹا_Extractor::getU16(uint64_t *انحراف_ptr, uint16_t *dst,
                                uint32_t گنتی) ثابت {
  واپس getUs<uint16_t>(انحراف_ptr, dst, گنتی, خالی_اشاریہ);
}

uint32_t ڈیٹا_Extractor::getU24(uint64_t *انحراف_Ptr, Error *Err) ثابت {
  uint24_t ExtractedVal = getU<uint24_t>(انحراف_Ptr, Err);
  // The 3 bytes are in the correct byte order for the host.
  واپس ExtractedVal.getAsUint32(sys::IsLittleEndianHost);
}

uint32_t ڈیٹا_Extractor::getU32(uint64_t *انحراف_ptr, llvm::Error *Err) ثابت {
  واپس getU<uint32_t>(انحراف_ptr, Err);
}

uint32_t *ڈیٹا_Extractor::getU32(uint64_t *انحراف_ptr, uint32_t *dst,
                                uint32_t گنتی) ثابت {
  واپس getUs<uint32_t>(انحراف_ptr, dst, گنتی, خالی_اشاریہ);
}

uint64_t ڈیٹا_Extractor::getU64(uint64_t *انحراف_ptr, llvm::Error *Err) ثابت {
  واپس getU<uint64_t>(انحراف_ptr, Err);
}

uint64_t *ڈیٹا_Extractor::getU64(uint64_t *انحراف_ptr, uint64_t *dst,
                                uint32_t گنتی) ثابت {
  واپس getUs<uint64_t>(انحراف_ptr, dst, گنتی, خالی_اشاریہ);
}

uint64_t ڈیٹا_Extractor::getUnsigned(uint64_t *انحراف_ptr, uint32_t byte_سائز,
                                    llvm::Error *Err) ثابت {
  انتخاب (byte_سائز) {
  حالت 1:
    واپس getU8(انحراف_ptr, Err);
  حالت 2:
    واپس getU16(انحراف_ptr, Err);
  حالت 4:
    واپس getU32(انحراف_ptr, Err);
  حالت 8:
    واپس getU64(انحراف_ptr, Err);
  }

  // For any other byte size, read the bytes and swap/shift if necessary.
  Error_As_برآمد_Parameter Err_As_برآمد(Err);
  uint64_t val = 0;
  اگر (isError(Err))
    واپس val;
  uint64_t انحراف_2 = *انحراف_ptr;
  اگر (!prepareRead(انحراف_2, byte_سائز, Err))
    واپس val;
  // Copy into the least significant bytes of val regardless of host
  // endianness.
  std::حافظہ_نقل(reinterpret_cast<حرف *>(&val) +
                  (sys::IsLittleEndianHost ? 0 : 8 - byte_سائز),
              &ڈیٹا_2.ڈیٹا()[انحراف_2], byte_سائز);
  // Swap the least significant bytes of val if endianness doesn't match.
  اگر (sys::IsLittleEndianHost != IsLittleEndian)
    val = sys::get_Swapped_بائٹیں(val) >> (8 * (8 - byte_سائز));

  *انحراف_ptr += byte_سائز;
  واپس val;
}

int64_t
ڈیٹا_Extractor::getSigned(uint64_t *انحراف_ptr, uint32_t byte_سائز) ثابت {
  انتخاب (byte_سائز) {
  حالت 1:
    واپس getS8(انحراف_ptr);
  حالت 2:
    واپس getS16(انحراف_ptr);
  حالت 4:
    واپس getS32(انحراف_ptr);
  حالت 8:
    واپس getS64(انحراف_ptr);
  }
  llvm_unreachable("getSigned unhandled case!");
}

StringRef ڈیٹا_Extractor::getCStrRef(uint64_t *انحراف_Ptr, Error *Err) ثابت {
  Error_As_برآمد_Parameter Err_As_برآمد(Err);
  اگر (isError(Err))
    واپس StringRef();

  uint64_t آغاز = *انحراف_Ptr;
  StringRef::سائز_type Pos = ڈیٹا_2.ڈھونڈو('\0', آغاز);
  اگر (Pos != StringRef::npos) {
    *انحراف_Ptr = Pos + 1;
    واپس StringRef(ڈیٹا_2.ڈیٹا() + آغاز, Pos - آغاز);
  }
  اگر (Err)
    *Err = createStringError(errc::illegal_byte_sequence,
                             "no null terminated string at offset 0x%" PRIx64,
                             آغاز);
  واپس StringRef();
}

StringRef ڈیٹا_Extractor::get_Fixed_لمبائی_String(uint64_t *انحراف_Ptr,
                                              uint64_t لمبائی,
                                              StringRef TrimChars) ثابت {
  StringRef بائٹیں_2(get_بائٹیں(انحراف_Ptr, لمبائی));
  واپس بائٹیں_2.trim(TrimChars);
}

StringRef ڈیٹا_Extractor::get_بائٹیں(uint64_t *انحراف_Ptr, uint64_t لمبائی,
                                  Error *Err) ثابت {
  Error_As_برآمد_Parameter Err_As_برآمد(Err);
  اگر (isError(Err))
    واپس StringRef();

  اگر (!prepareRead(*انحراف_Ptr, لمبائی, Err))
    واپس StringRef();

  StringRef نتیجہ = ڈیٹا_2.substr(*انحراف_Ptr, لمبائی);
  *انحراف_Ptr += لمبائی;
  واپس نتیجہ;
}

سانچہ <قسم_نام T>
ساکن T getLEB128(StringRef ڈیٹا_2, uint64_t *انحراف_Ptr, Error *Err,
                   T (&Decoder)(ثابت uint8_t *p, غیرعلامتی *n,
                                ثابت uint8_t *انتہا, ثابت حرف **error)) {
  ArrayRef<uint8_t> بائٹیں_2 = arrayRefFromStringRef(ڈیٹا_2);
  أكد(*انحراف_Ptr <= بائٹیں_2.سائز_2());
  Error_As_برآمد_Parameter Err_As_برآمد(Err);
  اگر (isError(Err))
    واپس T();

  ثابت حرف *error = خالی_اشاریہ;
  غیرعلامتی بائٹیں_read;
  T نتیجہ_2 =
      Decoder(بائٹیں_2.ڈیٹا() + *انحراف_Ptr, &بائٹیں_read, بائٹیں_2.انتہا(), &error);
  اگر (error) {
    اگر (Err)
      *Err = createStringError(errc::illegal_byte_sequence,
                               "unable to decode LEB128 at offset 0x%8.8" PRIx64
                               ": %s",
                               *انحراف_Ptr, error);
    واپس T();
  }
  *انحراف_Ptr += بائٹیں_read;
  واپس نتیجہ_2;
}

uint64_t ڈیٹا_Extractor::getULEB128(uint64_t *انحراف_ptr, Error *Err) ثابت {
  واپس getLEB128(ڈیٹا_2, انحراف_ptr, Err, decodeULEB128);
}

int64_t ڈیٹا_Extractor::getSLEB128(uint64_t *انحراف_ptr, Error *Err) ثابت {
  واپس getLEB128(ڈیٹا_2, انحراف_ptr, Err, decodeSLEB128);
}

APSInt ڈیٹا_Extractor::getSLEB128APSInt(uint64_t *انحراف_Ptr, Error *Err) ثابت {
  Error_As_برآمد_Parameter Err_As_برآمد(Err);
  اگر (isError(Err))
    واپس APSInt();

  uint64_t انحراف = *انحراف_Ptr;
  APInt قدر;
  غیرعلامتی سرکاؤ_2 = 0;
  uint8_t Byte;
  کرو {
    اگر (!prepareRead(انحراف, 1, Err))
      واپس APSInt();
    Byte = ڈیٹا_2[انحراف++];
    APInt قاش_2(7, Byte & 0x7f);
    اگر (سرکاؤ_2 == 0)
      قدر = قاش_2;
    ورنہ {
      قدر = قدر.zext(سرکاؤ_2 + 7);
      قدر.داخل_کرو_بٹ(قاش_2, سرکاؤ_2);
    }
    سرکاؤ_2 += 7;
  } جب (Byte & 0x80);

  // Sign extend Value to at least 64 bits to match getSLEB128.
  اگر (سرکاؤ_2 < 64)
    قدر = قدر.sext(64);

  *انحراف_Ptr = انحراف;
  واپس APSInt(std::move(قدر), /*isUnsigned=*/جھوٹ);
}

خالی ڈیٹا_Extractor::skip(Cursor &C, uint64_t لمبائی) ثابت {
  Error_As_برآمد_Parameter Err_As_برآمد(C.Err);
  اگر (isError(&C.Err))
    واپس;

  اگر (prepareRead(C.انحراف, لمبائی, &C.Err))
    C.انحراف += لمبائی;
}
