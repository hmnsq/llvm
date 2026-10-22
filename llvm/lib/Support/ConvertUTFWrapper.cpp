//===-- ConvertUTFWrapper.cpp - Wrap ConvertUTF.h with clang data types -----===
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/ADT/ArrayRef.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringRef.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/bit.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ConvertUTF.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ErrorHandling.h"
#ଅନ୍ତର୍ଭାଵ <string>
#ଅନ୍ତର୍ଭାଵ <vector>

ନାମାକାଶ llvm {

ତାର୍କିକ ConvertUTF8toWide(unsigned WideCharWidth, llvm::StringRef Source,
                       ଅକ୍ଷର *&ResultPtr, ଅଚଳ UTF8 *&ErrorPtr) {
  ପ୍ରତିଜ୍ଞା(WideCharWidth == 1 || WideCharWidth == 2 || WideCharWidth == 4);
  ConversionResult result = conversionOK;
  // Copy the character span over.
  ଯଦି (WideCharWidth == 1) {
    ଅଚଳ UTF8 *Pos = reinterpret_cast<ଅଚଳ UTF8*>(Source.begin());
    ଯଦି (!isLegalUTF8String(&Pos, reinterpret_cast<ଅଚଳ UTF8*>(Source.end()))) {
      result = sourceIllegal;
      ErrorPtr = Pos;
    } ଅନ୍ୟଥା {
      memcpy(ResultPtr, Source.data(), Source.size());
      ResultPtr += Source.size();
    }
  } ଅନ୍ୟଥା ଯଦି (WideCharWidth == 2) {
    ଅଚଳ UTF8 *sourceStart = (ଅଚଳ UTF8*)Source.data();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF16 *targetStart = reinterpret_cast<UTF16 *>(ResultPtr);
    ConversionFlags flags = strictConversion;
    result =
        ConvertUTF8toUTF16(&sourceStart, sourceStart + Source.size(),
                           &targetStart, targetStart + Source.size(), flags);
    ଯଦି (result == conversionOK)
      ResultPtr = reinterpret_cast<ଅକ୍ଷର *>(targetStart);
    ଅନ୍ୟଥା
      ErrorPtr = sourceStart;
  } ଅନ୍ୟଥା ଯଦି (WideCharWidth == 4) {
    ଅଚଳ UTF8 *sourceStart = (ଅଚଳ UTF8 *)Source.data();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF32 *targetStart = reinterpret_cast<UTF32 *>(ResultPtr);
    ConversionFlags flags = strictConversion;
    result =
        ConvertUTF8toUTF32(&sourceStart, sourceStart + Source.size(),
                           &targetStart, targetStart + Source.size(), flags);
    ଯଦି (result == conversionOK)
      ResultPtr = reinterpret_cast<ଅକ୍ଷର *>(targetStart);
    ଅନ୍ୟଥା
      ErrorPtr = sourceStart;
  }
  ପ୍ରତିଜ୍ଞା((result != targetExhausted) &&
         "ConvertUTF8toUTFXX exhausted target buffer");
  ଫେରନ୍ତୁ result == conversionOK;
}

ତାର୍କିକ ConvertCodePointToUTF8(unsigned Source, ଅକ୍ଷର *&ResultPtr) {
  ଅଚଳ UTF32 *SourceStart = &Source;
  ଅଚଳ UTF32 *SourceEnd = SourceStart + 1;
  UTF8 *TargetStart = reinterpret_cast<UTF8 *>(ResultPtr);
  UTF8 *TargetEnd = TargetStart + 4;
  ConversionResult CR = ConvertUTF32toUTF8(
      &SourceStart, SourceEnd, &TargetStart, TargetEnd, strictConversion);
  ଯଦି (CR != conversionOK)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  ResultPtr = reinterpret_cast<ଅକ୍ଷର *>(TargetStart);
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ତାର୍କିକ hasUTF16ByteOrderMark(ArrayRef<ଅକ୍ଷର> S) {
  ଫେରନ୍ତୁ (S.size() >= 2 && ((S[0] == '\xff' && S[1] == '\xfe') ||
                            (S[0] == '\xfe' && S[1] == '\xff')));
}

ତାର୍କିକ convertUTF16ToUTF8String(ArrayRef<ଅକ୍ଷର> SrcBytes, std::string &Out) {
  ପ୍ରତିଜ୍ଞା(Out.empty());

  // Error out on an uneven byte count.
  ଯଦି (SrcBytes.size() % 2)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  // Avoid OOB by returning early on empty input.
  ଯଦି (SrcBytes.empty())
    ଫେରନ୍ତୁ ସତ୍ୟ;

  ଅଚଳ UTF16 *Src = reinterpret_cast<ଅଚଳ UTF16 *>(SrcBytes.begin());
  ଅଚଳ UTF16 *SrcEnd = reinterpret_cast<ଅଚଳ UTF16 *>(SrcBytes.end());

  ପ୍ରତିଜ୍ଞା((uintptr_t)Src % ଆକାର(UTF16) == 0);

  // Byteswap if necessary.
  std::vector<UTF16> ByteSwapped;
  ଯଦି (Src[0] == UNI_UTF16_BYTE_ORDER_MARK_SWAPPED) {
    ByteSwapped.ଭର୍ତ୍ତି(ByteSwapped.end(), Src, SrcEnd);
    ପାଇଁ (UTF16 &I : ByteSwapped)
      I = llvm::byteswap<uint16_t>(I);
    Src = &ByteSwapped[0];
    SrcEnd = &ByteSwapped[ByteSwapped.size() - 1] + 1;
  }

  // Skip the BOM for conversion.
  ଯଦି (Src[0] == UNI_UTF16_BYTE_ORDER_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  Out.resize(SrcBytes.size() * UNI_MAX_UTF8_BYTES_PER_CODE_POINT + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&Out[0]);
  UTF8 *DstEnd = Dst + Out.size();

  ConversionResult CR =
      ConvertUTF16toUTF8(&Src, SrcEnd, &Dst, DstEnd, strictConversion);
  ପ୍ରତିଜ୍ଞା(CR != targetExhausted);

  ଯଦି (CR != conversionOK) {
    Out.clear();
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }

  Out.resize(reinterpret_cast<ଅକ୍ଷର *>(Dst) - &Out[0]);
  Out.ଠେଲ_back(0);
  Out.କାଢ଼_back();
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ତାର୍କିକ convertUTF16ToUTF8String(ArrayRef<UTF16> Src, std::string &Out) {
  ଫେରନ୍ତୁ convertUTF16ToUTF8String(
      llvm::ArrayRef<ଅକ୍ଷର>(reinterpret_cast<ଅଚଳ ଅକ୍ଷର *>(Src.data()),
                           Src.size() * ଆକାର(UTF16)),
      Out);
}

ତାର୍କିକ convertUTF32ToUTF8String(ArrayRef<ଅକ୍ଷର> SrcBytes, std::string &Out) {
  ପ୍ରତିଜ୍ଞା(Out.empty());

  // Error out on an uneven byte count.
  ଯଦି (SrcBytes.size() % 4)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  // Avoid OOB by returning early on empty input.
  ଯଦି (SrcBytes.empty())
    ଫେରନ୍ତୁ ସତ୍ୟ;

  ଅଚଳ UTF32 *Src = reinterpret_cast<ଅଚଳ UTF32 *>(SrcBytes.begin());
  ଅଚଳ UTF32 *SrcEnd = reinterpret_cast<ଅଚଳ UTF32 *>(SrcBytes.end());

  ପ୍ରତିଜ୍ଞା((uintptr_t)Src % ଆକାର(UTF32) == 0);

  // Byteswap if necessary.
  std::vector<UTF32> ByteSwapped;
  ଯଦି (Src[0] == UNI_UTF32_BYTE_ORDER_MARK_SWAPPED) {
    ByteSwapped.ଭର୍ତ୍ତି(ByteSwapped.end(), Src, SrcEnd);
    ପାଇଁ (UTF32 &I : ByteSwapped)
      I = llvm::byteswap<uint32_t>(I);
    Src = &ByteSwapped[0];
    SrcEnd = &ByteSwapped[ByteSwapped.size() - 1] + 1;
  }

  // Skip the BOM for conversion.
  ଯଦି (Src[0] == UNI_UTF32_BYTE_ORDER_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  Out.resize(SrcBytes.size() * UNI_MAX_UTF8_BYTES_PER_CODE_POINT + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&Out[0]);
  UTF8 *DstEnd = Dst + Out.size();

  ConversionResult CR =
      ConvertUTF32toUTF8(&Src, SrcEnd, &Dst, DstEnd, strictConversion);
  ପ୍ରତିଜ୍ଞା(CR != targetExhausted);

  ଯଦି (CR != conversionOK) {
    Out.clear();
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }

  Out.resize(reinterpret_cast<ଅକ୍ଷର *>(Dst) - &Out[0]);
  Out.ଠେଲ_back(0);
  Out.କାଢ଼_back();
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ତାର୍କିକ convertUTF32ToUTF8String(ArrayRef<UTF32> Src, std::string &Out) {
  ଫେରନ୍ତୁ convertUTF32ToUTF8String(
      llvm::ArrayRef<ଅକ୍ଷର>(reinterpret_cast<ଅଚଳ ଅକ୍ଷର *>(Src.data()),
                           Src.size() * ଆକାର(UTF32)),
      Out);
}

ତାର୍କିକ convertUTF8ToUTF16String(StringRef SrcUTF8,
                              SmallVectorImpl<UTF16> &DstUTF16) {
  ପ୍ରତିଜ୍ଞା(DstUTF16.empty());

  // Avoid OOB by returning early on empty input.
  ଯଦି (SrcUTF8.empty()) {
    DstUTF16.ଠେଲ_back(0);
    DstUTF16.କାଢ଼_back();
    ଫେରନ୍ତୁ ସତ୍ୟ;
  }

  ଅଚଳ UTF8 *Src = reinterpret_cast<ଅଚଳ UTF8 *>(SrcUTF8.begin());
  ଅଚଳ UTF8 *SrcEnd = reinterpret_cast<ଅଚଳ UTF8 *>(SrcUTF8.end());

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
  ପ୍ରତିଜ୍ଞା(CR != targetExhausted);

  ଯଦି (CR != conversionOK) {
    DstUTF16.clear();
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }

  DstUTF16.resize(Dst - &DstUTF16[0]);
  DstUTF16.ଠେଲ_back(0);
  DstUTF16.କାଢ଼_back();
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

static_assert(ଆକାର(wchar_t) == 1 || ଆକାର(wchar_t) == 2 ||
                  ଆକାର(wchar_t) == 4,
              "Expected wchar_t to be 1, 2, or 4 bytes");

template <typename TResult>
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ତାର୍କିକ ConvertUTF8toWideInternal(llvm::StringRef Source,
                                             TResult &Result) {
  // Even in the case of UTF-16, the number of bytes in a UTF-8 string is
  // at least as large as the number of elements in the resulting wide
  // string, because surrogate pairs take at least 4 bytes in UTF-8.
  Result.resize(Source.size() + 1);
  ଅକ୍ଷର *ResultPtr = reinterpret_cast<ଅକ୍ଷର *>(&Result[0]);
  ଅଚଳ UTF8 *ErrorPtr;
  ଯଦି (!ConvertUTF8toWide(ଆକାର(wchar_t), Source, ResultPtr, ErrorPtr)) {
    Result.clear();
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }
  Result.resize(reinterpret_cast<wchar_t *>(ResultPtr) - &Result[0]);
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ତାର୍କିକ ConvertUTF8toWide(llvm::StringRef Source, std::wstring &Result) {
  ଫେରନ୍ତୁ ConvertUTF8toWideInternal(Source, Result);
}

ତାର୍କିକ ConvertUTF8toWide(ଅଚଳ ଅକ୍ଷର *Source, std::wstring &Result) {
  ଯଦି (!Source) {
    Result.clear();
    ଫେରନ୍ତୁ ସତ୍ୟ;
  }
  ଫେରନ୍ତୁ ConvertUTF8toWide(llvm::StringRef(Source), Result);
}

ତାର୍କିକ convertWideToUTF8(std::wstring_view Source, std::string &Result) {
  ଯଦି (ଆକାର(wchar_t) == 1) {
    ଅଚଳ UTF8 *Start = reinterpret_cast<ଅଚଳ UTF8 *>(Source.data());
    ଅଚଳ UTF8 *End =
        reinterpret_cast<ଅଚଳ UTF8 *>(Source.data() + Source.size());
    ଯଦି (!isLegalUTF8String(&Start, End))
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
    Result.resize(Source.size());
    memcpy(&Result[0], Source.data(), Source.size());
    ଫେରନ୍ତୁ ସତ୍ୟ;
  } ଅନ୍ୟଥା ଯଦି (ଆକାର(wchar_t) == 2) {
    ଫେରନ୍ତୁ convertUTF16ToUTF8String(
        llvm::ArrayRef<UTF16>(reinterpret_cast<ଅଚଳ UTF16 *>(Source.data()),
                              Source.size()),
        Result);
  } ଅନ୍ୟଥା ଯଦି (ଆକାର(wchar_t) == 4) {
    ଅଚଳ UTF32 *Start = reinterpret_cast<ଅଚଳ UTF32 *>(Source.data());
    ଅଚଳ UTF32 *End =
        reinterpret_cast<ଅଚଳ UTF32 *>(Source.data() + Source.size());
    Result.resize(UNI_MAX_UTF8_BYTES_PER_CODE_POINT * Source.size());
    UTF8 *ResultPtr = reinterpret_cast<UTF8 *>(&Result[0]);
    UTF8 *ResultEnd = reinterpret_cast<UTF8 *>(&Result[0] + Result.size());
    ଯଦି (ConvertUTF32toUTF8(&Start, End, &ResultPtr, ResultEnd,
                           strictConversion) == conversionOK) {
      Result.resize(reinterpret_cast<ଅକ୍ଷର *>(ResultPtr) - &Result[0]);
      ଫେରନ୍ତୁ ସତ୍ୟ;
    } ଅନ୍ୟଥା {
      Result.clear();
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
    }
  } ଅନ୍ୟଥା {
    llvm_unreachable(
        "Control should never reach this point; see static_assert further up");
  }
}

ତାର୍କିକ IsSingleCodeUnitUTF8Codepoint(unsigned V) { ଫେରନ୍ତୁ V <= 0x7F; }

ତାର୍କିକ IsSingleCodeUnitUTF16Codepoint(unsigned V) {
  ଫେରନ୍ତୁ V <= 0xD7FF || (V >= 0xE000 && V <= 0xFFFF);
}

ତାର୍କିକ IsSingleCodeUnitUTF32Codepoint(unsigned V) {
  ଫେରନ୍ତୁ V <= 0xD7FF || (V >= 0xE000 && V <= 0x10FFFF);
}

} // end namespace llvm

