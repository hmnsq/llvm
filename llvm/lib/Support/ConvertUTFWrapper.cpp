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

منطقي ConvertUTF8toWide(بینښه Wide_Char_پلنوالی, llvm::StringRef سرچینه,
                       حرف *&پایله_Ptr, ثابت UTF8 *&ErrorPtr) {
  أكد(Wide_Char_پلنوالی == 1 || Wide_Char_پلنوالی == 2 || Wide_Char_پلنوالی == 4);
  Conversion_پایله پایله_2 = conversionOK;
  // Copy the character span over.
  إذا (Wide_Char_پلنوالی == 1) {
    ثابت UTF8 *Pos = reinterpret_cast<ثابت UTF8*>(سرچینه.begin());
    إذا (!isLegalUTF8String(&Pos, reinterpret_cast<ثابت UTF8*>(سرچینه.پای_ټکی()))) {
      پایله_2 = سرچینه_Illegal;
      ErrorPtr = Pos;
    } وإلا {
      حافظه_نقل(پایله_Ptr, سرچینه.معلومات(), سرچینه.اندازه_2());
      پایله_Ptr += سرچینه.اندازه_2();
    }
  } وإلا إذا (Wide_Char_پلنوالی == 2) {
    ثابت UTF8 *سرچینه_پیل = (ثابت UTF8*)سرچینه.معلومات();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF16 *موخه_پیل = reinterpret_cast<UTF16 *>(پایله_Ptr);
    ConversionFlags flags = strictConversion;
    پایله_2 =
        ConvertUTF8toUTF16(&سرچینه_پیل, سرچینه_پیل + سرچینه.اندازه_2(),
                           &موخه_پیل, موخه_پیل + سرچینه.اندازه_2(), flags);
    إذا (پایله_2 == conversionOK)
      پایله_Ptr = reinterpret_cast<حرف *>(موخه_پیل);
    وإلا
      ErrorPtr = سرچینه_پیل;
  } وإلا إذا (Wide_Char_پلنوالی == 4) {
    ثابت UTF8 *سرچینه_پیل = (ثابت UTF8 *)سرچینه.معلومات();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF32 *موخه_پیل = reinterpret_cast<UTF32 *>(پایله_Ptr);
    ConversionFlags flags = strictConversion;
    پایله_2 =
        ConvertUTF8toUTF32(&سرچینه_پیل, سرچینه_پیل + سرچینه.اندازه_2(),
                           &موخه_پیل, موخه_پیل + سرچینه.اندازه_2(), flags);
    إذا (پایله_2 == conversionOK)
      پایله_Ptr = reinterpret_cast<حرف *>(موخه_پیل);
    وإلا
      ErrorPtr = سرچینه_پیل;
  }
  أكد((پایله_2 != موخه_Exhausted) &&
         "ConvertUTF8toUTFXX exhausted target buffer");
  أرجع پایله_2 == conversionOK;
}

منطقي Convert_کوډ_ټکی_منزل_UTF8(بینښه سرچینه, حرف *&پایله_Ptr) {
  ثابت UTF32 *سرچینه_پیل_2 = &سرچینه;
  ثابت UTF32 *سرچینه_پای_ټکی_2 = سرچینه_پیل_2 + 1;
  UTF8 *موخه_پیل_2 = reinterpret_cast<UTF8 *>(پایله_Ptr);
  UTF8 *موخه_پای_ټکی_2 = موخه_پیل_2 + 4;
  Conversion_پایله CR = ConvertUTF32toUTF8(
      &سرچینه_پیل_2, سرچینه_پای_ټکی_2, &موخه_پیل_2, موخه_پای_ټکی_2, strictConversion);
  إذا (CR != conversionOK)
    أرجع خطأ;

  پایله_Ptr = reinterpret_cast<حرف *>(موخه_پیل_2);
  أرجع ریښتیا;
}

منطقي has_UTF16_Byte_ترتیب_Mark(ArrayRef<حرف> S) {
  أرجع (S.اندازه_2() >= 2 && ((S[0] == '\xff' && S[1] == '\xfe') ||
                            (S[0] == '\xfe' && S[1] == '\xff')));
}

منطقي convert_UTF16_منزل_UTF8_String(ArrayRef<حرف> Src_بایټونه, std::string &بهر) {
  أكد(بهر.تشه());

  // Error out on an uneven byte count.
  إذا (Src_بایټونه.اندازه_2() % 2)
    أرجع خطأ;

  // Avoid OOB by returning early on empty input.
  إذا (Src_بایټونه.تشه())
    أرجع ریښتیا;

  ثابت UTF16 *Src = reinterpret_cast<ثابت UTF16 *>(Src_بایټونه.begin());
  ثابت UTF16 *Src_پای_ټکی = reinterpret_cast<ثابت UTF16 *>(Src_بایټونه.پای_ټکی());

  أكد((uintptr_t)Src % حجم(UTF16) == 0);

  // Byteswap if necessary.
  std::ویکټور<UTF16> ByteSwapped;
  إذا (Src[0] == UNI_UTF16_BYTE_ترتیب_MARK_SWAPPED) {
    ByteSwapped.ورننباسه(ByteSwapped.پای_ټکی(), Src, Src_پای_ټکی);
    لكل (UTF16 &I : ByteSwapped)
      I = llvm::byteswap<uint16_t>(I);
    Src = &ByteSwapped[0];
    Src_پای_ټکی = &ByteSwapped[ByteSwapped.اندازه_2() - 1] + 1;
  }

  // Skip the BOM for conversion.
  إذا (Src[0] == UNI_UTF16_BYTE_ترتیب_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  بهر.resize(Src_بایټونه.اندازه_2() * UNI_MAX_UTF8_بایټونه_فی_کوډ_ټکی + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&بهر[0]);
  UTF8 *Dst_پای_ټکی = Dst + بهر.اندازه_2();

  Conversion_پایله CR =
      ConvertUTF16toUTF8(&Src, Src_پای_ټکی, &Dst, Dst_پای_ټکی, strictConversion);
  أكد(CR != موخه_Exhausted);

  إذا (CR != conversionOK) {
    بهر.clear();
    أرجع خطأ;
  }

  بهر.resize(reinterpret_cast<حرف *>(Dst) - &بهر[0]);
  بهر.ټېل_شا(0);
  بهر.راوباسه_شا();
  أرجع ریښتیا;
}

منطقي convert_UTF16_منزل_UTF8_String(ArrayRef<UTF16> Src, std::string &بهر) {
  أرجع convert_UTF16_منزل_UTF8_String(
      llvm::ArrayRef<حرف>(reinterpret_cast<ثابت حرف *>(Src.معلومات()),
                           Src.اندازه_2() * حجم(UTF16)),
      بهر);
}

منطقي convert_UTF32_منزل_UTF8_String(ArrayRef<حرف> Src_بایټونه, std::string &بهر) {
  أكد(بهر.تشه());

  // Error out on an uneven byte count.
  إذا (Src_بایټونه.اندازه_2() % 4)
    أرجع خطأ;

  // Avoid OOB by returning early on empty input.
  إذا (Src_بایټونه.تشه())
    أرجع ریښتیا;

  ثابت UTF32 *Src = reinterpret_cast<ثابت UTF32 *>(Src_بایټونه.begin());
  ثابت UTF32 *Src_پای_ټکی = reinterpret_cast<ثابت UTF32 *>(Src_بایټونه.پای_ټکی());

  أكد((uintptr_t)Src % حجم(UTF32) == 0);

  // Byteswap if necessary.
  std::ویکټور<UTF32> ByteSwapped;
  إذا (Src[0] == UNI_UTF32_BYTE_ترتیب_MARK_SWAPPED) {
    ByteSwapped.ورننباسه(ByteSwapped.پای_ټکی(), Src, Src_پای_ټکی);
    لكل (UTF32 &I : ByteSwapped)
      I = llvm::byteswap<uint32_t>(I);
    Src = &ByteSwapped[0];
    Src_پای_ټکی = &ByteSwapped[ByteSwapped.اندازه_2() - 1] + 1;
  }

  // Skip the BOM for conversion.
  إذا (Src[0] == UNI_UTF32_BYTE_ترتیب_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  بهر.resize(Src_بایټونه.اندازه_2() * UNI_MAX_UTF8_بایټونه_فی_کوډ_ټکی + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&بهر[0]);
  UTF8 *Dst_پای_ټکی = Dst + بهر.اندازه_2();

  Conversion_پایله CR =
      ConvertUTF32toUTF8(&Src, Src_پای_ټکی, &Dst, Dst_پای_ټکی, strictConversion);
  أكد(CR != موخه_Exhausted);

  إذا (CR != conversionOK) {
    بهر.clear();
    أرجع خطأ;
  }

  بهر.resize(reinterpret_cast<حرف *>(Dst) - &بهر[0]);
  بهر.ټېل_شا(0);
  بهر.راوباسه_شا();
  أرجع ریښتیا;
}

منطقي convert_UTF32_منزل_UTF8_String(ArrayRef<UTF32> Src, std::string &بهر) {
  أرجع convert_UTF32_منزل_UTF8_String(
      llvm::ArrayRef<حرف>(reinterpret_cast<ثابت حرف *>(Src.معلومات()),
                           Src.اندازه_2() * حجم(UTF32)),
      بهر);
}

منطقي convert_UTF8_منزل_UTF16_String(StringRef SrcUTF8,
                              Small_ویکټور_Impl<UTF16> &DstUTF16) {
  أكد(DstUTF16.تشه());

  // Avoid OOB by returning early on empty input.
  إذا (SrcUTF8.تشه()) {
    DstUTF16.ټېل_شا(0);
    DstUTF16.راوباسه_شا();
    أرجع ریښتیا;
  }

  ثابت UTF8 *Src = reinterpret_cast<ثابت UTF8 *>(SrcUTF8.begin());
  ثابت UTF8 *Src_پای_ټکی = reinterpret_cast<ثابت UTF8 *>(SrcUTF8.پای_ټکی());

  // Allocate the same number of UTF-16 code units as UTF-8 code units. Encoding
  // as UTF-16 should always require the same amount or less code units than the
  // UTF-8 encoding.  Allocate one extra byte for the null terminator though,
  // so that someone calling DstUTF16.data() gets a null terminated string.
  // We resize down later so we don't have to worry that this over allocates.
  DstUTF16.resize(SrcUTF8.اندازه_2()+1);
  UTF16 *Dst = &DstUTF16[0];
  UTF16 *Dst_پای_ټکی = Dst + DstUTF16.اندازه_2();

  Conversion_پایله CR =
      ConvertUTF8toUTF16(&Src, Src_پای_ټکی, &Dst, Dst_پای_ټکی, strictConversion);
  أكد(CR != موخه_Exhausted);

  إذا (CR != conversionOK) {
    DstUTF16.clear();
    أرجع خطأ;
  }

  DstUTF16.resize(Dst - &DstUTF16[0]);
  DstUTF16.ټېل_شا(0);
  DstUTF16.راوباسه_شا();
  أرجع ریښتیا;
}

static_assert(حجم(wchar_t) == 1 || حجم(wchar_t) == 2 ||
                  حجم(wchar_t) == 4,
              "Expected wchar_t to be 1, 2, or 4 bytes");

کينډۍ <ډول_نوم TResult>
ساكن مضمن منطقي Convert_UTF8to_Wide_دننی(llvm::StringRef سرچینه,
                                             TResult &پایله) {
  // Even in the case of UTF-16, the number of bytes in a UTF-8 string is
  // at least as large as the number of elements in the resulting wide
  // string, because surrogate pairs take at least 4 bytes in UTF-8.
  پایله.resize(سرچینه.اندازه_2() + 1);
  حرف *پایله_Ptr = reinterpret_cast<حرف *>(&پایله[0]);
  ثابت UTF8 *ErrorPtr;
  إذا (!ConvertUTF8toWide(حجم(wchar_t), سرچینه, پایله_Ptr, ErrorPtr)) {
    پایله.clear();
    أرجع خطأ;
  }
  پایله.resize(reinterpret_cast<wchar_t *>(پایله_Ptr) - &پایله[0]);
  أرجع ریښتیا;
}

منطقي ConvertUTF8toWide(llvm::StringRef سرچینه, std::wstring &پایله) {
  أرجع Convert_UTF8to_Wide_دننی(سرچینه, پایله);
}

منطقي ConvertUTF8toWide(ثابت حرف *سرچینه, std::wstring &پایله) {
  إذا (!سرچینه) {
    پایله.clear();
    أرجع ریښتیا;
  }
  أرجع ConvertUTF8toWide(llvm::StringRef(سرچینه), پایله);
}

منطقي convert_Wide_منزل_UTF8(std::wstring_view سرچینه, std::string &پایله) {
  إذا (حجم(wchar_t) == 1) {
    ثابت UTF8 *پیل = reinterpret_cast<ثابت UTF8 *>(سرچینه.معلومات());
    ثابت UTF8 *پای_ټکی_2 =
        reinterpret_cast<ثابت UTF8 *>(سرچینه.معلومات() + سرچینه.اندازه_2());
    إذا (!isLegalUTF8String(&پیل, پای_ټکی_2))
      أرجع خطأ;
    پایله.resize(سرچینه.اندازه_2());
    حافظه_نقل(&پایله[0], سرچینه.معلومات(), سرچینه.اندازه_2());
    أرجع ریښتیا;
  } وإلا إذا (حجم(wchar_t) == 2) {
    أرجع convert_UTF16_منزل_UTF8_String(
        llvm::ArrayRef<UTF16>(reinterpret_cast<ثابت UTF16 *>(سرچینه.معلومات()),
                              سرچینه.اندازه_2()),
        پایله);
  } وإلا إذا (حجم(wchar_t) == 4) {
    ثابت UTF32 *پیل = reinterpret_cast<ثابت UTF32 *>(سرچینه.معلومات());
    ثابت UTF32 *پای_ټکی_2 =
        reinterpret_cast<ثابت UTF32 *>(سرچینه.معلومات() + سرچینه.اندازه_2());
    پایله.resize(UNI_MAX_UTF8_بایټونه_فی_کوډ_ټکی * سرچینه.اندازه_2());
    UTF8 *پایله_Ptr = reinterpret_cast<UTF8 *>(&پایله[0]);
    UTF8 *پایله_پای_ټکی = reinterpret_cast<UTF8 *>(&پایله[0] + پایله.اندازه_2());
    إذا (ConvertUTF32toUTF8(&پیل, پای_ټکی_2, &پایله_Ptr, پایله_پای_ټکی,
                           strictConversion) == conversionOK) {
      پایله.resize(reinterpret_cast<حرف *>(پایله_Ptr) - &پایله[0]);
      أرجع ریښتیا;
    } وإلا {
      پایله.clear();
      أرجع خطأ;
    }
  } وإلا {
    llvm_unreachable(
        "Control should never reach this point; see static_assert further up");
  }
}

منطقي Is_Single_کوډ_Unit_UTF8_Codepoint(بینښه V) { أرجع V <= 0x7F; }

منطقي Is_Single_کوډ_Unit_UTF16_Codepoint(بینښه V) {
  أرجع V <= 0xD7FF || (V >= 0xE000 && V <= 0xFFFF);
}

منطقي Is_Single_کوډ_Unit_UTF32_Codepoint(بینښه V) {
  أرجع V <= 0xD7FF || (V >= 0xE000 && V <= 0x10FFFF);
}

} // end namespace llvm

