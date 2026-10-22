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

ورتوں نطاق llvm;

منطقي انکڑے_Extractor::prepareRead(uint64_t وتھ, uint64_t ناپ,
                                Error *E) ثابت {
  إذا (is_Valid_وتھ_For_انکڑے_دا_ناپ(وتھ, ناپ))
    أرجع سچ;
  إذا (E) {
    إذا (وتھ <= انکڑے_2.ناپ_2())
      *E = createStringError(
          errc::illegal_byte_sequence,
          "unexpected end of data at offset 0x%zx while reading [0x%" PRIx64
          ", 0x%" PRIx64 ")",
          انکڑے_2.ناپ_2(), وتھ, وتھ + ناپ);
    وإلا
      *E = createStringError(errc::invalid_دلیل,
                             "offset 0x%" PRIx64
                             " is beyond the end of data at 0x%zx",
                             وتھ, انکڑے_2.ناپ_2());
  }
  أرجع خطأ;
}

ساكن منطقي isError(Error *E) { أرجع E && *E; }

سانچا <قسم_ناں T>
T انکڑے_Extractor::getU(uint64_t *وتھ_ptr, Error *Err) ثابت {
  Error_As_باہرلا_Parameter Err_As_باہرلا(Err);
  T val = 0;
  إذا (isError(Err))
    أرجع val;

  uint64_t وتھ_2 = *وتھ_ptr;
  إذا (!prepareRead(وتھ_2, حجم(T), Err))
    أرجع val;
  std::حافظہ_نقل(&val, &انکڑے_2.انکڑے()[وتھ_2], حجم(val));
  إذا (sys::IsLittleEndianHost != IsLittleEndian)
    sys::وٹاندرا_Byte_ترتیب(val);

  // Advance the offset
  *وتھ_ptr += حجم(val);
  أرجع val;
}

سانچا <قسم_ناں T>
T *انکڑے_Extractor::getUs(uint64_t *وتھ_ptr, T *dst, uint32_t گنتی,
                        Error *Err) ثابت {
  Error_As_باہرلا_Parameter Err_As_باہرلا(Err);
  إذا (isError(Err))
    أرجع خالی_سوچک;

  uint64_t وتھ_2 = *وتھ_ptr;

  إذا (!prepareRead(وتھ_2, حجم(*dst) * گنتی, Err))
    أرجع خالی_سوچک;
  لكل (T *قدر_ptr = dst, *سرا = dst + گنتی; قدر_ptr != سرا;
       ++قدر_ptr, وتھ_2 += حجم(*dst))
    *قدر_ptr = getU<T>(وتھ_ptr, Err);
  // Advance the offset
  *وتھ_ptr = وتھ_2;
  // Return a non-NULL pointer to the converted data as an indicator of
  // success
  أرجع dst;
}

uint8_t انکڑے_Extractor::getU8(uint64_t *وتھ_ptr, llvm::Error *Err) ثابت {
  أرجع getU<uint8_t>(وتھ_ptr, Err);
}

uint8_t *انکڑے_Extractor::getU8(uint64_t *وتھ_ptr, uint8_t *dst,
                              uint32_t گنتی) ثابت {
  أرجع getUs<uint8_t>(وتھ_ptr, dst, گنتی, خالی_سوچک);
}

uint8_t *انکڑے_Extractor::getU8(Cursor &C, uint8_t *Dst, uint32_t گنتی_2) ثابت {
  أرجع getUs<uint8_t>(&C.وتھ, Dst, گنتی_2, &C.Err);
}

uint16_t انکڑے_Extractor::getU16(uint64_t *وتھ_ptr, llvm::Error *Err) ثابت {
  أرجع getU<uint16_t>(وتھ_ptr, Err);
}

uint16_t *انکڑے_Extractor::getU16(uint64_t *وتھ_ptr, uint16_t *dst,
                                uint32_t گنتی) ثابت {
  أرجع getUs<uint16_t>(وتھ_ptr, dst, گنتی, خالی_سوچک);
}

uint32_t انکڑے_Extractor::getU24(uint64_t *وتھ_Ptr, Error *Err) ثابت {
  uint24_t ExtractedVal = getU<uint24_t>(وتھ_Ptr, Err);
  // The 3 bytes are in the correct byte order for the host.
  أرجع ExtractedVal.getAsUint32(sys::IsLittleEndianHost);
}

uint32_t انکڑے_Extractor::getU32(uint64_t *وتھ_ptr, llvm::Error *Err) ثابت {
  أرجع getU<uint32_t>(وتھ_ptr, Err);
}

uint32_t *انکڑے_Extractor::getU32(uint64_t *وتھ_ptr, uint32_t *dst,
                                uint32_t گنتی) ثابت {
  أرجع getUs<uint32_t>(وتھ_ptr, dst, گنتی, خالی_سوچک);
}

uint64_t انکڑے_Extractor::getU64(uint64_t *وتھ_ptr, llvm::Error *Err) ثابت {
  أرجع getU<uint64_t>(وتھ_ptr, Err);
}

uint64_t *انکڑے_Extractor::getU64(uint64_t *وتھ_ptr, uint64_t *dst,
                                uint32_t گنتی) ثابت {
  أرجع getUs<uint64_t>(وتھ_ptr, dst, گنتی, خالی_سوچک);
}

uint64_t انکڑے_Extractor::getUnsigned(uint64_t *وتھ_ptr, uint32_t byte_ناپ,
                                    llvm::Error *Err) ثابت {
  بدل (byte_ناپ) {
  حالة 1:
    أرجع getU8(وتھ_ptr, Err);
  حالة 2:
    أرجع getU16(وتھ_ptr, Err);
  حالة 4:
    أرجع getU32(وتھ_ptr, Err);
  حالة 8:
    أرجع getU64(وتھ_ptr, Err);
  }

  // For any other byte size, read the bytes and swap/shift if necessary.
  Error_As_باہرلا_Parameter Err_As_باہرلا(Err);
  uint64_t val = 0;
  إذا (isError(Err))
    أرجع val;
  uint64_t وتھ_2 = *وتھ_ptr;
  إذا (!prepareRead(وتھ_2, byte_ناپ, Err))
    أرجع val;
  // Copy into the least significant bytes of val regardless of host
  // endianness.
  std::حافظہ_نقل(reinterpret_cast<حرف *>(&val) +
                  (sys::IsLittleEndianHost ? 0 : 8 - byte_ناپ),
              &انکڑے_2.انکڑے()[وتھ_2], byte_ناپ);
  // Swap the least significant bytes of val if endianness doesn't match.
  إذا (sys::IsLittleEndianHost != IsLittleEndian)
    val = sys::get_Swapped_بائٹاں(val) >> (8 * (8 - byte_ناپ));

  *وتھ_ptr += byte_ناپ;
  أرجع val;
}

int64_t
انکڑے_Extractor::getSigned(uint64_t *وتھ_ptr, uint32_t byte_ناپ) ثابت {
  بدل (byte_ناپ) {
  حالة 1:
    أرجع getS8(وتھ_ptr);
  حالة 2:
    أرجع getS16(وتھ_ptr);
  حالة 4:
    أرجع getS32(وتھ_ptr);
  حالة 8:
    أرجع getS64(وتھ_ptr);
  }
  llvm_unreachable("getSigned unhandled case!");
}

StringRef انکڑے_Extractor::getCStrRef(uint64_t *وتھ_Ptr, Error *Err) ثابت {
  Error_As_باہرلا_Parameter Err_As_باہرلا(Err);
  إذا (isError(Err))
    أرجع StringRef();

  uint64_t شروع = *وتھ_Ptr;
  StringRef::ناپ_type Pos = انکڑے_2.لبھو('\0', شروع);
  إذا (Pos != StringRef::npos) {
    *وتھ_Ptr = Pos + 1;
    أرجع StringRef(انکڑے_2.انکڑے() + شروع, Pos - شروع);
  }
  إذا (Err)
    *Err = createStringError(errc::illegal_byte_sequence,
                             "no null terminated string at offset 0x%" PRIx64,
                             شروع);
  أرجع StringRef();
}

StringRef انکڑے_Extractor::get_Fixed_لمائی_String(uint64_t *وتھ_Ptr,
                                              uint64_t لمائی,
                                              StringRef TrimChars) ثابت {
  StringRef بائٹاں_2(get_بائٹاں(وتھ_Ptr, لمائی));
  أرجع بائٹاں_2.trim(TrimChars);
}

StringRef انکڑے_Extractor::get_بائٹاں(uint64_t *وتھ_Ptr, uint64_t لمائی,
                                  Error *Err) ثابت {
  Error_As_باہرلا_Parameter Err_As_باہرلا(Err);
  إذا (isError(Err))
    أرجع StringRef();

  إذا (!prepareRead(*وتھ_Ptr, لمائی, Err))
    أرجع StringRef();

  StringRef نتیجہ = انکڑے_2.substr(*وتھ_Ptr, لمائی);
  *وتھ_Ptr += لمائی;
  أرجع نتیجہ;
}

سانچا <قسم_ناں T>
ساكن T getLEB128(StringRef انکڑے_2, uint64_t *وتھ_Ptr, Error *Err,
                   T (&Decoder)(ثابت uint8_t *p, بنانشانی *n,
                                ثابت uint8_t *سرا, ثابت حرف **error)) {
  ArrayRef<uint8_t> بائٹاں_2 = arrayRefFromStringRef(انکڑے_2);
  أكد(*وتھ_Ptr <= بائٹاں_2.ناپ_2());
  Error_As_باہرلا_Parameter Err_As_باہرلا(Err);
  إذا (isError(Err))
    أرجع T();

  ثابت حرف *error = خالی_سوچک;
  بنانشانی بائٹاں_read;
  T نتیجہ_2 =
      Decoder(بائٹاں_2.انکڑے() + *وتھ_Ptr, &بائٹاں_read, بائٹاں_2.سرا(), &error);
  إذا (error) {
    إذا (Err)
      *Err = createStringError(errc::illegal_byte_sequence,
                               "unable to decode LEB128 at offset 0x%8.8" PRIx64
                               ": %s",
                               *وتھ_Ptr, error);
    أرجع T();
  }
  *وتھ_Ptr += بائٹاں_read;
  أرجع نتیجہ_2;
}

uint64_t انکڑے_Extractor::getULEB128(uint64_t *وتھ_ptr, Error *Err) ثابت {
  أرجع getLEB128(انکڑے_2, وتھ_ptr, Err, decodeULEB128);
}

int64_t انکڑے_Extractor::getSLEB128(uint64_t *وتھ_ptr, Error *Err) ثابت {
  أرجع getLEB128(انکڑے_2, وتھ_ptr, Err, decodeSLEB128);
}

APSInt انکڑے_Extractor::getSLEB128APSInt(uint64_t *وتھ_Ptr, Error *Err) ثابت {
  Error_As_باہرلا_Parameter Err_As_باہرلا(Err);
  إذا (isError(Err))
    أرجع APSInt();

  uint64_t وتھ = *وتھ_Ptr;
  APInt قدر;
  بنانشانی کھسکاؤ_2 = 0;
  uint8_t Byte;
  افعل {
    إذا (!prepareRead(وتھ, 1, Err))
      أرجع APSInt();
    Byte = انکڑے_2[وتھ++];
    APInt ٹکڑا_2(7, Byte & 0x7f);
    إذا (کھسکاؤ_2 == 0)
      قدر = ٹکڑا_2;
    وإلا {
      قدر = قدر.zext(کھسکاؤ_2 + 7);
      قدر.داخل_کرو_بٹ(ٹکڑا_2, کھسکاؤ_2);
    }
    کھسکاؤ_2 += 7;
  } طالما (Byte & 0x80);

  // Sign extend Value to at least 64 bits to match getSLEB128.
  إذا (کھسکاؤ_2 < 64)
    قدر = قدر.sext(64);

  *وتھ_Ptr = وتھ;
  أرجع APSInt(std::move(قدر), /*isUnsigned=*/خطأ);
}

فراغ انکڑے_Extractor::skip(Cursor &C, uint64_t لمائی) ثابت {
  Error_As_باہرلا_Parameter Err_As_باہرلا(C.Err);
  إذا (isError(&C.Err))
    أرجع;

  إذا (prepareRead(C.وتھ, لمائی, &C.Err))
    C.وتھ += لمائی;
}
