//===-- ConvertUTFWrapper.cpp - Wrap ConvertUTF.h with clang data types -----===
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/ADT/ArrayRef.h"
#അന്തര്ഭാവ "llvm/ADT/StringRef.h"
#അന്തര്ഭാവ "llvm/ADT/bit.h"
#അന്തര്ഭാവ "llvm/Support/ConvertUTF.h"
#അന്തര്ഭാവ "llvm/Support/ErrorHandling.h"
#അന്തര്ഭാവ <string>
#അന്തര്ഭാവ <vector>

നാമാകാശ llvm {

തർക്കം ConvertUTF8toWide(unsigned WideCharWidth, llvm::StringRef Source,
                       അക്ഷരം *&ResultPtr, സ്ഥിരം UTF8 *&ErrorPtr) {
  പ്രതിജ്ഞാ(WideCharWidth == 1 || WideCharWidth == 2 || WideCharWidth == 4);
  ConversionResult result = conversionOK;
  // Copy the character span over.
  എങ്കിൽ (WideCharWidth == 1) {
    സ്ഥിരം UTF8 *Pos = reinterpret_cast<സ്ഥിരം UTF8*>(Source.begin());
    എങ്കിൽ (!isLegalUTF8String(&Pos, reinterpret_cast<സ്ഥിരം UTF8*>(Source.end()))) {
      result = sourceIllegal;
      ErrorPtr = Pos;
    } അല്ലെങ്കിൽ {
      memcpy(ResultPtr, Source.data(), Source.size());
      ResultPtr += Source.size();
    }
  } അല്ലെങ്കിൽ എങ്കിൽ (WideCharWidth == 2) {
    സ്ഥിരം UTF8 *sourceStart = (സ്ഥിരം UTF8*)Source.data();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF16 *targetStart = reinterpret_cast<UTF16 *>(ResultPtr);
    ConversionFlags flags = strictConversion;
    result =
        ConvertUTF8toUTF16(&sourceStart, sourceStart + Source.size(),
                           &targetStart, targetStart + Source.size(), flags);
    എങ്കിൽ (result == conversionOK)
      ResultPtr = reinterpret_cast<അക്ഷരം *>(targetStart);
    അല്ലെങ്കിൽ
      ErrorPtr = sourceStart;
  } അല്ലെങ്കിൽ എങ്കിൽ (WideCharWidth == 4) {
    സ്ഥിരം UTF8 *sourceStart = (സ്ഥിരം UTF8 *)Source.data();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF32 *targetStart = reinterpret_cast<UTF32 *>(ResultPtr);
    ConversionFlags flags = strictConversion;
    result =
        ConvertUTF8toUTF32(&sourceStart, sourceStart + Source.size(),
                           &targetStart, targetStart + Source.size(), flags);
    എങ്കിൽ (result == conversionOK)
      ResultPtr = reinterpret_cast<അക്ഷരം *>(targetStart);
    അല്ലെങ്കിൽ
      ErrorPtr = sourceStart;
  }
  പ്രതിജ്ഞാ((result != targetExhausted) &&
         "ConvertUTF8toUTFXX exhausted target buffer");
  തിരികെ result == conversionOK;
}

തർക്കം ConvertCodePointToUTF8(unsigned Source, അക്ഷരം *&ResultPtr) {
  സ്ഥിരം UTF32 *SourceStart = &Source;
  സ്ഥിരം UTF32 *SourceEnd = SourceStart + 1;
  UTF8 *TargetStart = reinterpret_cast<UTF8 *>(ResultPtr);
  UTF8 *TargetEnd = TargetStart + 4;
  ConversionResult CR = ConvertUTF32toUTF8(
      &SourceStart, SourceEnd, &TargetStart, TargetEnd, strictConversion);
  എങ്കിൽ (CR != conversionOK)
    തിരികെ അസത്യം;

  ResultPtr = reinterpret_cast<അക്ഷരം *>(TargetStart);
  തിരികെ സത്യം;
}

തർക്കം hasUTF16ByteOrderMark(ArrayRef<അക്ഷരം> S) {
  തിരികെ (S.size() >= 2 && ((S[0] == '\xff' && S[1] == '\xfe') ||
                            (S[0] == '\xfe' && S[1] == '\xff')));
}

തർക്കം convertUTF16ToUTF8String(ArrayRef<അക്ഷരം> SrcBytes, std::string &Out) {
  പ്രതിജ്ഞാ(Out.empty());

  // Error out on an uneven byte count.
  എങ്കിൽ (SrcBytes.size() % 2)
    തിരികെ അസത്യം;

  // Avoid OOB by returning early on empty input.
  എങ്കിൽ (SrcBytes.empty())
    തിരികെ സത്യം;

  സ്ഥിരം UTF16 *Src = reinterpret_cast<സ്ഥിരം UTF16 *>(SrcBytes.begin());
  സ്ഥിരം UTF16 *SrcEnd = reinterpret_cast<സ്ഥിരം UTF16 *>(SrcBytes.end());

  പ്രതിജ്ഞാ((uintptr_t)Src % വലുപ്പം(UTF16) == 0);

  // Byteswap if necessary.
  std::vector<UTF16> ByteSwapped;
  എങ്കിൽ (Src[0] == UNI_UTF16_BYTE_ORDER_MARK_SWAPPED) {
    ByteSwapped.തിരുകുക(ByteSwapped.end(), Src, SrcEnd);
    വേണ്ടി (UTF16 &I : ByteSwapped)
      I = llvm::byteswap<uint16_t>(I);
    Src = &ByteSwapped[0];
    SrcEnd = &ByteSwapped[ByteSwapped.size() - 1] + 1;
  }

  // Skip the BOM for conversion.
  എങ്കിൽ (Src[0] == UNI_UTF16_BYTE_ORDER_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  Out.resize(SrcBytes.size() * UNI_MAX_UTF8_BYTES_PER_CODE_POINT + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&Out[0]);
  UTF8 *DstEnd = Dst + Out.size();

  ConversionResult CR =
      ConvertUTF16toUTF8(&Src, SrcEnd, &Dst, DstEnd, strictConversion);
  പ്രതിജ്ഞാ(CR != targetExhausted);

  എങ്കിൽ (CR != conversionOK) {
    Out.clear();
    തിരികെ അസത്യം;
  }

  Out.resize(reinterpret_cast<അക്ഷരം *>(Dst) - &Out[0]);
  Out.തള്ളുക_back(0);
  Out.എടുക്കുക_back();
  തിരികെ സത്യം;
}

തർക്കം convertUTF16ToUTF8String(ArrayRef<UTF16> Src, std::string &Out) {
  തിരികെ convertUTF16ToUTF8String(
      llvm::ArrayRef<അക്ഷരം>(reinterpret_cast<സ്ഥിരം അക്ഷരം *>(Src.data()),
                           Src.size() * വലുപ്പം(UTF16)),
      Out);
}

തർക്കം convertUTF32ToUTF8String(ArrayRef<അക്ഷരം> SrcBytes, std::string &Out) {
  പ്രതിജ്ഞാ(Out.empty());

  // Error out on an uneven byte count.
  എങ്കിൽ (SrcBytes.size() % 4)
    തിരികെ അസത്യം;

  // Avoid OOB by returning early on empty input.
  എങ്കിൽ (SrcBytes.empty())
    തിരികെ സത്യം;

  സ്ഥിരം UTF32 *Src = reinterpret_cast<സ്ഥിരം UTF32 *>(SrcBytes.begin());
  സ്ഥിരം UTF32 *SrcEnd = reinterpret_cast<സ്ഥിരം UTF32 *>(SrcBytes.end());

  പ്രതിജ്ഞാ((uintptr_t)Src % വലുപ്പം(UTF32) == 0);

  // Byteswap if necessary.
  std::vector<UTF32> ByteSwapped;
  എങ്കിൽ (Src[0] == UNI_UTF32_BYTE_ORDER_MARK_SWAPPED) {
    ByteSwapped.തിരുകുക(ByteSwapped.end(), Src, SrcEnd);
    വേണ്ടി (UTF32 &I : ByteSwapped)
      I = llvm::byteswap<uint32_t>(I);
    Src = &ByteSwapped[0];
    SrcEnd = &ByteSwapped[ByteSwapped.size() - 1] + 1;
  }

  // Skip the BOM for conversion.
  എങ്കിൽ (Src[0] == UNI_UTF32_BYTE_ORDER_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  Out.resize(SrcBytes.size() * UNI_MAX_UTF8_BYTES_PER_CODE_POINT + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&Out[0]);
  UTF8 *DstEnd = Dst + Out.size();

  ConversionResult CR =
      ConvertUTF32toUTF8(&Src, SrcEnd, &Dst, DstEnd, strictConversion);
  പ്രതിജ്ഞാ(CR != targetExhausted);

  എങ്കിൽ (CR != conversionOK) {
    Out.clear();
    തിരികെ അസത്യം;
  }

  Out.resize(reinterpret_cast<അക്ഷരം *>(Dst) - &Out[0]);
  Out.തള്ളുക_back(0);
  Out.എടുക്കുക_back();
  തിരികെ സത്യം;
}

തർക്കം convertUTF32ToUTF8String(ArrayRef<UTF32> Src, std::string &Out) {
  തിരികെ convertUTF32ToUTF8String(
      llvm::ArrayRef<അക്ഷരം>(reinterpret_cast<സ്ഥിരം അക്ഷരം *>(Src.data()),
                           Src.size() * വലുപ്പം(UTF32)),
      Out);
}

തർക്കം convertUTF8ToUTF16String(StringRef SrcUTF8,
                              SmallVectorImpl<UTF16> &DstUTF16) {
  പ്രതിജ്ഞാ(DstUTF16.empty());

  // Avoid OOB by returning early on empty input.
  എങ്കിൽ (SrcUTF8.empty()) {
    DstUTF16.തള്ളുക_back(0);
    DstUTF16.എടുക്കുക_back();
    തിരികെ സത്യം;
  }

  സ്ഥിരം UTF8 *Src = reinterpret_cast<സ്ഥിരം UTF8 *>(SrcUTF8.begin());
  സ്ഥിരം UTF8 *SrcEnd = reinterpret_cast<സ്ഥിരം UTF8 *>(SrcUTF8.end());

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
  പ്രതിജ്ഞാ(CR != targetExhausted);

  എങ്കിൽ (CR != conversionOK) {
    DstUTF16.clear();
    തിരികെ അസത്യം;
  }

  DstUTF16.resize(Dst - &DstUTF16[0]);
  DstUTF16.തള്ളുക_back(0);
  DstUTF16.എടുക്കുക_back();
  തിരികെ സത്യം;
}

static_assert(വലുപ്പം(wchar_t) == 1 || വലുപ്പം(wchar_t) == 2 ||
                  വലുപ്പം(wchar_t) == 4,
              "Expected wchar_t to be 1, 2, or 4 bytes");

template <typename TResult>
സ്ഥിതം വരിയിൽ തർക്കം ConvertUTF8toWideInternal(llvm::StringRef Source,
                                             TResult &Result) {
  // Even in the case of UTF-16, the number of bytes in a UTF-8 string is
  // at least as large as the number of elements in the resulting wide
  // string, because surrogate pairs take at least 4 bytes in UTF-8.
  Result.resize(Source.size() + 1);
  അക്ഷരം *ResultPtr = reinterpret_cast<അക്ഷരം *>(&Result[0]);
  സ്ഥിരം UTF8 *ErrorPtr;
  എങ്കിൽ (!ConvertUTF8toWide(വലുപ്പം(wchar_t), Source, ResultPtr, ErrorPtr)) {
    Result.clear();
    തിരികെ അസത്യം;
  }
  Result.resize(reinterpret_cast<wchar_t *>(ResultPtr) - &Result[0]);
  തിരികെ സത്യം;
}

തർക്കം ConvertUTF8toWide(llvm::StringRef Source, std::wstring &Result) {
  തിരികെ ConvertUTF8toWideInternal(Source, Result);
}

തർക്കം ConvertUTF8toWide(സ്ഥിരം അക്ഷരം *Source, std::wstring &Result) {
  എങ്കിൽ (!Source) {
    Result.clear();
    തിരികെ സത്യം;
  }
  തിരികെ ConvertUTF8toWide(llvm::StringRef(Source), Result);
}

തർക്കം convertWideToUTF8(std::wstring_view Source, std::string &Result) {
  എങ്കിൽ (വലുപ്പം(wchar_t) == 1) {
    സ്ഥിരം UTF8 *Start = reinterpret_cast<സ്ഥിരം UTF8 *>(Source.data());
    സ്ഥിരം UTF8 *End =
        reinterpret_cast<സ്ഥിരം UTF8 *>(Source.data() + Source.size());
    എങ്കിൽ (!isLegalUTF8String(&Start, End))
      തിരികെ അസത്യം;
    Result.resize(Source.size());
    memcpy(&Result[0], Source.data(), Source.size());
    തിരികെ സത്യം;
  } അല്ലെങ്കിൽ എങ്കിൽ (വലുപ്പം(wchar_t) == 2) {
    തിരികെ convertUTF16ToUTF8String(
        llvm::ArrayRef<UTF16>(reinterpret_cast<സ്ഥിരം UTF16 *>(Source.data()),
                              Source.size()),
        Result);
  } അല്ലെങ്കിൽ എങ്കിൽ (വലുപ്പം(wchar_t) == 4) {
    സ്ഥിരം UTF32 *Start = reinterpret_cast<സ്ഥിരം UTF32 *>(Source.data());
    സ്ഥിരം UTF32 *End =
        reinterpret_cast<സ്ഥിരം UTF32 *>(Source.data() + Source.size());
    Result.resize(UNI_MAX_UTF8_BYTES_PER_CODE_POINT * Source.size());
    UTF8 *ResultPtr = reinterpret_cast<UTF8 *>(&Result[0]);
    UTF8 *ResultEnd = reinterpret_cast<UTF8 *>(&Result[0] + Result.size());
    എങ്കിൽ (ConvertUTF32toUTF8(&Start, End, &ResultPtr, ResultEnd,
                           strictConversion) == conversionOK) {
      Result.resize(reinterpret_cast<അക്ഷരം *>(ResultPtr) - &Result[0]);
      തിരികെ സത്യം;
    } അല്ലെങ്കിൽ {
      Result.clear();
      തിരികെ അസത്യം;
    }
  } അല്ലെങ്കിൽ {
    llvm_unreachable(
        "Control should never reach this point; see static_assert further up");
  }
}

തർക്കം IsSingleCodeUnitUTF8Codepoint(unsigned V) { തിരികെ V <= 0x7F; }

തർക്കം IsSingleCodeUnitUTF16Codepoint(unsigned V) {
  തിരികെ V <= 0xD7FF || (V >= 0xE000 && V <= 0xFFFF);
}

തർക്കം IsSingleCodeUnitUTF32Codepoint(unsigned V) {
  തിരികെ V <= 0xD7FF || (V >= 0xE000 && V <= 0x10FFFF);
}

} // end namespace llvm

