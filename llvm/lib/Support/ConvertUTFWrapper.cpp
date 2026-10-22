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
  यदि (WideCharWidth == 1) {
    ध्रुव UTF8 *Pos = reinterpret_cast<ध्रुव UTF8*>(Source.begin());
    यदि (!isLegalUTF8String(&Pos, reinterpret_cast<ध्रुव UTF8*>(Source.end()))) {
      result = sourceIllegal;
      ErrorPtr = Pos;
    } अन्यथा {
      memcpy(ResultPtr, Source.data(), Source.size());
      ResultPtr += Source.size();
    }
  } अन्यथा यदि (WideCharWidth == 2) {
    ध्रुव UTF8 *sourceStart = (ध्रुव UTF8*)Source.data();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF16 *targetStart = reinterpret_cast<UTF16 *>(ResultPtr);
    ConversionFlags flags = strictConversion;
    result =
        ConvertUTF8toUTF16(&sourceStart, sourceStart + Source.size(),
                           &targetStart, targetStart + Source.size(), flags);
    यदि (result == conversionOK)
      ResultPtr = reinterpret_cast<वर्ण *>(targetStart);
    अन्यथा
      ErrorPtr = sourceStart;
  } अन्यथा यदि (WideCharWidth == 4) {
    ध्रुव UTF8 *sourceStart = (ध्रुव UTF8 *)Source.data();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF32 *targetStart = reinterpret_cast<UTF32 *>(ResultPtr);
    ConversionFlags flags = strictConversion;
    result =
        ConvertUTF8toUTF32(&sourceStart, sourceStart + Source.size(),
                           &targetStart, targetStart + Source.size(), flags);
    यदि (result == conversionOK)
      ResultPtr = reinterpret_cast<वर्ण *>(targetStart);
    अन्यथा
      ErrorPtr = sourceStart;
  }
  प्रतिज्ञा((result != targetExhausted) &&
         "ConvertUTF8toUTFXX exhausted target buffer");
  प्रत्यागम result == conversionOK;
}

तार्किक ConvertCodePointToUTF8(unsigned Source, वर्ण *&ResultPtr) {
  ध्रुव UTF32 *SourceStart = &Source;
  ध्रुव UTF32 *SourceEnd = SourceStart + 1;
  UTF8 *TargetStart = reinterpret_cast<UTF8 *>(ResultPtr);
  UTF8 *TargetEnd = TargetStart + 4;
  ConversionResult CR = ConvertUTF32toUTF8(
      &SourceStart, SourceEnd, &TargetStart, TargetEnd, strictConversion);
  यदि (CR != conversionOK)
    प्रत्यागम असत्य;

  ResultPtr = reinterpret_cast<वर्ण *>(TargetStart);
  प्रत्यागम सत्य;
}

तार्किक hasUTF16ByteOrderMark(ArrayRef<वर्ण> S) {
  प्रत्यागम (S.size() >= 2 && ((S[0] == '\xff' && S[1] == '\xfe') ||
                            (S[0] == '\xfe' && S[1] == '\xff')));
}

तार्किक convertUTF16ToUTF8String(ArrayRef<वर्ण> SrcBytes, std::string &Out) {
  प्रतिज्ञा(Out.empty());

  // Error out on an uneven byte count.
  यदि (SrcBytes.size() % 2)
    प्रत्यागम असत्य;

  // Avoid OOB by returning early on empty input.
  यदि (SrcBytes.empty())
    प्रत्यागम सत्य;

  ध्रुव UTF16 *Src = reinterpret_cast<ध्रुव UTF16 *>(SrcBytes.begin());
  ध्रुव UTF16 *SrcEnd = reinterpret_cast<ध्रुव UTF16 *>(SrcBytes.end());

  प्रतिज्ञा((uintptr_t)Src % परिमाण(UTF16) == 0);

  // Byteswap if necessary.
  std::vector<UTF16> ByteSwapped;
  यदि (Src[0] == UNI_UTF16_BYTE_ORDER_MARK_SWAPPED) {
    ByteSwapped.घाला(ByteSwapped.end(), Src, SrcEnd);
    प्रति (UTF16 &I : ByteSwapped)
      I = llvm::byteswap<uint16_t>(I);
    Src = &ByteSwapped[0];
    SrcEnd = &ByteSwapped[ByteSwapped.size() - 1] + 1;
  }

  // Skip the BOM for conversion.
  यदि (Src[0] == UNI_UTF16_BYTE_ORDER_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  Out.resize(SrcBytes.size() * UNI_MAX_UTF8_BYTES_PER_CODE_POINT + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&Out[0]);
  UTF8 *DstEnd = Dst + Out.size();

  ConversionResult CR =
      ConvertUTF16toUTF8(&Src, SrcEnd, &Dst, DstEnd, strictConversion);
  प्रतिज्ञा(CR != targetExhausted);

  यदि (CR != conversionOK) {
    Out.clear();
    प्रत्यागम असत्य;
  }

  Out.resize(reinterpret_cast<वर्ण *>(Dst) - &Out[0]);
  Out.ढकला_back(0);
  Out.काढा_back();
  प्रत्यागम सत्य;
}

तार्किक convertUTF16ToUTF8String(ArrayRef<UTF16> Src, std::string &Out) {
  प्रत्यागम convertUTF16ToUTF8String(
      llvm::ArrayRef<वर्ण>(reinterpret_cast<ध्रुव वर्ण *>(Src.data()),
                           Src.size() * परिमाण(UTF16)),
      Out);
}

तार्किक convertUTF32ToUTF8String(ArrayRef<वर्ण> SrcBytes, std::string &Out) {
  प्रतिज्ञा(Out.empty());

  // Error out on an uneven byte count.
  यदि (SrcBytes.size() % 4)
    प्रत्यागम असत्य;

  // Avoid OOB by returning early on empty input.
  यदि (SrcBytes.empty())
    प्रत्यागम सत्य;

  ध्रुव UTF32 *Src = reinterpret_cast<ध्रुव UTF32 *>(SrcBytes.begin());
  ध्रुव UTF32 *SrcEnd = reinterpret_cast<ध्रुव UTF32 *>(SrcBytes.end());

  प्रतिज्ञा((uintptr_t)Src % परिमाण(UTF32) == 0);

  // Byteswap if necessary.
  std::vector<UTF32> ByteSwapped;
  यदि (Src[0] == UNI_UTF32_BYTE_ORDER_MARK_SWAPPED) {
    ByteSwapped.घाला(ByteSwapped.end(), Src, SrcEnd);
    प्रति (UTF32 &I : ByteSwapped)
      I = llvm::byteswap<uint32_t>(I);
    Src = &ByteSwapped[0];
    SrcEnd = &ByteSwapped[ByteSwapped.size() - 1] + 1;
  }

  // Skip the BOM for conversion.
  यदि (Src[0] == UNI_UTF32_BYTE_ORDER_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  Out.resize(SrcBytes.size() * UNI_MAX_UTF8_BYTES_PER_CODE_POINT + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&Out[0]);
  UTF8 *DstEnd = Dst + Out.size();

  ConversionResult CR =
      ConvertUTF32toUTF8(&Src, SrcEnd, &Dst, DstEnd, strictConversion);
  प्रतिज्ञा(CR != targetExhausted);

  यदि (CR != conversionOK) {
    Out.clear();
    प्रत्यागम असत्य;
  }

  Out.resize(reinterpret_cast<वर्ण *>(Dst) - &Out[0]);
  Out.ढकला_back(0);
  Out.काढा_back();
  प्रत्यागम सत्य;
}

तार्किक convertUTF32ToUTF8String(ArrayRef<UTF32> Src, std::string &Out) {
  प्रत्यागम convertUTF32ToUTF8String(
      llvm::ArrayRef<वर्ण>(reinterpret_cast<ध्रुव वर्ण *>(Src.data()),
                           Src.size() * परिमाण(UTF32)),
      Out);
}

तार्किक convertUTF8ToUTF16String(StringRef SrcUTF8,
                              SmallVectorImpl<UTF16> &DstUTF16) {
  प्रतिज्ञा(DstUTF16.empty());

  // Avoid OOB by returning early on empty input.
  यदि (SrcUTF8.empty()) {
    DstUTF16.ढकला_back(0);
    DstUTF16.काढा_back();
    प्रत्यागम सत्य;
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

  यदि (CR != conversionOK) {
    DstUTF16.clear();
    प्रत्यागम असत्य;
  }

  DstUTF16.resize(Dst - &DstUTF16[0]);
  DstUTF16.ढकला_back(0);
  DstUTF16.काढा_back();
  प्रत्यागम सत्य;
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
  यदि (!ConvertUTF8toWide(परिमाण(wchar_t), Source, ResultPtr, ErrorPtr)) {
    Result.clear();
    प्रत्यागम असत्य;
  }
  Result.resize(reinterpret_cast<wchar_t *>(ResultPtr) - &Result[0]);
  प्रत्यागम सत्य;
}

तार्किक ConvertUTF8toWide(llvm::StringRef Source, std::wstring &Result) {
  प्रत्यागम ConvertUTF8toWideInternal(Source, Result);
}

तार्किक ConvertUTF8toWide(ध्रुव वर्ण *Source, std::wstring &Result) {
  यदि (!Source) {
    Result.clear();
    प्रत्यागम सत्य;
  }
  प्रत्यागम ConvertUTF8toWide(llvm::StringRef(Source), Result);
}

तार्किक convertWideToUTF8(std::wstring_view Source, std::string &Result) {
  यदि (परिमाण(wchar_t) == 1) {
    ध्रुव UTF8 *Start = reinterpret_cast<ध्रुव UTF8 *>(Source.data());
    ध्रुव UTF8 *End =
        reinterpret_cast<ध्रुव UTF8 *>(Source.data() + Source.size());
    यदि (!isLegalUTF8String(&Start, End))
      प्रत्यागम असत्य;
    Result.resize(Source.size());
    memcpy(&Result[0], Source.data(), Source.size());
    प्रत्यागम सत्य;
  } अन्यथा यदि (परिमाण(wchar_t) == 2) {
    प्रत्यागम convertUTF16ToUTF8String(
        llvm::ArrayRef<UTF16>(reinterpret_cast<ध्रुव UTF16 *>(Source.data()),
                              Source.size()),
        Result);
  } अन्यथा यदि (परिमाण(wchar_t) == 4) {
    ध्रुव UTF32 *Start = reinterpret_cast<ध्रुव UTF32 *>(Source.data());
    ध्रुव UTF32 *End =
        reinterpret_cast<ध्रुव UTF32 *>(Source.data() + Source.size());
    Result.resize(UNI_MAX_UTF8_BYTES_PER_CODE_POINT * Source.size());
    UTF8 *ResultPtr = reinterpret_cast<UTF8 *>(&Result[0]);
    UTF8 *ResultEnd = reinterpret_cast<UTF8 *>(&Result[0] + Result.size());
    यदि (ConvertUTF32toUTF8(&Start, End, &ResultPtr, ResultEnd,
                           strictConversion) == conversionOK) {
      Result.resize(reinterpret_cast<वर्ण *>(ResultPtr) - &Result[0]);
      प्रत्यागम सत्य;
    } अन्यथा {
      Result.clear();
      प्रत्यागम असत्य;
    }
  } अन्यथा {
    llvm_unreachable(
        "Control should never reach this point; see static_assert further up");
  }
}

तार्किक IsSingleCodeUnitUTF8Codepoint(unsigned V) { प्रत्यागम V <= 0x7F; }

तार्किक IsSingleCodeUnitUTF16Codepoint(unsigned V) {
  प्रत्यागम V <= 0xD7FF || (V >= 0xE000 && V <= 0xFFFF);
}

तार्किक IsSingleCodeUnitUTF32Codepoint(unsigned V) {
  प्रत्यागम V <= 0xD7FF || (V >= 0xE000 && V <= 0x10FFFF);
}

} // end namespace llvm

