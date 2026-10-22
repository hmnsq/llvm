//===-- ConvertUTFWrapper.cpp - Wrap ConvertUTF.h with clang data types -----===
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/ADT/ArrayRef.h"
#شامل "llvm/ADT/StringRef.h"
#شامل "llvm/ADT/bit.h"
#شامل "llvm/Support/ConvertUTF.h"
#شامل "llvm/Support/ErrorHandling.h"
#شامل <string>
#شامل <vector>

نامخلا llvm {

منطقی ConvertUTF8toWide(غیرعلامتی Wide_Char_چوڑائی, llvm::StringRef ماخذ,
                       حرف *&نتیجہ_Ptr, ثابت UTF8 *&ErrorPtr) {
  أكد(Wide_Char_چوڑائی == 1 || Wide_Char_چوڑائی == 2 || Wide_Char_چوڑائی == 4);
  Conversion_نتیجہ نتیجہ_2 = conversionOK;
  // Copy the character span over.
  اگر (Wide_Char_چوڑائی == 1) {
    ثابت UTF8 *Pos = reinterpret_cast<ثابت UTF8*>(ماخذ.begin());
    اگر (!isLegalUTF8String(&Pos, reinterpret_cast<ثابت UTF8*>(ماخذ.انتہا()))) {
      نتیجہ_2 = ماخذ_Illegal;
      ErrorPtr = Pos;
    } ورنہ {
      حافظہ_نقل(نتیجہ_Ptr, ماخذ.ڈیٹا(), ماخذ.سائز_2());
      نتیجہ_Ptr += ماخذ.سائز_2();
    }
  } ورنہ اگر (Wide_Char_چوڑائی == 2) {
    ثابت UTF8 *ماخذ_آغاز = (ثابت UTF8*)ماخذ.ڈیٹا();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF16 *ہدف_آغاز = reinterpret_cast<UTF16 *>(نتیجہ_Ptr);
    ConversionFlags flags = strictConversion;
    نتیجہ_2 =
        ConvertUTF8toUTF16(&ماخذ_آغاز, ماخذ_آغاز + ماخذ.سائز_2(),
                           &ہدف_آغاز, ہدف_آغاز + ماخذ.سائز_2(), flags);
    اگر (نتیجہ_2 == conversionOK)
      نتیجہ_Ptr = reinterpret_cast<حرف *>(ہدف_آغاز);
    ورنہ
      ErrorPtr = ماخذ_آغاز;
  } ورنہ اگر (Wide_Char_چوڑائی == 4) {
    ثابت UTF8 *ماخذ_آغاز = (ثابت UTF8 *)ماخذ.ڈیٹا();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF32 *ہدف_آغاز = reinterpret_cast<UTF32 *>(نتیجہ_Ptr);
    ConversionFlags flags = strictConversion;
    نتیجہ_2 =
        ConvertUTF8toUTF32(&ماخذ_آغاز, ماخذ_آغاز + ماخذ.سائز_2(),
                           &ہدف_آغاز, ہدف_آغاز + ماخذ.سائز_2(), flags);
    اگر (نتیجہ_2 == conversionOK)
      نتیجہ_Ptr = reinterpret_cast<حرف *>(ہدف_آغاز);
    ورنہ
      ErrorPtr = ماخذ_آغاز;
  }
  أكد((نتیجہ_2 != ہدف_Exhausted) &&
         "ConvertUTF8toUTFXX exhausted target buffer");
  واپس نتیجہ_2 == conversionOK;
}

منطقی Convert_رمز_نقطہ_مقصد_UTF8(غیرعلامتی ماخذ, حرف *&نتیجہ_Ptr) {
  ثابت UTF32 *ماخذ_آغاز_2 = &ماخذ;
  ثابت UTF32 *ماخذ_انتہا_2 = ماخذ_آغاز_2 + 1;
  UTF8 *ہدف_آغاز_2 = reinterpret_cast<UTF8 *>(نتیجہ_Ptr);
  UTF8 *ہدف_انتہا_2 = ہدف_آغاز_2 + 4;
  Conversion_نتیجہ CR = ConvertUTF32toUTF8(
      &ماخذ_آغاز_2, ماخذ_انتہا_2, &ہدف_آغاز_2, ہدف_انتہا_2, strictConversion);
  اگر (CR != conversionOK)
    واپس جھوٹ;

  نتیجہ_Ptr = reinterpret_cast<حرف *>(ہدف_آغاز_2);
  واپس سچ;
}

منطقی has_UTF16_Byte_ترتیب_Mark(ArrayRef<حرف> S) {
  واپس (S.سائز_2() >= 2 && ((S[0] == '\xff' && S[1] == '\xfe') ||
                            (S[0] == '\xfe' && S[1] == '\xff')));
}

منطقی convert_UTF16_مقصد_UTF8_String(ArrayRef<حرف> Src_بائٹیں, std::string &برآمد) {
  أكد(برآمد.خالی_خانے());

  // Error out on an uneven byte count.
  اگر (Src_بائٹیں.سائز_2() % 2)
    واپس جھوٹ;

  // Avoid OOB by returning early on empty input.
  اگر (Src_بائٹیں.خالی_خانے())
    واپس سچ;

  ثابت UTF16 *Src = reinterpret_cast<ثابت UTF16 *>(Src_بائٹیں.begin());
  ثابت UTF16 *Src_انتہا = reinterpret_cast<ثابت UTF16 *>(Src_بائٹیں.انتہا());

  أكد((uintptr_t)Src % حجم(UTF16) == 0);

  // Byteswap if necessary.
  std::سمتیہ<UTF16> ByteSwapped;
  اگر (Src[0] == UNI_UTF16_BYTE_ترتیب_MARK_SWAPPED) {
    ByteSwapped.داخل_کرو(ByteSwapped.انتہا(), Src, Src_انتہا);
    برائے (UTF16 &I : ByteSwapped)
      I = llvm::byteswap<uint16_t>(I);
    Src = &ByteSwapped[0];
    Src_انتہا = &ByteSwapped[ByteSwapped.سائز_2() - 1] + 1;
  }

  // Skip the BOM for conversion.
  اگر (Src[0] == UNI_UTF16_BYTE_ترتیب_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  برآمد.resize(Src_بائٹیں.سائز_2() * UNI_MAX_UTF8_بائٹیں_فی_رمز_نقطہ + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&برآمد[0]);
  UTF8 *Dst_انتہا = Dst + برآمد.سائز_2();

  Conversion_نتیجہ CR =
      ConvertUTF16toUTF8(&Src, Src_انتہا, &Dst, Dst_انتہا, strictConversion);
  أكد(CR != ہدف_Exhausted);

  اگر (CR != conversionOK) {
    برآمد.clear();
    واپس جھوٹ;
  }

  برآمد.resize(reinterpret_cast<حرف *>(Dst) - &برآمد[0]);
  برآمد.ڈالو_عقب(0);
  برآمد.نکالو_عقب();
  واپس سچ;
}

منطقی convert_UTF16_مقصد_UTF8_String(ArrayRef<UTF16> Src, std::string &برآمد) {
  واپس convert_UTF16_مقصد_UTF8_String(
      llvm::ArrayRef<حرف>(reinterpret_cast<ثابت حرف *>(Src.ڈیٹا()),
                           Src.سائز_2() * حجم(UTF16)),
      برآمد);
}

منطقی convert_UTF32_مقصد_UTF8_String(ArrayRef<حرف> Src_بائٹیں, std::string &برآمد) {
  أكد(برآمد.خالی_خانے());

  // Error out on an uneven byte count.
  اگر (Src_بائٹیں.سائز_2() % 4)
    واپس جھوٹ;

  // Avoid OOB by returning early on empty input.
  اگر (Src_بائٹیں.خالی_خانے())
    واپس سچ;

  ثابت UTF32 *Src = reinterpret_cast<ثابت UTF32 *>(Src_بائٹیں.begin());
  ثابت UTF32 *Src_انتہا = reinterpret_cast<ثابت UTF32 *>(Src_بائٹیں.انتہا());

  أكد((uintptr_t)Src % حجم(UTF32) == 0);

  // Byteswap if necessary.
  std::سمتیہ<UTF32> ByteSwapped;
  اگر (Src[0] == UNI_UTF32_BYTE_ترتیب_MARK_SWAPPED) {
    ByteSwapped.داخل_کرو(ByteSwapped.انتہا(), Src, Src_انتہا);
    برائے (UTF32 &I : ByteSwapped)
      I = llvm::byteswap<uint32_t>(I);
    Src = &ByteSwapped[0];
    Src_انتہا = &ByteSwapped[ByteSwapped.سائز_2() - 1] + 1;
  }

  // Skip the BOM for conversion.
  اگر (Src[0] == UNI_UTF32_BYTE_ترتیب_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  برآمد.resize(Src_بائٹیں.سائز_2() * UNI_MAX_UTF8_بائٹیں_فی_رمز_نقطہ + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&برآمد[0]);
  UTF8 *Dst_انتہا = Dst + برآمد.سائز_2();

  Conversion_نتیجہ CR =
      ConvertUTF32toUTF8(&Src, Src_انتہا, &Dst, Dst_انتہا, strictConversion);
  أكد(CR != ہدف_Exhausted);

  اگر (CR != conversionOK) {
    برآمد.clear();
    واپس جھوٹ;
  }

  برآمد.resize(reinterpret_cast<حرف *>(Dst) - &برآمد[0]);
  برآمد.ڈالو_عقب(0);
  برآمد.نکالو_عقب();
  واپس سچ;
}

منطقی convert_UTF32_مقصد_UTF8_String(ArrayRef<UTF32> Src, std::string &برآمد) {
  واپس convert_UTF32_مقصد_UTF8_String(
      llvm::ArrayRef<حرف>(reinterpret_cast<ثابت حرف *>(Src.ڈیٹا()),
                           Src.سائز_2() * حجم(UTF32)),
      برآمد);
}

منطقی convert_UTF8_مقصد_UTF16_String(StringRef SrcUTF8,
                              Small_سمتیہ_Impl<UTF16> &DstUTF16) {
  أكد(DstUTF16.خالی_خانے());

  // Avoid OOB by returning early on empty input.
  اگر (SrcUTF8.خالی_خانے()) {
    DstUTF16.ڈالو_عقب(0);
    DstUTF16.نکالو_عقب();
    واپس سچ;
  }

  ثابت UTF8 *Src = reinterpret_cast<ثابت UTF8 *>(SrcUTF8.begin());
  ثابت UTF8 *Src_انتہا = reinterpret_cast<ثابت UTF8 *>(SrcUTF8.انتہا());

  // Allocate the same number of UTF-16 code units as UTF-8 code units. Encoding
  // as UTF-16 should always require the same amount or less code units than the
  // UTF-8 encoding.  Allocate one extra byte for the null terminator though,
  // so that someone calling DstUTF16.data() gets a null terminated string.
  // We resize down later so we don't have to worry that this over allocates.
  DstUTF16.resize(SrcUTF8.سائز_2()+1);
  UTF16 *Dst = &DstUTF16[0];
  UTF16 *Dst_انتہا = Dst + DstUTF16.سائز_2();

  Conversion_نتیجہ CR =
      ConvertUTF8toUTF16(&Src, Src_انتہا, &Dst, Dst_انتہا, strictConversion);
  أكد(CR != ہدف_Exhausted);

  اگر (CR != conversionOK) {
    DstUTF16.clear();
    واپس جھوٹ;
  }

  DstUTF16.resize(Dst - &DstUTF16[0]);
  DstUTF16.ڈالو_عقب(0);
  DstUTF16.نکالو_عقب();
  واپس سچ;
}

static_assert(حجم(wchar_t) == 1 || حجم(wchar_t) == 2 ||
                  حجم(wchar_t) == 4,
              "Expected wchar_t to be 1, 2, or 4 bytes");

سانچہ <قسم_نام TResult>
ساکن اندرونی منطقی Convert_UTF8to_Wide_اندرونی(llvm::StringRef ماخذ,
                                             TResult &نتیجہ) {
  // Even in the case of UTF-16, the number of bytes in a UTF-8 string is
  // at least as large as the number of elements in the resulting wide
  // string, because surrogate pairs take at least 4 bytes in UTF-8.
  نتیجہ.resize(ماخذ.سائز_2() + 1);
  حرف *نتیجہ_Ptr = reinterpret_cast<حرف *>(&نتیجہ[0]);
  ثابت UTF8 *ErrorPtr;
  اگر (!ConvertUTF8toWide(حجم(wchar_t), ماخذ, نتیجہ_Ptr, ErrorPtr)) {
    نتیجہ.clear();
    واپس جھوٹ;
  }
  نتیجہ.resize(reinterpret_cast<wchar_t *>(نتیجہ_Ptr) - &نتیجہ[0]);
  واپس سچ;
}

منطقی ConvertUTF8toWide(llvm::StringRef ماخذ, std::wstring &نتیجہ) {
  واپس Convert_UTF8to_Wide_اندرونی(ماخذ, نتیجہ);
}

منطقی ConvertUTF8toWide(ثابت حرف *ماخذ, std::wstring &نتیجہ) {
  اگر (!ماخذ) {
    نتیجہ.clear();
    واپس سچ;
  }
  واپس ConvertUTF8toWide(llvm::StringRef(ماخذ), نتیجہ);
}

منطقی convert_Wide_مقصد_UTF8(std::wstring_view ماخذ, std::string &نتیجہ) {
  اگر (حجم(wchar_t) == 1) {
    ثابت UTF8 *آغاز = reinterpret_cast<ثابت UTF8 *>(ماخذ.ڈیٹا());
    ثابت UTF8 *انتہا_2 =
        reinterpret_cast<ثابت UTF8 *>(ماخذ.ڈیٹا() + ماخذ.سائز_2());
    اگر (!isLegalUTF8String(&آغاز, انتہا_2))
      واپس جھوٹ;
    نتیجہ.resize(ماخذ.سائز_2());
    حافظہ_نقل(&نتیجہ[0], ماخذ.ڈیٹا(), ماخذ.سائز_2());
    واپس سچ;
  } ورنہ اگر (حجم(wchar_t) == 2) {
    واپس convert_UTF16_مقصد_UTF8_String(
        llvm::ArrayRef<UTF16>(reinterpret_cast<ثابت UTF16 *>(ماخذ.ڈیٹا()),
                              ماخذ.سائز_2()),
        نتیجہ);
  } ورنہ اگر (حجم(wchar_t) == 4) {
    ثابت UTF32 *آغاز = reinterpret_cast<ثابت UTF32 *>(ماخذ.ڈیٹا());
    ثابت UTF32 *انتہا_2 =
        reinterpret_cast<ثابت UTF32 *>(ماخذ.ڈیٹا() + ماخذ.سائز_2());
    نتیجہ.resize(UNI_MAX_UTF8_بائٹیں_فی_رمز_نقطہ * ماخذ.سائز_2());
    UTF8 *نتیجہ_Ptr = reinterpret_cast<UTF8 *>(&نتیجہ[0]);
    UTF8 *نتیجہ_انتہا = reinterpret_cast<UTF8 *>(&نتیجہ[0] + نتیجہ.سائز_2());
    اگر (ConvertUTF32toUTF8(&آغاز, انتہا_2, &نتیجہ_Ptr, نتیجہ_انتہا,
                           strictConversion) == conversionOK) {
      نتیجہ.resize(reinterpret_cast<حرف *>(نتیجہ_Ptr) - &نتیجہ[0]);
      واپس سچ;
    } ورنہ {
      نتیجہ.clear();
      واپس جھوٹ;
    }
  } ورنہ {
    llvm_unreachable(
        "Control should never reach this point; see static_assert further up");
  }
}

منطقی Is_Single_رمز_Unit_UTF8_Codepoint(غیرعلامتی V) { واپس V <= 0x7F; }

منطقی Is_Single_رمز_Unit_UTF16_Codepoint(غیرعلامتی V) {
  واپس V <= 0xD7FF || (V >= 0xE000 && V <= 0xFFFF);
}

منطقی Is_Single_رمز_Unit_UTF32_Codepoint(غیرعلامتی V) {
  واپس V <= 0xD7FF || (V >= 0xE000 && V <= 0x10FFFF);
}

} // end namespace llvm

