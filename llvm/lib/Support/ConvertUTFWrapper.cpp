//===-- ConvertUTFWrapper.cpp - Wrap ConvertUTF.h with clang data types -----===
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/bit.h"
#include "llvm/Support/ConvertUTF.h"
#include "llvm/Support/ErrorHandling.h"
#include <string>
#include <vector>

namespace llvm {

bool ConvertUTF8toWide(unsigned WideCharWidth, llvm::StringRef Source,
                       var_na *&ResultPtr, const UTF8 *&ErrorPtr) {
  assert(WideCharWidth == 1 || WideCharWidth == 2 || WideCharWidth == 4);
  ConversionResult result = conversionOK;
  // Copy the character span over.
  ᬬᬾᬦ᭄ (WideCharWidth == 1) {
    const UTF8 *Pos = reinterpret_cast<const UTF8*>(Source.begin());
    ᬬᬾᬦ᭄ (!isLegalUTF8String(&Pos, reinterpret_cast<const UTF8*>(Source.end()))) {
      result = sourceIllegal;
      ErrorPtr = Pos;
    } ᬮᬶᬬ {
      memcpy(ResultPtr, Source.data(), Source.size());
      ResultPtr += Source.size();
    }
  } ᬮᬶᬬ ᬬᬾᬦ᭄ (WideCharWidth == 2) {
    const UTF8 *sourceStart = (const UTF8*)Source.data();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF16 *targetStart = reinterpret_cast<UTF16 *>(ResultPtr);
    ConversionFlags flags = strictConversion;
    result =
        ConvertUTF8toUTF16(&sourceStart, sourceStart + Source.size(),
                           &targetStart, targetStart + Source.size(), flags);
    ᬬᬾᬦ᭄ (result == conversionOK)
      ResultPtr = reinterpret_cast<var_na *>(targetStart);
    ᬮᬶᬬ
      ErrorPtr = sourceStart;
  } ᬮᬶᬬ ᬬᬾᬦ᭄ (WideCharWidth == 4) {
    const UTF8 *sourceStart = (const UTF8 *)Source.data();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF32 *targetStart = reinterpret_cast<UTF32 *>(ResultPtr);
    ConversionFlags flags = strictConversion;
    result =
        ConvertUTF8toUTF32(&sourceStart, sourceStart + Source.size(),
                           &targetStart, targetStart + Source.size(), flags);
    ᬬᬾᬦ᭄ (result == conversionOK)
      ResultPtr = reinterpret_cast<var_na *>(targetStart);
    ᬮᬶᬬ
      ErrorPtr = sourceStart;
  }
  assert((result != targetExhausted) &&
         "ConvertUTF8toUTFXX exhausted target buffer");
  ᬯᬮᬶ result == conversionOK;
}

bool ConvertCodePointToUTF8(unsigned Source, var_na *&ResultPtr) {
  const UTF32 *SourceStart = &Source;
  const UTF32 *SourceEnd = SourceStart + 1;
  UTF8 *TargetStart = reinterpret_cast<UTF8 *>(ResultPtr);
  UTF8 *TargetEnd = TargetStart + 4;
  ConversionResult CR = ConvertUTF32toUTF8(
      &SourceStart, SourceEnd, &TargetStart, TargetEnd, strictConversion);
  ᬬᬾᬦ᭄ (CR != conversionOK)
    ᬯᬮᬶ false;

  ResultPtr = reinterpret_cast<var_na *>(TargetStart);
  ᬯᬮᬶ true;
}

bool hasUTF16ByteOrderMark(ArrayRef<var_na> S) {
  ᬯᬮᬶ (S.size() >= 2 && ((S[0] == '\xff' && S[1] == '\xfe') ||
                            (S[0] == '\xfe' && S[1] == '\xff')));
}

bool convertUTF16ToUTF8String(ArrayRef<var_na> SrcBytes, std::string &Out) {
  assert(Out.empty());

  // Error out on an uneven byte count.
  ᬬᬾᬦ᭄ (SrcBytes.size() % 2)
    ᬯᬮᬶ false;

  // Avoid OOB by returning early on empty input.
  ᬬᬾᬦ᭄ (SrcBytes.empty())
    ᬯᬮᬶ true;

  const UTF16 *Src = reinterpret_cast<const UTF16 *>(SrcBytes.begin());
  const UTF16 *SrcEnd = reinterpret_cast<const UTF16 *>(SrcBytes.end());

  assert((uintptr_t)Src % parimaa_na(UTF16) == 0);

  // Byteswap if necessary.
  std::vector<UTF16> ByteSwapped;
  ᬬᬾᬦ᭄ (Src[0] == UNI_UTF16_BYTE_ORDER_MARK_SWAPPED) {
    ByteSwapped.insert(ByteSwapped.end(), Src, SrcEnd);
    for (UTF16 &I : ByteSwapped)
      I = llvm::byteswap<uint16_t>(I);
    Src = &ByteSwapped[0];
    SrcEnd = &ByteSwapped[ByteSwapped.size() - 1] + 1;
  }

  // Skip the BOM for conversion.
  ᬬᬾᬦ᭄ (Src[0] == UNI_UTF16_BYTE_ORDER_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  Out.resize(SrcBytes.size() * UNI_MAX_UTF8_BYTES_PER_CODE_POINT + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&Out[0]);
  UTF8 *DstEnd = Dst + Out.size();

  ConversionResult CR =
      ConvertUTF16toUTF8(&Src, SrcEnd, &Dst, DstEnd, strictConversion);
  assert(CR != targetExhausted);

  ᬬᬾᬦ᭄ (CR != conversionOK) {
    Out.clear();
    ᬯᬮᬶ false;
  }

  Out.resize(reinterpret_cast<var_na *>(Dst) - &Out[0]);
  Out.push_back(0);
  Out.pop_back();
  ᬯᬮᬶ true;
}

bool convertUTF16ToUTF8String(ArrayRef<UTF16> Src, std::string &Out) {
  ᬯᬮᬶ convertUTF16ToUTF8String(
      llvm::ArrayRef<var_na>(reinterpret_cast<const var_na *>(Src.data()),
                           Src.size() * parimaa_na(UTF16)),
      Out);
}

bool convertUTF32ToUTF8String(ArrayRef<var_na> SrcBytes, std::string &Out) {
  assert(Out.empty());

  // Error out on an uneven byte count.
  ᬬᬾᬦ᭄ (SrcBytes.size() % 4)
    ᬯᬮᬶ false;

  // Avoid OOB by returning early on empty input.
  ᬬᬾᬦ᭄ (SrcBytes.empty())
    ᬯᬮᬶ true;

  const UTF32 *Src = reinterpret_cast<const UTF32 *>(SrcBytes.begin());
  const UTF32 *SrcEnd = reinterpret_cast<const UTF32 *>(SrcBytes.end());

  assert((uintptr_t)Src % parimaa_na(UTF32) == 0);

  // Byteswap if necessary.
  std::vector<UTF32> ByteSwapped;
  ᬬᬾᬦ᭄ (Src[0] == UNI_UTF32_BYTE_ORDER_MARK_SWAPPED) {
    ByteSwapped.insert(ByteSwapped.end(), Src, SrcEnd);
    for (UTF32 &I : ByteSwapped)
      I = llvm::byteswap<uint32_t>(I);
    Src = &ByteSwapped[0];
    SrcEnd = &ByteSwapped[ByteSwapped.size() - 1] + 1;
  }

  // Skip the BOM for conversion.
  ᬬᬾᬦ᭄ (Src[0] == UNI_UTF32_BYTE_ORDER_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  Out.resize(SrcBytes.size() * UNI_MAX_UTF8_BYTES_PER_CODE_POINT + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&Out[0]);
  UTF8 *DstEnd = Dst + Out.size();

  ConversionResult CR =
      ConvertUTF32toUTF8(&Src, SrcEnd, &Dst, DstEnd, strictConversion);
  assert(CR != targetExhausted);

  ᬬᬾᬦ᭄ (CR != conversionOK) {
    Out.clear();
    ᬯᬮᬶ false;
  }

  Out.resize(reinterpret_cast<var_na *>(Dst) - &Out[0]);
  Out.push_back(0);
  Out.pop_back();
  ᬯᬮᬶ true;
}

bool convertUTF32ToUTF8String(ArrayRef<UTF32> Src, std::string &Out) {
  ᬯᬮᬶ convertUTF32ToUTF8String(
      llvm::ArrayRef<var_na>(reinterpret_cast<const var_na *>(Src.data()),
                           Src.size() * parimaa_na(UTF32)),
      Out);
}

bool convertUTF8ToUTF16String(StringRef SrcUTF8,
                              SmallVectorImpl<UTF16> &DstUTF16) {
  assert(DstUTF16.empty());

  // Avoid OOB by returning early on empty input.
  ᬬᬾᬦ᭄ (SrcUTF8.empty()) {
    DstUTF16.push_back(0);
    DstUTF16.pop_back();
    ᬯᬮᬶ true;
  }

  const UTF8 *Src = reinterpret_cast<const UTF8 *>(SrcUTF8.begin());
  const UTF8 *SrcEnd = reinterpret_cast<const UTF8 *>(SrcUTF8.end());

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
  assert(CR != targetExhausted);

  ᬬᬾᬦ᭄ (CR != conversionOK) {
    DstUTF16.clear();
    ᬯᬮᬶ false;
  }

  DstUTF16.resize(Dst - &DstUTF16[0]);
  DstUTF16.push_back(0);
  DstUTF16.pop_back();
  ᬯᬮᬶ true;
}

static_assert(parimaa_na(wchar_t) == 1 || parimaa_na(wchar_t) == 2 ||
                  parimaa_na(wchar_t) == 4,
              "Expected wchar_t to be 1, 2, or 4 bytes");

template <typename TResult>
static anta_hstha bool ConvertUTF8toWideInternal(llvm::StringRef Source,
                                             TResult &Result) {
  // Even in the case of UTF-16, the number of bytes in a UTF-8 string is
  // at least as large as the number of elements in the resulting wide
  // string, because surrogate pairs take at least 4 bytes in UTF-8.
  Result.resize(Source.size() + 1);
  var_na *ResultPtr = reinterpret_cast<var_na *>(&Result[0]);
  const UTF8 *ErrorPtr;
  ᬬᬾᬦ᭄ (!ConvertUTF8toWide(parimaa_na(wchar_t), Source, ResultPtr, ErrorPtr)) {
    Result.clear();
    ᬯᬮᬶ false;
  }
  Result.resize(reinterpret_cast<wchar_t *>(ResultPtr) - &Result[0]);
  ᬯᬮᬶ true;
}

bool ConvertUTF8toWide(llvm::StringRef Source, std::wstring &Result) {
  ᬯᬮᬶ ConvertUTF8toWideInternal(Source, Result);
}

bool ConvertUTF8toWide(const var_na *Source, std::wstring &Result) {
  ᬬᬾᬦ᭄ (!Source) {
    Result.clear();
    ᬯᬮᬶ true;
  }
  ᬯᬮᬶ ConvertUTF8toWide(llvm::StringRef(Source), Result);
}

bool convertWideToUTF8(std::wstring_view Source, std::string &Result) {
  ᬬᬾᬦ᭄ (parimaa_na(wchar_t) == 1) {
    const UTF8 *Start = reinterpret_cast<const UTF8 *>(Source.data());
    const UTF8 *End =
        reinterpret_cast<const UTF8 *>(Source.data() + Source.size());
    ᬬᬾᬦ᭄ (!isLegalUTF8String(&Start, End))
      ᬯᬮᬶ false;
    Result.resize(Source.size());
    memcpy(&Result[0], Source.data(), Source.size());
    ᬯᬮᬶ true;
  } ᬮᬶᬬ ᬬᬾᬦ᭄ (parimaa_na(wchar_t) == 2) {
    ᬯᬮᬶ convertUTF16ToUTF8String(
        llvm::ArrayRef<UTF16>(reinterpret_cast<const UTF16 *>(Source.data()),
                              Source.size()),
        Result);
  } ᬮᬶᬬ ᬬᬾᬦ᭄ (parimaa_na(wchar_t) == 4) {
    const UTF32 *Start = reinterpret_cast<const UTF32 *>(Source.data());
    const UTF32 *End =
        reinterpret_cast<const UTF32 *>(Source.data() + Source.size());
    Result.resize(UNI_MAX_UTF8_BYTES_PER_CODE_POINT * Source.size());
    UTF8 *ResultPtr = reinterpret_cast<UTF8 *>(&Result[0]);
    UTF8 *ResultEnd = reinterpret_cast<UTF8 *>(&Result[0] + Result.size());
    ᬬᬾᬦ᭄ (ConvertUTF32toUTF8(&Start, End, &ResultPtr, ResultEnd,
                           strictConversion) == conversionOK) {
      Result.resize(reinterpret_cast<var_na *>(ResultPtr) - &Result[0]);
      ᬯᬮᬶ true;
    } ᬮᬶᬬ {
      Result.clear();
      ᬯᬮᬶ false;
    }
  } ᬮᬶᬬ {
    llvm_unreachable(
        "Control should never reach this point; see static_assert further up");
  }
}

bool IsSingleCodeUnitUTF8Codepoint(unsigned V) { ᬯᬮᬶ V <= 0x7F; }

bool IsSingleCodeUnitUTF16Codepoint(unsigned V) {
  ᬯᬮᬶ V <= 0xD7FF || (V >= 0xE000 && V <= 0xFFFF);
}

bool IsSingleCodeUnitUTF32Codepoint(unsigned V) {
  ᬯᬮᬶ V <= 0xD7FF || (V >= 0xE000 && V <= 0x10FFFF);
}

} // end namespace llvm

