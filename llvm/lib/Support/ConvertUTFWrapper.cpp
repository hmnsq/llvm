//===-- ConvertUTFWrapper.cpp - Wrap ConvertUTF.h with clang data types -----===
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/ADT/ArrayRef.h"
#अन्तर्भाव "llvm/ADT/StringRef.h"
#अन्तर्भाव "llvm/ADT/bit.h"
#अन्तर्भाव "llvm/Support/ConvertUTF.h"
#अन्तर्भाव "llvm/Support/ErrorHandling.h"
#अन्तर्भाव <string>
#अन्तर्भाव <vector>

नामाकाश llvm {

तार्किक ConvertUTF8toWide(unsigned WideCharWidth, llvm::StringRef Source,
                       वर्ण *&ResultPtr, ध्रुव UTF8 *&ErrorPtr) {
  प्रतिज्ञा(WideCharWidth == 1 || WideCharWidth == 2 || WideCharWidth == 4);
  ConversionResult result = conversionOK;
  // Copy the character span over.
  सचे (WideCharWidth == 1) {
    ध्रुव UTF8 *Pos = reinterpret_cast<ध्रुव UTF8*>(Source.begin());
    सचे (!isLegalUTF8String(&Pos, reinterpret_cast<ध्रुव UTF8*>(Source.end()))) {
      result = sourceIllegal;
      ErrorPtr = Pos;
    } अञ्ञथा {
      memcpy(ResultPtr, Source.data(), Source.size());
      ResultPtr += Source.size();
    }
  } अञ्ञथा सचे (WideCharWidth == 2) {
    ध्रुव UTF8 *sourceStart = (ध्रुव UTF8*)Source.data();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF16 *targetStart = reinterpret_cast<UTF16 *>(ResultPtr);
    ConversionFlags flags = strictConversion;
    result =
        ConvertUTF8toUTF16(&sourceStart, sourceStart + Source.size(),
                           &targetStart, targetStart + Source.size(), flags);
    सचे (result == conversionOK)
      ResultPtr = reinterpret_cast<वर्ण *>(targetStart);
    अञ्ञथा
      ErrorPtr = sourceStart;
  } अञ्ञथा सचे (WideCharWidth == 4) {
    ध्रुव UTF8 *sourceStart = (ध्रुव UTF8 *)Source.data();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF32 *targetStart = reinterpret_cast<UTF32 *>(ResultPtr);
    ConversionFlags flags = strictConversion;
    result =
        ConvertUTF8toUTF32(&sourceStart, sourceStart + Source.size(),
                           &targetStart, targetStart + Source.size(), flags);
    सचे (result == conversionOK)
      ResultPtr = reinterpret_cast<वर्ण *>(targetStart);
    अञ्ञथा
      ErrorPtr = sourceStart;
  }
  प्रतिज्ञा((result != targetExhausted) &&
         "ConvertUTF8toUTFXX exhausted target buffer");
  पटिनिवत्त result == conversionOK;
}

तार्किक ConvertCodePointToUTF8(unsigned Source, वर्ण *&ResultPtr) {
  ध्रुव UTF32 *SourceStart = &Source;
  ध्रुव UTF32 *SourceEnd = SourceStart + 1;
  UTF8 *TargetStart = reinterpret_cast<UTF8 *>(ResultPtr);
  UTF8 *TargetEnd = TargetStart + 4;
  ConversionResult CR = ConvertUTF32toUTF8(
      &SourceStart, SourceEnd, &TargetStart, TargetEnd, strictConversion);
  सचे (CR != conversionOK)
    पटिनिवत्त मिच्छा;

  ResultPtr = reinterpret_cast<वर्ण *>(TargetStart);
  पटिनिवत्त सच्च;
}

तार्किक hasUTF16ByteOrderMark(ArrayRef<वर्ण> S) {
  पटिनिवत्त (S.size() >= 2 && ((S[0] == '\xff' && S[1] == '\xfe') ||
                            (S[0] == '\xfe' && S[1] == '\xff')));
}

तार्किक convertUTF16ToUTF8String(ArrayRef<वर्ण> SrcBytes, std::string &Out) {
  प्रतिज्ञा(Out.empty());

  // Error out on an uneven byte count.
  सचे (SrcBytes.size() % 2)
    पटिनिवत्त मिच्छा;

  // Avoid OOB by returning early on empty input.
  सचे (SrcBytes.empty())
    पटिनिवत्त सच्च;

  ध्रुव UTF16 *Src = reinterpret_cast<ध्रुव UTF16 *>(SrcBytes.begin());
  ध्रुव UTF16 *SrcEnd = reinterpret_cast<ध्रुव UTF16 *>(SrcBytes.end());

  प्रतिज्ञा((uintptr_t)Src % परिमाण(UTF16) == 0);

  // Byteswap if necessary.
  std::vector<UTF16> ByteSwapped;
  सचे (Src[0] == UNI_UTF16_BYTE_ORDER_MARK_SWAPPED) {
    ByteSwapped.निवेशय(ByteSwapped.end(), Src, SrcEnd);
    अत्थाय (UTF16 &I : ByteSwapped)
      I = llvm::byteswap<uint16_t>(I);
    Src = &ByteSwapped[0];
    SrcEnd = &ByteSwapped[ByteSwapped.size() - 1] + 1;
  }

  // Skip the BOM for conversion.
  सचे (Src[0] == UNI_UTF16_BYTE_ORDER_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  Out.resize(SrcBytes.size() * UNI_MAX_UTF8_BYTES_PER_CODE_POINT + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&Out[0]);
  UTF8 *DstEnd = Dst + Out.size();

  ConversionResult CR =
      ConvertUTF16toUTF8(&Src, SrcEnd, &Dst, DstEnd, strictConversion);
  प्रतिज्ञा(CR != targetExhausted);

  सचे (CR != conversionOK) {
    Out.clear();
    पटिनिवत्त मिच्छा;
  }

  Out.resize(reinterpret_cast<वर्ण *>(Dst) - &Out[0]);
  Out.प्रक्षिप_back(0);
  Out.निष्कासय_back();
  पटिनिवत्त सच्च;
}

तार्किक convertUTF16ToUTF8String(ArrayRef<UTF16> Src, std::string &Out) {
  पटिनिवत्त convertUTF16ToUTF8String(
      llvm::ArrayRef<वर्ण>(reinterpret_cast<ध्रुव वर्ण *>(Src.data()),
                           Src.size() * परिमाण(UTF16)),
      Out);
}

तार्किक convertUTF32ToUTF8String(ArrayRef<वर्ण> SrcBytes, std::string &Out) {
  प्रतिज्ञा(Out.empty());

  // Error out on an uneven byte count.
  सचे (SrcBytes.size() % 4)
    पटिनिवत्त मिच्छा;

  // Avoid OOB by returning early on empty input.
  सचे (SrcBytes.empty())
    पटिनिवत्त सच्च;

  ध्रुव UTF32 *Src = reinterpret_cast<ध्रुव UTF32 *>(SrcBytes.begin());
  ध्रुव UTF32 *SrcEnd = reinterpret_cast<ध्रुव UTF32 *>(SrcBytes.end());

  प्रतिज्ञा((uintptr_t)Src % परिमाण(UTF32) == 0);

  // Byteswap if necessary.
  std::vector<UTF32> ByteSwapped;
  सचे (Src[0] == UNI_UTF32_BYTE_ORDER_MARK_SWAPPED) {
    ByteSwapped.निवेशय(ByteSwapped.end(), Src, SrcEnd);
    अत्थाय (UTF32 &I : ByteSwapped)
      I = llvm::byteswap<uint32_t>(I);
    Src = &ByteSwapped[0];
    SrcEnd = &ByteSwapped[ByteSwapped.size() - 1] + 1;
  }

  // Skip the BOM for conversion.
  सचे (Src[0] == UNI_UTF32_BYTE_ORDER_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  Out.resize(SrcBytes.size() * UNI_MAX_UTF8_BYTES_PER_CODE_POINT + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&Out[0]);
  UTF8 *DstEnd = Dst + Out.size();

  ConversionResult CR =
      ConvertUTF32toUTF8(&Src, SrcEnd, &Dst, DstEnd, strictConversion);
  प्रतिज्ञा(CR != targetExhausted);

  सचे (CR != conversionOK) {
    Out.clear();
    पटिनिवत्त मिच्छा;
  }

  Out.resize(reinterpret_cast<वर्ण *>(Dst) - &Out[0]);
  Out.प्रक्षिप_back(0);
  Out.निष्कासय_back();
  पटिनिवत्त सच्च;
}

तार्किक convertUTF32ToUTF8String(ArrayRef<UTF32> Src, std::string &Out) {
  पटिनिवत्त convertUTF32ToUTF8String(
      llvm::ArrayRef<वर्ण>(reinterpret_cast<ध्रुव वर्ण *>(Src.data()),
                           Src.size() * परिमाण(UTF32)),
      Out);
}

तार्किक convertUTF8ToUTF16String(StringRef SrcUTF8,
                              SmallVectorImpl<UTF16> &DstUTF16) {
  प्रतिज्ञा(DstUTF16.empty());

  // Avoid OOB by returning early on empty input.
  सचे (SrcUTF8.empty()) {
    DstUTF16.प्रक्षिप_back(0);
    DstUTF16.निष्कासय_back();
    पटिनिवत्त सच्च;
  }

  ध्रुव UTF8 *Src = reinterpret_cast<ध्रुव UTF8 *>(SrcUTF8.begin());
  ध्रुव UTF8 *SrcEnd = reinterpret_cast<ध्रुव UTF8 *>(SrcUTF8.end());

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
  प्रतिज्ञा(CR != targetExhausted);

  सचे (CR != conversionOK) {
    DstUTF16.clear();
    पटिनिवत्त मिच्छा;
  }

  DstUTF16.resize(Dst - &DstUTF16[0]);
  DstUTF16.प्रक्षिप_back(0);
  DstUTF16.निष्कासय_back();
  पटिनिवत्त सच्च;
}

static_assert(परिमाण(wchar_t) == 1 || परिमाण(wchar_t) == 2 ||
                  परिमाण(wchar_t) == 4,
              "Expected wchar_t to be 1, 2, or 4 bytes");

template <typename TResult>
स्थिर अन्तःस्थ तार्किक ConvertUTF8toWideInternal(llvm::StringRef Source,
                                             TResult &Result) {
  // Even in the case of UTF-16, the number of bytes in a UTF-8 string is
  // at least as large as the number of elements in the resulting wide
  // string, because surrogate pairs take at least 4 bytes in UTF-8.
  Result.resize(Source.size() + 1);
  वर्ण *ResultPtr = reinterpret_cast<वर्ण *>(&Result[0]);
  ध्रुव UTF8 *ErrorPtr;
  सचे (!ConvertUTF8toWide(परिमाण(wchar_t), Source, ResultPtr, ErrorPtr)) {
    Result.clear();
    पटिनिवत्त मिच्छा;
  }
  Result.resize(reinterpret_cast<wchar_t *>(ResultPtr) - &Result[0]);
  पटिनिवत्त सच्च;
}

तार्किक ConvertUTF8toWide(llvm::StringRef Source, std::wstring &Result) {
  पटिनिवत्त ConvertUTF8toWideInternal(Source, Result);
}

तार्किक ConvertUTF8toWide(ध्रुव वर्ण *Source, std::wstring &Result) {
  सचे (!Source) {
    Result.clear();
    पटिनिवत्त सच्च;
  }
  पटिनिवत्त ConvertUTF8toWide(llvm::StringRef(Source), Result);
}

तार्किक convertWideToUTF8(std::wstring_view Source, std::string &Result) {
  सचे (परिमाण(wchar_t) == 1) {
    ध्रुव UTF8 *Start = reinterpret_cast<ध्रुव UTF8 *>(Source.data());
    ध्रुव UTF8 *End =
        reinterpret_cast<ध्रुव UTF8 *>(Source.data() + Source.size());
    सचे (!isLegalUTF8String(&Start, End))
      पटिनिवत्त मिच्छा;
    Result.resize(Source.size());
    memcpy(&Result[0], Source.data(), Source.size());
    पटिनिवत्त सच्च;
  } अञ्ञथा सचे (परिमाण(wchar_t) == 2) {
    पटिनिवत्त convertUTF16ToUTF8String(
        llvm::ArrayRef<UTF16>(reinterpret_cast<ध्रुव UTF16 *>(Source.data()),
                              Source.size()),
        Result);
  } अञ्ञथा सचे (परिमाण(wchar_t) == 4) {
    ध्रुव UTF32 *Start = reinterpret_cast<ध्रुव UTF32 *>(Source.data());
    ध्रुव UTF32 *End =
        reinterpret_cast<ध्रुव UTF32 *>(Source.data() + Source.size());
    Result.resize(UNI_MAX_UTF8_BYTES_PER_CODE_POINT * Source.size());
    UTF8 *ResultPtr = reinterpret_cast<UTF8 *>(&Result[0]);
    UTF8 *ResultEnd = reinterpret_cast<UTF8 *>(&Result[0] + Result.size());
    सचे (ConvertUTF32toUTF8(&Start, End, &ResultPtr, ResultEnd,
                           strictConversion) == conversionOK) {
      Result.resize(reinterpret_cast<वर्ण *>(ResultPtr) - &Result[0]);
      पटिनिवत्त सच्च;
    } अञ्ञथा {
      Result.clear();
      पटिनिवत्त मिच्छा;
    }
  } अञ्ञथा {
    llvm_unreachable(
        "Control should never reach this point; see static_assert further up");
  }
}

तार्किक IsSingleCodeUnitUTF8Codepoint(unsigned V) { पटिनिवत्त V <= 0x7F; }

तार्किक IsSingleCodeUnitUTF16Codepoint(unsigned V) {
  पटिनिवत्त V <= 0xD7FF || (V >= 0xE000 && V <= 0xFFFF);
}

तार्किक IsSingleCodeUnitUTF32Codepoint(unsigned V) {
  पटिनिवत्त V <= 0xD7FF || (V >= 0xE000 && V <= 0x10FFFF);
}

} // end namespace llvm

