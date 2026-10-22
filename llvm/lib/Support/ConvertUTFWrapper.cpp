//===-- ConvertUTFWrapper.cpp - Wrap ConvertUTF.h with clang data types -----===
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/ADT/ArrayRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/bit.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ConvertUTF.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ErrorHandling.h"
#ಅನ್ತರ್ಭಾವ <string>
#ಅನ್ತರ್ಭಾವ <vector>

ನಾಮಾಕಾಶ llvm {

ತಾರ್ಕಿಕ ConvertUTF8toWide(ಚಿಹ್ನೆರಹಿತ Wide_Char_ಅಗಲ, llvm::StringRef ಮೂಲ,
                       ಅಕ್ಷರ *&ಫಲಿತಾಂಶ_Ptr, ಸ್ಥಿರಾಂಕ UTF8 *&ErrorPtr) {
  ಪ್ರತಿಜ್ಞಾ(Wide_Char_ಅಗಲ == 1 || Wide_Char_ಅಗಲ == 2 || Wide_Char_ಅಗಲ == 4);
  Conversion_ಫಲಿತಾಂಶ ಫಲಿತಾಂಶ_2 = conversionOK;
  // Copy the character span over.
  ಆದರೆ (Wide_Char_ಅಗಲ == 1) {
    ಸ್ಥಿರಾಂಕ UTF8 *Pos = reinterpret_cast<ಸ್ಥಿರಾಂಕ UTF8*>(ಮೂಲ.begin());
    ಆದರೆ (!isLegalUTF8String(&Pos, reinterpret_cast<ಸ್ಥಿರಾಂಕ UTF8*>(ಮೂಲ.ಕೊನೆ()))) {
      ಫಲಿತಾಂಶ_2 = ಮೂಲ_Illegal;
      ErrorPtr = Pos;
    } ಇಲ್ಲದಿದ್ದರೆ {
      ಸ್ಮರಣೆ_ನಕಲು(ಫಲಿತಾಂಶ_Ptr, ಮೂಲ.ದತ್ತಾಂಶ(), ಮೂಲ.ಗಾತ್ರ_3());
      ಫಲಿತಾಂಶ_Ptr += ಮೂಲ.ಗಾತ್ರ_3();
    }
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Wide_Char_ಅಗಲ == 2) {
    ಸ್ಥಿರಾಂಕ UTF8 *ಮೂಲ_ಆರಂಭ = (ಸ್ಥಿರಾಂಕ UTF8*)ಮೂಲ.ದತ್ತಾಂಶ();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF16 *ಗುರಿ_ಆರಂಭ = reinterpret_cast<UTF16 *>(ಫಲಿತಾಂಶ_Ptr);
    ConversionFlags flags = strictConversion;
    ಫಲಿತಾಂಶ_2 =
        ConvertUTF8toUTF16(&ಮೂಲ_ಆರಂಭ, ಮೂಲ_ಆರಂಭ + ಮೂಲ.ಗಾತ್ರ_3(),
                           &ಗುರಿ_ಆರಂಭ, ಗುರಿ_ಆರಂಭ + ಮೂಲ.ಗಾತ್ರ_3(), flags);
    ಆದರೆ (ಫಲಿತಾಂಶ_2 == conversionOK)
      ಫಲಿತಾಂಶ_Ptr = reinterpret_cast<ಅಕ್ಷರ *>(ಗುರಿ_ಆರಂಭ);
    ಇಲ್ಲದಿದ್ದರೆ
      ErrorPtr = ಮೂಲ_ಆರಂಭ;
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Wide_Char_ಅಗಲ == 4) {
    ಸ್ಥಿರಾಂಕ UTF8 *ಮೂಲ_ಆರಂಭ = (ಸ್ಥಿರಾಂಕ UTF8 *)ಮೂಲ.ದತ್ತಾಂಶ();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF32 *ಗುರಿ_ಆರಂಭ = reinterpret_cast<UTF32 *>(ಫಲಿತಾಂಶ_Ptr);
    ConversionFlags flags = strictConversion;
    ಫಲಿತಾಂಶ_2 =
        ConvertUTF8toUTF32(&ಮೂಲ_ಆರಂಭ, ಮೂಲ_ಆರಂಭ + ಮೂಲ.ಗಾತ್ರ_3(),
                           &ಗುರಿ_ಆರಂಭ, ಗುರಿ_ಆರಂಭ + ಮೂಲ.ಗಾತ್ರ_3(), flags);
    ಆದರೆ (ಫಲಿತಾಂಶ_2 == conversionOK)
      ಫಲಿತಾಂಶ_Ptr = reinterpret_cast<ಅಕ್ಷರ *>(ಗುರಿ_ಆರಂಭ);
    ಇಲ್ಲದಿದ್ದರೆ
      ErrorPtr = ಮೂಲ_ಆರಂಭ;
  }
  ಪ್ರತಿಜ್ಞಾ((ಫಲಿತಾಂಶ_2 != ಗುರಿ_Exhausted) &&
         "ConvertUTF8toUTFXX exhausted target buffer");
  ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ_2 == conversionOK;
}

ತಾರ್ಕಿಕ Convert_ಸಂಕೇತ_ಬಿಂದು_ಗಮ್ಯ_UTF8(ಚಿಹ್ನೆರಹಿತ ಮೂಲ, ಅಕ್ಷರ *&ಫಲಿತಾಂಶ_Ptr) {
  ಸ್ಥಿರಾಂಕ UTF32 *ಮೂಲ_ಆರಂಭ_2 = &ಮೂಲ;
  ಸ್ಥಿರಾಂಕ UTF32 *ಮೂಲ_ಕೊನೆ_2 = ಮೂಲ_ಆರಂಭ_2 + 1;
  UTF8 *ಗುರಿ_ಆರಂಭ_2 = reinterpret_cast<UTF8 *>(ಫಲಿತಾಂಶ_Ptr);
  UTF8 *ಗುರಿ_ಕೊನೆ_2 = ಗುರಿ_ಆರಂಭ_2 + 4;
  Conversion_ಫಲಿತಾಂಶ CR = ConvertUTF32toUTF8(
      &ಮೂಲ_ಆರಂಭ_2, ಮೂಲ_ಕೊನೆ_2, &ಗುರಿ_ಆರಂಭ_2, ಗುರಿ_ಕೊನೆ_2, strictConversion);
  ಆದರೆ (CR != conversionOK)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  ಫಲಿತಾಂಶ_Ptr = reinterpret_cast<ಅಕ್ಷರ *>(ಗುರಿ_ಆರಂಭ_2);
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ತಾರ್ಕಿಕ has_UTF16_Byte_ಕ್ರಮ_Mark(ArrayRef<ಅಕ್ಷರ> S) {
  ಹಿಂತಿರುಗಿ (S.ಗಾತ್ರ_3() >= 2 && ((S[0] == '\xff' && S[1] == '\xfe') ||
                            (S[0] == '\xfe' && S[1] == '\xff')));
}

ತಾರ್ಕಿಕ convert_UTF16_ಗಮ್ಯ_UTF8_String(ArrayRef<ಅಕ್ಷರ> Src_ಬೈಟುಗಳು, std::string &ಹೊರಹೋಗು) {
  ಪ್ರತಿಜ್ಞಾ(ಹೊರಹೋಗು.ಖಾಲಿಗಳು());

  // Error out on an uneven byte count.
  ಆದರೆ (Src_ಬೈಟುಗಳು.ಗಾತ್ರ_3() % 2)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  // Avoid OOB by returning early on empty input.
  ಆದರೆ (Src_ಬೈಟುಗಳು.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ ನಿಜ;

  ಸ್ಥಿರಾಂಕ UTF16 *Src = reinterpret_cast<ಸ್ಥಿರಾಂಕ UTF16 *>(Src_ಬೈಟುಗಳು.begin());
  ಸ್ಥಿರಾಂಕ UTF16 *Src_ಕೊನೆ = reinterpret_cast<ಸ್ಥಿರಾಂಕ UTF16 *>(Src_ಬೈಟುಗಳು.ಕೊನೆ());

  ಪ್ರತಿಜ್ಞಾ((uintptr_t)Src % ಗಾತ್ರ(UTF16) == 0);

  // Byteswap if necessary.
  std::ಸದಿಶ<UTF16> ByteSwapped;
  ಆದರೆ (Src[0] == UNI_UTF16_BYTE_ಕ್ರಮ_MARK_SWAPPED) {
    ByteSwapped.ಸೇರಿಸು(ByteSwapped.ಕೊನೆ(), Src, Src_ಕೊನೆ);
    ಗಾಗಿ (UTF16 &I : ByteSwapped)
      I = llvm::byteswap<uint16_t>(I);
    Src = &ByteSwapped[0];
    Src_ಕೊನೆ = &ByteSwapped[ByteSwapped.ಗಾತ್ರ_3() - 1] + 1;
  }

  // Skip the BOM for conversion.
  ಆದರೆ (Src[0] == UNI_UTF16_BYTE_ಕ್ರಮ_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  ಹೊರಹೋಗು.resize(Src_ಬೈಟುಗಳು.ಗಾತ್ರ_3() * UNI_MAX_UTF8_ಬೈಟುಗಳು_ಪ್ರತಿ_ಸಂಕೇತ_ಬಿಂದು + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&ಹೊರಹೋಗು[0]);
  UTF8 *Dst_ಕೊನೆ = Dst + ಹೊರಹೋಗು.ಗಾತ್ರ_3();

  Conversion_ಫಲಿತಾಂಶ CR =
      ConvertUTF16toUTF8(&Src, Src_ಕೊನೆ, &Dst, Dst_ಕೊನೆ, strictConversion);
  ಪ್ರತಿಜ್ಞಾ(CR != ಗುರಿ_Exhausted);

  ಆದರೆ (CR != conversionOK) {
    ಹೊರಹೋಗು.clear();
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }

  ಹೊರಹೋಗು.resize(reinterpret_cast<ಅಕ್ಷರ *>(Dst) - &ಹೊರಹೋಗು[0]);
  ಹೊರಹೋಗು.ತಳ್ಳು_ಹಿಂಭಾಗ(0);
  ಹೊರಹೋಗು.ತೆಗೆ_ಹಿಂಭಾಗ();
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ತಾರ್ಕಿಕ convert_UTF16_ಗಮ್ಯ_UTF8_String(ArrayRef<UTF16> Src, std::string &ಹೊರಹೋಗು) {
  ಹಿಂತಿರುಗಿ convert_UTF16_ಗಮ್ಯ_UTF8_String(
      llvm::ArrayRef<ಅಕ್ಷರ>(reinterpret_cast<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *>(Src.ದತ್ತಾಂಶ()),
                           Src.ಗಾತ್ರ_3() * ಗಾತ್ರ(UTF16)),
      ಹೊರಹೋಗು);
}

ತಾರ್ಕಿಕ convert_UTF32_ಗಮ್ಯ_UTF8_String(ArrayRef<ಅಕ್ಷರ> Src_ಬೈಟುಗಳು, std::string &ಹೊರಹೋಗು) {
  ಪ್ರತಿಜ್ಞಾ(ಹೊರಹೋಗು.ಖಾಲಿಗಳು());

  // Error out on an uneven byte count.
  ಆದರೆ (Src_ಬೈಟುಗಳು.ಗಾತ್ರ_3() % 4)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  // Avoid OOB by returning early on empty input.
  ಆದರೆ (Src_ಬೈಟುಗಳು.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ ನಿಜ;

  ಸ್ಥಿರಾಂಕ UTF32 *Src = reinterpret_cast<ಸ್ಥಿರಾಂಕ UTF32 *>(Src_ಬೈಟುಗಳು.begin());
  ಸ್ಥಿರಾಂಕ UTF32 *Src_ಕೊನೆ = reinterpret_cast<ಸ್ಥಿರಾಂಕ UTF32 *>(Src_ಬೈಟುಗಳು.ಕೊನೆ());

  ಪ್ರತಿಜ್ಞಾ((uintptr_t)Src % ಗಾತ್ರ(UTF32) == 0);

  // Byteswap if necessary.
  std::ಸದಿಶ<UTF32> ByteSwapped;
  ಆದರೆ (Src[0] == UNI_UTF32_BYTE_ಕ್ರಮ_MARK_SWAPPED) {
    ByteSwapped.ಸೇರಿಸು(ByteSwapped.ಕೊನೆ(), Src, Src_ಕೊನೆ);
    ಗಾಗಿ (UTF32 &I : ByteSwapped)
      I = llvm::byteswap<uint32_t>(I);
    Src = &ByteSwapped[0];
    Src_ಕೊನೆ = &ByteSwapped[ByteSwapped.ಗಾತ್ರ_3() - 1] + 1;
  }

  // Skip the BOM for conversion.
  ಆದರೆ (Src[0] == UNI_UTF32_BYTE_ಕ್ರಮ_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  ಹೊರಹೋಗು.resize(Src_ಬೈಟುಗಳು.ಗಾತ್ರ_3() * UNI_MAX_UTF8_ಬೈಟುಗಳು_ಪ್ರತಿ_ಸಂಕೇತ_ಬಿಂದು + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&ಹೊರಹೋಗು[0]);
  UTF8 *Dst_ಕೊನೆ = Dst + ಹೊರಹೋಗು.ಗಾತ್ರ_3();

  Conversion_ಫಲಿತಾಂಶ CR =
      ConvertUTF32toUTF8(&Src, Src_ಕೊನೆ, &Dst, Dst_ಕೊನೆ, strictConversion);
  ಪ್ರತಿಜ್ಞಾ(CR != ಗುರಿ_Exhausted);

  ಆದರೆ (CR != conversionOK) {
    ಹೊರಹೋಗು.clear();
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }

  ಹೊರಹೋಗು.resize(reinterpret_cast<ಅಕ್ಷರ *>(Dst) - &ಹೊರಹೋಗು[0]);
  ಹೊರಹೋಗು.ತಳ್ಳು_ಹಿಂಭಾಗ(0);
  ಹೊರಹೋಗು.ತೆಗೆ_ಹಿಂಭಾಗ();
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ತಾರ್ಕಿಕ convert_UTF32_ಗಮ್ಯ_UTF8_String(ArrayRef<UTF32> Src, std::string &ಹೊರಹೋಗು) {
  ಹಿಂತಿರುಗಿ convert_UTF32_ಗಮ್ಯ_UTF8_String(
      llvm::ArrayRef<ಅಕ್ಷರ>(reinterpret_cast<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *>(Src.ದತ್ತಾಂಶ()),
                           Src.ಗಾತ್ರ_3() * ಗಾತ್ರ(UTF32)),
      ಹೊರಹೋಗು);
}

ತಾರ್ಕಿಕ convert_UTF8_ಗಮ್ಯ_UTF16_String(StringRef SrcUTF8,
                              Small_ಸದಿಶ_Impl<UTF16> &DstUTF16) {
  ಪ್ರತಿಜ್ಞಾ(DstUTF16.ಖಾಲಿಗಳು());

  // Avoid OOB by returning early on empty input.
  ಆದರೆ (SrcUTF8.ಖಾಲಿಗಳು()) {
    DstUTF16.ತಳ್ಳು_ಹಿಂಭಾಗ(0);
    DstUTF16.ತೆಗೆ_ಹಿಂಭಾಗ();
    ಹಿಂತಿರುಗಿ ನಿಜ;
  }

  ಸ್ಥಿರಾಂಕ UTF8 *Src = reinterpret_cast<ಸ್ಥಿರಾಂಕ UTF8 *>(SrcUTF8.begin());
  ಸ್ಥಿರಾಂಕ UTF8 *Src_ಕೊನೆ = reinterpret_cast<ಸ್ಥಿರಾಂಕ UTF8 *>(SrcUTF8.ಕೊನೆ());

  // Allocate the same number of UTF-16 code units as UTF-8 code units. Encoding
  // as UTF-16 should always require the same amount or less code units than the
  // UTF-8 encoding.  Allocate one extra byte for the null terminator though,
  // so that someone calling DstUTF16.data() gets a null terminated string.
  // We resize down later so we don't have to worry that this over allocates.
  DstUTF16.resize(SrcUTF8.ಗಾತ್ರ_3()+1);
  UTF16 *Dst = &DstUTF16[0];
  UTF16 *Dst_ಕೊನೆ = Dst + DstUTF16.ಗಾತ್ರ_3();

  Conversion_ಫಲಿತಾಂಶ CR =
      ConvertUTF8toUTF16(&Src, Src_ಕೊನೆ, &Dst, Dst_ಕೊನೆ, strictConversion);
  ಪ್ರತಿಜ್ಞಾ(CR != ಗುರಿ_Exhausted);

  ಆದರೆ (CR != conversionOK) {
    DstUTF16.clear();
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }

  DstUTF16.resize(Dst - &DstUTF16[0]);
  DstUTF16.ತಳ್ಳು_ಹಿಂಭಾಗ(0);
  DstUTF16.ತೆಗೆ_ಹಿಂಭಾಗ();
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

static_assert(ಗಾತ್ರ(wchar_t) == 1 || ಗಾತ್ರ(wchar_t) == 2 ||
                  ಗಾತ್ರ(wchar_t) == 4,
              "Expected wchar_t to be 1, 2, or 4 bytes");

ಮಾದರಿ <ಬಗೆ_ಹೆಸರು TResult>
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ತಾರ್ಕಿಕ Convert_UTF8to_Wide_ಆಂತರಿಕ(llvm::StringRef ಮೂಲ,
                                             TResult &ಫಲಿತಾಂಶ) {
  // Even in the case of UTF-16, the number of bytes in a UTF-8 string is
  // at least as large as the number of elements in the resulting wide
  // string, because surrogate pairs take at least 4 bytes in UTF-8.
  ಫಲಿತಾಂಶ.resize(ಮೂಲ.ಗಾತ್ರ_3() + 1);
  ಅಕ್ಷರ *ಫಲಿತಾಂಶ_Ptr = reinterpret_cast<ಅಕ್ಷರ *>(&ಫಲಿತಾಂಶ[0]);
  ಸ್ಥಿರಾಂಕ UTF8 *ErrorPtr;
  ಆದರೆ (!ConvertUTF8toWide(ಗಾತ್ರ(wchar_t), ಮೂಲ, ಫಲಿತಾಂಶ_Ptr, ErrorPtr)) {
    ಫಲಿತಾಂಶ.clear();
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }
  ಫಲಿತಾಂಶ.resize(reinterpret_cast<wchar_t *>(ಫಲಿತಾಂಶ_Ptr) - &ಫಲಿತಾಂಶ[0]);
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ತಾರ್ಕಿಕ ConvertUTF8toWide(llvm::StringRef ಮೂಲ, std::wstring &ಫಲಿತಾಂಶ) {
  ಹಿಂತಿರುಗಿ Convert_UTF8to_Wide_ಆಂತರಿಕ(ಮೂಲ, ಫಲಿತಾಂಶ);
}

ತಾರ್ಕಿಕ ConvertUTF8toWide(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮೂಲ, std::wstring &ಫಲಿತಾಂಶ) {
  ಆದರೆ (!ಮೂಲ) {
    ಫಲಿತಾಂಶ.clear();
    ಹಿಂತಿರುಗಿ ನಿಜ;
  }
  ಹಿಂತಿರುಗಿ ConvertUTF8toWide(llvm::StringRef(ಮೂಲ), ಫಲಿತಾಂಶ);
}

ತಾರ್ಕಿಕ convert_Wide_ಗಮ್ಯ_UTF8(std::wstring_view ಮೂಲ, std::string &ಫಲಿತಾಂಶ) {
  ಆದರೆ (ಗಾತ್ರ(wchar_t) == 1) {
    ಸ್ಥಿರಾಂಕ UTF8 *ಆರಂಭ = reinterpret_cast<ಸ್ಥಿರಾಂಕ UTF8 *>(ಮೂಲ.ದತ್ತಾಂಶ());
    ಸ್ಥಿರಾಂಕ UTF8 *ಕೊನೆ_2 =
        reinterpret_cast<ಸ್ಥಿರಾಂಕ UTF8 *>(ಮೂಲ.ದತ್ತಾಂಶ() + ಮೂಲ.ಗಾತ್ರ_3());
    ಆದರೆ (!isLegalUTF8String(&ಆರಂಭ, ಕೊನೆ_2))
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    ಫಲಿತಾಂಶ.resize(ಮೂಲ.ಗಾತ್ರ_3());
    ಸ್ಮರಣೆ_ನಕಲು(&ಫಲಿತಾಂಶ[0], ಮೂಲ.ದತ್ತಾಂಶ(), ಮೂಲ.ಗಾತ್ರ_3());
    ಹಿಂತಿರುಗಿ ನಿಜ;
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಗಾತ್ರ(wchar_t) == 2) {
    ಹಿಂತಿರುಗಿ convert_UTF16_ಗಮ್ಯ_UTF8_String(
        llvm::ArrayRef<UTF16>(reinterpret_cast<ಸ್ಥಿರಾಂಕ UTF16 *>(ಮೂಲ.ದತ್ತಾಂಶ()),
                              ಮೂಲ.ಗಾತ್ರ_3()),
        ಫಲಿತಾಂಶ);
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಗಾತ್ರ(wchar_t) == 4) {
    ಸ್ಥಿರಾಂಕ UTF32 *ಆರಂಭ = reinterpret_cast<ಸ್ಥಿರಾಂಕ UTF32 *>(ಮೂಲ.ದತ್ತಾಂಶ());
    ಸ್ಥಿರಾಂಕ UTF32 *ಕೊನೆ_2 =
        reinterpret_cast<ಸ್ಥಿರಾಂಕ UTF32 *>(ಮೂಲ.ದತ್ತಾಂಶ() + ಮೂಲ.ಗಾತ್ರ_3());
    ಫಲಿತಾಂಶ.resize(UNI_MAX_UTF8_ಬೈಟುಗಳು_ಪ್ರತಿ_ಸಂಕೇತ_ಬಿಂದು * ಮೂಲ.ಗಾತ್ರ_3());
    UTF8 *ಫಲಿತಾಂಶ_Ptr = reinterpret_cast<UTF8 *>(&ಫಲಿತಾಂಶ[0]);
    UTF8 *ಫಲಿತಾಂಶ_ಕೊನೆ = reinterpret_cast<UTF8 *>(&ಫಲಿತಾಂಶ[0] + ಫಲಿತಾಂಶ.ಗಾತ್ರ_3());
    ಆದರೆ (ConvertUTF32toUTF8(&ಆರಂಭ, ಕೊನೆ_2, &ಫಲಿತಾಂಶ_Ptr, ಫಲಿತಾಂಶ_ಕೊನೆ,
                           strictConversion) == conversionOK) {
      ಫಲಿತಾಂಶ.resize(reinterpret_cast<ಅಕ್ಷರ *>(ಫಲಿತಾಂಶ_Ptr) - &ಫಲಿತಾಂಶ[0]);
      ಹಿಂತಿರುಗಿ ನಿಜ;
    } ಇಲ್ಲದಿದ್ದರೆ {
      ಫಲಿತಾಂಶ.clear();
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    }
  } ಇಲ್ಲದಿದ್ದರೆ {
    llvm_unreachable(
        "Control should never reach this point; see static_assert further up");
  }
}

ತಾರ್ಕಿಕ Is_Single_ಸಂಕೇತ_Unit_UTF8_Codepoint(ಚಿಹ್ನೆರಹಿತ V) { ಹಿಂತಿರುಗಿ V <= 0x7F; }

ತಾರ್ಕಿಕ Is_Single_ಸಂಕೇತ_Unit_UTF16_Codepoint(ಚಿಹ್ನೆರಹಿತ V) {
  ಹಿಂತಿರುಗಿ V <= 0xD7FF || (V >= 0xE000 && V <= 0xFFFF);
}

ತಾರ್ಕಿಕ Is_Single_ಸಂಕೇತ_Unit_UTF32_Codepoint(ಚಿಹ್ನೆರಹಿತ V) {
  ಹಿಂತಿರುಗಿ V <= 0xD7FF || (V >= 0xE000 && V <= 0x10FFFF);
}

} // end namespace llvm

