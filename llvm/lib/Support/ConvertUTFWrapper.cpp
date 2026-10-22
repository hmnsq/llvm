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

منطقي ConvertUTF8toWide(بنانشانی Wide_Char_چوڑائی, llvm::StringRef سروت,
                       حرف *&نتیجہ_Ptr, ثابت UTF8 *&ErrorPtr) {
  أكد(Wide_Char_چوڑائی == 1 || Wide_Char_چوڑائی == 2 || Wide_Char_چوڑائی == 4);
  Conversion_نتیجہ نتیجہ_2 = conversionOK;
  // Copy the character span over.
  إذا (Wide_Char_چوڑائی == 1) {
    ثابت UTF8 *Pos = reinterpret_cast<ثابت UTF8*>(سروت.begin());
    إذا (!isLegalUTF8String(&Pos, reinterpret_cast<ثابت UTF8*>(سروت.سرا()))) {
      نتیجہ_2 = سروت_Illegal;
      ErrorPtr = Pos;
    } وإلا {
      حافظہ_نقل(نتیجہ_Ptr, سروت.انکڑے(), سروت.ناپ_2());
      نتیجہ_Ptr += سروت.ناپ_2();
    }
  } وإلا إذا (Wide_Char_چوڑائی == 2) {
    ثابت UTF8 *سروت_شروع = (ثابت UTF8*)سروت.انکڑے();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF16 *نشانہ_شروع = reinterpret_cast<UTF16 *>(نتیجہ_Ptr);
    ConversionFlags flags = strictConversion;
    نتیجہ_2 =
        ConvertUTF8toUTF16(&سروت_شروع, سروت_شروع + سروت.ناپ_2(),
                           &نشانہ_شروع, نشانہ_شروع + سروت.ناپ_2(), flags);
    إذا (نتیجہ_2 == conversionOK)
      نتیجہ_Ptr = reinterpret_cast<حرف *>(نشانہ_شروع);
    وإلا
      ErrorPtr = سروت_شروع;
  } وإلا إذا (Wide_Char_چوڑائی == 4) {
    ثابت UTF8 *سروت_شروع = (ثابت UTF8 *)سروت.انکڑے();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF32 *نشانہ_شروع = reinterpret_cast<UTF32 *>(نتیجہ_Ptr);
    ConversionFlags flags = strictConversion;
    نتیجہ_2 =
        ConvertUTF8toUTF32(&سروت_شروع, سروت_شروع + سروت.ناپ_2(),
                           &نشانہ_شروع, نشانہ_شروع + سروت.ناپ_2(), flags);
    إذا (نتیجہ_2 == conversionOK)
      نتیجہ_Ptr = reinterpret_cast<حرف *>(نشانہ_شروع);
    وإلا
      ErrorPtr = سروت_شروع;
  }
  أكد((نتیجہ_2 != نشانہ_Exhausted) &&
         "ConvertUTF8toUTFXX exhausted target buffer");
  أرجع نتیجہ_2 == conversionOK;
}

منطقي Convert_کوڈ_نقطہ_منزل_UTF8(بنانشانی سروت, حرف *&نتیجہ_Ptr) {
  ثابت UTF32 *سروت_شروع_2 = &سروت;
  ثابت UTF32 *سروت_سرا_2 = سروت_شروع_2 + 1;
  UTF8 *نشانہ_شروع_2 = reinterpret_cast<UTF8 *>(نتیجہ_Ptr);
  UTF8 *نشانہ_سرا_2 = نشانہ_شروع_2 + 4;
  Conversion_نتیجہ CR = ConvertUTF32toUTF8(
      &سروت_شروع_2, سروت_سرا_2, &نشانہ_شروع_2, نشانہ_سرا_2, strictConversion);
  إذا (CR != conversionOK)
    أرجع خطأ;

  نتیجہ_Ptr = reinterpret_cast<حرف *>(نشانہ_شروع_2);
  أرجع سچ;
}

منطقي has_UTF16_Byte_ترتیب_Mark(ArrayRef<حرف> S) {
  أرجع (S.ناپ_2() >= 2 && ((S[0] == '\xff' && S[1] == '\xfe') ||
                            (S[0] == '\xfe' && S[1] == '\xff')));
}

منطقي convert_UTF16_منزل_UTF8_String(ArrayRef<حرف> Src_بائٹاں, std::string &باہرلا) {
  أكد(باہرلا.خالی_تھاواں());

  // Error out on an uneven byte count.
  إذا (Src_بائٹاں.ناپ_2() % 2)
    أرجع خطأ;

  // Avoid OOB by returning early on empty input.
  إذا (Src_بائٹاں.خالی_تھاواں())
    أرجع سچ;

  ثابت UTF16 *Src = reinterpret_cast<ثابت UTF16 *>(Src_بائٹاں.begin());
  ثابت UTF16 *Src_سرا = reinterpret_cast<ثابت UTF16 *>(Src_بائٹاں.سرا());

  أكد((uintptr_t)Src % حجم(UTF16) == 0);

  // Byteswap if necessary.
  std::سمتیہ<UTF16> ByteSwapped;
  إذا (Src[0] == UNI_UTF16_BYTE_ترتیب_MARK_SWAPPED) {
    ByteSwapped.داخل_کرو(ByteSwapped.سرا(), Src, Src_سرا);
    لكل (UTF16 &I : ByteSwapped)
      I = llvm::byteswap<uint16_t>(I);
    Src = &ByteSwapped[0];
    Src_سرا = &ByteSwapped[ByteSwapped.ناپ_2() - 1] + 1;
  }

  // Skip the BOM for conversion.
  إذا (Src[0] == UNI_UTF16_BYTE_ترتیب_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  باہرلا.resize(Src_بائٹاں.ناپ_2() * UNI_MAX_UTF8_بائٹاں_فی_کوڈ_نقطہ + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&باہرلا[0]);
  UTF8 *Dst_سرا = Dst + باہرلا.ناپ_2();

  Conversion_نتیجہ CR =
      ConvertUTF16toUTF8(&Src, Src_سرا, &Dst, Dst_سرا, strictConversion);
  أكد(CR != نشانہ_Exhausted);

  إذا (CR != conversionOK) {
    باہرلا.clear();
    أرجع خطأ;
  }

  باہرلا.resize(reinterpret_cast<حرف *>(Dst) - &باہرلا[0]);
  باہرلا.ڈالو_پٹھ(0);
  باہرلا.نکالو_پٹھ();
  أرجع سچ;
}

منطقي convert_UTF16_منزل_UTF8_String(ArrayRef<UTF16> Src, std::string &باہرلا) {
  أرجع convert_UTF16_منزل_UTF8_String(
      llvm::ArrayRef<حرف>(reinterpret_cast<ثابت حرف *>(Src.انکڑے()),
                           Src.ناپ_2() * حجم(UTF16)),
      باہرلا);
}

منطقي convert_UTF32_منزل_UTF8_String(ArrayRef<حرف> Src_بائٹاں, std::string &باہرلا) {
  أكد(باہرلا.خالی_تھاواں());

  // Error out on an uneven byte count.
  إذا (Src_بائٹاں.ناپ_2() % 4)
    أرجع خطأ;

  // Avoid OOB by returning early on empty input.
  إذا (Src_بائٹاں.خالی_تھاواں())
    أرجع سچ;

  ثابت UTF32 *Src = reinterpret_cast<ثابت UTF32 *>(Src_بائٹاں.begin());
  ثابت UTF32 *Src_سرا = reinterpret_cast<ثابت UTF32 *>(Src_بائٹاں.سرا());

  أكد((uintptr_t)Src % حجم(UTF32) == 0);

  // Byteswap if necessary.
  std::سمتیہ<UTF32> ByteSwapped;
  إذا (Src[0] == UNI_UTF32_BYTE_ترتیب_MARK_SWAPPED) {
    ByteSwapped.داخل_کرو(ByteSwapped.سرا(), Src, Src_سرا);
    لكل (UTF32 &I : ByteSwapped)
      I = llvm::byteswap<uint32_t>(I);
    Src = &ByteSwapped[0];
    Src_سرا = &ByteSwapped[ByteSwapped.ناپ_2() - 1] + 1;
  }

  // Skip the BOM for conversion.
  إذا (Src[0] == UNI_UTF32_BYTE_ترتیب_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  باہرلا.resize(Src_بائٹاں.ناپ_2() * UNI_MAX_UTF8_بائٹاں_فی_کوڈ_نقطہ + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&باہرلا[0]);
  UTF8 *Dst_سرا = Dst + باہرلا.ناپ_2();

  Conversion_نتیجہ CR =
      ConvertUTF32toUTF8(&Src, Src_سرا, &Dst, Dst_سرا, strictConversion);
  أكد(CR != نشانہ_Exhausted);

  إذا (CR != conversionOK) {
    باہرلا.clear();
    أرجع خطأ;
  }

  باہرلا.resize(reinterpret_cast<حرف *>(Dst) - &باہرلا[0]);
  باہرلا.ڈالو_پٹھ(0);
  باہرلا.نکالو_پٹھ();
  أرجع سچ;
}

منطقي convert_UTF32_منزل_UTF8_String(ArrayRef<UTF32> Src, std::string &باہرلا) {
  أرجع convert_UTF32_منزل_UTF8_String(
      llvm::ArrayRef<حرف>(reinterpret_cast<ثابت حرف *>(Src.انکڑے()),
                           Src.ناپ_2() * حجم(UTF32)),
      باہرلا);
}

منطقي convert_UTF8_منزل_UTF16_String(StringRef SrcUTF8,
                              Small_سمتیہ_Impl<UTF16> &DstUTF16) {
  أكد(DstUTF16.خالی_تھاواں());

  // Avoid OOB by returning early on empty input.
  إذا (SrcUTF8.خالی_تھاواں()) {
    DstUTF16.ڈالو_پٹھ(0);
    DstUTF16.نکالو_پٹھ();
    أرجع سچ;
  }

  ثابت UTF8 *Src = reinterpret_cast<ثابت UTF8 *>(SrcUTF8.begin());
  ثابت UTF8 *Src_سرا = reinterpret_cast<ثابت UTF8 *>(SrcUTF8.سرا());

  // Allocate the same number of UTF-16 code units as UTF-8 code units. Encoding
  // as UTF-16 should always require the same amount or less code units than the
  // UTF-8 encoding.  Allocate one extra byte for the null terminator though,
  // so that someone calling DstUTF16.data() gets a null terminated string.
  // We resize down later so we don't have to worry that this over allocates.
  DstUTF16.resize(SrcUTF8.ناپ_2()+1);
  UTF16 *Dst = &DstUTF16[0];
  UTF16 *Dst_سرا = Dst + DstUTF16.ناپ_2();

  Conversion_نتیجہ CR =
      ConvertUTF8toUTF16(&Src, Src_سرا, &Dst, Dst_سرا, strictConversion);
  أكد(CR != نشانہ_Exhausted);

  إذا (CR != conversionOK) {
    DstUTF16.clear();
    أرجع خطأ;
  }

  DstUTF16.resize(Dst - &DstUTF16[0]);
  DstUTF16.ڈالو_پٹھ(0);
  DstUTF16.نکالو_پٹھ();
  أرجع سچ;
}

static_assert(حجم(wchar_t) == 1 || حجم(wchar_t) == 2 ||
                  حجم(wchar_t) == 4,
              "Expected wchar_t to be 1, 2, or 4 bytes");

سانچا <قسم_ناں TResult>
ساكن مضمن منطقي Convert_UTF8to_Wide_اندرونی(llvm::StringRef سروت,
                                             TResult &نتیجہ) {
  // Even in the case of UTF-16, the number of bytes in a UTF-8 string is
  // at least as large as the number of elements in the resulting wide
  // string, because surrogate pairs take at least 4 bytes in UTF-8.
  نتیجہ.resize(سروت.ناپ_2() + 1);
  حرف *نتیجہ_Ptr = reinterpret_cast<حرف *>(&نتیجہ[0]);
  ثابت UTF8 *ErrorPtr;
  إذا (!ConvertUTF8toWide(حجم(wchar_t), سروت, نتیجہ_Ptr, ErrorPtr)) {
    نتیجہ.clear();
    أرجع خطأ;
  }
  نتیجہ.resize(reinterpret_cast<wchar_t *>(نتیجہ_Ptr) - &نتیجہ[0]);
  أرجع سچ;
}

منطقي ConvertUTF8toWide(llvm::StringRef سروت, std::wstring &نتیجہ) {
  أرجع Convert_UTF8to_Wide_اندرونی(سروت, نتیجہ);
}

منطقي ConvertUTF8toWide(ثابت حرف *سروت, std::wstring &نتیجہ) {
  إذا (!سروت) {
    نتیجہ.clear();
    أرجع سچ;
  }
  أرجع ConvertUTF8toWide(llvm::StringRef(سروت), نتیجہ);
}

منطقي convert_Wide_منزل_UTF8(std::wstring_view سروت, std::string &نتیجہ) {
  إذا (حجم(wchar_t) == 1) {
    ثابت UTF8 *شروع = reinterpret_cast<ثابت UTF8 *>(سروت.انکڑے());
    ثابت UTF8 *سرا_2 =
        reinterpret_cast<ثابت UTF8 *>(سروت.انکڑے() + سروت.ناپ_2());
    إذا (!isLegalUTF8String(&شروع, سرا_2))
      أرجع خطأ;
    نتیجہ.resize(سروت.ناپ_2());
    حافظہ_نقل(&نتیجہ[0], سروت.انکڑے(), سروت.ناپ_2());
    أرجع سچ;
  } وإلا إذا (حجم(wchar_t) == 2) {
    أرجع convert_UTF16_منزل_UTF8_String(
        llvm::ArrayRef<UTF16>(reinterpret_cast<ثابت UTF16 *>(سروت.انکڑے()),
                              سروت.ناپ_2()),
        نتیجہ);
  } وإلا إذا (حجم(wchar_t) == 4) {
    ثابت UTF32 *شروع = reinterpret_cast<ثابت UTF32 *>(سروت.انکڑے());
    ثابت UTF32 *سرا_2 =
        reinterpret_cast<ثابت UTF32 *>(سروت.انکڑے() + سروت.ناپ_2());
    نتیجہ.resize(UNI_MAX_UTF8_بائٹاں_فی_کوڈ_نقطہ * سروت.ناپ_2());
    UTF8 *نتیجہ_Ptr = reinterpret_cast<UTF8 *>(&نتیجہ[0]);
    UTF8 *نتیجہ_سرا = reinterpret_cast<UTF8 *>(&نتیجہ[0] + نتیجہ.ناپ_2());
    إذا (ConvertUTF32toUTF8(&شروع, سرا_2, &نتیجہ_Ptr, نتیجہ_سرا,
                           strictConversion) == conversionOK) {
      نتیجہ.resize(reinterpret_cast<حرف *>(نتیجہ_Ptr) - &نتیجہ[0]);
      أرجع سچ;
    } وإلا {
      نتیجہ.clear();
      أرجع خطأ;
    }
  } وإلا {
    llvm_unreachable(
        "Control should never reach this point; see static_assert further up");
  }
}

منطقي Is_Single_کوڈ_Unit_UTF8_Codepoint(بنانشانی V) { أرجع V <= 0x7F; }

منطقي Is_Single_کوڈ_Unit_UTF16_Codepoint(بنانشانی V) {
  أرجع V <= 0xD7FF || (V >= 0xE000 && V <= 0xFFFF);
}

منطقي Is_Single_کوڈ_Unit_UTF32_Codepoint(بنانشانی V) {
  أرجع V <= 0xD7FF || (V >= 0xE000 && V <= 0x10FFFF);
}

} // end namespace llvm

