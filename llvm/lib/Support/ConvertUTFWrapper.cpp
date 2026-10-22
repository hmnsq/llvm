//===-- ConvertUTFWrapper.cpp - Wrap ConvertUTF.h with clang data types -----===
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/ADT/ArrayRef.h"
#அந்தர்பாவ "llvm/ADT/StringRef.h"
#அந்தர்பாவ "llvm/ADT/bit.h"
#அந்தர்பாவ "llvm/Support/ConvertUTF.h"
#அந்தர்பாவ "llvm/Support/ErrorHandling.h"
#அந்தர்பாவ <string>
#அந்தர்பாவ <vector>

நாமாகாஶ llvm {

தருக்க ConvertUTF8toWide(unsigned WideCharWidth, llvm::StringRef Source,
                       எழுத்து *&ResultPtr, மாறிலி UTF8 *&ErrorPtr) {
  ப்ரதிஜ்ஞா(WideCharWidth == 1 || WideCharWidth == 2 || WideCharWidth == 4);
  ConversionResult result = conversionOK;
  // Copy the character span over.
  எனில் (WideCharWidth == 1) {
    மாறிலி UTF8 *Pos = reinterpret_cast<மாறிலி UTF8*>(Source.begin());
    எனில் (!isLegalUTF8String(&Pos, reinterpret_cast<மாறிலி UTF8*>(Source.end()))) {
      result = sourceIllegal;
      ErrorPtr = Pos;
    } இல்லையெனில் {
      memcpy(ResultPtr, Source.data(), Source.size());
      ResultPtr += Source.size();
    }
  } இல்லையெனில் எனில் (WideCharWidth == 2) {
    மாறிலி UTF8 *sourceStart = (மாறிலி UTF8*)Source.data();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF16 *targetStart = reinterpret_cast<UTF16 *>(ResultPtr);
    ConversionFlags flags = strictConversion;
    result =
        ConvertUTF8toUTF16(&sourceStart, sourceStart + Source.size(),
                           &targetStart, targetStart + Source.size(), flags);
    எனில் (result == conversionOK)
      ResultPtr = reinterpret_cast<எழுத்து *>(targetStart);
    இல்லையெனில்
      ErrorPtr = sourceStart;
  } இல்லையெனில் எனில் (WideCharWidth == 4) {
    மாறிலி UTF8 *sourceStart = (மாறிலி UTF8 *)Source.data();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF32 *targetStart = reinterpret_cast<UTF32 *>(ResultPtr);
    ConversionFlags flags = strictConversion;
    result =
        ConvertUTF8toUTF32(&sourceStart, sourceStart + Source.size(),
                           &targetStart, targetStart + Source.size(), flags);
    எனில் (result == conversionOK)
      ResultPtr = reinterpret_cast<எழுத்து *>(targetStart);
    இல்லையெனில்
      ErrorPtr = sourceStart;
  }
  ப்ரதிஜ்ஞா((result != targetExhausted) &&
         "ConvertUTF8toUTFXX exhausted target buffer");
  திரும்பு result == conversionOK;
}

தருக்க ConvertCodePointToUTF8(unsigned Source, எழுத்து *&ResultPtr) {
  மாறிலி UTF32 *SourceStart = &Source;
  மாறிலி UTF32 *SourceEnd = SourceStart + 1;
  UTF8 *TargetStart = reinterpret_cast<UTF8 *>(ResultPtr);
  UTF8 *TargetEnd = TargetStart + 4;
  ConversionResult CR = ConvertUTF32toUTF8(
      &SourceStart, SourceEnd, &TargetStart, TargetEnd, strictConversion);
  எனில் (CR != conversionOK)
    திரும்பு பொய்;

  ResultPtr = reinterpret_cast<எழுத்து *>(TargetStart);
  திரும்பு உண்மை;
}

தருக்க hasUTF16ByteOrderMark(ArrayRef<எழுத்து> S) {
  திரும்பு (S.size() >= 2 && ((S[0] == '\xff' && S[1] == '\xfe') ||
                            (S[0] == '\xfe' && S[1] == '\xff')));
}

தருக்க convertUTF16ToUTF8String(ArrayRef<எழுத்து> SrcBytes, std::string &Out) {
  ப்ரதிஜ்ஞா(Out.empty());

  // Error out on an uneven byte count.
  எனில் (SrcBytes.size() % 2)
    திரும்பு பொய்;

  // Avoid OOB by returning early on empty input.
  எனில் (SrcBytes.empty())
    திரும்பு உண்மை;

  மாறிலி UTF16 *Src = reinterpret_cast<மாறிலி UTF16 *>(SrcBytes.begin());
  மாறிலி UTF16 *SrcEnd = reinterpret_cast<மாறிலி UTF16 *>(SrcBytes.end());

  ப்ரதிஜ்ஞா((uintptr_t)Src % அளவு(UTF16) == 0);

  // Byteswap if necessary.
  std::vector<UTF16> ByteSwapped;
  எனில் (Src[0] == UNI_UTF16_BYTE_ORDER_MARK_SWAPPED) {
    ByteSwapped.செருகு(ByteSwapped.end(), Src, SrcEnd);
    க்கு (UTF16 &I : ByteSwapped)
      I = llvm::byteswap<uint16_t>(I);
    Src = &ByteSwapped[0];
    SrcEnd = &ByteSwapped[ByteSwapped.size() - 1] + 1;
  }

  // Skip the BOM for conversion.
  எனில் (Src[0] == UNI_UTF16_BYTE_ORDER_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  Out.resize(SrcBytes.size() * UNI_MAX_UTF8_BYTES_PER_CODE_POINT + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&Out[0]);
  UTF8 *DstEnd = Dst + Out.size();

  ConversionResult CR =
      ConvertUTF16toUTF8(&Src, SrcEnd, &Dst, DstEnd, strictConversion);
  ப்ரதிஜ்ஞா(CR != targetExhausted);

  எனில் (CR != conversionOK) {
    Out.clear();
    திரும்பு பொய்;
  }

  Out.resize(reinterpret_cast<எழுத்து *>(Dst) - &Out[0]);
  Out.தள்ளு_back(0);
  Out.எடு_back();
  திரும்பு உண்மை;
}

தருக்க convertUTF16ToUTF8String(ArrayRef<UTF16> Src, std::string &Out) {
  திரும்பு convertUTF16ToUTF8String(
      llvm::ArrayRef<எழுத்து>(reinterpret_cast<மாறிலி எழுத்து *>(Src.data()),
                           Src.size() * அளவு(UTF16)),
      Out);
}

தருக்க convertUTF32ToUTF8String(ArrayRef<எழுத்து> SrcBytes, std::string &Out) {
  ப்ரதிஜ்ஞா(Out.empty());

  // Error out on an uneven byte count.
  எனில் (SrcBytes.size() % 4)
    திரும்பு பொய்;

  // Avoid OOB by returning early on empty input.
  எனில் (SrcBytes.empty())
    திரும்பு உண்மை;

  மாறிலி UTF32 *Src = reinterpret_cast<மாறிலி UTF32 *>(SrcBytes.begin());
  மாறிலி UTF32 *SrcEnd = reinterpret_cast<மாறிலி UTF32 *>(SrcBytes.end());

  ப்ரதிஜ்ஞா((uintptr_t)Src % அளவு(UTF32) == 0);

  // Byteswap if necessary.
  std::vector<UTF32> ByteSwapped;
  எனில் (Src[0] == UNI_UTF32_BYTE_ORDER_MARK_SWAPPED) {
    ByteSwapped.செருகு(ByteSwapped.end(), Src, SrcEnd);
    க்கு (UTF32 &I : ByteSwapped)
      I = llvm::byteswap<uint32_t>(I);
    Src = &ByteSwapped[0];
    SrcEnd = &ByteSwapped[ByteSwapped.size() - 1] + 1;
  }

  // Skip the BOM for conversion.
  எனில் (Src[0] == UNI_UTF32_BYTE_ORDER_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  Out.resize(SrcBytes.size() * UNI_MAX_UTF8_BYTES_PER_CODE_POINT + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&Out[0]);
  UTF8 *DstEnd = Dst + Out.size();

  ConversionResult CR =
      ConvertUTF32toUTF8(&Src, SrcEnd, &Dst, DstEnd, strictConversion);
  ப்ரதிஜ்ஞா(CR != targetExhausted);

  எனில் (CR != conversionOK) {
    Out.clear();
    திரும்பு பொய்;
  }

  Out.resize(reinterpret_cast<எழுத்து *>(Dst) - &Out[0]);
  Out.தள்ளு_back(0);
  Out.எடு_back();
  திரும்பு உண்மை;
}

தருக்க convertUTF32ToUTF8String(ArrayRef<UTF32> Src, std::string &Out) {
  திரும்பு convertUTF32ToUTF8String(
      llvm::ArrayRef<எழுத்து>(reinterpret_cast<மாறிலி எழுத்து *>(Src.data()),
                           Src.size() * அளவு(UTF32)),
      Out);
}

தருக்க convertUTF8ToUTF16String(StringRef SrcUTF8,
                              SmallVectorImpl<UTF16> &DstUTF16) {
  ப்ரதிஜ்ஞா(DstUTF16.empty());

  // Avoid OOB by returning early on empty input.
  எனில் (SrcUTF8.empty()) {
    DstUTF16.தள்ளு_back(0);
    DstUTF16.எடு_back();
    திரும்பு உண்மை;
  }

  மாறிலி UTF8 *Src = reinterpret_cast<மாறிலி UTF8 *>(SrcUTF8.begin());
  மாறிலி UTF8 *SrcEnd = reinterpret_cast<மாறிலி UTF8 *>(SrcUTF8.end());

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
  ப்ரதிஜ்ஞா(CR != targetExhausted);

  எனில் (CR != conversionOK) {
    DstUTF16.clear();
    திரும்பு பொய்;
  }

  DstUTF16.resize(Dst - &DstUTF16[0]);
  DstUTF16.தள்ளு_back(0);
  DstUTF16.எடு_back();
  திரும்பு உண்மை;
}

static_assert(அளவு(wchar_t) == 1 || அளவு(wchar_t) == 2 ||
                  அளவு(wchar_t) == 4,
              "Expected wchar_t to be 1, 2, or 4 bytes");

template <typename TResult>
நிலையான உள்வரி தருக்க ConvertUTF8toWideInternal(llvm::StringRef Source,
                                             TResult &Result) {
  // Even in the case of UTF-16, the number of bytes in a UTF-8 string is
  // at least as large as the number of elements in the resulting wide
  // string, because surrogate pairs take at least 4 bytes in UTF-8.
  Result.resize(Source.size() + 1);
  எழுத்து *ResultPtr = reinterpret_cast<எழுத்து *>(&Result[0]);
  மாறிலி UTF8 *ErrorPtr;
  எனில் (!ConvertUTF8toWide(அளவு(wchar_t), Source, ResultPtr, ErrorPtr)) {
    Result.clear();
    திரும்பு பொய்;
  }
  Result.resize(reinterpret_cast<wchar_t *>(ResultPtr) - &Result[0]);
  திரும்பு உண்மை;
}

தருக்க ConvertUTF8toWide(llvm::StringRef Source, std::wstring &Result) {
  திரும்பு ConvertUTF8toWideInternal(Source, Result);
}

தருக்க ConvertUTF8toWide(மாறிலி எழுத்து *Source, std::wstring &Result) {
  எனில் (!Source) {
    Result.clear();
    திரும்பு உண்மை;
  }
  திரும்பு ConvertUTF8toWide(llvm::StringRef(Source), Result);
}

தருக்க convertWideToUTF8(std::wstring_view Source, std::string &Result) {
  எனில் (அளவு(wchar_t) == 1) {
    மாறிலி UTF8 *Start = reinterpret_cast<மாறிலி UTF8 *>(Source.data());
    மாறிலி UTF8 *End =
        reinterpret_cast<மாறிலி UTF8 *>(Source.data() + Source.size());
    எனில் (!isLegalUTF8String(&Start, End))
      திரும்பு பொய்;
    Result.resize(Source.size());
    memcpy(&Result[0], Source.data(), Source.size());
    திரும்பு உண்மை;
  } இல்லையெனில் எனில் (அளவு(wchar_t) == 2) {
    திரும்பு convertUTF16ToUTF8String(
        llvm::ArrayRef<UTF16>(reinterpret_cast<மாறிலி UTF16 *>(Source.data()),
                              Source.size()),
        Result);
  } இல்லையெனில் எனில் (அளவு(wchar_t) == 4) {
    மாறிலி UTF32 *Start = reinterpret_cast<மாறிலி UTF32 *>(Source.data());
    மாறிலி UTF32 *End =
        reinterpret_cast<மாறிலி UTF32 *>(Source.data() + Source.size());
    Result.resize(UNI_MAX_UTF8_BYTES_PER_CODE_POINT * Source.size());
    UTF8 *ResultPtr = reinterpret_cast<UTF8 *>(&Result[0]);
    UTF8 *ResultEnd = reinterpret_cast<UTF8 *>(&Result[0] + Result.size());
    எனில் (ConvertUTF32toUTF8(&Start, End, &ResultPtr, ResultEnd,
                           strictConversion) == conversionOK) {
      Result.resize(reinterpret_cast<எழுத்து *>(ResultPtr) - &Result[0]);
      திரும்பு உண்மை;
    } இல்லையெனில் {
      Result.clear();
      திரும்பு பொய்;
    }
  } இல்லையெனில் {
    llvm_unreachable(
        "Control should never reach this point; see static_assert further up");
  }
}

தருக்க IsSingleCodeUnitUTF8Codepoint(unsigned V) { திரும்பு V <= 0x7F; }

தருக்க IsSingleCodeUnitUTF16Codepoint(unsigned V) {
  திரும்பு V <= 0xD7FF || (V >= 0xE000 && V <= 0xFFFF);
}

தருக்க IsSingleCodeUnitUTF32Codepoint(unsigned V) {
  திரும்பு V <= 0xD7FF || (V >= 0xE000 && V <= 0x10FFFF);
}

} // end namespace llvm

