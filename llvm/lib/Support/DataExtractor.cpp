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

کارول نطاق llvm;

منطقي معلومات_Extractor::prepareRead(uint64_t دننه_واټن, uint64_t اندازه,
                                Error *E) ثابت {
  إذا (is_Valid_دننه_واټن_For_معلومات_د_اندازه(دننه_واټن, اندازه))
    أرجع ریښتیا;
  إذا (E) {
    إذا (دننه_واټن <= معلومات_2.اندازه_2())
      *E = createStringError(
          errc::illegal_byte_sequence,
          "unexpected end of data at offset 0x%zx while reading [0x%" PRIx64
          ", 0x%" PRIx64 ")",
          معلومات_2.اندازه_2(), دننه_واټن, دننه_واټن + اندازه);
    وإلا
      *E = createStringError(errc::invalid_دلیل,
                             "offset 0x%" PRIx64
                             " is beyond the end of data at 0x%zx",
                             دننه_واټن, معلومات_2.اندازه_2());
  }
  أرجع خطأ;
}

ساكن منطقي isError(Error *E) { أرجع E && *E; }

کينډۍ <ډول_نوم T>
T معلومات_Extractor::getU(uint64_t *دننه_واټن_ptr, Error *Err) ثابت {
  Error_As_بهر_Parameter Err_As_بهر(Err);
  T val = 0;
  إذا (isError(Err))
    أرجع val;

  uint64_t دننه_واټن_2 = *دننه_واټن_ptr;
  إذا (!prepareRead(دننه_واټن_2, حجم(T), Err))
    أرجع val;
  std::حافظه_نقل(&val, &معلومات_2.معلومات()[دننه_واټن_2], حجم(val));
  إذا (sys::IsLittleEndianHost != IsLittleEndian)
    sys::بدلون_Byte_ترتیب(val);

  // Advance the offset
  *دننه_واټن_ptr += حجم(val);
  أرجع val;
}

کينډۍ <ډول_نوم T>
T *معلومات_Extractor::getUs(uint64_t *دننه_واټن_ptr, T *dst, uint32_t شمېر,
                        Error *Err) ثابت {
  Error_As_بهر_Parameter Err_As_بهر(Err);
  إذا (isError(Err))
    أرجع تش_اشاره;

  uint64_t دننه_واټن_2 = *دننه_واټن_ptr;

  إذا (!prepareRead(دننه_واټن_2, حجم(*dst) * شمېر, Err))
    أرجع تش_اشاره;
  لكل (T *ارزښت_ptr = dst, *پای_ټکی = dst + شمېر; ارزښت_ptr != پای_ټکی;
       ++ارزښت_ptr, دننه_واټن_2 += حجم(*dst))
    *ارزښت_ptr = getU<T>(دننه_واټن_ptr, Err);
  // Advance the offset
  *دننه_واټن_ptr = دننه_واټن_2;
  // Return a non-NULL pointer to the converted data as an indicator of
  // success
  أرجع dst;
}

uint8_t معلومات_Extractor::getU8(uint64_t *دننه_واټن_ptr, llvm::Error *Err) ثابت {
  أرجع getU<uint8_t>(دننه_واټن_ptr, Err);
}

uint8_t *معلومات_Extractor::getU8(uint64_t *دننه_واټن_ptr, uint8_t *dst,
                              uint32_t شمېر) ثابت {
  أرجع getUs<uint8_t>(دننه_واټن_ptr, dst, شمېر, تش_اشاره);
}

uint8_t *معلومات_Extractor::getU8(Cursor &C, uint8_t *Dst, uint32_t شمېر_2) ثابت {
  أرجع getUs<uint8_t>(&C.دننه_واټن, Dst, شمېر_2, &C.Err);
}

uint16_t معلومات_Extractor::getU16(uint64_t *دننه_واټن_ptr, llvm::Error *Err) ثابت {
  أرجع getU<uint16_t>(دننه_واټن_ptr, Err);
}

uint16_t *معلومات_Extractor::getU16(uint64_t *دننه_واټن_ptr, uint16_t *dst,
                                uint32_t شمېر) ثابت {
  أرجع getUs<uint16_t>(دننه_واټن_ptr, dst, شمېر, تش_اشاره);
}

uint32_t معلومات_Extractor::getU24(uint64_t *دننه_واټن_Ptr, Error *Err) ثابت {
  uint24_t ExtractedVal = getU<uint24_t>(دننه_واټن_Ptr, Err);
  // The 3 bytes are in the correct byte order for the host.
  أرجع ExtractedVal.getAsUint32(sys::IsLittleEndianHost);
}

uint32_t معلومات_Extractor::getU32(uint64_t *دننه_واټن_ptr, llvm::Error *Err) ثابت {
  أرجع getU<uint32_t>(دننه_واټن_ptr, Err);
}

uint32_t *معلومات_Extractor::getU32(uint64_t *دننه_واټن_ptr, uint32_t *dst,
                                uint32_t شمېر) ثابت {
  أرجع getUs<uint32_t>(دننه_واټن_ptr, dst, شمېر, تش_اشاره);
}

uint64_t معلومات_Extractor::getU64(uint64_t *دننه_واټن_ptr, llvm::Error *Err) ثابت {
  أرجع getU<uint64_t>(دننه_واټن_ptr, Err);
}

uint64_t *معلومات_Extractor::getU64(uint64_t *دننه_واټن_ptr, uint64_t *dst,
                                uint32_t شمېر) ثابت {
  أرجع getUs<uint64_t>(دننه_واټن_ptr, dst, شمېر, تش_اشاره);
}

uint64_t معلومات_Extractor::getUnsigned(uint64_t *دننه_واټن_ptr, uint32_t byte_اندازه,
                                    llvm::Error *Err) ثابت {
  بدل (byte_اندازه) {
  حالة 1:
    أرجع getU8(دننه_واټن_ptr, Err);
  حالة 2:
    أرجع getU16(دننه_واټن_ptr, Err);
  حالة 4:
    أرجع getU32(دننه_واټن_ptr, Err);
  حالة 8:
    أرجع getU64(دننه_واټن_ptr, Err);
  }

  // For any other byte size, read the bytes and swap/shift if necessary.
  Error_As_بهر_Parameter Err_As_بهر(Err);
  uint64_t val = 0;
  إذا (isError(Err))
    أرجع val;
  uint64_t دننه_واټن_2 = *دننه_واټن_ptr;
  إذا (!prepareRead(دننه_واټن_2, byte_اندازه, Err))
    أرجع val;
  // Copy into the least significant bytes of val regardless of host
  // endianness.
  std::حافظه_نقل(reinterpret_cast<حرف *>(&val) +
                  (sys::IsLittleEndianHost ? 0 : 8 - byte_اندازه),
              &معلومات_2.معلومات()[دننه_واټن_2], byte_اندازه);
  // Swap the least significant bytes of val if endianness doesn't match.
  إذا (sys::IsLittleEndianHost != IsLittleEndian)
    val = sys::get_Swapped_بایټونه(val) >> (8 * (8 - byte_اندازه));

  *دننه_واټن_ptr += byte_اندازه;
  أرجع val;
}

int64_t
معلومات_Extractor::getSigned(uint64_t *دننه_واټن_ptr, uint32_t byte_اندازه) ثابت {
  بدل (byte_اندازه) {
  حالة 1:
    أرجع getS8(دننه_واټن_ptr);
  حالة 2:
    أرجع getS16(دننه_واټن_ptr);
  حالة 4:
    أرجع getS32(دننه_واټن_ptr);
  حالة 8:
    أرجع getS64(دننه_واټن_ptr);
  }
  llvm_unreachable("getSigned unhandled case!");
}

StringRef معلومات_Extractor::getCStrRef(uint64_t *دننه_واټن_Ptr, Error *Err) ثابت {
  Error_As_بهر_Parameter Err_As_بهر(Err);
  إذا (isError(Err))
    أرجع StringRef();

  uint64_t پیل = *دننه_واټن_Ptr;
  StringRef::اندازه_type Pos = معلومات_2.پیدا_کړه('\0', پیل);
  إذا (Pos != StringRef::npos) {
    *دننه_واټن_Ptr = Pos + 1;
    أرجع StringRef(معلومات_2.معلومات() + پیل, Pos - پیل);
  }
  إذا (Err)
    *Err = createStringError(errc::illegal_byte_sequence,
                             "no null terminated string at offset 0x%" PRIx64,
                             پیل);
  أرجع StringRef();
}

StringRef معلومات_Extractor::get_Fixed_اوږدتیا_String(uint64_t *دننه_واټن_Ptr,
                                              uint64_t اوږدتیا,
                                              StringRef TrimChars) ثابت {
  StringRef بایټونه_2(get_بایټونه(دننه_واټن_Ptr, اوږدتیا));
  أرجع بایټونه_2.trim(TrimChars);
}

StringRef معلومات_Extractor::get_بایټونه(uint64_t *دننه_واټن_Ptr, uint64_t اوږدتیا,
                                  Error *Err) ثابت {
  Error_As_بهر_Parameter Err_As_بهر(Err);
  إذا (isError(Err))
    أرجع StringRef();

  إذا (!prepareRead(*دننه_واټن_Ptr, اوږدتیا, Err))
    أرجع StringRef();

  StringRef پایله = معلومات_2.substr(*دننه_واټن_Ptr, اوږدتیا);
  *دننه_واټن_Ptr += اوږدتیا;
  أرجع پایله;
}

کينډۍ <ډول_نوم T>
ساكن T getLEB128(StringRef معلومات_2, uint64_t *دننه_واټن_Ptr, Error *Err,
                   T (&Decoder)(ثابت uint8_t *p, بینښه *n,
                                ثابت uint8_t *پای_ټکی, ثابت حرف **error)) {
  ArrayRef<uint8_t> بایټونه_2 = arrayRefFromStringRef(معلومات_2);
  أكد(*دننه_واټن_Ptr <= بایټونه_2.اندازه_2());
  Error_As_بهر_Parameter Err_As_بهر(Err);
  إذا (isError(Err))
    أرجع T();

  ثابت حرف *error = تش_اشاره;
  بینښه بایټونه_read;
  T پایله_2 =
      Decoder(بایټونه_2.معلومات() + *دننه_واټن_Ptr, &بایټونه_read, بایټونه_2.پای_ټکی(), &error);
  إذا (error) {
    إذا (Err)
      *Err = createStringError(errc::illegal_byte_sequence,
                               "unable to decode LEB128 at offset 0x%8.8" PRIx64
                               ": %s",
                               *دننه_واټن_Ptr, error);
    أرجع T();
  }
  *دننه_واټن_Ptr += بایټونه_read;
  أرجع پایله_2;
}

uint64_t معلومات_Extractor::getULEB128(uint64_t *دننه_واټن_ptr, Error *Err) ثابت {
  أرجع getLEB128(معلومات_2, دننه_واټن_ptr, Err, decodeULEB128);
}

int64_t معلومات_Extractor::getSLEB128(uint64_t *دننه_واټن_ptr, Error *Err) ثابت {
  أرجع getLEB128(معلومات_2, دننه_واټن_ptr, Err, decodeSLEB128);
}

APSInt معلومات_Extractor::getSLEB128APSInt(uint64_t *دننه_واټن_Ptr, Error *Err) ثابت {
  Error_As_بهر_Parameter Err_As_بهر(Err);
  إذا (isError(Err))
    أرجع APSInt();

  uint64_t دننه_واټن = *دننه_واټن_Ptr;
  APInt ارزښت;
  بینښه ښويدنه_2 = 0;
  uint8_t Byte;
  افعل {
    إذا (!prepareRead(دننه_واټن, 1, Err))
      أرجع APSInt();
    Byte = معلومات_2[دننه_واټن++];
    APInt ټوټه_2(7, Byte & 0x7f);
    إذا (ښويدنه_2 == 0)
      ارزښت = ټوټه_2;
    وإلا {
      ارزښت = ارزښت.zext(ښويدنه_2 + 7);
      ارزښت.ورننباسه_بټونه(ټوټه_2, ښويدنه_2);
    }
    ښويدنه_2 += 7;
  } طالما (Byte & 0x80);

  // Sign extend Value to at least 64 bits to match getSLEB128.
  إذا (ښويدنه_2 < 64)
    ارزښت = ارزښت.sext(64);

  *دننه_واټن_Ptr = دننه_واټن;
  أرجع APSInt(std::move(ارزښت), /*isUnsigned=*/خطأ);
}

فراغ معلومات_Extractor::skip(Cursor &C, uint64_t اوږدتیا) ثابت {
  Error_As_بهر_Parameter Err_As_بهر(C.Err);
  إذا (isError(&C.Err))
    أرجع;

  إذا (prepareRead(C.دننه_واټن, اوږدتیا, &C.Err))
    C.دننه_واټن += اوږدتیا;
}
