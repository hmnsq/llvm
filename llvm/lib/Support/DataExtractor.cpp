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

استفاده نطاق llvm;

منطقي داده_Extractor::prepareRead(uint64_t جابه_جایی, uint64_t اندازه,
                                Error *E) ثابت {
  إذا (is_Valid_جابه_جایی_For_داده_از_اندازه(جابه_جایی, اندازه))
    أرجع درست;
  إذا (E) {
    إذا (جابه_جایی <= داده_2.اندازه_2())
      *E = createStringError(
          errc::illegal_byte_sequence,
          "unexpected end of data at offset 0x%zx while reading [0x%" PRIx64
          ", 0x%" PRIx64 ")",
          داده_2.اندازه_2(), جابه_جایی, جابه_جایی + اندازه);
    وإلا
      *E = createStringError(errc::invalid_آرگومان,
                             "offset 0x%" PRIx64
                             " is beyond the end of data at 0x%zx",
                             جابه_جایی, داده_2.اندازه_2());
  }
  أرجع خطأ;
}

ساكن منطقي isError(Error *E) { أرجع E && *E; }

قالب <نام_نوع T>
T داده_Extractor::getU(uint64_t *جابه_جایی_ptr, Error *Err) ثابت {
  Error_As_بیرون_Parameter Err_As_بیرون(Err);
  T val = 0;
  إذا (isError(Err))
    أرجع val;

  uint64_t جابه_جایی_2 = *جابه_جایی_ptr;
  إذا (!prepareRead(جابه_جایی_2, حجم(T), Err))
    أرجع val;
  std::کپی_حافظه(&val, &داده_2.داده()[جابه_جایی_2], حجم(val));
  إذا (sys::IsLittleEndianHost != IsLittleEndian)
    sys::جابجا_Byte_ترتیب(val);

  // Advance the offset
  *جابه_جایی_ptr += حجم(val);
  أرجع val;
}

قالب <نام_نوع T>
T *داده_Extractor::getUs(uint64_t *جابه_جایی_ptr, T *dst, uint32_t شمار,
                        Error *Err) ثابت {
  Error_As_بیرون_Parameter Err_As_بیرون(Err);
  إذا (isError(Err))
    أرجع اشاره_تهی;

  uint64_t جابه_جایی_2 = *جابه_جایی_ptr;

  إذا (!prepareRead(جابه_جایی_2, حجم(*dst) * شمار, Err))
    أرجع اشاره_تهی;
  لكل (T *مقدار_ptr = dst, *انتها = dst + شمار; مقدار_ptr != انتها;
       ++مقدار_ptr, جابه_جایی_2 += حجم(*dst))
    *مقدار_ptr = getU<T>(جابه_جایی_ptr, Err);
  // Advance the offset
  *جابه_جایی_ptr = جابه_جایی_2;
  // Return a non-NULL pointer to the converted data as an indicator of
  // success
  أرجع dst;
}

uint8_t داده_Extractor::getU8(uint64_t *جابه_جایی_ptr, llvm::Error *Err) ثابت {
  أرجع getU<uint8_t>(جابه_جایی_ptr, Err);
}

uint8_t *داده_Extractor::getU8(uint64_t *جابه_جایی_ptr, uint8_t *dst,
                              uint32_t شمار) ثابت {
  أرجع getUs<uint8_t>(جابه_جایی_ptr, dst, شمار, اشاره_تهی);
}

uint8_t *داده_Extractor::getU8(Cursor &C, uint8_t *Dst, uint32_t شمار_2) ثابت {
  أرجع getUs<uint8_t>(&C.جابه_جایی, Dst, شمار_2, &C.Err);
}

uint16_t داده_Extractor::getU16(uint64_t *جابه_جایی_ptr, llvm::Error *Err) ثابت {
  أرجع getU<uint16_t>(جابه_جایی_ptr, Err);
}

uint16_t *داده_Extractor::getU16(uint64_t *جابه_جایی_ptr, uint16_t *dst,
                                uint32_t شمار) ثابت {
  أرجع getUs<uint16_t>(جابه_جایی_ptr, dst, شمار, اشاره_تهی);
}

uint32_t داده_Extractor::getU24(uint64_t *جابه_جایی_Ptr, Error *Err) ثابت {
  uint24_t ExtractedVal = getU<uint24_t>(جابه_جایی_Ptr, Err);
  // The 3 bytes are in the correct byte order for the host.
  أرجع ExtractedVal.getAsUint32(sys::IsLittleEndianHost);
}

uint32_t داده_Extractor::getU32(uint64_t *جابه_جایی_ptr, llvm::Error *Err) ثابت {
  أرجع getU<uint32_t>(جابه_جایی_ptr, Err);
}

uint32_t *داده_Extractor::getU32(uint64_t *جابه_جایی_ptr, uint32_t *dst,
                                uint32_t شمار) ثابت {
  أرجع getUs<uint32_t>(جابه_جایی_ptr, dst, شمار, اشاره_تهی);
}

uint64_t داده_Extractor::getU64(uint64_t *جابه_جایی_ptr, llvm::Error *Err) ثابت {
  أرجع getU<uint64_t>(جابه_جایی_ptr, Err);
}

uint64_t *داده_Extractor::getU64(uint64_t *جابه_جایی_ptr, uint64_t *dst,
                                uint32_t شمار) ثابت {
  أرجع getUs<uint64_t>(جابه_جایی_ptr, dst, شمار, اشاره_تهی);
}

uint64_t داده_Extractor::getUnsigned(uint64_t *جابه_جایی_ptr, uint32_t byte_اندازه,
                                    llvm::Error *Err) ثابت {
  بدل (byte_اندازه) {
  حالة 1:
    أرجع getU8(جابه_جایی_ptr, Err);
  حالة 2:
    أرجع getU16(جابه_جایی_ptr, Err);
  حالة 4:
    أرجع getU32(جابه_جایی_ptr, Err);
  حالة 8:
    أرجع getU64(جابه_جایی_ptr, Err);
  }

  // For any other byte size, read the bytes and swap/shift if necessary.
  Error_As_بیرون_Parameter Err_As_بیرون(Err);
  uint64_t val = 0;
  إذا (isError(Err))
    أرجع val;
  uint64_t جابه_جایی_2 = *جابه_جایی_ptr;
  إذا (!prepareRead(جابه_جایی_2, byte_اندازه, Err))
    أرجع val;
  // Copy into the least significant bytes of val regardless of host
  // endianness.
  std::کپی_حافظه(reinterpret_cast<حرف *>(&val) +
                  (sys::IsLittleEndianHost ? 0 : 8 - byte_اندازه),
              &داده_2.داده()[جابه_جایی_2], byte_اندازه);
  // Swap the least significant bytes of val if endianness doesn't match.
  إذا (sys::IsLittleEndianHost != IsLittleEndian)
    val = sys::get_Swapped_بایتها(val) >> (8 * (8 - byte_اندازه));

  *جابه_جایی_ptr += byte_اندازه;
  أرجع val;
}

int64_t
داده_Extractor::getSigned(uint64_t *جابه_جایی_ptr, uint32_t byte_اندازه) ثابت {
  بدل (byte_اندازه) {
  حالة 1:
    أرجع getS8(جابه_جایی_ptr);
  حالة 2:
    أرجع getS16(جابه_جایی_ptr);
  حالة 4:
    أرجع getS32(جابه_جایی_ptr);
  حالة 8:
    أرجع getS64(جابه_جایی_ptr);
  }
  llvm_unreachable("getSigned unhandled case!");
}

StringRef داده_Extractor::getCStrRef(uint64_t *جابه_جایی_Ptr, Error *Err) ثابت {
  Error_As_بیرون_Parameter Err_As_بیرون(Err);
  إذا (isError(Err))
    أرجع StringRef();

  uint64_t شروع = *جابه_جایی_Ptr;
  StringRef::اندازه_type Pos = داده_2.پیدا_کن('\0', شروع);
  إذا (Pos != StringRef::npos) {
    *جابه_جایی_Ptr = Pos + 1;
    أرجع StringRef(داده_2.داده() + شروع, Pos - شروع);
  }
  إذا (Err)
    *Err = createStringError(errc::illegal_byte_sequence,
                             "no null terminated string at offset 0x%" PRIx64,
                             شروع);
  أرجع StringRef();
}

StringRef داده_Extractor::get_Fixed_درازی_String(uint64_t *جابه_جایی_Ptr,
                                              uint64_t درازی,
                                              StringRef TrimChars) ثابت {
  StringRef بایتها_2(get_بایتها(جابه_جایی_Ptr, درازی));
  أرجع بایتها_2.trim(TrimChars);
}

StringRef داده_Extractor::get_بایتها(uint64_t *جابه_جایی_Ptr, uint64_t درازی,
                                  Error *Err) ثابت {
  Error_As_بیرون_Parameter Err_As_بیرون(Err);
  إذا (isError(Err))
    أرجع StringRef();

  إذا (!prepareRead(*جابه_جایی_Ptr, درازی, Err))
    أرجع StringRef();

  StringRef نتیجه = داده_2.substr(*جابه_جایی_Ptr, درازی);
  *جابه_جایی_Ptr += درازی;
  أرجع نتیجه;
}

قالب <نام_نوع T>
ساكن T getLEB128(StringRef داده_2, uint64_t *جابه_جایی_Ptr, Error *Err,
                   T (&Decoder)(ثابت uint8_t *p, بیعلامت *n,
                                ثابت uint8_t *انتها, ثابت حرف **error)) {
  ArrayRef<uint8_t> بایتها_2 = arrayRefFromStringRef(داده_2);
  أكد(*جابه_جایی_Ptr <= بایتها_2.اندازه_2());
  Error_As_بیرون_Parameter Err_As_بیرون(Err);
  إذا (isError(Err))
    أرجع T();

  ثابت حرف *error = اشاره_تهی;
  بیعلامت بایتها_read;
  T نتیجه_2 =
      Decoder(بایتها_2.داده() + *جابه_جایی_Ptr, &بایتها_read, بایتها_2.انتها(), &error);
  إذا (error) {
    إذا (Err)
      *Err = createStringError(errc::illegal_byte_sequence,
                               "unable to decode LEB128 at offset 0x%8.8" PRIx64
                               ": %s",
                               *جابه_جایی_Ptr, error);
    أرجع T();
  }
  *جابه_جایی_Ptr += بایتها_read;
  أرجع نتیجه_2;
}

uint64_t داده_Extractor::getULEB128(uint64_t *جابه_جایی_ptr, Error *Err) ثابت {
  أرجع getLEB128(داده_2, جابه_جایی_ptr, Err, decodeULEB128);
}

int64_t داده_Extractor::getSLEB128(uint64_t *جابه_جایی_ptr, Error *Err) ثابت {
  أرجع getLEB128(داده_2, جابه_جایی_ptr, Err, decodeSLEB128);
}

APSInt داده_Extractor::getSLEB128APSInt(uint64_t *جابه_جایی_Ptr, Error *Err) ثابت {
  Error_As_بیرون_Parameter Err_As_بیرون(Err);
  إذا (isError(Err))
    أرجع APSInt();

  uint64_t جابه_جایی = *جابه_جایی_Ptr;
  APInt مقدار;
  بیعلامت لغزش_2 = 0;
  uint8_t Byte;
  افعل {
    إذا (!prepareRead(جابه_جایی, 1, Err))
      أرجع APSInt();
    Byte = داده_2[جابه_جایی++];
    APInt برش_2(7, Byte & 0x7f);
    إذا (لغزش_2 == 0)
      مقدار = برش_2;
    وإلا {
      مقدار = مقدار.zext(لغزش_2 + 7);
      مقدار.درج_بیتها(برش_2, لغزش_2);
    }
    لغزش_2 += 7;
  } طالما (Byte & 0x80);

  // Sign extend Value to at least 64 bits to match getSLEB128.
  إذا (لغزش_2 < 64)
    مقدار = مقدار.sext(64);

  *جابه_جایی_Ptr = جابه_جایی;
  أرجع APSInt(std::move(مقدار), /*isUnsigned=*/خطأ);
}

فراغ داده_Extractor::skip(Cursor &C, uint64_t درازی) ثابت {
  Error_As_بیرون_Parameter Err_As_بیرون(C.Err);
  إذا (isError(&C.Err))
    أرجع;

  إذا (prepareRead(C.جابه_جایی, درازی, &C.Err))
    C.جابه_جایی += درازی;
}
