//===-- ConvertUTFWrapper.cpp - Wrap ConvertUTF.h with clang data types -----===
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/ADT/ArrayRef.h"
#અન્તર્ભાવ "llvm/ADT/StringRef.h"
#અન્તર્ભાવ "llvm/ADT/bit.h"
#અન્તર્ભાવ "llvm/Support/ConvertUTF.h"
#અન્તર્ભાવ "llvm/Support/ErrorHandling.h"
#અન્તર્ભાવ <string>
#અન્તર્ભાવ <vector>

નામાકાશ llvm {

તાર્કિક ConvertUTF8toWide(unsigned WideCharWidth, llvm::StringRef Source,
                       અક્ષર *&ResultPtr, અચલ UTF8 *&ErrorPtr) {
  પ્રતિજ્ઞા(WideCharWidth == 1 || WideCharWidth == 2 || WideCharWidth == 4);
  ConversionResult result = conversionOK;
  // Copy the character span over.
  જો (WideCharWidth == 1) {
    અચલ UTF8 *Pos = reinterpret_cast<અચલ UTF8*>(Source.begin());
    જો (!isLegalUTF8String(&Pos, reinterpret_cast<અચલ UTF8*>(Source.end()))) {
      result = sourceIllegal;
      ErrorPtr = Pos;
    } નહીંતર {
      memcpy(ResultPtr, Source.data(), Source.size());
      ResultPtr += Source.size();
    }
  } નહીંતર જો (WideCharWidth == 2) {
    અચલ UTF8 *sourceStart = (અચલ UTF8*)Source.data();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF16 *targetStart = reinterpret_cast<UTF16 *>(ResultPtr);
    ConversionFlags flags = strictConversion;
    result =
        ConvertUTF8toUTF16(&sourceStart, sourceStart + Source.size(),
                           &targetStart, targetStart + Source.size(), flags);
    જો (result == conversionOK)
      ResultPtr = reinterpret_cast<અક્ષર *>(targetStart);
    નહીંતર
      ErrorPtr = sourceStart;
  } નહીંતર જો (WideCharWidth == 4) {
    અચલ UTF8 *sourceStart = (અચલ UTF8 *)Source.data();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF32 *targetStart = reinterpret_cast<UTF32 *>(ResultPtr);
    ConversionFlags flags = strictConversion;
    result =
        ConvertUTF8toUTF32(&sourceStart, sourceStart + Source.size(),
                           &targetStart, targetStart + Source.size(), flags);
    જો (result == conversionOK)
      ResultPtr = reinterpret_cast<અક્ષર *>(targetStart);
    નહીંતર
      ErrorPtr = sourceStart;
  }
  પ્રતિજ્ઞા((result != targetExhausted) &&
         "ConvertUTF8toUTFXX exhausted target buffer");
  પાછા result == conversionOK;
}

તાર્કિક ConvertCodePointToUTF8(unsigned Source, અક્ષર *&ResultPtr) {
  અચલ UTF32 *SourceStart = &Source;
  અચલ UTF32 *SourceEnd = SourceStart + 1;
  UTF8 *TargetStart = reinterpret_cast<UTF8 *>(ResultPtr);
  UTF8 *TargetEnd = TargetStart + 4;
  ConversionResult CR = ConvertUTF32toUTF8(
      &SourceStart, SourceEnd, &TargetStart, TargetEnd, strictConversion);
  જો (CR != conversionOK)
    પાછા ખોટું;

  ResultPtr = reinterpret_cast<અક્ષર *>(TargetStart);
  પાછા સાચું;
}

તાર્કિક hasUTF16ByteOrderMark(ArrayRef<અક્ષર> S) {
  પાછા (S.size() >= 2 && ((S[0] == '\xff' && S[1] == '\xfe') ||
                            (S[0] == '\xfe' && S[1] == '\xff')));
}

તાર્કિક convertUTF16ToUTF8String(ArrayRef<અક્ષર> SrcBytes, std::string &Out) {
  પ્રતિજ્ઞા(Out.empty());

  // Error out on an uneven byte count.
  જો (SrcBytes.size() % 2)
    પાછા ખોટું;

  // Avoid OOB by returning early on empty input.
  જો (SrcBytes.empty())
    પાછા સાચું;

  અચલ UTF16 *Src = reinterpret_cast<અચલ UTF16 *>(SrcBytes.begin());
  અચલ UTF16 *SrcEnd = reinterpret_cast<અચલ UTF16 *>(SrcBytes.end());

  પ્રતિજ્ઞા((uintptr_t)Src % કદ(UTF16) == 0);

  // Byteswap if necessary.
  std::vector<UTF16> ByteSwapped;
  જો (Src[0] == UNI_UTF16_BYTE_ORDER_MARK_SWAPPED) {
    ByteSwapped.ઉમેરો(ByteSwapped.end(), Src, SrcEnd);
    માટે (UTF16 &I : ByteSwapped)
      I = llvm::byteswap<uint16_t>(I);
    Src = &ByteSwapped[0];
    SrcEnd = &ByteSwapped[ByteSwapped.size() - 1] + 1;
  }

  // Skip the BOM for conversion.
  જો (Src[0] == UNI_UTF16_BYTE_ORDER_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  Out.resize(SrcBytes.size() * UNI_MAX_UTF8_BYTES_PER_CODE_POINT + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&Out[0]);
  UTF8 *DstEnd = Dst + Out.size();

  ConversionResult CR =
      ConvertUTF16toUTF8(&Src, SrcEnd, &Dst, DstEnd, strictConversion);
  પ્રતિજ્ઞા(CR != targetExhausted);

  જો (CR != conversionOK) {
    Out.clear();
    પાછા ખોટું;
  }

  Out.resize(reinterpret_cast<અક્ષર *>(Dst) - &Out[0]);
  Out.ધકેલો_back(0);
  Out.કાઢો_back();
  પાછા સાચું;
}

તાર્કિક convertUTF16ToUTF8String(ArrayRef<UTF16> Src, std::string &Out) {
  પાછા convertUTF16ToUTF8String(
      llvm::ArrayRef<અક્ષર>(reinterpret_cast<અચલ અક્ષર *>(Src.data()),
                           Src.size() * કદ(UTF16)),
      Out);
}

તાર્કિક convertUTF32ToUTF8String(ArrayRef<અક્ષર> SrcBytes, std::string &Out) {
  પ્રતિજ્ઞા(Out.empty());

  // Error out on an uneven byte count.
  જો (SrcBytes.size() % 4)
    પાછા ખોટું;

  // Avoid OOB by returning early on empty input.
  જો (SrcBytes.empty())
    પાછા સાચું;

  અચલ UTF32 *Src = reinterpret_cast<અચલ UTF32 *>(SrcBytes.begin());
  અચલ UTF32 *SrcEnd = reinterpret_cast<અચલ UTF32 *>(SrcBytes.end());

  પ્રતિજ્ઞા((uintptr_t)Src % કદ(UTF32) == 0);

  // Byteswap if necessary.
  std::vector<UTF32> ByteSwapped;
  જો (Src[0] == UNI_UTF32_BYTE_ORDER_MARK_SWAPPED) {
    ByteSwapped.ઉમેરો(ByteSwapped.end(), Src, SrcEnd);
    માટે (UTF32 &I : ByteSwapped)
      I = llvm::byteswap<uint32_t>(I);
    Src = &ByteSwapped[0];
    SrcEnd = &ByteSwapped[ByteSwapped.size() - 1] + 1;
  }

  // Skip the BOM for conversion.
  જો (Src[0] == UNI_UTF32_BYTE_ORDER_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  Out.resize(SrcBytes.size() * UNI_MAX_UTF8_BYTES_PER_CODE_POINT + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&Out[0]);
  UTF8 *DstEnd = Dst + Out.size();

  ConversionResult CR =
      ConvertUTF32toUTF8(&Src, SrcEnd, &Dst, DstEnd, strictConversion);
  પ્રતિજ્ઞા(CR != targetExhausted);

  જો (CR != conversionOK) {
    Out.clear();
    પાછા ખોટું;
  }

  Out.resize(reinterpret_cast<અક્ષર *>(Dst) - &Out[0]);
  Out.ધકેલો_back(0);
  Out.કાઢો_back();
  પાછા સાચું;
}

તાર્કિક convertUTF32ToUTF8String(ArrayRef<UTF32> Src, std::string &Out) {
  પાછા convertUTF32ToUTF8String(
      llvm::ArrayRef<અક્ષર>(reinterpret_cast<અચલ અક્ષર *>(Src.data()),
                           Src.size() * કદ(UTF32)),
      Out);
}

તાર્કિક convertUTF8ToUTF16String(StringRef SrcUTF8,
                              SmallVectorImpl<UTF16> &DstUTF16) {
  પ્રતિજ્ઞા(DstUTF16.empty());

  // Avoid OOB by returning early on empty input.
  જો (SrcUTF8.empty()) {
    DstUTF16.ધકેલો_back(0);
    DstUTF16.કાઢો_back();
    પાછા સાચું;
  }

  અચલ UTF8 *Src = reinterpret_cast<અચલ UTF8 *>(SrcUTF8.begin());
  અચલ UTF8 *SrcEnd = reinterpret_cast<અચલ UTF8 *>(SrcUTF8.end());

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
  પ્રતિજ્ઞા(CR != targetExhausted);

  જો (CR != conversionOK) {
    DstUTF16.clear();
    પાછા ખોટું;
  }

  DstUTF16.resize(Dst - &DstUTF16[0]);
  DstUTF16.ધકેલો_back(0);
  DstUTF16.કાઢો_back();
  પાછા સાચું;
}

static_assert(કદ(wchar_t) == 1 || કદ(wchar_t) == 2 ||
                  કદ(wchar_t) == 4,
              "Expected wchar_t to be 1, 2, or 4 bytes");

template <typename TResult>
સ્થિર અંતઃરેખ તાર્કિક ConvertUTF8toWideInternal(llvm::StringRef Source,
                                             TResult &Result) {
  // Even in the case of UTF-16, the number of bytes in a UTF-8 string is
  // at least as large as the number of elements in the resulting wide
  // string, because surrogate pairs take at least 4 bytes in UTF-8.
  Result.resize(Source.size() + 1);
  અક્ષર *ResultPtr = reinterpret_cast<અક્ષર *>(&Result[0]);
  અચલ UTF8 *ErrorPtr;
  જો (!ConvertUTF8toWide(કદ(wchar_t), Source, ResultPtr, ErrorPtr)) {
    Result.clear();
    પાછા ખોટું;
  }
  Result.resize(reinterpret_cast<wchar_t *>(ResultPtr) - &Result[0]);
  પાછા સાચું;
}

તાર્કિક ConvertUTF8toWide(llvm::StringRef Source, std::wstring &Result) {
  પાછા ConvertUTF8toWideInternal(Source, Result);
}

તાર્કિક ConvertUTF8toWide(અચલ અક્ષર *Source, std::wstring &Result) {
  જો (!Source) {
    Result.clear();
    પાછા સાચું;
  }
  પાછા ConvertUTF8toWide(llvm::StringRef(Source), Result);
}

તાર્કિક convertWideToUTF8(std::wstring_view Source, std::string &Result) {
  જો (કદ(wchar_t) == 1) {
    અચલ UTF8 *Start = reinterpret_cast<અચલ UTF8 *>(Source.data());
    અચલ UTF8 *End =
        reinterpret_cast<અચલ UTF8 *>(Source.data() + Source.size());
    જો (!isLegalUTF8String(&Start, End))
      પાછા ખોટું;
    Result.resize(Source.size());
    memcpy(&Result[0], Source.data(), Source.size());
    પાછા સાચું;
  } નહીંતર જો (કદ(wchar_t) == 2) {
    પાછા convertUTF16ToUTF8String(
        llvm::ArrayRef<UTF16>(reinterpret_cast<અચલ UTF16 *>(Source.data()),
                              Source.size()),
        Result);
  } નહીંતર જો (કદ(wchar_t) == 4) {
    અચલ UTF32 *Start = reinterpret_cast<અચલ UTF32 *>(Source.data());
    અચલ UTF32 *End =
        reinterpret_cast<અચલ UTF32 *>(Source.data() + Source.size());
    Result.resize(UNI_MAX_UTF8_BYTES_PER_CODE_POINT * Source.size());
    UTF8 *ResultPtr = reinterpret_cast<UTF8 *>(&Result[0]);
    UTF8 *ResultEnd = reinterpret_cast<UTF8 *>(&Result[0] + Result.size());
    જો (ConvertUTF32toUTF8(&Start, End, &ResultPtr, ResultEnd,
                           strictConversion) == conversionOK) {
      Result.resize(reinterpret_cast<અક્ષર *>(ResultPtr) - &Result[0]);
      પાછા સાચું;
    } નહીંતર {
      Result.clear();
      પાછા ખોટું;
    }
  } નહીંતર {
    llvm_unreachable(
        "Control should never reach this point; see static_assert further up");
  }
}

તાર્કિક IsSingleCodeUnitUTF8Codepoint(unsigned V) { પાછા V <= 0x7F; }

તાર્કિક IsSingleCodeUnitUTF16Codepoint(unsigned V) {
  પાછા V <= 0xD7FF || (V >= 0xE000 && V <= 0xFFFF);
}

તાર્કિક IsSingleCodeUnitUTF32Codepoint(unsigned V) {
  પાછા V <= 0xD7FF || (V >= 0xE000 && V <= 0x10FFFF);
}

} // end namespace llvm

