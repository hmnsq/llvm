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

منطقي ConvertUTF8toWide(unsigned WideCharWidth, llvm::StringRef Source,
                       حرف *&ResultPtr, ثابت UTF8 *&ErrorPtr) {
  أكد(WideCharWidth == 1 || WideCharWidth == 2 || WideCharWidth == 4);
  ConversionResult result = conversionOK;
  // Copy the character span over.
  إذا (WideCharWidth == 1) {
    ثابت UTF8 *Pos = reinterpret_cast<ثابت UTF8*>(Source.begin());
    إذا (!isLegalUTF8String(&Pos, reinterpret_cast<ثابت UTF8*>(Source.end()))) {
      result = sourceIllegal;
      ErrorPtr = Pos;
    } وإلا {
      memcpy(ResultPtr, Source.data(), Source.size());
      ResultPtr += Source.size();
    }
  } وإلا إذا (WideCharWidth == 2) {
    ثابت UTF8 *sourceStart = (ثابت UTF8*)Source.data();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF16 *targetStart = reinterpret_cast<UTF16 *>(ResultPtr);
    ConversionFlags flags = strictConversion;
    result =
        ConvertUTF8toUTF16(&sourceStart, sourceStart + Source.size(),
                           &targetStart, targetStart + Source.size(), flags);
    إذا (result == conversionOK)
      ResultPtr = reinterpret_cast<حرف *>(targetStart);
    وإلا
      ErrorPtr = sourceStart;
  } وإلا إذا (WideCharWidth == 4) {
    ثابت UTF8 *sourceStart = (ثابت UTF8 *)Source.data();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF32 *targetStart = reinterpret_cast<UTF32 *>(ResultPtr);
    ConversionFlags flags = strictConversion;
    result =
        ConvertUTF8toUTF32(&sourceStart, sourceStart + Source.size(),
                           &targetStart, targetStart + Source.size(), flags);
    إذا (result == conversionOK)
      ResultPtr = reinterpret_cast<حرف *>(targetStart);
    وإلا
      ErrorPtr = sourceStart;
  }
  أكد((result != targetExhausted) &&
         "ConvertUTF8toUTFXX exhausted target buffer");
  أرجع result == conversionOK;
}

منطقي ConvertCodePointToUTF8(unsigned Source, حرف *&ResultPtr) {
  ثابت UTF32 *SourceStart = &Source;
  ثابت UTF32 *SourceEnd = SourceStart + 1;
  UTF8 *TargetStart = reinterpret_cast<UTF8 *>(ResultPtr);
  UTF8 *TargetEnd = TargetStart + 4;
  ConversionResult CR = ConvertUTF32toUTF8(
      &SourceStart, SourceEnd, &TargetStart, TargetEnd, strictConversion);
  إذا (CR != conversionOK)
    أرجع خطأ;

  ResultPtr = reinterpret_cast<حرف *>(TargetStart);
  أرجع true;
}

منطقي hasUTF16ByteOrderMark(ArrayRef<حرف> S) {
  أرجع (S.size() >= 2 && ((S[0] == '\xff' && S[1] == '\xfe') ||
                            (S[0] == '\xfe' && S[1] == '\xff')));
}

منطقي convertUTF16ToUTF8String(ArrayRef<حرف> SrcBytes, std::string &Out) {
  أكد(Out.empty());

  // Error out on an uneven byte count.
  إذا (SrcBytes.size() % 2)
    أرجع خطأ;

  // Avoid OOB by returning early on empty input.
  إذا (SrcBytes.empty())
    أرجع true;

  ثابت UTF16 *Src = reinterpret_cast<ثابت UTF16 *>(SrcBytes.begin());
  ثابت UTF16 *SrcEnd = reinterpret_cast<ثابت UTF16 *>(SrcBytes.end());

  أكد((uintptr_t)Src % حجم(UTF16) == 0);

  // Byteswap if necessary.
  std::vector<UTF16> ByteSwapped;
  إذا (Src[0] == UNI_UTF16_BYTE_ORDER_MARK_SWAPPED) {
    ByteSwapped.سيسيڤ(ByteSwapped.end(), Src, SrcEnd);
    لكل (UTF16 &I : ByteSwapped)
      I = llvm::byteswap<uint16_t>(I);
    Src = &ByteSwapped[0];
    SrcEnd = &ByteSwapped[ByteSwapped.size() - 1] + 1;
  }

  // Skip the BOM for conversion.
  إذا (Src[0] == UNI_UTF16_BYTE_ORDER_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  Out.resize(SrcBytes.size() * UNI_MAX_UTF8_BYTES_PER_CODE_POINT + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&Out[0]);
  UTF8 *DstEnd = Dst + Out.size();

  ConversionResult CR =
      ConvertUTF16toUTF8(&Src, SrcEnd, &Dst, DstEnd, strictConversion);
  أكد(CR != targetExhausted);

  إذا (CR != conversionOK) {
    Out.clear();
    أرجع خطأ;
  }

  Out.resize(reinterpret_cast<حرف *>(Dst) - &Out[0]);
  Out.تولق_back(0);
  Out.کلوار_back();
  أرجع true;
}

منطقي convertUTF16ToUTF8String(ArrayRef<UTF16> Src, std::string &Out) {
  أرجع convertUTF16ToUTF8String(
      llvm::ArrayRef<حرف>(reinterpret_cast<ثابت حرف *>(Src.data()),
                           Src.size() * حجم(UTF16)),
      Out);
}

منطقي convertUTF32ToUTF8String(ArrayRef<حرف> SrcBytes, std::string &Out) {
  أكد(Out.empty());

  // Error out on an uneven byte count.
  إذا (SrcBytes.size() % 4)
    أرجع خطأ;

  // Avoid OOB by returning early on empty input.
  إذا (SrcBytes.empty())
    أرجع true;

  ثابت UTF32 *Src = reinterpret_cast<ثابت UTF32 *>(SrcBytes.begin());
  ثابت UTF32 *SrcEnd = reinterpret_cast<ثابت UTF32 *>(SrcBytes.end());

  أكد((uintptr_t)Src % حجم(UTF32) == 0);

  // Byteswap if necessary.
  std::vector<UTF32> ByteSwapped;
  إذا (Src[0] == UNI_UTF32_BYTE_ORDER_MARK_SWAPPED) {
    ByteSwapped.سيسيڤ(ByteSwapped.end(), Src, SrcEnd);
    لكل (UTF32 &I : ByteSwapped)
      I = llvm::byteswap<uint32_t>(I);
    Src = &ByteSwapped[0];
    SrcEnd = &ByteSwapped[ByteSwapped.size() - 1] + 1;
  }

  // Skip the BOM for conversion.
  إذا (Src[0] == UNI_UTF32_BYTE_ORDER_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  Out.resize(SrcBytes.size() * UNI_MAX_UTF8_BYTES_PER_CODE_POINT + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&Out[0]);
  UTF8 *DstEnd = Dst + Out.size();

  ConversionResult CR =
      ConvertUTF32toUTF8(&Src, SrcEnd, &Dst, DstEnd, strictConversion);
  أكد(CR != targetExhausted);

  إذا (CR != conversionOK) {
    Out.clear();
    أرجع خطأ;
  }

  Out.resize(reinterpret_cast<حرف *>(Dst) - &Out[0]);
  Out.تولق_back(0);
  Out.کلوار_back();
  أرجع true;
}

منطقي convertUTF32ToUTF8String(ArrayRef<UTF32> Src, std::string &Out) {
  أرجع convertUTF32ToUTF8String(
      llvm::ArrayRef<حرف>(reinterpret_cast<ثابت حرف *>(Src.data()),
                           Src.size() * حجم(UTF32)),
      Out);
}

منطقي convertUTF8ToUTF16String(StringRef SrcUTF8,
                              SmallVectorImpl<UTF16> &DstUTF16) {
  أكد(DstUTF16.empty());

  // Avoid OOB by returning early on empty input.
  إذا (SrcUTF8.empty()) {
    DstUTF16.تولق_back(0);
    DstUTF16.کلوار_back();
    أرجع true;
  }

  ثابت UTF8 *Src = reinterpret_cast<ثابت UTF8 *>(SrcUTF8.begin());
  ثابت UTF8 *SrcEnd = reinterpret_cast<ثابت UTF8 *>(SrcUTF8.end());

  // Allocate the same number of UTF-16 code units as UTF-8 code units. Encoding
  // as UTF-16 should always require the same amount or less code units than the
  // UTF-8 encoding.  Allocate one extra byte for the null terminator though,
  // so that someone calling DstUTF16.data() gets a null terminated string.
  // We resize down later so we don't have to worry that this over allocates.
  DstUTF16.resize(SrcUTF8.size()+1);
  UTF16 *Dst = &DstUTF16[0];
  UTF16 *DstEnd = Dst + DstUTF16.size();

  ConversionResult CR =
      ConvertUTF8toUTF16(&Src, SrcEnd, &Dst, DstEnd, strictConversion);
  أكد(CR != targetExhausted);

  إذا (CR != conversionOK) {
    DstUTF16.clear();
    أرجع خطأ;
  }

  DstUTF16.resize(Dst - &DstUTF16[0]);
  DstUTF16.تولق_back(0);
  DstUTF16.کلوار_back();
  أرجع true;
}

static_assert(حجم(wchar_t) == 1 || حجم(wchar_t) == 2 ||
                  حجم(wchar_t) == 4,
              "Expected wchar_t to be 1, 2, or 4 bytes");

template <typename TResult>
ساكن مضمن منطقي ConvertUTF8toWideInternal(llvm::StringRef Source,
                                             TResult &Result) {
  // Even in the case of UTF-16, the number of bytes in a UTF-8 string is
  // at least as large as the number of elements in the resulting wide
  // string, because surrogate pairs take at least 4 bytes in UTF-8.
  Result.resize(Source.size() + 1);
  حرف *ResultPtr = reinterpret_cast<حرف *>(&Result[0]);
  ثابت UTF8 *ErrorPtr;
  إذا (!ConvertUTF8toWide(حجم(wchar_t), Source, ResultPtr, ErrorPtr)) {
    Result.clear();
    أرجع خطأ;
  }
  Result.resize(reinterpret_cast<wchar_t *>(ResultPtr) - &Result[0]);
  أرجع true;
}

منطقي ConvertUTF8toWide(llvm::StringRef Source, std::wstring &Result) {
  أرجع ConvertUTF8toWideInternal(Source, Result);
}

منطقي ConvertUTF8toWide(ثابت حرف *Source, std::wstring &Result) {
  إذا (!Source) {
    Result.clear();
    أرجع true;
  }
  أرجع ConvertUTF8toWide(llvm::StringRef(Source), Result);
}

منطقي convertWideToUTF8(std::wstring_view Source, std::string &Result) {
  إذا (حجم(wchar_t) == 1) {
    ثابت UTF8 *Start = reinterpret_cast<ثابت UTF8 *>(Source.data());
    ثابت UTF8 *End =
        reinterpret_cast<ثابت UTF8 *>(Source.data() + Source.size());
    إذا (!isLegalUTF8String(&Start, End))
      أرجع خطأ;
    Result.resize(Source.size());
    memcpy(&Result[0], Source.data(), Source.size());
    أرجع true;
  } وإلا إذا (حجم(wchar_t) == 2) {
    أرجع convertUTF16ToUTF8String(
        llvm::ArrayRef<UTF16>(reinterpret_cast<ثابت UTF16 *>(Source.data()),
                              Source.size()),
        Result);
  } وإلا إذا (حجم(wchar_t) == 4) {
    ثابت UTF32 *Start = reinterpret_cast<ثابت UTF32 *>(Source.data());
    ثابت UTF32 *End =
        reinterpret_cast<ثابت UTF32 *>(Source.data() + Source.size());
    Result.resize(UNI_MAX_UTF8_BYTES_PER_CODE_POINT * Source.size());
    UTF8 *ResultPtr = reinterpret_cast<UTF8 *>(&Result[0]);
    UTF8 *ResultEnd = reinterpret_cast<UTF8 *>(&Result[0] + Result.size());
    إذا (ConvertUTF32toUTF8(&Start, End, &ResultPtr, ResultEnd,
                           strictConversion) == conversionOK) {
      Result.resize(reinterpret_cast<حرف *>(ResultPtr) - &Result[0]);
      أرجع true;
    } وإلا {
      Result.clear();
      أرجع خطأ;
    }
  } وإلا {
    llvm_unreachable(
        "Control should never reach this point; see static_assert further up");
  }
}

منطقي IsSingleCodeUnitUTF8Codepoint(unsigned V) { أرجع V <= 0x7F; }

منطقي IsSingleCodeUnitUTF16Codepoint(unsigned V) {
  أرجع V <= 0xD7FF || (V >= 0xE000 && V <= 0xFFFF);
}

منطقي IsSingleCodeUnitUTF32Codepoint(unsigned V) {
  أرجع V <= 0xD7FF || (V >= 0xE000 && V <= 0x10FFFF);
}

} // end namespace llvm

