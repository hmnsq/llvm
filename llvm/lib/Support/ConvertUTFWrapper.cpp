//===-- ConvertUTFWrapper.cpp - Wrap ConvertUTF.h with clang data types -----===
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/ADT/ArrayRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/bit.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ConvertUTF.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ErrorHandling.h"
#ಅನ್ತರ್ಭಾವ <string>
#ಅನ್ತರ್ಭಾವ <vector>

ನಾಮಾಕಾಶ llvm {

ತಾರ್ಕಿಕ ConvertUTF8toWide(unsigned WideCharWidth, llvm::StringRef Source,
                       ವರ್ಣ *&ResultPtr, ಧ್ರುವ UTF8 *&ErrorPtr) {
  ಪ್ರತಿಜ್ಞಾ(WideCharWidth == 1 || WideCharWidth == 2 || WideCharWidth == 4);
  ConversionResult result = conversionOK;
  // Copy the character span over.
  ಯದಿ (WideCharWidth == 1) {
    ಧ್ರುವ UTF8 *Pos = reinterpret_cast<ಧ್ರುವ UTF8*>(Source.begin());
    ಯದಿ (!isLegalUTF8String(&Pos, reinterpret_cast<ಧ್ರುವ UTF8*>(Source.end()))) {
      result = sourceIllegal;
      ErrorPtr = Pos;
    } ಅನ್ಯಥಾ {
      memcpy(ResultPtr, Source.data(), Source.size());
      ResultPtr += Source.size();
    }
  } ಅನ್ಯಥಾ ಯದಿ (WideCharWidth == 2) {
    ಧ್ರುವ UTF8 *sourceStart = (ಧ್ರುವ UTF8*)Source.data();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF16 *targetStart = reinterpret_cast<UTF16 *>(ResultPtr);
    ConversionFlags flags = strictConversion;
    result =
        ConvertUTF8toUTF16(&sourceStart, sourceStart + Source.size(),
                           &targetStart, targetStart + Source.size(), flags);
    ಯದಿ (result == conversionOK)
      ResultPtr = reinterpret_cast<ವರ್ಣ *>(targetStart);
    ಅನ್ಯಥಾ
      ErrorPtr = sourceStart;
  } ಅನ್ಯಥಾ ಯದಿ (WideCharWidth == 4) {
    ಧ್ರುವ UTF8 *sourceStart = (ಧ್ರುವ UTF8 *)Source.data();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF32 *targetStart = reinterpret_cast<UTF32 *>(ResultPtr);
    ConversionFlags flags = strictConversion;
    result =
        ConvertUTF8toUTF32(&sourceStart, sourceStart + Source.size(),
                           &targetStart, targetStart + Source.size(), flags);
    ಯದಿ (result == conversionOK)
      ResultPtr = reinterpret_cast<ವರ್ಣ *>(targetStart);
    ಅನ್ಯಥಾ
      ErrorPtr = sourceStart;
  }
  ಪ್ರತಿಜ್ಞಾ((result != targetExhausted) &&
         "ConvertUTF8toUTFXX exhausted target buffer");
  ಪ್ರತ್ಯಾಗಮ result == conversionOK;
}

ತಾರ್ಕಿಕ ConvertCodePointToUTF8(unsigned Source, ವರ್ಣ *&ResultPtr) {
  ಧ್ರುವ UTF32 *SourceStart = &Source;
  ಧ್ರುವ UTF32 *SourceEnd = SourceStart + 1;
  UTF8 *TargetStart = reinterpret_cast<UTF8 *>(ResultPtr);
  UTF8 *TargetEnd = TargetStart + 4;
  ConversionResult CR = ConvertUTF32toUTF8(
      &SourceStart, SourceEnd, &TargetStart, TargetEnd, strictConversion);
  ಯದಿ (CR != conversionOK)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  ResultPtr = reinterpret_cast<ವರ್ಣ *>(TargetStart);
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ತಾರ್ಕಿಕ hasUTF16ByteOrderMark(ArrayRef<ವರ್ಣ> S) {
  ಪ್ರತ್ಯಾಗಮ (S.size() >= 2 && ((S[0] == '\xff' && S[1] == '\xfe') ||
                            (S[0] == '\xfe' && S[1] == '\xff')));
}

ತಾರ್ಕಿಕ convertUTF16ToUTF8String(ArrayRef<ವರ್ಣ> SrcBytes, std::string &Out) {
  ಪ್ರತಿಜ್ಞಾ(Out.empty());

  // Error out on an uneven byte count.
  ಯದಿ (SrcBytes.size() % 2)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  // Avoid OOB by returning early on empty input.
  ಯದಿ (SrcBytes.empty())
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

  ಧ್ರುವ UTF16 *Src = reinterpret_cast<ಧ್ರುವ UTF16 *>(SrcBytes.begin());
  ಧ್ರುವ UTF16 *SrcEnd = reinterpret_cast<ಧ್ರುವ UTF16 *>(SrcBytes.end());

  ಪ್ರತಿಜ್ಞಾ((uintptr_t)Src % ಪರಿಮಾಣ(UTF16) == 0);

  // Byteswap if necessary.
  std::vector<UTF16> ByteSwapped;
  ಯದಿ (Src[0] == UNI_UTF16_BYTE_ORDER_MARK_SWAPPED) {
    ByteSwapped.ಸೇರಿಸು(ByteSwapped.end(), Src, SrcEnd);
    ಪ್ರತಿ (UTF16 &I : ByteSwapped)
      I = llvm::byteswap<uint16_t>(I);
    Src = &ByteSwapped[0];
    SrcEnd = &ByteSwapped[ByteSwapped.size() - 1] + 1;
  }

  // Skip the BOM for conversion.
  ಯದಿ (Src[0] == UNI_UTF16_BYTE_ORDER_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  Out.resize(SrcBytes.size() * UNI_MAX_UTF8_BYTES_PER_CODE_POINT + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&Out[0]);
  UTF8 *DstEnd = Dst + Out.size();

  ConversionResult CR =
      ConvertUTF16toUTF8(&Src, SrcEnd, &Dst, DstEnd, strictConversion);
  ಪ್ರತಿಜ್ಞಾ(CR != targetExhausted);

  ಯದಿ (CR != conversionOK) {
    Out.clear();
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }

  Out.resize(reinterpret_cast<ವರ್ಣ *>(Dst) - &Out[0]);
  Out.ತಳ್ಳು_back(0);
  Out.ತೆಗೆ_back();
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ತಾರ್ಕಿಕ convertUTF16ToUTF8String(ArrayRef<UTF16> Src, std::string &Out) {
  ಪ್ರತ್ಯಾಗಮ convertUTF16ToUTF8String(
      llvm::ArrayRef<ವರ್ಣ>(reinterpret_cast<ಧ್ರುವ ವರ್ಣ *>(Src.data()),
                           Src.size() * ಪರಿಮಾಣ(UTF16)),
      Out);
}

ತಾರ್ಕಿಕ convertUTF32ToUTF8String(ArrayRef<ವರ್ಣ> SrcBytes, std::string &Out) {
  ಪ್ರತಿಜ್ಞಾ(Out.empty());

  // Error out on an uneven byte count.
  ಯದಿ (SrcBytes.size() % 4)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  // Avoid OOB by returning early on empty input.
  ಯದಿ (SrcBytes.empty())
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

  ಧ್ರುವ UTF32 *Src = reinterpret_cast<ಧ್ರುವ UTF32 *>(SrcBytes.begin());
  ಧ್ರುವ UTF32 *SrcEnd = reinterpret_cast<ಧ್ರುವ UTF32 *>(SrcBytes.end());

  ಪ್ರತಿಜ್ಞಾ((uintptr_t)Src % ಪರಿಮಾಣ(UTF32) == 0);

  // Byteswap if necessary.
  std::vector<UTF32> ByteSwapped;
  ಯದಿ (Src[0] == UNI_UTF32_BYTE_ORDER_MARK_SWAPPED) {
    ByteSwapped.ಸೇರಿಸು(ByteSwapped.end(), Src, SrcEnd);
    ಪ್ರತಿ (UTF32 &I : ByteSwapped)
      I = llvm::byteswap<uint32_t>(I);
    Src = &ByteSwapped[0];
    SrcEnd = &ByteSwapped[ByteSwapped.size() - 1] + 1;
  }

  // Skip the BOM for conversion.
  ಯದಿ (Src[0] == UNI_UTF32_BYTE_ORDER_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  Out.resize(SrcBytes.size() * UNI_MAX_UTF8_BYTES_PER_CODE_POINT + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&Out[0]);
  UTF8 *DstEnd = Dst + Out.size();

  ConversionResult CR =
      ConvertUTF32toUTF8(&Src, SrcEnd, &Dst, DstEnd, strictConversion);
  ಪ್ರತಿಜ್ಞಾ(CR != targetExhausted);

  ಯದಿ (CR != conversionOK) {
    Out.clear();
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }

  Out.resize(reinterpret_cast<ವರ್ಣ *>(Dst) - &Out[0]);
  Out.ತಳ್ಳು_back(0);
  Out.ತೆಗೆ_back();
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ತಾರ್ಕಿಕ convertUTF32ToUTF8String(ArrayRef<UTF32> Src, std::string &Out) {
  ಪ್ರತ್ಯಾಗಮ convertUTF32ToUTF8String(
      llvm::ArrayRef<ವರ್ಣ>(reinterpret_cast<ಧ್ರುವ ವರ್ಣ *>(Src.data()),
                           Src.size() * ಪರಿಮಾಣ(UTF32)),
      Out);
}

ತಾರ್ಕಿಕ convertUTF8ToUTF16String(StringRef SrcUTF8,
                              SmallVectorImpl<UTF16> &DstUTF16) {
  ಪ್ರತಿಜ್ಞಾ(DstUTF16.empty());

  // Avoid OOB by returning early on empty input.
  ಯದಿ (SrcUTF8.empty()) {
    DstUTF16.ತಳ್ಳು_back(0);
    DstUTF16.ತೆಗೆ_back();
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }

  ಧ್ರುವ UTF8 *Src = reinterpret_cast<ಧ್ರುವ UTF8 *>(SrcUTF8.begin());
  ಧ್ರುವ UTF8 *SrcEnd = reinterpret_cast<ಧ್ರುವ UTF8 *>(SrcUTF8.end());

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
  ಪ್ರತಿಜ್ಞಾ(CR != targetExhausted);

  ಯದಿ (CR != conversionOK) {
    DstUTF16.clear();
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }

  DstUTF16.resize(Dst - &DstUTF16[0]);
  DstUTF16.ತಳ್ಳು_back(0);
  DstUTF16.ತೆಗೆ_back();
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

static_assert(ಪರಿಮಾಣ(wchar_t) == 1 || ಪರಿಮಾಣ(wchar_t) == 2 ||
                  ಪರಿಮಾಣ(wchar_t) == 4,
              "Expected wchar_t to be 1, 2, or 4 bytes");

template <typename TResult>
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ತಾರ್ಕಿಕ ConvertUTF8toWideInternal(llvm::StringRef Source,
                                             TResult &Result) {
  // Even in the case of UTF-16, the number of bytes in a UTF-8 string is
  // at least as large as the number of elements in the resulting wide
  // string, because surrogate pairs take at least 4 bytes in UTF-8.
  Result.resize(Source.size() + 1);
  ವರ್ಣ *ResultPtr = reinterpret_cast<ವರ್ಣ *>(&Result[0]);
  ಧ್ರುವ UTF8 *ErrorPtr;
  ಯದಿ (!ConvertUTF8toWide(ಪರಿಮಾಣ(wchar_t), Source, ResultPtr, ErrorPtr)) {
    Result.clear();
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }
  Result.resize(reinterpret_cast<wchar_t *>(ResultPtr) - &Result[0]);
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ತಾರ್ಕಿಕ ConvertUTF8toWide(llvm::StringRef Source, std::wstring &Result) {
  ಪ್ರತ್ಯಾಗಮ ConvertUTF8toWideInternal(Source, Result);
}

ತಾರ್ಕಿಕ ConvertUTF8toWide(ಧ್ರುವ ವರ್ಣ *Source, std::wstring &Result) {
  ಯದಿ (!Source) {
    Result.clear();
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }
  ಪ್ರತ್ಯಾಗಮ ConvertUTF8toWide(llvm::StringRef(Source), Result);
}

ತಾರ್ಕಿಕ convertWideToUTF8(std::wstring_view Source, std::string &Result) {
  ಯದಿ (ಪರಿಮಾಣ(wchar_t) == 1) {
    ಧ್ರುವ UTF8 *Start = reinterpret_cast<ಧ್ರುವ UTF8 *>(Source.data());
    ಧ್ರುವ UTF8 *End =
        reinterpret_cast<ಧ್ರುವ UTF8 *>(Source.data() + Source.size());
    ಯದಿ (!isLegalUTF8String(&Start, End))
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
    Result.resize(Source.size());
    memcpy(&Result[0], Source.data(), Source.size());
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  } ಅನ್ಯಥಾ ಯದಿ (ಪರಿಮಾಣ(wchar_t) == 2) {
    ಪ್ರತ್ಯಾಗಮ convertUTF16ToUTF8String(
        llvm::ArrayRef<UTF16>(reinterpret_cast<ಧ್ರುವ UTF16 *>(Source.data()),
                              Source.size()),
        Result);
  } ಅನ್ಯಥಾ ಯದಿ (ಪರಿಮಾಣ(wchar_t) == 4) {
    ಧ್ರುವ UTF32 *Start = reinterpret_cast<ಧ್ರುವ UTF32 *>(Source.data());
    ಧ್ರುವ UTF32 *End =
        reinterpret_cast<ಧ್ರುವ UTF32 *>(Source.data() + Source.size());
    Result.resize(UNI_MAX_UTF8_BYTES_PER_CODE_POINT * Source.size());
    UTF8 *ResultPtr = reinterpret_cast<UTF8 *>(&Result[0]);
    UTF8 *ResultEnd = reinterpret_cast<UTF8 *>(&Result[0] + Result.size());
    ಯದಿ (ConvertUTF32toUTF8(&Start, End, &ResultPtr, ResultEnd,
                           strictConversion) == conversionOK) {
      Result.resize(reinterpret_cast<ವರ್ಣ *>(ResultPtr) - &Result[0]);
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
    } ಅನ್ಯಥಾ {
      Result.clear();
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
    }
  } ಅನ್ಯಥಾ {
    llvm_unreachable(
        "Control should never reach this point; see static_assert further up");
  }
}

ತಾರ್ಕಿಕ IsSingleCodeUnitUTF8Codepoint(unsigned V) { ಪ್ರತ್ಯಾಗಮ V <= 0x7F; }

ತಾರ್ಕಿಕ IsSingleCodeUnitUTF16Codepoint(unsigned V) {
  ಪ್ರತ್ಯಾಗಮ V <= 0xD7FF || (V >= 0xE000 && V <= 0xFFFF);
}

ತಾರ್ಕಿಕ IsSingleCodeUnitUTF32Codepoint(unsigned V) {
  ಪ್ರತ್ಯಾಗಮ V <= 0xD7FF || (V >= 0xE000 && V <= 0x10FFFF);
}

} // end namespace llvm

