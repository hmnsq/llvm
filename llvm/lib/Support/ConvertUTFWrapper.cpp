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

যৌক্তিক ConvertUTF8toWide(unsigned WideCharWidth, llvm::StringRef Source,
                       অক্ষর *&ফলাফল_Ptr, ধ্রুবক UTF8 *&ErrorPtr) {
  প্রতিজ্ঞা(WideCharWidth == 1 || WideCharWidth == 2 || WideCharWidth == 4);
  Conversion_ফলাফল ফলাফল_2 = conversionOK;
  // Copy the character span over.
  যদি (WideCharWidth == 1) {
    ধ্রুবক UTF8 *Pos = reinterpret_cast<ধ্রুবক UTF8*>(Source.begin());
    যদি (!isLegalUTF8String(&Pos, reinterpret_cast<ধ্রুবক UTF8*>(Source.end()))) {
      ফলাফল_2 = sourceIllegal;
      ErrorPtr = Pos;
    } অন্যথা {
      memcpy(ফলাফল_Ptr, Source.তথ্য(), Source.আকার_3());
      ফলাফল_Ptr += Source.আকার_3();
    }
  } অন্যথা যদি (WideCharWidth == 2) {
    ধ্রুবক UTF8 *sourceStart = (ধ্রুবক UTF8*)Source.তথ্য();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF16 *targetStart = reinterpret_cast<UTF16 *>(ফলাফল_Ptr);
    ConversionFlags flags = strictConversion;
    ফলাফল_2 =
        ConvertUTF8toUTF16(&sourceStart, sourceStart + Source.আকার_3(),
                           &targetStart, targetStart + Source.আকার_3(), flags);
    যদি (ফলাফল_2 == conversionOK)
      ফলাফল_Ptr = reinterpret_cast<অক্ষর *>(targetStart);
    অন্যথা
      ErrorPtr = sourceStart;
  } অন্যথা যদি (WideCharWidth == 4) {
    ধ্রুবক UTF8 *sourceStart = (ধ্রুবক UTF8 *)Source.তথ্য();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF32 *targetStart = reinterpret_cast<UTF32 *>(ফলাফল_Ptr);
    ConversionFlags flags = strictConversion;
    ফলাফল_2 =
        ConvertUTF8toUTF32(&sourceStart, sourceStart + Source.আকার_3(),
                           &targetStart, targetStart + Source.আকার_3(), flags);
    যদি (ফলাফল_2 == conversionOK)
      ফলাফল_Ptr = reinterpret_cast<অক্ষর *>(targetStart);
    অন্যথা
      ErrorPtr = sourceStart;
  }
  প্রতিজ্ঞা((ফলাফল_2 != targetExhausted) &&
         "ConvertUTF8toUTFXX exhausted target buffer");
  ফেরত ফলাফল_2 == conversionOK;
}

যৌক্তিক ConvertCodePointToUTF8(unsigned Source, অক্ষর *&ফলাফল_Ptr) {
  ধ্রুবক UTF32 *SourceStart = &Source;
  ধ্রুবক UTF32 *SourceEnd = SourceStart + 1;
  UTF8 *TargetStart = reinterpret_cast<UTF8 *>(ফলাফল_Ptr);
  UTF8 *TargetEnd = TargetStart + 4;
  Conversion_ফলাফল CR = ConvertUTF32toUTF8(
      &SourceStart, SourceEnd, &TargetStart, TargetEnd, strictConversion);
  যদি (CR != conversionOK)
    ফেরত মিথ্যা;

  ফলাফল_Ptr = reinterpret_cast<অক্ষর *>(TargetStart);
  ফেরত সত্য;
}

যৌক্তিক hasUTF16ByteOrderMark(ArrayRef<অক্ষর> S) {
  ফেরত (S.আকার_3() >= 2 && ((S[0] == '\xff' && S[1] == '\xfe') ||
                            (S[0] == '\xfe' && S[1] == '\xff')));
}

যৌক্তিক convertUTF16ToUTF8String(ArrayRef<অক্ষর> SrcBytes, std::string &Out) {
  প্রতিজ্ঞা(Out.empty());

  // Error out on an uneven byte count.
  যদি (SrcBytes.আকার_3() % 2)
    ফেরত মিথ্যা;

  // Avoid OOB by returning early on empty input.
  যদি (SrcBytes.empty())
    ফেরত সত্য;

  ধ্রুবক UTF16 *Src = reinterpret_cast<ধ্রুবক UTF16 *>(SrcBytes.begin());
  ধ্রুবক UTF16 *SrcEnd = reinterpret_cast<ধ্রুবক UTF16 *>(SrcBytes.end());

  প্রতিজ্ঞা((uintptr_t)Src % আকার(UTF16) == 0);

  // Byteswap if necessary.
  std::vector<UTF16> ByteSwapped;
  যদি (Src[0] == UNI_UTF16_BYTE_ORDER_MARK_SWAPPED) {
    ByteSwapped.ঢোকাও(ByteSwapped.end(), Src, SrcEnd);
    জন্য (UTF16 &I : ByteSwapped)
      I = llvm::byteswap<uint16_t>(I);
    Src = &ByteSwapped[0];
    SrcEnd = &ByteSwapped[ByteSwapped.আকার_3() - 1] + 1;
  }

  // Skip the BOM for conversion.
  যদি (Src[0] == UNI_UTF16_BYTE_ORDER_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  Out.resize(SrcBytes.আকার_3() * UNI_MAX_UTF8_BYTES_PER_CODE_POINT + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&Out[0]);
  UTF8 *DstEnd = Dst + Out.আকার_3();

  Conversion_ফলাফল CR =
      ConvertUTF16toUTF8(&Src, SrcEnd, &Dst, DstEnd, strictConversion);
  প্রতিজ্ঞা(CR != targetExhausted);

  যদি (CR != conversionOK) {
    Out.clear();
    ফেরত মিথ্যা;
  }

  Out.resize(reinterpret_cast<অক্ষর *>(Dst) - &Out[0]);
  Out.ঠেলো_back(0);
  Out.বের_করো_back();
  ফেরত সত্য;
}

যৌক্তিক convertUTF16ToUTF8String(ArrayRef<UTF16> Src, std::string &Out) {
  ফেরত convertUTF16ToUTF8String(
      llvm::ArrayRef<অক্ষর>(reinterpret_cast<ধ্রুবক অক্ষর *>(Src.তথ্য()),
                           Src.আকার_3() * আকার(UTF16)),
      Out);
}

যৌক্তিক convertUTF32ToUTF8String(ArrayRef<অক্ষর> SrcBytes, std::string &Out) {
  প্রতিজ্ঞা(Out.empty());

  // Error out on an uneven byte count.
  যদি (SrcBytes.আকার_3() % 4)
    ফেরত মিথ্যা;

  // Avoid OOB by returning early on empty input.
  যদি (SrcBytes.empty())
    ফেরত সত্য;

  ধ্রুবক UTF32 *Src = reinterpret_cast<ধ্রুবক UTF32 *>(SrcBytes.begin());
  ধ্রুবক UTF32 *SrcEnd = reinterpret_cast<ধ্রুবক UTF32 *>(SrcBytes.end());

  প্রতিজ্ঞা((uintptr_t)Src % আকার(UTF32) == 0);

  // Byteswap if necessary.
  std::vector<UTF32> ByteSwapped;
  যদি (Src[0] == UNI_UTF32_BYTE_ORDER_MARK_SWAPPED) {
    ByteSwapped.ঢোকাও(ByteSwapped.end(), Src, SrcEnd);
    জন্য (UTF32 &I : ByteSwapped)
      I = llvm::byteswap<uint32_t>(I);
    Src = &ByteSwapped[0];
    SrcEnd = &ByteSwapped[ByteSwapped.আকার_3() - 1] + 1;
  }

  // Skip the BOM for conversion.
  যদি (Src[0] == UNI_UTF32_BYTE_ORDER_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  Out.resize(SrcBytes.আকার_3() * UNI_MAX_UTF8_BYTES_PER_CODE_POINT + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&Out[0]);
  UTF8 *DstEnd = Dst + Out.আকার_3();

  Conversion_ফলাফল CR =
      ConvertUTF32toUTF8(&Src, SrcEnd, &Dst, DstEnd, strictConversion);
  প্রতিজ্ঞা(CR != targetExhausted);

  যদি (CR != conversionOK) {
    Out.clear();
    ফেরত মিথ্যা;
  }

  Out.resize(reinterpret_cast<অক্ষর *>(Dst) - &Out[0]);
  Out.ঠেলো_back(0);
  Out.বের_করো_back();
  ফেরত সত্য;
}

যৌক্তিক convertUTF32ToUTF8String(ArrayRef<UTF32> Src, std::string &Out) {
  ফেরত convertUTF32ToUTF8String(
      llvm::ArrayRef<অক্ষর>(reinterpret_cast<ধ্রুবক অক্ষর *>(Src.তথ্য()),
                           Src.আকার_3() * আকার(UTF32)),
      Out);
}

যৌক্তিক convertUTF8ToUTF16String(StringRef SrcUTF8,
                              SmallVectorImpl<UTF16> &DstUTF16) {
  প্রতিজ্ঞা(DstUTF16.empty());

  // Avoid OOB by returning early on empty input.
  যদি (SrcUTF8.empty()) {
    DstUTF16.ঠেলো_back(0);
    DstUTF16.বের_করো_back();
    ফেরত সত্য;
  }

  ধ্রুবক UTF8 *Src = reinterpret_cast<ধ্রুবক UTF8 *>(SrcUTF8.begin());
  ধ্রুবক UTF8 *SrcEnd = reinterpret_cast<ধ্রুবক UTF8 *>(SrcUTF8.end());

  // Allocate the same number of UTF-16 code units as UTF-8 code units. Encoding
  // as UTF-16 should always require the same amount or less code units than the
  // UTF-8 encoding.  Allocate one extra byte for the null terminator though,
  // so that someone calling DstUTF16.data() gets a null terminated string.
  // We resize down later so we don't have to worry that this over allocates.
  DstUTF16.resize(SrcUTF8.আকার_3()+1);
  UTF16 *Dst = &DstUTF16[0];
  UTF16 *DstEnd = Dst + DstUTF16.আকার_3();

  Conversion_ফলাফল CR =
      ConvertUTF8toUTF16(&Src, SrcEnd, &Dst, DstEnd, strictConversion);
  প্রতিজ্ঞা(CR != targetExhausted);

  যদি (CR != conversionOK) {
    DstUTF16.clear();
    ফেরত মিথ্যা;
  }

  DstUTF16.resize(Dst - &DstUTF16[0]);
  DstUTF16.ঠেলো_back(0);
  DstUTF16.বের_করো_back();
  ফেরত সত্য;
}

static_assert(আকার(wchar_t) == 1 || আকার(wchar_t) == 2 ||
                  আকার(wchar_t) == 4,
              "Expected wchar_t to be 1, 2, or 4 bytes");

template <typename TResult>
স্থির অন্তঃরেখ যৌক্তিক ConvertUTF8toWideInternal(llvm::StringRef Source,
                                             TResult &ফলাফল) {
  // Even in the case of UTF-16, the number of bytes in a UTF-8 string is
  // at least as large as the number of elements in the resulting wide
  // string, because surrogate pairs take at least 4 bytes in UTF-8.
  ফলাফল.resize(Source.আকার_3() + 1);
  অক্ষর *ফলাফল_Ptr = reinterpret_cast<অক্ষর *>(&ফলাফল[0]);
  ধ্রুবক UTF8 *ErrorPtr;
  যদি (!ConvertUTF8toWide(আকার(wchar_t), Source, ফলাফল_Ptr, ErrorPtr)) {
    ফলাফল.clear();
    ফেরত মিথ্যা;
  }
  ফলাফল.resize(reinterpret_cast<wchar_t *>(ফলাফল_Ptr) - &ফলাফল[0]);
  ফেরত সত্য;
}

যৌক্তিক ConvertUTF8toWide(llvm::StringRef Source, std::wstring &ফলাফল) {
  ফেরত ConvertUTF8toWideInternal(Source, ফলাফল);
}

যৌক্তিক ConvertUTF8toWide(ধ্রুবক অক্ষর *Source, std::wstring &ফলাফল) {
  যদি (!Source) {
    ফলাফল.clear();
    ফেরত সত্য;
  }
  ফেরত ConvertUTF8toWide(llvm::StringRef(Source), ফলাফল);
}

যৌক্তিক convertWideToUTF8(std::wstring_view Source, std::string &ফলাফল) {
  যদি (আকার(wchar_t) == 1) {
    ধ্রুবক UTF8 *Start = reinterpret_cast<ধ্রুবক UTF8 *>(Source.তথ্য());
    ধ্রুবক UTF8 *End =
        reinterpret_cast<ধ্রুবক UTF8 *>(Source.তথ্য() + Source.আকার_3());
    যদি (!isLegalUTF8String(&Start, End))
      ফেরত মিথ্যা;
    ফলাফল.resize(Source.আকার_3());
    memcpy(&ফলাফল[0], Source.তথ্য(), Source.আকার_3());
    ফেরত সত্য;
  } অন্যথা যদি (আকার(wchar_t) == 2) {
    ফেরত convertUTF16ToUTF8String(
        llvm::ArrayRef<UTF16>(reinterpret_cast<ধ্রুবক UTF16 *>(Source.তথ্য()),
                              Source.আকার_3()),
        ফলাফল);
  } অন্যথা যদি (আকার(wchar_t) == 4) {
    ধ্রুবক UTF32 *Start = reinterpret_cast<ধ্রুবক UTF32 *>(Source.তথ্য());
    ধ্রুবক UTF32 *End =
        reinterpret_cast<ধ্রুবক UTF32 *>(Source.তথ্য() + Source.আকার_3());
    ফলাফল.resize(UNI_MAX_UTF8_BYTES_PER_CODE_POINT * Source.আকার_3());
    UTF8 *ফলাফল_Ptr = reinterpret_cast<UTF8 *>(&ফলাফল[0]);
    UTF8 *ফলাফল_End = reinterpret_cast<UTF8 *>(&ফলাফল[0] + ফলাফল.আকার_3());
    যদি (ConvertUTF32toUTF8(&Start, End, &ফলাফল_Ptr, ফলাফল_End,
                           strictConversion) == conversionOK) {
      ফলাফল.resize(reinterpret_cast<অক্ষর *>(ফলাফল_Ptr) - &ফলাফল[0]);
      ফেরত সত্য;
    } অন্যথা {
      ফলাফল.clear();
      ফেরত মিথ্যা;
    }
  } অন্যথা {
    llvm_unreachable(
        "Control should never reach this point; see static_assert further up");
  }
}

যৌক্তিক IsSingleCodeUnitUTF8Codepoint(unsigned V) { ফেরত V <= 0x7F; }

যৌক্তিক IsSingleCodeUnitUTF16Codepoint(unsigned V) {
  ফেরত V <= 0xD7FF || (V >= 0xE000 && V <= 0xFFFF);
}

যৌক্তিক IsSingleCodeUnitUTF32Codepoint(unsigned V) {
  ফেরত V <= 0xD7FF || (V >= 0xE000 && V <= 0x10FFFF);
}

} // end namespace llvm

