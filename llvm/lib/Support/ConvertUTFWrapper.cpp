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

तार्किक ConvertUTF8toWide(अचिन्हित WideCharWidth, llvm::StringRef Source,
                       अक्षर *&ResultPtr, स्थिरांक UTF8 *&ErrorPtr) {
  प्रतिज्ञा(WideCharWidth == 1 || WideCharWidth == 2 || WideCharWidth == 4);
  ConversionResult result = conversionOK;
  // Copy the character span over.
  जर (WideCharWidth == 1) {
    स्थिरांक UTF8 *Pos = reinterpret_cast<स्थिरांक UTF8*>(Source.begin());
    जर (!isLegalUTF8String(&Pos, reinterpret_cast<स्थिरांक UTF8*>(Source.end()))) {
      result = sourceIllegal;
      ErrorPtr = Pos;
    } अन्यथा {
      स_नकल(ResultPtr, Source.data(), Source.size());
      ResultPtr += Source.size();
    }
  } अन्यथा जर (WideCharWidth == 2) {
    स्थिरांक UTF8 *sourceStart = (स्थिरांक UTF8*)Source.data();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF16 *targetStart = reinterpret_cast<UTF16 *>(ResultPtr);
    ConversionFlags flags = strictConversion;
    result =
        ConvertUTF8toUTF16(&sourceStart, sourceStart + Source.size(),
                           &targetStart, targetStart + Source.size(), flags);
    जर (result == conversionOK)
      ResultPtr = reinterpret_cast<अक्षर *>(targetStart);
    अन्यथा
      ErrorPtr = sourceStart;
  } अन्यथा जर (WideCharWidth == 4) {
    स्थिरांक UTF8 *sourceStart = (स्थिरांक UTF8 *)Source.data();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF32 *targetStart = reinterpret_cast<UTF32 *>(ResultPtr);
    ConversionFlags flags = strictConversion;
    result =
        ConvertUTF8toUTF32(&sourceStart, sourceStart + Source.size(),
                           &targetStart, targetStart + Source.size(), flags);
    जर (result == conversionOK)
      ResultPtr = reinterpret_cast<अक्षर *>(targetStart);
    अन्यथा
      ErrorPtr = sourceStart;
  }
  प्रतिज्ञा((result != targetExhausted) &&
         "ConvertUTF8toUTFXX exhausted target buffer");
  परत result == conversionOK;
}

तार्किक ConvertCodePointToUTF8(अचिन्हित Source, अक्षर *&ResultPtr) {
  स्थिरांक UTF32 *SourceStart = &Source;
  स्थिरांक UTF32 *SourceEnd = SourceStart + 1;
  UTF8 *TargetStart = reinterpret_cast<UTF8 *>(ResultPtr);
  UTF8 *TargetEnd = TargetStart + 4;
  ConversionResult CR = ConvertUTF32toUTF8(
      &SourceStart, SourceEnd, &TargetStart, TargetEnd, strictConversion);
  जर (CR != conversionOK)
    परत असत्य;

  ResultPtr = reinterpret_cast<अक्षर *>(TargetStart);
  परत सत्य;
}

तार्किक hasUTF16ByteOrderMark(ArrayRef<अक्षर> S) {
  परत (S.size() >= 2 && ((S[0] == '\xff' && S[1] == '\xfe') ||
                            (S[0] == '\xfe' && S[1] == '\xff')));
}

तार्किक convertUTF16ToUTF8String(ArrayRef<अक्षर> SrcBytes, std::string &Out) {
  प्रतिज्ञा(Out.empty());

  // Error out on an uneven byte count.
  जर (SrcBytes.size() % 2)
    परत असत्य;

  // Avoid OOB by returning early on empty input.
  जर (SrcBytes.empty())
    परत सत्य;

  स्थिरांक UTF16 *Src = reinterpret_cast<स्थिरांक UTF16 *>(SrcBytes.begin());
  स्थिरांक UTF16 *SrcEnd = reinterpret_cast<स्थिरांक UTF16 *>(SrcBytes.end());

  प्रतिज्ञा((uintptr_t)Src % आकार(UTF16) == 0);

  // Byteswap if necessary.
  std::vector<UTF16> ByteSwapped;
  जर (Src[0] == UNI_UTF16_BYTE_ORDER_MARK_SWAPPED) {
    ByteSwapped.घाला(ByteSwapped.end(), Src, SrcEnd);
    साठी (UTF16 &I : ByteSwapped)
      I = llvm::byteswap<uint16_t>(I);
    Src = &ByteSwapped[0];
    SrcEnd = &ByteSwapped[ByteSwapped.size() - 1] + 1;
  }

  // Skip the BOM for conversion.
  जर (Src[0] == UNI_UTF16_BYTE_ORDER_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  Out.resize(SrcBytes.size() * UNI_MAX_UTF8_BYTES_PER_CODE_POINT + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&Out[0]);
  UTF8 *DstEnd = Dst + Out.size();

  ConversionResult CR =
      ConvertUTF16toUTF8(&Src, SrcEnd, &Dst, DstEnd, strictConversion);
  प्रतिज्ञा(CR != targetExhausted);

  जर (CR != conversionOK) {
    Out.clear();
    परत असत्य;
  }

  Out.resize(reinterpret_cast<अक्षर *>(Dst) - &Out[0]);
  Out.ढकला_back(0);
  Out.काढा_back();
  परत सत्य;
}

तार्किक convertUTF16ToUTF8String(ArrayRef<UTF16> Src, std::string &Out) {
  परत convertUTF16ToUTF8String(
      llvm::ArrayRef<अक्षर>(reinterpret_cast<स्थिरांक अक्षर *>(Src.data()),
                           Src.size() * आकार(UTF16)),
      Out);
}

तार्किक convertUTF32ToUTF8String(ArrayRef<अक्षर> SrcBytes, std::string &Out) {
  प्रतिज्ञा(Out.empty());

  // Error out on an uneven byte count.
  जर (SrcBytes.size() % 4)
    परत असत्य;

  // Avoid OOB by returning early on empty input.
  जर (SrcBytes.empty())
    परत सत्य;

  स्थिरांक UTF32 *Src = reinterpret_cast<स्थिरांक UTF32 *>(SrcBytes.begin());
  स्थिरांक UTF32 *SrcEnd = reinterpret_cast<स्थिरांक UTF32 *>(SrcBytes.end());

  प्रतिज्ञा((uintptr_t)Src % आकार(UTF32) == 0);

  // Byteswap if necessary.
  std::vector<UTF32> ByteSwapped;
  जर (Src[0] == UNI_UTF32_BYTE_ORDER_MARK_SWAPPED) {
    ByteSwapped.घाला(ByteSwapped.end(), Src, SrcEnd);
    साठी (UTF32 &I : ByteSwapped)
      I = llvm::byteswap<uint32_t>(I);
    Src = &ByteSwapped[0];
    SrcEnd = &ByteSwapped[ByteSwapped.size() - 1] + 1;
  }

  // Skip the BOM for conversion.
  जर (Src[0] == UNI_UTF32_BYTE_ORDER_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  Out.resize(SrcBytes.size() * UNI_MAX_UTF8_BYTES_PER_CODE_POINT + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&Out[0]);
  UTF8 *DstEnd = Dst + Out.size();

  ConversionResult CR =
      ConvertUTF32toUTF8(&Src, SrcEnd, &Dst, DstEnd, strictConversion);
  प्रतिज्ञा(CR != targetExhausted);

  जर (CR != conversionOK) {
    Out.clear();
    परत असत्य;
  }

  Out.resize(reinterpret_cast<अक्षर *>(Dst) - &Out[0]);
  Out.ढकला_back(0);
  Out.काढा_back();
  परत सत्य;
}

तार्किक convertUTF32ToUTF8String(ArrayRef<UTF32> Src, std::string &Out) {
  परत convertUTF32ToUTF8String(
      llvm::ArrayRef<अक्षर>(reinterpret_cast<स्थिरांक अक्षर *>(Src.data()),
                           Src.size() * आकार(UTF32)),
      Out);
}

तार्किक convertUTF8ToUTF16String(StringRef SrcUTF8,
                              SmallVectorImpl<UTF16> &DstUTF16) {
  प्रतिज्ञा(DstUTF16.empty());

  // Avoid OOB by returning early on empty input.
  जर (SrcUTF8.empty()) {
    DstUTF16.ढकला_back(0);
    DstUTF16.काढा_back();
    परत सत्य;
  }

  स्थिरांक UTF8 *Src = reinterpret_cast<स्थिरांक UTF8 *>(SrcUTF8.begin());
  स्थिरांक UTF8 *SrcEnd = reinterpret_cast<स्थिरांक UTF8 *>(SrcUTF8.end());

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

  जर (CR != conversionOK) {
    DstUTF16.clear();
    परत असत्य;
  }

  DstUTF16.resize(Dst - &DstUTF16[0]);
  DstUTF16.ढकला_back(0);
  DstUTF16.काढा_back();
  परत सत्य;
}

static_assert(आकार(ब_अक्षर_प्रकार) == 1 || आकार(ब_अक्षर_प्रकार) == 2 ||
                  आकार(ब_अक्षर_प्रकार) == 4,
              "Expected wchar_t to be 1, 2, or 4 bytes");

ढाँचा <typename TResult>
स्थिर ओळीत तार्किक ConvertUTF8toWideInternal(llvm::StringRef Source,
                                             TResult &Result) {
  // Even in the case of UTF-16, the number of bytes in a UTF-8 string is
  // at least as large as the number of elements in the resulting wide
  // string, because surrogate pairs take at least 4 bytes in UTF-8.
  Result.resize(Source.size() + 1);
  अक्षर *ResultPtr = reinterpret_cast<अक्षर *>(&Result[0]);
  स्थिरांक UTF8 *ErrorPtr;
  जर (!ConvertUTF8toWide(आकार(ब_अक्षर_प्रकार), Source, ResultPtr, ErrorPtr)) {
    Result.clear();
    परत असत्य;
  }
  Result.resize(reinterpret_cast<ब_अक्षर_प्रकार *>(ResultPtr) - &Result[0]);
  परत सत्य;
}

तार्किक ConvertUTF8toWide(llvm::StringRef Source, std::wstring &Result) {
  परत ConvertUTF8toWideInternal(Source, Result);
}

तार्किक ConvertUTF8toWide(स्थिरांक अक्षर *Source, std::wstring &Result) {
  जर (!Source) {
    Result.clear();
    परत सत्य;
  }
  परत ConvertUTF8toWide(llvm::StringRef(Source), Result);
}

तार्किक convertWideToUTF8(std::wstring_view Source, std::string &Result) {
  जर (आकार(ब_अक्षर_प्रकार) == 1) {
    स्थिरांक UTF8 *Start = reinterpret_cast<स्थिरांक UTF8 *>(Source.data());
    स्थिरांक UTF8 *End =
        reinterpret_cast<स्थिरांक UTF8 *>(Source.data() + Source.size());
    जर (!isLegalUTF8String(&Start, End))
      परत असत्य;
    Result.resize(Source.size());
    स_नकल(&Result[0], Source.data(), Source.size());
    परत सत्य;
  } अन्यथा जर (आकार(ब_अक्षर_प्रकार) == 2) {
    परत convertUTF16ToUTF8String(
        llvm::ArrayRef<UTF16>(reinterpret_cast<स्थिरांक UTF16 *>(Source.data()),
                              Source.size()),
        Result);
  } अन्यथा जर (आकार(ब_अक्षर_प्रकार) == 4) {
    स्थिरांक UTF32 *Start = reinterpret_cast<स्थिरांक UTF32 *>(Source.data());
    स्थिरांक UTF32 *End =
        reinterpret_cast<स्थिरांक UTF32 *>(Source.data() + Source.size());
    Result.resize(UNI_MAX_UTF8_BYTES_PER_CODE_POINT * Source.size());
    UTF8 *ResultPtr = reinterpret_cast<UTF8 *>(&Result[0]);
    UTF8 *ResultEnd = reinterpret_cast<UTF8 *>(&Result[0] + Result.size());
    जर (ConvertUTF32toUTF8(&Start, End, &ResultPtr, ResultEnd,
                           strictConversion) == conversionOK) {
      Result.resize(reinterpret_cast<अक्षर *>(ResultPtr) - &Result[0]);
      परत सत्य;
    } अन्यथा {
      Result.clear();
      परत असत्य;
    }
  } अन्यथा {
    llvm_unreachable(
        "Control should never reach this point; see static_assert further up");
  }
}

तार्किक IsSingleCodeUnitUTF8Codepoint(अचिन्हित V) { परत V <= 0x7F; }

तार्किक IsSingleCodeUnitUTF16Codepoint(अचिन्हित V) {
  परत V <= 0xD7FF || (V >= 0xE000 && V <= 0xFFFF);
}

तार्किक IsSingleCodeUnitUTF32Codepoint(अचिन्हित V) {
  परत V <= 0xD7FF || (V >= 0xE000 && V <= 0x10FFFF);
}

} // end namespace llvm

