//===-- ConvertUTFWrapper.cpp - Wrap ConvertUTF.h with clang data types -----===
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/ADT/ArrayRef.h"
#অন্তর্ভাব "llvm/ADT/StringRef.h"
#অন্তর্ভাব "llvm/ADT/bit.h"
#অন্তর্ভাব "llvm/Support/ConvertUTF.h"
#অন্তর্ভাব "llvm/Support/ErrorHandling.h"
#অন্তর্ভাব <string>
#অন্তর্ভাব <vector>

নামাকাশ llvm {

তার্কিক ConvertUTF8toWide(unsigned WideCharWidth, llvm::StringRef Source,
                       বর্ণ *&ResultPtr, ধ্রুব UTF8 *&ErrorPtr) {
  প্রতিজ্ঞা(WideCharWidth == 1 || WideCharWidth == 2 || WideCharWidth == 4);
  ConversionResult result = conversionOK;
  // Copy the character span over.
  যদি (WideCharWidth == 1) {
    ধ্রুব UTF8 *Pos = reinterpret_cast<ধ্রুব UTF8*>(Source.begin());
    যদি (!isLegalUTF8String(&Pos, reinterpret_cast<ধ্রুব UTF8*>(Source.end()))) {
      result = sourceIllegal;
      ErrorPtr = Pos;
    } অন্যথা {
      memcpy(ResultPtr, Source.data(), Source.size());
      ResultPtr += Source.size();
    }
  } অন্যথা যদি (WideCharWidth == 2) {
    ধ্রুব UTF8 *sourceStart = (ধ্রুব UTF8*)Source.data();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF16 *targetStart = reinterpret_cast<UTF16 *>(ResultPtr);
    ConversionFlags flags = strictConversion;
    result =
        ConvertUTF8toUTF16(&sourceStart, sourceStart + Source.size(),
                           &targetStart, targetStart + Source.size(), flags);
    যদি (result == conversionOK)
      ResultPtr = reinterpret_cast<বর্ণ *>(targetStart);
    অন্যথা
      ErrorPtr = sourceStart;
  } অন্যথা যদি (WideCharWidth == 4) {
    ধ্রুব UTF8 *sourceStart = (ধ্রুব UTF8 *)Source.data();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF32 *targetStart = reinterpret_cast<UTF32 *>(ResultPtr);
    ConversionFlags flags = strictConversion;
    result =
        ConvertUTF8toUTF32(&sourceStart, sourceStart + Source.size(),
                           &targetStart, targetStart + Source.size(), flags);
    যদি (result == conversionOK)
      ResultPtr = reinterpret_cast<বর্ণ *>(targetStart);
    অন্যথা
      ErrorPtr = sourceStart;
  }
  প্রতিজ্ঞা((result != targetExhausted) &&
         "ConvertUTF8toUTFXX exhausted target buffer");
  প্রত্যাগম result == conversionOK;
}

তার্কিক ConvertCodePointToUTF8(unsigned Source, বর্ণ *&ResultPtr) {
  ধ্রুব UTF32 *SourceStart = &Source;
  ধ্রুব UTF32 *SourceEnd = SourceStart + 1;
  UTF8 *TargetStart = reinterpret_cast<UTF8 *>(ResultPtr);
  UTF8 *TargetEnd = TargetStart + 4;
  ConversionResult CR = ConvertUTF32toUTF8(
      &SourceStart, SourceEnd, &TargetStart, TargetEnd, strictConversion);
  যদি (CR != conversionOK)
    প্রত্যাগম অসত্য;

  ResultPtr = reinterpret_cast<বর্ণ *>(TargetStart);
  প্রত্যাগম সত্য;
}

তার্কিক hasUTF16ByteOrderMark(ArrayRef<বর্ণ> S) {
  প্রত্যাগম (S.size() >= 2 && ((S[0] == '\xff' && S[1] == '\xfe') ||
                            (S[0] == '\xfe' && S[1] == '\xff')));
}

তার্কিক convertUTF16ToUTF8String(ArrayRef<বর্ণ> SrcBytes, std::string &Out) {
  প্রতিজ্ঞা(Out.empty());

  // Error out on an uneven byte count.
  যদি (SrcBytes.size() % 2)
    প্রত্যাগম অসত্য;

  // Avoid OOB by returning early on empty input.
  যদি (SrcBytes.empty())
    প্রত্যাগম সত্য;

  ধ্রুব UTF16 *Src = reinterpret_cast<ধ্রুব UTF16 *>(SrcBytes.begin());
  ধ্রুব UTF16 *SrcEnd = reinterpret_cast<ধ্রুব UTF16 *>(SrcBytes.end());

  প্রতিজ্ঞা((uintptr_t)Src % পরিমাণ(UTF16) == 0);

  // Byteswap if necessary.
  std::vector<UTF16> ByteSwapped;
  যদি (Src[0] == UNI_UTF16_BYTE_ORDER_MARK_SWAPPED) {
    ByteSwapped.ঢোকাও(ByteSwapped.end(), Src, SrcEnd);
    প্রতি (UTF16 &I : ByteSwapped)
      I = llvm::byteswap<uint16_t>(I);
    Src = &ByteSwapped[0];
    SrcEnd = &ByteSwapped[ByteSwapped.size() - 1] + 1;
  }

  // Skip the BOM for conversion.
  যদি (Src[0] == UNI_UTF16_BYTE_ORDER_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  Out.resize(SrcBytes.size() * UNI_MAX_UTF8_BYTES_PER_CODE_POINT + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&Out[0]);
  UTF8 *DstEnd = Dst + Out.size();

  ConversionResult CR =
      ConvertUTF16toUTF8(&Src, SrcEnd, &Dst, DstEnd, strictConversion);
  প্রতিজ্ঞা(CR != targetExhausted);

  যদি (CR != conversionOK) {
    Out.clear();
    প্রত্যাগম অসত্য;
  }

  Out.resize(reinterpret_cast<বর্ণ *>(Dst) - &Out[0]);
  Out.ঠেলো_back(0);
  Out.বের_করো_back();
  প্রত্যাগম সত্য;
}

তার্কিক convertUTF16ToUTF8String(ArrayRef<UTF16> Src, std::string &Out) {
  প্রত্যাগম convertUTF16ToUTF8String(
      llvm::ArrayRef<বর্ণ>(reinterpret_cast<ধ্রুব বর্ণ *>(Src.data()),
                           Src.size() * পরিমাণ(UTF16)),
      Out);
}

তার্কিক convertUTF32ToUTF8String(ArrayRef<বর্ণ> SrcBytes, std::string &Out) {
  প্রতিজ্ঞা(Out.empty());

  // Error out on an uneven byte count.
  যদি (SrcBytes.size() % 4)
    প্রত্যাগম অসত্য;

  // Avoid OOB by returning early on empty input.
  যদি (SrcBytes.empty())
    প্রত্যাগম সত্য;

  ধ্রুব UTF32 *Src = reinterpret_cast<ধ্রুব UTF32 *>(SrcBytes.begin());
  ধ্রুব UTF32 *SrcEnd = reinterpret_cast<ধ্রুব UTF32 *>(SrcBytes.end());

  প্রতিজ্ঞা((uintptr_t)Src % পরিমাণ(UTF32) == 0);

  // Byteswap if necessary.
  std::vector<UTF32> ByteSwapped;
  যদি (Src[0] == UNI_UTF32_BYTE_ORDER_MARK_SWAPPED) {
    ByteSwapped.ঢোকাও(ByteSwapped.end(), Src, SrcEnd);
    প্রতি (UTF32 &I : ByteSwapped)
      I = llvm::byteswap<uint32_t>(I);
    Src = &ByteSwapped[0];
    SrcEnd = &ByteSwapped[ByteSwapped.size() - 1] + 1;
  }

  // Skip the BOM for conversion.
  যদি (Src[0] == UNI_UTF32_BYTE_ORDER_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  Out.resize(SrcBytes.size() * UNI_MAX_UTF8_BYTES_PER_CODE_POINT + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&Out[0]);
  UTF8 *DstEnd = Dst + Out.size();

  ConversionResult CR =
      ConvertUTF32toUTF8(&Src, SrcEnd, &Dst, DstEnd, strictConversion);
  প্রতিজ্ঞা(CR != targetExhausted);

  যদি (CR != conversionOK) {
    Out.clear();
    প্রত্যাগম অসত্য;
  }

  Out.resize(reinterpret_cast<বর্ণ *>(Dst) - &Out[0]);
  Out.ঠেলো_back(0);
  Out.বের_করো_back();
  প্রত্যাগম সত্য;
}

তার্কিক convertUTF32ToUTF8String(ArrayRef<UTF32> Src, std::string &Out) {
  প্রত্যাগম convertUTF32ToUTF8String(
      llvm::ArrayRef<বর্ণ>(reinterpret_cast<ধ্রুব বর্ণ *>(Src.data()),
                           Src.size() * পরিমাণ(UTF32)),
      Out);
}

তার্কিক convertUTF8ToUTF16String(StringRef SrcUTF8,
                              SmallVectorImpl<UTF16> &DstUTF16) {
  প্রতিজ্ঞা(DstUTF16.empty());

  // Avoid OOB by returning early on empty input.
  যদি (SrcUTF8.empty()) {
    DstUTF16.ঠেলো_back(0);
    DstUTF16.বের_করো_back();
    প্রত্যাগম সত্য;
  }

  ধ্রুব UTF8 *Src = reinterpret_cast<ধ্রুব UTF8 *>(SrcUTF8.begin());
  ধ্রুব UTF8 *SrcEnd = reinterpret_cast<ধ্রুব UTF8 *>(SrcUTF8.end());

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
  প্রতিজ্ঞা(CR != targetExhausted);

  যদি (CR != conversionOK) {
    DstUTF16.clear();
    প্রত্যাগম অসত্য;
  }

  DstUTF16.resize(Dst - &DstUTF16[0]);
  DstUTF16.ঠেলো_back(0);
  DstUTF16.বের_করো_back();
  প্রত্যাগম সত্য;
}

static_assert(পরিমাণ(wchar_t) == 1 || পরিমাণ(wchar_t) == 2 ||
                  পরিমাণ(wchar_t) == 4,
              "Expected wchar_t to be 1, 2, or 4 bytes");

template <typename TResult>
স্থির অন্তঃস্থ তার্কিক ConvertUTF8toWideInternal(llvm::StringRef Source,
                                             TResult &Result) {
  // Even in the case of UTF-16, the number of bytes in a UTF-8 string is
  // at least as large as the number of elements in the resulting wide
  // string, because surrogate pairs take at least 4 bytes in UTF-8.
  Result.resize(Source.size() + 1);
  বর্ণ *ResultPtr = reinterpret_cast<বর্ণ *>(&Result[0]);
  ধ্রুব UTF8 *ErrorPtr;
  যদি (!ConvertUTF8toWide(পরিমাণ(wchar_t), Source, ResultPtr, ErrorPtr)) {
    Result.clear();
    প্রত্যাগম অসত্য;
  }
  Result.resize(reinterpret_cast<wchar_t *>(ResultPtr) - &Result[0]);
  প্রত্যাগম সত্য;
}

তার্কিক ConvertUTF8toWide(llvm::StringRef Source, std::wstring &Result) {
  প্রত্যাগম ConvertUTF8toWideInternal(Source, Result);
}

তার্কিক ConvertUTF8toWide(ধ্রুব বর্ণ *Source, std::wstring &Result) {
  যদি (!Source) {
    Result.clear();
    প্রত্যাগম সত্য;
  }
  প্রত্যাগম ConvertUTF8toWide(llvm::StringRef(Source), Result);
}

তার্কিক convertWideToUTF8(std::wstring_view Source, std::string &Result) {
  যদি (পরিমাণ(wchar_t) == 1) {
    ধ্রুব UTF8 *Start = reinterpret_cast<ধ্রুব UTF8 *>(Source.data());
    ধ্রুব UTF8 *End =
        reinterpret_cast<ধ্রুব UTF8 *>(Source.data() + Source.size());
    যদি (!isLegalUTF8String(&Start, End))
      প্রত্যাগম অসত্য;
    Result.resize(Source.size());
    memcpy(&Result[0], Source.data(), Source.size());
    প্রত্যাগম সত্য;
  } অন্যথা যদি (পরিমাণ(wchar_t) == 2) {
    প্রত্যাগম convertUTF16ToUTF8String(
        llvm::ArrayRef<UTF16>(reinterpret_cast<ধ্রুব UTF16 *>(Source.data()),
                              Source.size()),
        Result);
  } অন্যথা যদি (পরিমাণ(wchar_t) == 4) {
    ধ্রুব UTF32 *Start = reinterpret_cast<ধ্রুব UTF32 *>(Source.data());
    ধ্রুব UTF32 *End =
        reinterpret_cast<ধ্রুব UTF32 *>(Source.data() + Source.size());
    Result.resize(UNI_MAX_UTF8_BYTES_PER_CODE_POINT * Source.size());
    UTF8 *ResultPtr = reinterpret_cast<UTF8 *>(&Result[0]);
    UTF8 *ResultEnd = reinterpret_cast<UTF8 *>(&Result[0] + Result.size());
    যদি (ConvertUTF32toUTF8(&Start, End, &ResultPtr, ResultEnd,
                           strictConversion) == conversionOK) {
      Result.resize(reinterpret_cast<বর্ণ *>(ResultPtr) - &Result[0]);
      প্রত্যাগম সত্য;
    } অন্যথা {
      Result.clear();
      প্রত্যাগম অসত্য;
    }
  } অন্যথা {
    llvm_unreachable(
        "Control should never reach this point; see static_assert further up");
  }
}

তার্কিক IsSingleCodeUnitUTF8Codepoint(unsigned V) { প্রত্যাগম V <= 0x7F; }

তার্কিক IsSingleCodeUnitUTF16Codepoint(unsigned V) {
  প্রত্যাগম V <= 0xD7FF || (V >= 0xE000 && V <= 0xFFFF);
}

তার্কিক IsSingleCodeUnitUTF32Codepoint(unsigned V) {
  প্রত্যাগম V <= 0xD7FF || (V >= 0xE000 && V <= 0x10FFFF);
}

} // end namespace llvm

