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

منطقي ConvertUTF8toWide(بنانشان Wide_Char_ويڪر, llvm::StringRef ذريعو,
                       حرف *&نتيجو_Ptr, ثابت UTF8 *&ErrorPtr) {
  أكد(Wide_Char_ويڪر == 1 || Wide_Char_ويڪر == 2 || Wide_Char_ويڪر == 4);
  Conversion_نتيجو نتيجو_2 = conversionOK;
  // Copy the character span over.
  إذا (Wide_Char_ويڪر == 1) {
    ثابت UTF8 *Pos = reinterpret_cast<ثابت UTF8*>(ذريعو.begin());
    إذا (!isLegalUTF8String(&Pos, reinterpret_cast<ثابت UTF8*>(ذريعو.پڇاڙي()))) {
      نتيجو_2 = ذريعو_Illegal;
      ErrorPtr = Pos;
    } وإلا {
      يادگيري_نقل(نتيجو_Ptr, ذريعو.معلومات(), ذريعو.سائيز_2());
      نتيجو_Ptr += ذريعو.سائيز_2();
    }
  } وإلا إذا (Wide_Char_ويڪر == 2) {
    ثابت UTF8 *ذريعو_شروعات = (ثابت UTF8*)ذريعو.معلومات();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF16 *هدف_شروعات = reinterpret_cast<UTF16 *>(نتيجو_Ptr);
    ConversionFlags flags = strictConversion;
    نتيجو_2 =
        ConvertUTF8toUTF16(&ذريعو_شروعات, ذريعو_شروعات + ذريعو.سائيز_2(),
                           &هدف_شروعات, هدف_شروعات + ذريعو.سائيز_2(), flags);
    إذا (نتيجو_2 == conversionOK)
      نتيجو_Ptr = reinterpret_cast<حرف *>(هدف_شروعات);
    وإلا
      ErrorPtr = ذريعو_شروعات;
  } وإلا إذا (Wide_Char_ويڪر == 4) {
    ثابت UTF8 *ذريعو_شروعات = (ثابت UTF8 *)ذريعو.معلومات();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF32 *هدف_شروعات = reinterpret_cast<UTF32 *>(نتيجو_Ptr);
    ConversionFlags flags = strictConversion;
    نتيجو_2 =
        ConvertUTF8toUTF32(&ذريعو_شروعات, ذريعو_شروعات + ذريعو.سائيز_2(),
                           &هدف_شروعات, هدف_شروعات + ذريعو.سائيز_2(), flags);
    إذا (نتيجو_2 == conversionOK)
      نتيجو_Ptr = reinterpret_cast<حرف *>(هدف_شروعات);
    وإلا
      ErrorPtr = ذريعو_شروعات;
  }
  أكد((نتيجو_2 != هدف_Exhausted) &&
         "ConvertUTF8toUTFXX exhausted target buffer");
  أرجع نتيجو_2 == conversionOK;
}

منطقي Convert_ڪوڊ_نقطو_منزل_UTF8(بنانشان ذريعو, حرف *&نتيجو_Ptr) {
  ثابت UTF32 *ذريعو_شروعات_2 = &ذريعو;
  ثابت UTF32 *ذريعو_پڇاڙي_2 = ذريعو_شروعات_2 + 1;
  UTF8 *هدف_شروعات_2 = reinterpret_cast<UTF8 *>(نتيجو_Ptr);
  UTF8 *هدف_پڇاڙي_2 = هدف_شروعات_2 + 4;
  Conversion_نتيجو CR = ConvertUTF32toUTF8(
      &ذريعو_شروعات_2, ذريعو_پڇاڙي_2, &هدف_شروعات_2, هدف_پڇاڙي_2, strictConversion);
  إذا (CR != conversionOK)
    أرجع خطأ;

  نتيجو_Ptr = reinterpret_cast<حرف *>(هدف_شروعات_2);
  أرجع سچ;
}

منطقي has_UTF16_Byte_ترتيب_Mark(ArrayRef<حرف> S) {
  أرجع (S.سائيز_2() >= 2 && ((S[0] == '\xff' && S[1] == '\xfe') ||
                            (S[0] == '\xfe' && S[1] == '\xff')));
}

منطقي convert_UTF16_منزل_UTF8_String(ArrayRef<حرف> Src_بائيٽون, std::string &ٻاهريون) {
  أكد(ٻاهريون.خالي_جايون());

  // Error out on an uneven byte count.
  إذا (Src_بائيٽون.سائيز_2() % 2)
    أرجع خطأ;

  // Avoid OOB by returning early on empty input.
  إذا (Src_بائيٽون.خالي_جايون())
    أرجع سچ;

  ثابت UTF16 *Src = reinterpret_cast<ثابت UTF16 *>(Src_بائيٽون.begin());
  ثابت UTF16 *Src_پڇاڙي = reinterpret_cast<ثابت UTF16 *>(Src_بائيٽون.پڇاڙي());

  أكد((uintptr_t)Src % حجم(UTF16) == 0);

  // Byteswap if necessary.
  std::ويڪٽر<UTF16> ByteSwapped;
  إذا (Src[0] == UNI_UTF16_BYTE_ترتيب_MARK_SWAPPED) {
    ByteSwapped.وجهو(ByteSwapped.پڇاڙي(), Src, Src_پڇاڙي);
    لكل (UTF16 &I : ByteSwapped)
      I = llvm::byteswap<uint16_t>(I);
    Src = &ByteSwapped[0];
    Src_پڇاڙي = &ByteSwapped[ByteSwapped.سائيز_2() - 1] + 1;
  }

  // Skip the BOM for conversion.
  إذا (Src[0] == UNI_UTF16_BYTE_ترتيب_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  ٻاهريون.resize(Src_بائيٽون.سائيز_2() * UNI_MAX_UTF8_بائيٽون_في_ڪوڊ_نقطو + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&ٻاهريون[0]);
  UTF8 *Dst_پڇاڙي = Dst + ٻاهريون.سائيز_2();

  Conversion_نتيجو CR =
      ConvertUTF16toUTF8(&Src, Src_پڇاڙي, &Dst, Dst_پڇاڙي, strictConversion);
  أكد(CR != هدف_Exhausted);

  إذا (CR != conversionOK) {
    ٻاهريون.clear();
    أرجع خطأ;
  }

  ٻاهريون.resize(reinterpret_cast<حرف *>(Dst) - &ٻاهريون[0]);
  ٻاهريون.ڌڪو_پٺ(0);
  ٻاهريون.ڪڍو_پٺ();
  أرجع سچ;
}

منطقي convert_UTF16_منزل_UTF8_String(ArrayRef<UTF16> Src, std::string &ٻاهريون) {
  أرجع convert_UTF16_منزل_UTF8_String(
      llvm::ArrayRef<حرف>(reinterpret_cast<ثابت حرف *>(Src.معلومات()),
                           Src.سائيز_2() * حجم(UTF16)),
      ٻاهريون);
}

منطقي convert_UTF32_منزل_UTF8_String(ArrayRef<حرف> Src_بائيٽون, std::string &ٻاهريون) {
  أكد(ٻاهريون.خالي_جايون());

  // Error out on an uneven byte count.
  إذا (Src_بائيٽون.سائيز_2() % 4)
    أرجع خطأ;

  // Avoid OOB by returning early on empty input.
  إذا (Src_بائيٽون.خالي_جايون())
    أرجع سچ;

  ثابت UTF32 *Src = reinterpret_cast<ثابت UTF32 *>(Src_بائيٽون.begin());
  ثابت UTF32 *Src_پڇاڙي = reinterpret_cast<ثابت UTF32 *>(Src_بائيٽون.پڇاڙي());

  أكد((uintptr_t)Src % حجم(UTF32) == 0);

  // Byteswap if necessary.
  std::ويڪٽر<UTF32> ByteSwapped;
  إذا (Src[0] == UNI_UTF32_BYTE_ترتيب_MARK_SWAPPED) {
    ByteSwapped.وجهو(ByteSwapped.پڇاڙي(), Src, Src_پڇاڙي);
    لكل (UTF32 &I : ByteSwapped)
      I = llvm::byteswap<uint32_t>(I);
    Src = &ByteSwapped[0];
    Src_پڇاڙي = &ByteSwapped[ByteSwapped.سائيز_2() - 1] + 1;
  }

  // Skip the BOM for conversion.
  إذا (Src[0] == UNI_UTF32_BYTE_ترتيب_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  ٻاهريون.resize(Src_بائيٽون.سائيز_2() * UNI_MAX_UTF8_بائيٽون_في_ڪوڊ_نقطو + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&ٻاهريون[0]);
  UTF8 *Dst_پڇاڙي = Dst + ٻاهريون.سائيز_2();

  Conversion_نتيجو CR =
      ConvertUTF32toUTF8(&Src, Src_پڇاڙي, &Dst, Dst_پڇاڙي, strictConversion);
  أكد(CR != هدف_Exhausted);

  إذا (CR != conversionOK) {
    ٻاهريون.clear();
    أرجع خطأ;
  }

  ٻاهريون.resize(reinterpret_cast<حرف *>(Dst) - &ٻاهريون[0]);
  ٻاهريون.ڌڪو_پٺ(0);
  ٻاهريون.ڪڍو_پٺ();
  أرجع سچ;
}

منطقي convert_UTF32_منزل_UTF8_String(ArrayRef<UTF32> Src, std::string &ٻاهريون) {
  أرجع convert_UTF32_منزل_UTF8_String(
      llvm::ArrayRef<حرف>(reinterpret_cast<ثابت حرف *>(Src.معلومات()),
                           Src.سائيز_2() * حجم(UTF32)),
      ٻاهريون);
}

منطقي convert_UTF8_منزل_UTF16_String(StringRef SrcUTF8,
                              Small_ويڪٽر_Impl<UTF16> &DstUTF16) {
  أكد(DstUTF16.خالي_جايون());

  // Avoid OOB by returning early on empty input.
  إذا (SrcUTF8.خالي_جايون()) {
    DstUTF16.ڌڪو_پٺ(0);
    DstUTF16.ڪڍو_پٺ();
    أرجع سچ;
  }

  ثابت UTF8 *Src = reinterpret_cast<ثابت UTF8 *>(SrcUTF8.begin());
  ثابت UTF8 *Src_پڇاڙي = reinterpret_cast<ثابت UTF8 *>(SrcUTF8.پڇاڙي());

  // Allocate the same number of UTF-16 code units as UTF-8 code units. Encoding
  // as UTF-16 should always require the same amount or less code units than the
  // UTF-8 encoding.  Allocate one extra byte for the null terminator though,
  // so that someone calling DstUTF16.data() gets a null terminated string.
  // We resize down later so we don't have to worry that this over allocates.
  DstUTF16.resize(SrcUTF8.سائيز_2()+1);
  UTF16 *Dst = &DstUTF16[0];
  UTF16 *Dst_پڇاڙي = Dst + DstUTF16.سائيز_2();

  Conversion_نتيجو CR =
      ConvertUTF8toUTF16(&Src, Src_پڇاڙي, &Dst, Dst_پڇاڙي, strictConversion);
  أكد(CR != هدف_Exhausted);

  إذا (CR != conversionOK) {
    DstUTF16.clear();
    أرجع خطأ;
  }

  DstUTF16.resize(Dst - &DstUTF16[0]);
  DstUTF16.ڌڪو_پٺ(0);
  DstUTF16.ڪڍو_پٺ();
  أرجع سچ;
}

static_assert(حجم(wchar_t) == 1 || حجم(wchar_t) == 2 ||
                  حجم(wchar_t) == 4,
              "Expected wchar_t to be 1, 2, or 4 bytes");

سانچو <قسم_نالو TResult>
ساكن مضمن منطقي Convert_UTF8to_Wide_اندروني(llvm::StringRef ذريعو,
                                             TResult &نتيجو) {
  // Even in the case of UTF-16, the number of bytes in a UTF-8 string is
  // at least as large as the number of elements in the resulting wide
  // string, because surrogate pairs take at least 4 bytes in UTF-8.
  نتيجو.resize(ذريعو.سائيز_2() + 1);
  حرف *نتيجو_Ptr = reinterpret_cast<حرف *>(&نتيجو[0]);
  ثابت UTF8 *ErrorPtr;
  إذا (!ConvertUTF8toWide(حجم(wchar_t), ذريعو, نتيجو_Ptr, ErrorPtr)) {
    نتيجو.clear();
    أرجع خطأ;
  }
  نتيجو.resize(reinterpret_cast<wchar_t *>(نتيجو_Ptr) - &نتيجو[0]);
  أرجع سچ;
}

منطقي ConvertUTF8toWide(llvm::StringRef ذريعو, std::wstring &نتيجو) {
  أرجع Convert_UTF8to_Wide_اندروني(ذريعو, نتيجو);
}

منطقي ConvertUTF8toWide(ثابت حرف *ذريعو, std::wstring &نتيجو) {
  إذا (!ذريعو) {
    نتيجو.clear();
    أرجع سچ;
  }
  أرجع ConvertUTF8toWide(llvm::StringRef(ذريعو), نتيجو);
}

منطقي convert_Wide_منزل_UTF8(std::wstring_view ذريعو, std::string &نتيجو) {
  إذا (حجم(wchar_t) == 1) {
    ثابت UTF8 *شروعات = reinterpret_cast<ثابت UTF8 *>(ذريعو.معلومات());
    ثابت UTF8 *پڇاڙي_2 =
        reinterpret_cast<ثابت UTF8 *>(ذريعو.معلومات() + ذريعو.سائيز_2());
    إذا (!isLegalUTF8String(&شروعات, پڇاڙي_2))
      أرجع خطأ;
    نتيجو.resize(ذريعو.سائيز_2());
    يادگيري_نقل(&نتيجو[0], ذريعو.معلومات(), ذريعو.سائيز_2());
    أرجع سچ;
  } وإلا إذا (حجم(wchar_t) == 2) {
    أرجع convert_UTF16_منزل_UTF8_String(
        llvm::ArrayRef<UTF16>(reinterpret_cast<ثابت UTF16 *>(ذريعو.معلومات()),
                              ذريعو.سائيز_2()),
        نتيجو);
  } وإلا إذا (حجم(wchar_t) == 4) {
    ثابت UTF32 *شروعات = reinterpret_cast<ثابت UTF32 *>(ذريعو.معلومات());
    ثابت UTF32 *پڇاڙي_2 =
        reinterpret_cast<ثابت UTF32 *>(ذريعو.معلومات() + ذريعو.سائيز_2());
    نتيجو.resize(UNI_MAX_UTF8_بائيٽون_في_ڪوڊ_نقطو * ذريعو.سائيز_2());
    UTF8 *نتيجو_Ptr = reinterpret_cast<UTF8 *>(&نتيجو[0]);
    UTF8 *نتيجو_پڇاڙي = reinterpret_cast<UTF8 *>(&نتيجو[0] + نتيجو.سائيز_2());
    إذا (ConvertUTF32toUTF8(&شروعات, پڇاڙي_2, &نتيجو_Ptr, نتيجو_پڇاڙي,
                           strictConversion) == conversionOK) {
      نتيجو.resize(reinterpret_cast<حرف *>(نتيجو_Ptr) - &نتيجو[0]);
      أرجع سچ;
    } وإلا {
      نتيجو.clear();
      أرجع خطأ;
    }
  } وإلا {
    llvm_unreachable(
        "Control should never reach this point; see static_assert further up");
  }
}

منطقي Is_Single_ڪوڊ_Unit_UTF8_Codepoint(بنانشان V) { أرجع V <= 0x7F; }

منطقي Is_Single_ڪوڊ_Unit_UTF16_Codepoint(بنانشان V) {
  أرجع V <= 0xD7FF || (V >= 0xE000 && V <= 0xFFFF);
}

منطقي Is_Single_ڪوڊ_Unit_UTF32_Codepoint(بنانشان V) {
  أرجع V <= 0xD7FF || (V >= 0xE000 && V <= 0x10FFFF);
}

} // end namespace llvm

