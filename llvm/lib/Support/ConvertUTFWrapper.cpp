//===-- ConvertUTFWrapper.cpp - Wrap ConvertUTF.h with clang data types -----===
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/ADT/ArrayRef.h"
#ضمن "llvm/ADT/StringRef.h"
#ضمن "llvm/ADT/bit.h"
#ضمن "llvm/Support/ConvertUTF.h"
#ضمن "llvm/Support/ErrorHandling.h"
#ضمن <string>
#ضمن <vector>

نطاق llvm {

منطقي ConvertUTF8toWide(بیعلامت Wide_Char_عرض, llvm::StringRef منبع,
                       حرف *&نتیجه_Ptr, ثابت UTF8 *&ErrorPtr) {
  أكد(Wide_Char_عرض == 1 || Wide_Char_عرض == 2 || Wide_Char_عرض == 4);
  Conversion_نتیجه نتیجه_2 = conversionOK;
  // Copy the character span over.
  إذا (Wide_Char_عرض == 1) {
    ثابت UTF8 *Pos = reinterpret_cast<ثابت UTF8*>(منبع.begin());
    إذا (!isLegalUTF8String(&Pos, reinterpret_cast<ثابت UTF8*>(منبع.انتها()))) {
      نتیجه_2 = منبع_Illegal;
      ErrorPtr = Pos;
    } وإلا {
      کپی_حافظه(نتیجه_Ptr, منبع.داده(), منبع.اندازه_2());
      نتیجه_Ptr += منبع.اندازه_2();
    }
  } وإلا إذا (Wide_Char_عرض == 2) {
    ثابت UTF8 *منبع_شروع = (ثابت UTF8*)منبع.داده();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF16 *هدف_شروع = reinterpret_cast<UTF16 *>(نتیجه_Ptr);
    ConversionFlags flags = strictConversion;
    نتیجه_2 =
        ConvertUTF8toUTF16(&منبع_شروع, منبع_شروع + منبع.اندازه_2(),
                           &هدف_شروع, هدف_شروع + منبع.اندازه_2(), flags);
    إذا (نتیجه_2 == conversionOK)
      نتیجه_Ptr = reinterpret_cast<حرف *>(هدف_شروع);
    وإلا
      ErrorPtr = منبع_شروع;
  } وإلا إذا (Wide_Char_عرض == 4) {
    ثابت UTF8 *منبع_شروع = (ثابت UTF8 *)منبع.داده();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF32 *هدف_شروع = reinterpret_cast<UTF32 *>(نتیجه_Ptr);
    ConversionFlags flags = strictConversion;
    نتیجه_2 =
        ConvertUTF8toUTF32(&منبع_شروع, منبع_شروع + منبع.اندازه_2(),
                           &هدف_شروع, هدف_شروع + منبع.اندازه_2(), flags);
    إذا (نتیجه_2 == conversionOK)
      نتیجه_Ptr = reinterpret_cast<حرف *>(هدف_شروع);
    وإلا
      ErrorPtr = منبع_شروع;
  }
  أكد((نتیجه_2 != هدف_Exhausted) &&
         "ConvertUTF8toUTFXX exhausted target buffer");
  أرجع نتیجه_2 == conversionOK;
}

منطقي Convert_کد_نقطه_مقصد_UTF8(بیعلامت منبع, حرف *&نتیجه_Ptr) {
  ثابت UTF32 *منبع_شروع_2 = &منبع;
  ثابت UTF32 *منبع_انتها_2 = منبع_شروع_2 + 1;
  UTF8 *هدف_شروع_2 = reinterpret_cast<UTF8 *>(نتیجه_Ptr);
  UTF8 *هدف_انتها_2 = هدف_شروع_2 + 4;
  Conversion_نتیجه CR = ConvertUTF32toUTF8(
      &منبع_شروع_2, منبع_انتها_2, &هدف_شروع_2, هدف_انتها_2, strictConversion);
  إذا (CR != conversionOK)
    أرجع خطأ;

  نتیجه_Ptr = reinterpret_cast<حرف *>(هدف_شروع_2);
  أرجع درست;
}

منطقي has_UTF16_Byte_ترتیب_Mark(ArrayRef<حرف> S) {
  أرجع (S.اندازه_2() >= 2 && ((S[0] == '\xff' && S[1] == '\xfe') ||
                            (S[0] == '\xfe' && S[1] == '\xff')));
}

منطقي convert_UTF16_مقصد_UTF8_String(ArrayRef<حرف> Src_بایتها, std::string &بیرون) {
  أكد(بیرون.خالی());

  // Error out on an uneven byte count.
  إذا (Src_بایتها.اندازه_2() % 2)
    أرجع خطأ;

  // Avoid OOB by returning early on empty input.
  إذا (Src_بایتها.خالی())
    أرجع درست;

  ثابت UTF16 *Src = reinterpret_cast<ثابت UTF16 *>(Src_بایتها.begin());
  ثابت UTF16 *Src_انتها = reinterpret_cast<ثابت UTF16 *>(Src_بایتها.انتها());

  أكد((uintptr_t)Src % حجم(UTF16) == 0);

  // Byteswap if necessary.
  std::وکتور<UTF16> ByteSwapped;
  إذا (Src[0] == UNI_UTF16_BYTE_ترتیب_MARK_SWAPPED) {
    ByteSwapped.درج(ByteSwapped.انتها(), Src, Src_انتها);
    لكل (UTF16 &I : ByteSwapped)
      I = llvm::byteswap<uint16_t>(I);
    Src = &ByteSwapped[0];
    Src_انتها = &ByteSwapped[ByteSwapped.اندازه_2() - 1] + 1;
  }

  // Skip the BOM for conversion.
  إذا (Src[0] == UNI_UTF16_BYTE_ترتیب_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  بیرون.resize(Src_بایتها.اندازه_2() * UNI_MAX_UTF8_بایتها_در_هر_کد_نقطه + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&بیرون[0]);
  UTF8 *Dst_انتها = Dst + بیرون.اندازه_2();

  Conversion_نتیجه CR =
      ConvertUTF16toUTF8(&Src, Src_انتها, &Dst, Dst_انتها, strictConversion);
  أكد(CR != هدف_Exhausted);

  إذا (CR != conversionOK) {
    بیرون.clear();
    أرجع خطأ;
  }

  بیرون.resize(reinterpret_cast<حرف *>(Dst) - &بیرون[0]);
  بیرون.افزودن_پشت(0);
  بیرون.برداشت_پشت();
  أرجع درست;
}

منطقي convert_UTF16_مقصد_UTF8_String(ArrayRef<UTF16> Src, std::string &بیرون) {
  أرجع convert_UTF16_مقصد_UTF8_String(
      llvm::ArrayRef<حرف>(reinterpret_cast<ثابت حرف *>(Src.داده()),
                           Src.اندازه_2() * حجم(UTF16)),
      بیرون);
}

منطقي convert_UTF32_مقصد_UTF8_String(ArrayRef<حرف> Src_بایتها, std::string &بیرون) {
  أكد(بیرون.خالی());

  // Error out on an uneven byte count.
  إذا (Src_بایتها.اندازه_2() % 4)
    أرجع خطأ;

  // Avoid OOB by returning early on empty input.
  إذا (Src_بایتها.خالی())
    أرجع درست;

  ثابت UTF32 *Src = reinterpret_cast<ثابت UTF32 *>(Src_بایتها.begin());
  ثابت UTF32 *Src_انتها = reinterpret_cast<ثابت UTF32 *>(Src_بایتها.انتها());

  أكد((uintptr_t)Src % حجم(UTF32) == 0);

  // Byteswap if necessary.
  std::وکتور<UTF32> ByteSwapped;
  إذا (Src[0] == UNI_UTF32_BYTE_ترتیب_MARK_SWAPPED) {
    ByteSwapped.درج(ByteSwapped.انتها(), Src, Src_انتها);
    لكل (UTF32 &I : ByteSwapped)
      I = llvm::byteswap<uint32_t>(I);
    Src = &ByteSwapped[0];
    Src_انتها = &ByteSwapped[ByteSwapped.اندازه_2() - 1] + 1;
  }

  // Skip the BOM for conversion.
  إذا (Src[0] == UNI_UTF32_BYTE_ترتیب_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  بیرون.resize(Src_بایتها.اندازه_2() * UNI_MAX_UTF8_بایتها_در_هر_کد_نقطه + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&بیرون[0]);
  UTF8 *Dst_انتها = Dst + بیرون.اندازه_2();

  Conversion_نتیجه CR =
      ConvertUTF32toUTF8(&Src, Src_انتها, &Dst, Dst_انتها, strictConversion);
  أكد(CR != هدف_Exhausted);

  إذا (CR != conversionOK) {
    بیرون.clear();
    أرجع خطأ;
  }

  بیرون.resize(reinterpret_cast<حرف *>(Dst) - &بیرون[0]);
  بیرون.افزودن_پشت(0);
  بیرون.برداشت_پشت();
  أرجع درست;
}

منطقي convert_UTF32_مقصد_UTF8_String(ArrayRef<UTF32> Src, std::string &بیرون) {
  أرجع convert_UTF32_مقصد_UTF8_String(
      llvm::ArrayRef<حرف>(reinterpret_cast<ثابت حرف *>(Src.داده()),
                           Src.اندازه_2() * حجم(UTF32)),
      بیرون);
}

منطقي convert_UTF8_مقصد_UTF16_String(StringRef SrcUTF8,
                              Small_وکتور_Impl<UTF16> &DstUTF16) {
  أكد(DstUTF16.خالی());

  // Avoid OOB by returning early on empty input.
  إذا (SrcUTF8.خالی()) {
    DstUTF16.افزودن_پشت(0);
    DstUTF16.برداشت_پشت();
    أرجع درست;
  }

  ثابت UTF8 *Src = reinterpret_cast<ثابت UTF8 *>(SrcUTF8.begin());
  ثابت UTF8 *Src_انتها = reinterpret_cast<ثابت UTF8 *>(SrcUTF8.انتها());

  // Allocate the same number of UTF-16 code units as UTF-8 code units. Encoding
  // as UTF-16 should always require the same amount or less code units than the
  // UTF-8 encoding.  Allocate one extra byte for the null terminator though,
  // so that someone calling DstUTF16.data() gets a null terminated string.
  // We resize down later so we don't have to worry that this over allocates.
  DstUTF16.resize(SrcUTF8.اندازه_2()+1);
  UTF16 *Dst = &DstUTF16[0];
  UTF16 *Dst_انتها = Dst + DstUTF16.اندازه_2();

  Conversion_نتیجه CR =
      ConvertUTF8toUTF16(&Src, Src_انتها, &Dst, Dst_انتها, strictConversion);
  أكد(CR != هدف_Exhausted);

  إذا (CR != conversionOK) {
    DstUTF16.clear();
    أرجع خطأ;
  }

  DstUTF16.resize(Dst - &DstUTF16[0]);
  DstUTF16.افزودن_پشت(0);
  DstUTF16.برداشت_پشت();
  أرجع درست;
}

static_assert(حجم(wchar_t) == 1 || حجم(wchar_t) == 2 ||
                  حجم(wchar_t) == 4,
              "Expected wchar_t to be 1, 2, or 4 bytes");

قالب <نام_نوع TResult>
ساكن مضمن منطقي Convert_UTF8to_Wide_داخلی(llvm::StringRef منبع,
                                             TResult &نتیجه) {
  // Even in the case of UTF-16, the number of bytes in a UTF-8 string is
  // at least as large as the number of elements in the resulting wide
  // string, because surrogate pairs take at least 4 bytes in UTF-8.
  نتیجه.resize(منبع.اندازه_2() + 1);
  حرف *نتیجه_Ptr = reinterpret_cast<حرف *>(&نتیجه[0]);
  ثابت UTF8 *ErrorPtr;
  إذا (!ConvertUTF8toWide(حجم(wchar_t), منبع, نتیجه_Ptr, ErrorPtr)) {
    نتیجه.clear();
    أرجع خطأ;
  }
  نتیجه.resize(reinterpret_cast<wchar_t *>(نتیجه_Ptr) - &نتیجه[0]);
  أرجع درست;
}

منطقي ConvertUTF8toWide(llvm::StringRef منبع, std::wstring &نتیجه) {
  أرجع Convert_UTF8to_Wide_داخلی(منبع, نتیجه);
}

منطقي ConvertUTF8toWide(ثابت حرف *منبع, std::wstring &نتیجه) {
  إذا (!منبع) {
    نتیجه.clear();
    أرجع درست;
  }
  أرجع ConvertUTF8toWide(llvm::StringRef(منبع), نتیجه);
}

منطقي convert_Wide_مقصد_UTF8(std::wstring_view منبع, std::string &نتیجه) {
  إذا (حجم(wchar_t) == 1) {
    ثابت UTF8 *شروع = reinterpret_cast<ثابت UTF8 *>(منبع.داده());
    ثابت UTF8 *انتها_2 =
        reinterpret_cast<ثابت UTF8 *>(منبع.داده() + منبع.اندازه_2());
    إذا (!isLegalUTF8String(&شروع, انتها_2))
      أرجع خطأ;
    نتیجه.resize(منبع.اندازه_2());
    کپی_حافظه(&نتیجه[0], منبع.داده(), منبع.اندازه_2());
    أرجع درست;
  } وإلا إذا (حجم(wchar_t) == 2) {
    أرجع convert_UTF16_مقصد_UTF8_String(
        llvm::ArrayRef<UTF16>(reinterpret_cast<ثابت UTF16 *>(منبع.داده()),
                              منبع.اندازه_2()),
        نتیجه);
  } وإلا إذا (حجم(wchar_t) == 4) {
    ثابت UTF32 *شروع = reinterpret_cast<ثابت UTF32 *>(منبع.داده());
    ثابت UTF32 *انتها_2 =
        reinterpret_cast<ثابت UTF32 *>(منبع.داده() + منبع.اندازه_2());
    نتیجه.resize(UNI_MAX_UTF8_بایتها_در_هر_کد_نقطه * منبع.اندازه_2());
    UTF8 *نتیجه_Ptr = reinterpret_cast<UTF8 *>(&نتیجه[0]);
    UTF8 *نتیجه_انتها = reinterpret_cast<UTF8 *>(&نتیجه[0] + نتیجه.اندازه_2());
    إذا (ConvertUTF32toUTF8(&شروع, انتها_2, &نتیجه_Ptr, نتیجه_انتها,
                           strictConversion) == conversionOK) {
      نتیجه.resize(reinterpret_cast<حرف *>(نتیجه_Ptr) - &نتیجه[0]);
      أرجع درست;
    } وإلا {
      نتیجه.clear();
      أرجع خطأ;
    }
  } وإلا {
    llvm_unreachable(
        "Control should never reach this point; see static_assert further up");
  }
}

منطقي Is_Single_کد_Unit_UTF8_Codepoint(بیعلامت V) { أرجع V <= 0x7F; }

منطقي Is_Single_کد_Unit_UTF16_Codepoint(بیعلامت V) {
  أرجع V <= 0xD7FF || (V >= 0xE000 && V <= 0xFFFF);
}

منطقي Is_Single_کد_Unit_UTF32_Codepoint(بیعلامت V) {
  أرجع V <= 0xD7FF || (V >= 0xE000 && V <= 0x10FFFF);
}

} // end namespace llvm

