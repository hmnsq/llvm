//===-- ConvertUTFWrapper.cpp - Wrap ConvertUTF.h with clang data types -----===
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/ADT/ArrayRef.h"
#समावेश "llvm/ADT/StringRef.h"
#समावेश "llvm/ADT/bit.h"
#समावेश "llvm/Support/ConvertUTF.h"
#समावेश "llvm/Support/ErrorHandling.h"
#समावेश <string>
#समावेश <vector>

namespace llvm {

bool ConvertUTF8toWide(अचिन्हित WideCharWidth, llvm::StringRef Source,
                       अक्षर *&परिणाम_Ptr, स्थिर UTF8 *&ErrorPtr) {
  निश्चित(WideCharWidth == 1 || WideCharWidth == 2 || WideCharWidth == 4);
  Conversion_परिणाम परिणाम_2 = conversionOK;
  // Copy the character span over.
  अगर (WideCharWidth == 1) {
    स्थिर UTF8 *Pos = reinterpret_cast<स्थिर UTF8*>(Source.begin());
    अगर (!isLegalUTF8String(&Pos, reinterpret_cast<स्थिर UTF8*>(Source.end()))) {
      परिणाम_2 = sourceIllegal;
      ErrorPtr = Pos;
    } अन्यथा {
      स_नकल(परिणाम_Ptr, Source.तथ्य(), Source.आकार_2());
      परिणाम_Ptr += Source.आकार_2();
    }
  } अन्यथा अगर (WideCharWidth == 2) {
    स्थिर UTF8 *sourceStart = (स्थिर UTF8*)Source.तथ्य();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF16 *targetStart = reinterpret_cast<UTF16 *>(परिणाम_Ptr);
    ConversionFlags flags = strictConversion;
    परिणाम_2 =
        ConvertUTF8toUTF16(&sourceStart, sourceStart + Source.आकार_2(),
                           &targetStart, targetStart + Source.आकार_2(), flags);
    अगर (परिणाम_2 == conversionOK)
      परिणाम_Ptr = reinterpret_cast<अक्षर *>(targetStart);
    अन्यथा
      ErrorPtr = sourceStart;
  } अन्यथा अगर (WideCharWidth == 4) {
    स्थिर UTF8 *sourceStart = (स्थिर UTF8 *)Source.तथ्य();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF32 *targetStart = reinterpret_cast<UTF32 *>(परिणाम_Ptr);
    ConversionFlags flags = strictConversion;
    परिणाम_2 =
        ConvertUTF8toUTF32(&sourceStart, sourceStart + Source.आकार_2(),
                           &targetStart, targetStart + Source.आकार_2(), flags);
    अगर (परिणाम_2 == conversionOK)
      परिणाम_Ptr = reinterpret_cast<अक्षर *>(targetStart);
    अन्यथा
      ErrorPtr = sourceStart;
  }
  निश्चित((परिणाम_2 != targetExhausted) &&
         "ConvertUTF8toUTFXX exhausted target buffer");
  वापस परिणाम_2 == conversionOK;
}

bool ConvertCodePointToUTF8(अचिन्हित Source, अक्षर *&परिणाम_Ptr) {
  स्थिर UTF32 *SourceStart = &Source;
  स्थिर UTF32 *SourceEnd = SourceStart + 1;
  UTF8 *TargetStart = reinterpret_cast<UTF8 *>(परिणाम_Ptr);
  UTF8 *TargetEnd = TargetStart + 4;
  Conversion_परिणाम CR = ConvertUTF32toUTF8(
      &SourceStart, SourceEnd, &TargetStart, TargetEnd, strictConversion);
  अगर (CR != conversionOK)
    वापस false;

  परिणाम_Ptr = reinterpret_cast<अक्षर *>(TargetStart);
  वापस true;
}

bool hasUTF16ByteOrderMark(ArrayRef<अक्षर> S) {
  वापस (S.आकार_2() >= 2 && ((S[0] == '\xff' && S[1] == '\xfe') ||
                            (S[0] == '\xfe' && S[1] == '\xff')));
}

bool convertUTF16ToUTF8String(ArrayRef<अक्षर> SrcBytes, std::string &Out) {
  निश्चित(Out.empty());

  // Error out on an uneven byte count.
  अगर (SrcBytes.आकार_2() % 2)
    वापस false;

  // Avoid OOB by returning early on empty input.
  अगर (SrcBytes.empty())
    वापस true;

  स्थिर UTF16 *Src = reinterpret_cast<स्थिर UTF16 *>(SrcBytes.begin());
  स्थिर UTF16 *SrcEnd = reinterpret_cast<स्थिर UTF16 *>(SrcBytes.end());

  निश्चित((uintptr_t)Src % माप(UTF16) == 0);

  // Byteswap if necessary.
  std::vector<UTF16> ByteSwapped;
  अगर (Src[0] == UNI_UTF16_BYTE_ORDER_MARK_SWAPPED) {
    ByteSwapped.जोड़ो(ByteSwapped.end(), Src, SrcEnd);
    क्रम (UTF16 &I : ByteSwapped)
      I = llvm::byteswap<uint16_t>(I);
    Src = &ByteSwapped[0];
    SrcEnd = &ByteSwapped[ByteSwapped.आकार_2() - 1] + 1;
  }

  // Skip the BOM for conversion.
  अगर (Src[0] == UNI_UTF16_BYTE_ORDER_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  Out.resize(SrcBytes.आकार_2() * UNI_MAX_UTF8_BYTES_PER_CODE_POINT + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&Out[0]);
  UTF8 *DstEnd = Dst + Out.आकार_2();

  Conversion_परिणाम CR =
      ConvertUTF16toUTF8(&Src, SrcEnd, &Dst, DstEnd, strictConversion);
  निश्चित(CR != targetExhausted);

  अगर (CR != conversionOK) {
    Out.सफा();
    वापस false;
  }

  Out.resize(reinterpret_cast<अक्षर *>(Dst) - &Out[0]);
  Out.धकेलो_back(0);
  Out.निकालो_back();
  वापस true;
}

bool convertUTF16ToUTF8String(ArrayRef<UTF16> Src, std::string &Out) {
  वापस convertUTF16ToUTF8String(
      llvm::ArrayRef<अक्षर>(reinterpret_cast<स्थिर अक्षर *>(Src.तथ्य()),
                           Src.आकार_2() * माप(UTF16)),
      Out);
}

bool convertUTF32ToUTF8String(ArrayRef<अक्षर> SrcBytes, std::string &Out) {
  निश्चित(Out.empty());

  // Error out on an uneven byte count.
  अगर (SrcBytes.आकार_2() % 4)
    वापस false;

  // Avoid OOB by returning early on empty input.
  अगर (SrcBytes.empty())
    वापस true;

  स्थिर UTF32 *Src = reinterpret_cast<स्थिर UTF32 *>(SrcBytes.begin());
  स्थिर UTF32 *SrcEnd = reinterpret_cast<स्थिर UTF32 *>(SrcBytes.end());

  निश्चित((uintptr_t)Src % माप(UTF32) == 0);

  // Byteswap if necessary.
  std::vector<UTF32> ByteSwapped;
  अगर (Src[0] == UNI_UTF32_BYTE_ORDER_MARK_SWAPPED) {
    ByteSwapped.जोड़ो(ByteSwapped.end(), Src, SrcEnd);
    क्रम (UTF32 &I : ByteSwapped)
      I = llvm::byteswap<uint32_t>(I);
    Src = &ByteSwapped[0];
    SrcEnd = &ByteSwapped[ByteSwapped.आकार_2() - 1] + 1;
  }

  // Skip the BOM for conversion.
  अगर (Src[0] == UNI_UTF32_BYTE_ORDER_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  Out.resize(SrcBytes.आकार_2() * UNI_MAX_UTF8_BYTES_PER_CODE_POINT + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&Out[0]);
  UTF8 *DstEnd = Dst + Out.आकार_2();

  Conversion_परिणाम CR =
      ConvertUTF32toUTF8(&Src, SrcEnd, &Dst, DstEnd, strictConversion);
  निश्चित(CR != targetExhausted);

  अगर (CR != conversionOK) {
    Out.सफा();
    वापस false;
  }

  Out.resize(reinterpret_cast<अक्षर *>(Dst) - &Out[0]);
  Out.धकेलो_back(0);
  Out.निकालो_back();
  वापस true;
}

bool convertUTF32ToUTF8String(ArrayRef<UTF32> Src, std::string &Out) {
  वापस convertUTF32ToUTF8String(
      llvm::ArrayRef<अक्षर>(reinterpret_cast<स्थिर अक्षर *>(Src.तथ्य()),
                           Src.आकार_2() * माप(UTF32)),
      Out);
}

bool convertUTF8ToUTF16String(StringRef SrcUTF8,
                              SmallVectorImpl<UTF16> &DstUTF16) {
  निश्चित(DstUTF16.empty());

  // Avoid OOB by returning early on empty input.
  अगर (SrcUTF8.empty()) {
    DstUTF16.धकेलो_back(0);
    DstUTF16.निकालो_back();
    वापस true;
  }

  स्थिर UTF8 *Src = reinterpret_cast<स्थिर UTF8 *>(SrcUTF8.begin());
  स्थिर UTF8 *SrcEnd = reinterpret_cast<स्थिर UTF8 *>(SrcUTF8.end());

  // Allocate the same number of UTF-16 code units as UTF-8 code units. Encoding
  // as UTF-16 should always require the same amount or less code units than the
  // UTF-8 encoding.  Allocate one extra byte for the null terminator though,
  // so that someone calling DstUTF16.data() gets a null terminated string.
  // We resize down later so we don't have to worry that this over allocates.
  DstUTF16.resize(SrcUTF8.आकार_2()+1);
  UTF16 *Dst = &DstUTF16[0];
  UTF16 *DstEnd = Dst + DstUTF16.आकार_2();

  Conversion_परिणाम CR =
      ConvertUTF8toUTF16(&Src, SrcEnd, &Dst, DstEnd, strictConversion);
  निश्चित(CR != targetExhausted);

  अगर (CR != conversionOK) {
    DstUTF16.सफा();
    वापस false;
  }

  DstUTF16.resize(Dst - &DstUTF16[0]);
  DstUTF16.धकेलो_back(0);
  DstUTF16.निकालो_back();
  वापस true;
}

static_assert(माप(ब_अक्षर_प्रकार) == 1 || माप(ब_अक्षर_प्रकार) == 2 ||
                  माप(ब_अक्षर_प्रकार) == 4,
              "Expected wchar_t to be 1, 2, or 4 bytes");

ढाँचा <typename TResult>
अटल अंतरभूत bool ConvertUTF8toWideInternal(llvm::StringRef Source,
                                             TResult &परिणाम) {
  // Even in the case of UTF-16, the number of bytes in a UTF-8 string is
  // at least as large as the number of elements in the resulting wide
  // string, because surrogate pairs take at least 4 bytes in UTF-8.
  परिणाम.resize(Source.आकार_2() + 1);
  अक्षर *परिणाम_Ptr = reinterpret_cast<अक्षर *>(&परिणाम[0]);
  स्थिर UTF8 *ErrorPtr;
  अगर (!ConvertUTF8toWide(माप(ब_अक्षर_प्रकार), Source, परिणाम_Ptr, ErrorPtr)) {
    परिणाम.सफा();
    वापस false;
  }
  परिणाम.resize(reinterpret_cast<ब_अक्षर_प्रकार *>(परिणाम_Ptr) - &परिणाम[0]);
  वापस true;
}

bool ConvertUTF8toWide(llvm::StringRef Source, std::wstring &परिणाम) {
  वापस ConvertUTF8toWideInternal(Source, परिणाम);
}

bool ConvertUTF8toWide(स्थिर अक्षर *Source, std::wstring &परिणाम) {
  अगर (!Source) {
    परिणाम.सफा();
    वापस true;
  }
  वापस ConvertUTF8toWide(llvm::StringRef(Source), परिणाम);
}

bool convertWideToUTF8(std::wstring_view Source, std::string &परिणाम) {
  अगर (माप(ब_अक्षर_प्रकार) == 1) {
    स्थिर UTF8 *Start = reinterpret_cast<स्थिर UTF8 *>(Source.तथ्य());
    स्थिर UTF8 *End =
        reinterpret_cast<स्थिर UTF8 *>(Source.तथ्य() + Source.आकार_2());
    अगर (!isLegalUTF8String(&Start, End))
      वापस false;
    परिणाम.resize(Source.आकार_2());
    स_नकल(&परिणाम[0], Source.तथ्य(), Source.आकार_2());
    वापस true;
  } अन्यथा अगर (माप(ब_अक्षर_प्रकार) == 2) {
    वापस convertUTF16ToUTF8String(
        llvm::ArrayRef<UTF16>(reinterpret_cast<स्थिर UTF16 *>(Source.तथ्य()),
                              Source.आकार_2()),
        परिणाम);
  } अन्यथा अगर (माप(ब_अक्षर_प्रकार) == 4) {
    स्थिर UTF32 *Start = reinterpret_cast<स्थिर UTF32 *>(Source.तथ्य());
    स्थिर UTF32 *End =
        reinterpret_cast<स्थिर UTF32 *>(Source.तथ्य() + Source.आकार_2());
    परिणाम.resize(UNI_MAX_UTF8_BYTES_PER_CODE_POINT * Source.आकार_2());
    UTF8 *परिणाम_Ptr = reinterpret_cast<UTF8 *>(&परिणाम[0]);
    UTF8 *परिणाम_End = reinterpret_cast<UTF8 *>(&परिणाम[0] + परिणाम.आकार_2());
    अगर (ConvertUTF32toUTF8(&Start, End, &परिणाम_Ptr, परिणाम_End,
                           strictConversion) == conversionOK) {
      परिणाम.resize(reinterpret_cast<अक्षर *>(परिणाम_Ptr) - &परिणाम[0]);
      वापस true;
    } अन्यथा {
      परिणाम.सफा();
      वापस false;
    }
  } अन्यथा {
    llvm_unreachable(
        "Control should never reach this point; see static_assert further up");
  }
}

bool IsSingleCodeUnitUTF8Codepoint(अचिन्हित V) { वापस V <= 0x7F; }

bool IsSingleCodeUnitUTF16Codepoint(अचिन्हित V) {
  वापस V <= 0xD7FF || (V >= 0xE000 && V <= 0xFFFF);
}

bool IsSingleCodeUnitUTF32Codepoint(अचिन्हित V) {
  वापस V <= 0xD7FF || (V >= 0xE000 && V <= 0x10FFFF);
}

} // end namespace llvm

