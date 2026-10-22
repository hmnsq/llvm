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

فضاینام llvm {

منطقی ConvertUTF8toWide(unsigned WideCharWidth, llvm::StringRef Source,
                       نویسه *&ResultPtr, ثابت UTF8 *&ErrorPtr) {
  أكد(WideCharWidth == 1 || WideCharWidth == 2 || WideCharWidth == 4);
  ConversionResult result = conversionOK;
  // Copy the character span over.
  اگر (WideCharWidth == 1) {
    ثابت UTF8 *Pos = reinterpret_cast<ثابت UTF8*>(Source.begin());
    اگر (!isLegalUTF8String(&Pos, reinterpret_cast<ثابت UTF8*>(Source.end()))) {
      result = sourceIllegal;
      ErrorPtr = Pos;
    } وگرنه {
      memcpy(ResultPtr, Source.data(), Source.size());
      ResultPtr += Source.size();
    }
  } وگرنه اگر (WideCharWidth == 2) {
    ثابت UTF8 *sourceStart = (ثابت UTF8*)Source.data();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF16 *targetStart = reinterpret_cast<UTF16 *>(ResultPtr);
    ConversionFlags flags = strictConversion;
    result =
        ConvertUTF8toUTF16(&sourceStart, sourceStart + Source.size(),
                           &targetStart, targetStart + Source.size(), flags);
    اگر (result == conversionOK)
      ResultPtr = reinterpret_cast<نویسه *>(targetStart);
    وگرنه
      ErrorPtr = sourceStart;
  } وگرنه اگر (WideCharWidth == 4) {
    ثابت UTF8 *sourceStart = (ثابت UTF8 *)Source.data();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF32 *targetStart = reinterpret_cast<UTF32 *>(ResultPtr);
    ConversionFlags flags = strictConversion;
    result =
        ConvertUTF8toUTF32(&sourceStart, sourceStart + Source.size(),
                           &targetStart, targetStart + Source.size(), flags);
    اگر (result == conversionOK)
      ResultPtr = reinterpret_cast<نویسه *>(targetStart);
    وگرنه
      ErrorPtr = sourceStart;
  }
  أكد((result != targetExhausted) &&
         "ConvertUTF8toUTFXX exhausted target buffer");
  بازگشت result == conversionOK;
}

منطقی ConvertCodePointToUTF8(unsigned Source, نویسه *&ResultPtr) {
  ثابت UTF32 *SourceStart = &Source;
  ثابت UTF32 *SourceEnd = SourceStart + 1;
  UTF8 *TargetStart = reinterpret_cast<UTF8 *>(ResultPtr);
  UTF8 *TargetEnd = TargetStart + 4;
  ConversionResult CR = ConvertUTF32toUTF8(
      &SourceStart, SourceEnd, &TargetStart, TargetEnd, strictConversion);
  اگر (CR != conversionOK)
    بازگشت نادرست;

  ResultPtr = reinterpret_cast<نویسه *>(TargetStart);
  بازگشت درست;
}

منطقی hasUTF16ByteOrderMark(ArrayRef<نویسه> S) {
  بازگشت (S.size() >= 2 && ((S[0] == '\xff' && S[1] == '\xfe') ||
                            (S[0] == '\xfe' && S[1] == '\xff')));
}

منطقی convertUTF16ToUTF8String(ArrayRef<نویسه> SrcBytes, std::string &Out) {
  أكد(Out.empty());

  // Error out on an uneven byte count.
  اگر (SrcBytes.size() % 2)
    بازگشت نادرست;

  // Avoid OOB by returning early on empty input.
  اگر (SrcBytes.empty())
    بازگشت درست;

  ثابت UTF16 *Src = reinterpret_cast<ثابت UTF16 *>(SrcBytes.begin());
  ثابت UTF16 *SrcEnd = reinterpret_cast<ثابت UTF16 *>(SrcBytes.end());

  أكد((uintptr_t)Src % اندازه(UTF16) == 0);

  // Byteswap if necessary.
  std::vector<UTF16> ByteSwapped;
  اگر (Src[0] == UNI_UTF16_BYTE_ORDER_MARK_SWAPPED) {
    ByteSwapped.درج(ByteSwapped.end(), Src, SrcEnd);
    برای (UTF16 &I : ByteSwapped)
      I = llvm::byteswap<uint16_t>(I);
    Src = &ByteSwapped[0];
    SrcEnd = &ByteSwapped[ByteSwapped.size() - 1] + 1;
  }

  // Skip the BOM for conversion.
  اگر (Src[0] == UNI_UTF16_BYTE_ORDER_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  Out.resize(SrcBytes.size() * UNI_MAX_UTF8_BYTES_PER_CODE_POINT + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&Out[0]);
  UTF8 *DstEnd = Dst + Out.size();

  ConversionResult CR =
      ConvertUTF16toUTF8(&Src, SrcEnd, &Dst, DstEnd, strictConversion);
  أكد(CR != targetExhausted);

  اگر (CR != conversionOK) {
    Out.clear();
    بازگشت نادرست;
  }

  Out.resize(reinterpret_cast<نویسه *>(Dst) - &Out[0]);
  Out.افزودن_back(0);
  Out.برداشت_back();
  بازگشت درست;
}

منطقی convertUTF16ToUTF8String(ArrayRef<UTF16> Src, std::string &Out) {
  بازگشت convertUTF16ToUTF8String(
      llvm::ArrayRef<نویسه>(reinterpret_cast<ثابت نویسه *>(Src.data()),
                           Src.size() * اندازه(UTF16)),
      Out);
}

منطقی convertUTF32ToUTF8String(ArrayRef<نویسه> SrcBytes, std::string &Out) {
  أكد(Out.empty());

  // Error out on an uneven byte count.
  اگر (SrcBytes.size() % 4)
    بازگشت نادرست;

  // Avoid OOB by returning early on empty input.
  اگر (SrcBytes.empty())
    بازگشت درست;

  ثابت UTF32 *Src = reinterpret_cast<ثابت UTF32 *>(SrcBytes.begin());
  ثابت UTF32 *SrcEnd = reinterpret_cast<ثابت UTF32 *>(SrcBytes.end());

  أكد((uintptr_t)Src % اندازه(UTF32) == 0);

  // Byteswap if necessary.
  std::vector<UTF32> ByteSwapped;
  اگر (Src[0] == UNI_UTF32_BYTE_ORDER_MARK_SWAPPED) {
    ByteSwapped.درج(ByteSwapped.end(), Src, SrcEnd);
    برای (UTF32 &I : ByteSwapped)
      I = llvm::byteswap<uint32_t>(I);
    Src = &ByteSwapped[0];
    SrcEnd = &ByteSwapped[ByteSwapped.size() - 1] + 1;
  }

  // Skip the BOM for conversion.
  اگر (Src[0] == UNI_UTF32_BYTE_ORDER_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  Out.resize(SrcBytes.size() * UNI_MAX_UTF8_BYTES_PER_CODE_POINT + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&Out[0]);
  UTF8 *DstEnd = Dst + Out.size();

  ConversionResult CR =
      ConvertUTF32toUTF8(&Src, SrcEnd, &Dst, DstEnd, strictConversion);
  أكد(CR != targetExhausted);

  اگر (CR != conversionOK) {
    Out.clear();
    بازگشت نادرست;
  }

  Out.resize(reinterpret_cast<نویسه *>(Dst) - &Out[0]);
  Out.افزودن_back(0);
  Out.برداشت_back();
  بازگشت درست;
}

منطقی convertUTF32ToUTF8String(ArrayRef<UTF32> Src, std::string &Out) {
  بازگشت convertUTF32ToUTF8String(
      llvm::ArrayRef<نویسه>(reinterpret_cast<ثابت نویسه *>(Src.data()),
                           Src.size() * اندازه(UTF32)),
      Out);
}

منطقی convertUTF8ToUTF16String(StringRef SrcUTF8,
                              SmallVectorImpl<UTF16> &DstUTF16) {
  أكد(DstUTF16.empty());

  // Avoid OOB by returning early on empty input.
  اگر (SrcUTF8.empty()) {
    DstUTF16.افزودن_back(0);
    DstUTF16.برداشت_back();
    بازگشت درست;
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

  اگر (CR != conversionOK) {
    DstUTF16.clear();
    بازگشت نادرست;
  }

  DstUTF16.resize(Dst - &DstUTF16[0]);
  DstUTF16.افزودن_back(0);
  DstUTF16.برداشت_back();
  بازگشت درست;
}

static_assert(اندازه(wchar_t) == 1 || اندازه(wchar_t) == 2 ||
                  اندازه(wchar_t) == 4,
              "Expected wchar_t to be 1, 2, or 4 bytes");

template <typename TResult>
ایستا درونخطی منطقی ConvertUTF8toWideInternal(llvm::StringRef Source,
                                             TResult &Result) {
  // Even in the case of UTF-16, the number of bytes in a UTF-8 string is
  // at least as large as the number of elements in the resulting wide
  // string, because surrogate pairs take at least 4 bytes in UTF-8.
  Result.resize(Source.size() + 1);
  نویسه *ResultPtr = reinterpret_cast<نویسه *>(&Result[0]);
  ثابت UTF8 *ErrorPtr;
  اگر (!ConvertUTF8toWide(اندازه(wchar_t), Source, ResultPtr, ErrorPtr)) {
    Result.clear();
    بازگشت نادرست;
  }
  Result.resize(reinterpret_cast<wchar_t *>(ResultPtr) - &Result[0]);
  بازگشت درست;
}

منطقی ConvertUTF8toWide(llvm::StringRef Source, std::wstring &Result) {
  بازگشت ConvertUTF8toWideInternal(Source, Result);
}

منطقی ConvertUTF8toWide(ثابت نویسه *Source, std::wstring &Result) {
  اگر (!Source) {
    Result.clear();
    بازگشت درست;
  }
  بازگشت ConvertUTF8toWide(llvm::StringRef(Source), Result);
}

منطقی convertWideToUTF8(std::wstring_view Source, std::string &Result) {
  اگر (اندازه(wchar_t) == 1) {
    ثابت UTF8 *Start = reinterpret_cast<ثابت UTF8 *>(Source.data());
    ثابت UTF8 *End =
        reinterpret_cast<ثابت UTF8 *>(Source.data() + Source.size());
    اگر (!isLegalUTF8String(&Start, End))
      بازگشت نادرست;
    Result.resize(Source.size());
    memcpy(&Result[0], Source.data(), Source.size());
    بازگشت درست;
  } وگرنه اگر (اندازه(wchar_t) == 2) {
    بازگشت convertUTF16ToUTF8String(
        llvm::ArrayRef<UTF16>(reinterpret_cast<ثابت UTF16 *>(Source.data()),
                              Source.size()),
        Result);
  } وگرنه اگر (اندازه(wchar_t) == 4) {
    ثابت UTF32 *Start = reinterpret_cast<ثابت UTF32 *>(Source.data());
    ثابت UTF32 *End =
        reinterpret_cast<ثابت UTF32 *>(Source.data() + Source.size());
    Result.resize(UNI_MAX_UTF8_BYTES_PER_CODE_POINT * Source.size());
    UTF8 *ResultPtr = reinterpret_cast<UTF8 *>(&Result[0]);
    UTF8 *ResultEnd = reinterpret_cast<UTF8 *>(&Result[0] + Result.size());
    اگر (ConvertUTF32toUTF8(&Start, End, &ResultPtr, ResultEnd,
                           strictConversion) == conversionOK) {
      Result.resize(reinterpret_cast<نویسه *>(ResultPtr) - &Result[0]);
      بازگشت درست;
    } وگرنه {
      Result.clear();
      بازگشت نادرست;
    }
  } وگرنه {
    llvm_unreachable(
        "Control should never reach this point; see static_assert further up");
  }
}

منطقی IsSingleCodeUnitUTF8Codepoint(unsigned V) { بازگشت V <= 0x7F; }

منطقی IsSingleCodeUnitUTF16Codepoint(unsigned V) {
  بازگشت V <= 0xD7FF || (V >= 0xE000 && V <= 0xFFFF);
}

منطقی IsSingleCodeUnitUTF32Codepoint(unsigned V) {
  بازگشت V <= 0xD7FF || (V >= 0xE000 && V <= 0x10FFFF);
}

} // end namespace llvm

