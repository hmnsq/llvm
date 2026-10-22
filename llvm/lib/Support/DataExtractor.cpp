//===-- DataExtractor.cpp -------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/DataExtractor.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/Support/Errc.h"
#ضمن "llvm/Support/ErrorHandling.h"
#ضمن "llvm/Support/LEB128.h"
#ضمن "llvm/Support/SwapByteOrder.h"

استعمال نطاق llvm;

منطقي معلومات_Extractor::prepareRead(uint64_t وٿي, uint64_t سائيز,
                                Error *E) ثابت {
  إذا (is_Valid_وٿي_For_معلومات_جو_سائيز(وٿي, سائيز))
    أرجع سچ;
  إذا (E) {
    إذا (وٿي <= معلومات_2.سائيز_2())
      *E = createStringError(
          errc::illegal_byte_sequence,
          "unexpected end of data at offset 0x%zx while reading [0x%" PRIx64
          ", 0x%" PRIx64 ")",
          معلومات_2.سائيز_2(), وٿي, وٿي + سائيز);
    وإلا
      *E = createStringError(errc::invalid_دليل,
                             "offset 0x%" PRIx64
                             " is beyond the end of data at 0x%zx",
                             وٿي, معلومات_2.سائيز_2());
  }
  أرجع خطأ;
}

ساكن منطقي isError(Error *E) { أرجع E && *E; }

سانچو <قسم_نالو T>
T معلومات_Extractor::getU(uint64_t *وٿي_ptr, Error *Err) ثابت {
  Error_As_ٻاهريون_Parameter Err_As_ٻاهريون(Err);
  T val = 0;
  إذا (isError(Err))
    أرجع val;

  uint64_t وٿي_2 = *وٿي_ptr;
  إذا (!prepareRead(وٿي_2, حجم(T), Err))
    أرجع val;
  std::يادگيري_نقل(&val, &معلومات_2.معلومات()[وٿي_2], حجم(val));
  إذا (sys::IsLittleEndianHost != IsLittleEndian)
    sys::مٽا_سٽا_Byte_ترتيب(val);

  // Advance the offset
  *وٿي_ptr += حجم(val);
  أرجع val;
}

سانچو <قسم_نالو T>
T *معلومات_Extractor::getUs(uint64_t *وٿي_ptr, T *dst, uint32_t ڳڻپ,
                        Error *Err) ثابت {
  Error_As_ٻاهريون_Parameter Err_As_ٻاهريون(Err);
  إذا (isError(Err))
    أرجع خالي_اشارو;

  uint64_t وٿي_2 = *وٿي_ptr;

  إذا (!prepareRead(وٿي_2, حجم(*dst) * ڳڻپ, Err))
    أرجع خالي_اشارو;
  لكل (T *قدر_ptr = dst, *پڇاڙي = dst + ڳڻپ; قدر_ptr != پڇاڙي;
       ++قدر_ptr, وٿي_2 += حجم(*dst))
    *قدر_ptr = getU<T>(وٿي_ptr, Err);
  // Advance the offset
  *وٿي_ptr = وٿي_2;
  // Return a non-NULL pointer to the converted data as an indicator of
  // success
  أرجع dst;
}

uint8_t معلومات_Extractor::getU8(uint64_t *وٿي_ptr, llvm::Error *Err) ثابت {
  أرجع getU<uint8_t>(وٿي_ptr, Err);
}

uint8_t *معلومات_Extractor::getU8(uint64_t *وٿي_ptr, uint8_t *dst,
                              uint32_t ڳڻپ) ثابت {
  أرجع getUs<uint8_t>(وٿي_ptr, dst, ڳڻپ, خالي_اشارو);
}

uint8_t *معلومات_Extractor::getU8(Cursor &C, uint8_t *Dst, uint32_t ڳڻپ_2) ثابت {
  أرجع getUs<uint8_t>(&C.وٿي, Dst, ڳڻپ_2, &C.Err);
}

uint16_t معلومات_Extractor::getU16(uint64_t *وٿي_ptr, llvm::Error *Err) ثابت {
  أرجع getU<uint16_t>(وٿي_ptr, Err);
}

uint16_t *معلومات_Extractor::getU16(uint64_t *وٿي_ptr, uint16_t *dst,
                                uint32_t ڳڻپ) ثابت {
  أرجع getUs<uint16_t>(وٿي_ptr, dst, ڳڻپ, خالي_اشارو);
}

uint32_t معلومات_Extractor::getU24(uint64_t *وٿي_Ptr, Error *Err) ثابت {
  uint24_t ExtractedVal = getU<uint24_t>(وٿي_Ptr, Err);
  // The 3 bytes are in the correct byte order for the host.
  أرجع ExtractedVal.getAsUint32(sys::IsLittleEndianHost);
}

uint32_t معلومات_Extractor::getU32(uint64_t *وٿي_ptr, llvm::Error *Err) ثابت {
  أرجع getU<uint32_t>(وٿي_ptr, Err);
}

uint32_t *معلومات_Extractor::getU32(uint64_t *وٿي_ptr, uint32_t *dst,
                                uint32_t ڳڻپ) ثابت {
  أرجع getUs<uint32_t>(وٿي_ptr, dst, ڳڻپ, خالي_اشارو);
}

uint64_t معلومات_Extractor::getU64(uint64_t *وٿي_ptr, llvm::Error *Err) ثابت {
  أرجع getU<uint64_t>(وٿي_ptr, Err);
}

uint64_t *معلومات_Extractor::getU64(uint64_t *وٿي_ptr, uint64_t *dst,
                                uint32_t ڳڻپ) ثابت {
  أرجع getUs<uint64_t>(وٿي_ptr, dst, ڳڻپ, خالي_اشارو);
}

uint64_t معلومات_Extractor::getUnsigned(uint64_t *وٿي_ptr, uint32_t byte_سائيز,
                                    llvm::Error *Err) ثابت {
  بدل (byte_سائيز) {
  حالة 1:
    أرجع getU8(وٿي_ptr, Err);
  حالة 2:
    أرجع getU16(وٿي_ptr, Err);
  حالة 4:
    أرجع getU32(وٿي_ptr, Err);
  حالة 8:
    أرجع getU64(وٿي_ptr, Err);
  }

  // For any other byte size, read the bytes and swap/shift if necessary.
  Error_As_ٻاهريون_Parameter Err_As_ٻاهريون(Err);
  uint64_t val = 0;
  إذا (isError(Err))
    أرجع val;
  uint64_t وٿي_2 = *وٿي_ptr;
  إذا (!prepareRead(وٿي_2, byte_سائيز, Err))
    أرجع val;
  // Copy into the least significant bytes of val regardless of host
  // endianness.
  std::يادگيري_نقل(reinterpret_cast<حرف *>(&val) +
                  (sys::IsLittleEndianHost ? 0 : 8 - byte_سائيز),
              &معلومات_2.معلومات()[وٿي_2], byte_سائيز);
  // Swap the least significant bytes of val if endianness doesn't match.
  إذا (sys::IsLittleEndianHost != IsLittleEndian)
    val = sys::get_Swapped_بائيٽون(val) >> (8 * (8 - byte_سائيز));

  *وٿي_ptr += byte_سائيز;
  أرجع val;
}

int64_t
معلومات_Extractor::getSigned(uint64_t *وٿي_ptr, uint32_t byte_سائيز) ثابت {
  بدل (byte_سائيز) {
  حالة 1:
    أرجع getS8(وٿي_ptr);
  حالة 2:
    أرجع getS16(وٿي_ptr);
  حالة 4:
    أرجع getS32(وٿي_ptr);
  حالة 8:
    أرجع getS64(وٿي_ptr);
  }
  llvm_unreachable("getSigned unhandled case!");
}

StringRef معلومات_Extractor::getCStrRef(uint64_t *وٿي_Ptr, Error *Err) ثابت {
  Error_As_ٻاهريون_Parameter Err_As_ٻاهريون(Err);
  إذا (isError(Err))
    أرجع StringRef();

  uint64_t شروعات = *وٿي_Ptr;
  StringRef::سائيز_type Pos = معلومات_2.ڳوليو('\0', شروعات);
  إذا (Pos != StringRef::npos) {
    *وٿي_Ptr = Pos + 1;
    أرجع StringRef(معلومات_2.معلومات() + شروعات, Pos - شروعات);
  }
  إذا (Err)
    *Err = createStringError(errc::illegal_byte_sequence,
                             "no null terminated string at offset 0x%" PRIx64,
                             شروعات);
  أرجع StringRef();
}

StringRef معلومات_Extractor::get_Fixed_ڊيگهائي_String(uint64_t *وٿي_Ptr,
                                              uint64_t ڊيگهائي,
                                              StringRef TrimChars) ثابت {
  StringRef بائيٽون_2(get_بائيٽون(وٿي_Ptr, ڊيگهائي));
  أرجع بائيٽون_2.trim(TrimChars);
}

StringRef معلومات_Extractor::get_بائيٽون(uint64_t *وٿي_Ptr, uint64_t ڊيگهائي,
                                  Error *Err) ثابت {
  Error_As_ٻاهريون_Parameter Err_As_ٻاهريون(Err);
  إذا (isError(Err))
    أرجع StringRef();

  إذا (!prepareRead(*وٿي_Ptr, ڊيگهائي, Err))
    أرجع StringRef();

  StringRef نتيجو = معلومات_2.substr(*وٿي_Ptr, ڊيگهائي);
  *وٿي_Ptr += ڊيگهائي;
  أرجع نتيجو;
}

سانچو <قسم_نالو T>
ساكن T getLEB128(StringRef معلومات_2, uint64_t *وٿي_Ptr, Error *Err,
                   T (&Decoder)(ثابت uint8_t *p, بنانشان *n,
                                ثابت uint8_t *پڇاڙي, ثابت حرف **error)) {
  ArrayRef<uint8_t> بائيٽون_2 = arrayRefFromStringRef(معلومات_2);
  أكد(*وٿي_Ptr <= بائيٽون_2.سائيز_2());
  Error_As_ٻاهريون_Parameter Err_As_ٻاهريون(Err);
  إذا (isError(Err))
    أرجع T();

  ثابت حرف *error = خالي_اشارو;
  بنانشان بائيٽون_read;
  T نتيجو_2 =
      Decoder(بائيٽون_2.معلومات() + *وٿي_Ptr, &بائيٽون_read, بائيٽون_2.پڇاڙي(), &error);
  إذا (error) {
    إذا (Err)
      *Err = createStringError(errc::illegal_byte_sequence,
                               "unable to decode LEB128 at offset 0x%8.8" PRIx64
                               ": %s",
                               *وٿي_Ptr, error);
    أرجع T();
  }
  *وٿي_Ptr += بائيٽون_read;
  أرجع نتيجو_2;
}

uint64_t معلومات_Extractor::getULEB128(uint64_t *وٿي_ptr, Error *Err) ثابت {
  أرجع getLEB128(معلومات_2, وٿي_ptr, Err, decodeULEB128);
}

int64_t معلومات_Extractor::getSLEB128(uint64_t *وٿي_ptr, Error *Err) ثابت {
  أرجع getLEB128(معلومات_2, وٿي_ptr, Err, decodeSLEB128);
}

APSInt معلومات_Extractor::getSLEB128APSInt(uint64_t *وٿي_Ptr, Error *Err) ثابت {
  Error_As_ٻاهريون_Parameter Err_As_ٻاهريون(Err);
  إذا (isError(Err))
    أرجع APSInt();

  uint64_t وٿي = *وٿي_Ptr;
  APInt قدر;
  بنانشان سرڪايو_2 = 0;
  uint8_t Byte;
  افعل {
    إذا (!prepareRead(وٿي, 1, Err))
      أرجع APSInt();
    Byte = معلومات_2[وٿي++];
    APInt ٽڪرو_2(7, Byte & 0x7f);
    إذا (سرڪايو_2 == 0)
      قدر = ٽڪرو_2;
    وإلا {
      قدر = قدر.zext(سرڪايو_2 + 7);
      قدر.وجهو_بٽ(ٽڪرو_2, سرڪايو_2);
    }
    سرڪايو_2 += 7;
  } طالما (Byte & 0x80);

  // Sign extend Value to at least 64 bits to match getSLEB128.
  إذا (سرڪايو_2 < 64)
    قدر = قدر.sext(64);

  *وٿي_Ptr = وٿي;
  أرجع APSInt(std::move(قدر), /*isUnsigned=*/خطأ);
}

فراغ معلومات_Extractor::skip(Cursor &C, uint64_t ڊيگهائي) ثابت {
  Error_As_ٻاهريون_Parameter Err_As_ٻاهريون(C.Err);
  إذا (isError(&C.Err))
    أرجع;

  إذا (prepareRead(C.وٿي, ڊيگهائي, &C.Err))
    C.وٿي += ڊيگهائي;
}
