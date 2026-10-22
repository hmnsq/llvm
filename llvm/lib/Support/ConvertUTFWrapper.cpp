//===-- ConvertUTFWrapper.cpp - Wrap ConvertUTF.h with clang data types -----===
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/ArrayRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/bit.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ConvertUTF.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ErrorHandling.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <string>
#ਅਨ੍ਤਰ੍ਭਾਵ <vector>

ਨਾਮਾਕਾਸ਼ llvm {

ਤਾਰਕਿਕ ConvertUTF8toWide(ਬਿਨਾਨਿਸ਼ਾਨੀ Wide_Char_ਚੌੜਾਈ, llvm::StringRef ਸਰੋਤ,
                       ਅੱਖਰ *&ਨਤੀਜਾ_Ptr, ਅਚਲ UTF8 *&ErrorPtr) {
  ਪ੍ਰਤਿਜ੍ਞਾ(Wide_Char_ਚੌੜਾਈ == 1 || Wide_Char_ਚੌੜਾਈ == 2 || Wide_Char_ਚੌੜਾਈ == 4);
  Conversion_ਨਤੀਜਾ ਨਤੀਜਾ_2 = conversionOK;
  // Copy the character span over.
  ਜੇ (Wide_Char_ਚੌੜਾਈ == 1) {
    ਅਚਲ UTF8 *Pos = reinterpret_cast<ਅਚਲ UTF8*>(ਸਰੋਤ.begin());
    ਜੇ (!isLegalUTF8String(&Pos, reinterpret_cast<ਅਚਲ UTF8*>(ਸਰੋਤ.ਸਿਰਾ()))) {
      ਨਤੀਜਾ_2 = ਸਰੋਤ_Illegal;
      ErrorPtr = Pos;
    } ਨਹੀਂਤਾਂ {
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਨਤੀਜਾ_Ptr, ਸਰੋਤ.ਅੰਕੜੇ(), ਸਰੋਤ.ਮਾਪ_2());
      ਨਤੀਜਾ_Ptr += ਸਰੋਤ.ਮਾਪ_2();
    }
  } ਨਹੀਂਤਾਂ ਜੇ (Wide_Char_ਚੌੜਾਈ == 2) {
    ਅਚਲ UTF8 *ਸਰੋਤ_ਸ਼ੁਰੂ = (ਅਚਲ UTF8*)ਸਰੋਤ.ਅੰਕੜੇ();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF16 *ਨਿਸ਼ਾਨਾ_ਸ਼ੁਰੂ = reinterpret_cast<UTF16 *>(ਨਤੀਜਾ_Ptr);
    ConversionFlags flags = strictConversion;
    ਨਤੀਜਾ_2 =
        ConvertUTF8toUTF16(&ਸਰੋਤ_ਸ਼ੁਰੂ, ਸਰੋਤ_ਸ਼ੁਰੂ + ਸਰੋਤ.ਮਾਪ_2(),
                           &ਨਿਸ਼ਾਨਾ_ਸ਼ੁਰੂ, ਨਿਸ਼ਾਨਾ_ਸ਼ੁਰੂ + ਸਰੋਤ.ਮਾਪ_2(), flags);
    ਜੇ (ਨਤੀਜਾ_2 == conversionOK)
      ਨਤੀਜਾ_Ptr = reinterpret_cast<ਅੱਖਰ *>(ਨਿਸ਼ਾਨਾ_ਸ਼ੁਰੂ);
    ਨਹੀਂਤਾਂ
      ErrorPtr = ਸਰੋਤ_ਸ਼ੁਰੂ;
  } ਨਹੀਂਤਾਂ ਜੇ (Wide_Char_ਚੌੜਾਈ == 4) {
    ਅਚਲ UTF8 *ਸਰੋਤ_ਸ਼ੁਰੂ = (ਅਚਲ UTF8 *)ਸਰੋਤ.ਅੰਕੜੇ();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF32 *ਨਿਸ਼ਾਨਾ_ਸ਼ੁਰੂ = reinterpret_cast<UTF32 *>(ਨਤੀਜਾ_Ptr);
    ConversionFlags flags = strictConversion;
    ਨਤੀਜਾ_2 =
        ConvertUTF8toUTF32(&ਸਰੋਤ_ਸ਼ੁਰੂ, ਸਰੋਤ_ਸ਼ੁਰੂ + ਸਰੋਤ.ਮਾਪ_2(),
                           &ਨਿਸ਼ਾਨਾ_ਸ਼ੁਰੂ, ਨਿਸ਼ਾਨਾ_ਸ਼ੁਰੂ + ਸਰੋਤ.ਮਾਪ_2(), flags);
    ਜੇ (ਨਤੀਜਾ_2 == conversionOK)
      ਨਤੀਜਾ_Ptr = reinterpret_cast<ਅੱਖਰ *>(ਨਿਸ਼ਾਨਾ_ਸ਼ੁਰੂ);
    ਨਹੀਂਤਾਂ
      ErrorPtr = ਸਰੋਤ_ਸ਼ੁਰੂ;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ((ਨਤੀਜਾ_2 != ਨਿਸ਼ਾਨਾ_Exhausted) &&
         "ConvertUTF8toUTFXX exhausted target buffer");
  ਵਾਪਸ ਨਤੀਜਾ_2 == conversionOK;
}

ਤਾਰਕਿਕ Convert_ਕੋਡ_ਬਿੰਦੂ_ਮੰਜ਼ਿਲ_UTF8(ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਰੋਤ, ਅੱਖਰ *&ਨਤੀਜਾ_Ptr) {
  ਅਚਲ UTF32 *ਸਰੋਤ_ਸ਼ੁਰੂ_2 = &ਸਰੋਤ;
  ਅਚਲ UTF32 *ਸਰੋਤ_ਸਿਰਾ_2 = ਸਰੋਤ_ਸ਼ੁਰੂ_2 + 1;
  UTF8 *ਨਿਸ਼ਾਨਾ_ਸ਼ੁਰੂ_2 = reinterpret_cast<UTF8 *>(ਨਤੀਜਾ_Ptr);
  UTF8 *ਨਿਸ਼ਾਨਾ_ਸਿਰਾ_2 = ਨਿਸ਼ਾਨਾ_ਸ਼ੁਰੂ_2 + 4;
  Conversion_ਨਤੀਜਾ CR = ConvertUTF32toUTF8(
      &ਸਰੋਤ_ਸ਼ੁਰੂ_2, ਸਰੋਤ_ਸਿਰਾ_2, &ਨਿਸ਼ਾਨਾ_ਸ਼ੁਰੂ_2, ਨਿਸ਼ਾਨਾ_ਸਿਰਾ_2, strictConversion);
  ਜੇ (CR != conversionOK)
    ਵਾਪਸ ਝੂਠ;

  ਨਤੀਜਾ_Ptr = reinterpret_cast<ਅੱਖਰ *>(ਨਿਸ਼ਾਨਾ_ਸ਼ੁਰੂ_2);
  ਵਾਪਸ ਸੱਚ;
}

ਤਾਰਕਿਕ has_UTF16_Byte_ਕ੍ਰਮ_Mark(ArrayRef<ਅੱਖਰ> S) {
  ਵਾਪਸ (S.ਮਾਪ_2() >= 2 && ((S[0] == '\xff' && S[1] == '\xfe') ||
                            (S[0] == '\xfe' && S[1] == '\xff')));
}

ਤਾਰਕਿਕ convert_UTF16_ਮੰਜ਼ਿਲ_UTF8_String(ArrayRef<ਅੱਖਰ> Src_ਬਾਈਟਾਂ, std::string &ਬਾਹਰਲਾ) {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਬਾਹਰਲਾ.ਖਾਲੀ_ਥਾਵਾਂ());

  // Error out on an uneven byte count.
  ਜੇ (Src_ਬਾਈਟਾਂ.ਮਾਪ_2() % 2)
    ਵਾਪਸ ਝੂਠ;

  // Avoid OOB by returning early on empty input.
  ਜੇ (Src_ਬਾਈਟਾਂ.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ ਸੱਚ;

  ਅਚਲ UTF16 *Src = reinterpret_cast<ਅਚਲ UTF16 *>(Src_ਬਾਈਟਾਂ.begin());
  ਅਚਲ UTF16 *Src_ਸਿਰਾ = reinterpret_cast<ਅਚਲ UTF16 *>(Src_ਬਾਈਟਾਂ.ਸਿਰਾ());

  ਪ੍ਰਤਿਜ੍ਞਾ((uintptr_t)Src % ਆਕਾਰ(UTF16) == 0);

  // Byteswap if necessary.
  std::ਸਦਿਸ਼<UTF16> ByteSwapped;
  ਜੇ (Src[0] == UNI_UTF16_BYTE_ਕ੍ਰਮ_MARK_SWAPPED) {
    ByteSwapped.ਪਾਓ(ByteSwapped.ਸਿਰਾ(), Src, Src_ਸਿਰਾ);
    ਲਈ (UTF16 &I : ByteSwapped)
      I = llvm::byteswap<uint16_t>(I);
    Src = &ByteSwapped[0];
    Src_ਸਿਰਾ = &ByteSwapped[ByteSwapped.ਮਾਪ_2() - 1] + 1;
  }

  // Skip the BOM for conversion.
  ਜੇ (Src[0] == UNI_UTF16_BYTE_ਕ੍ਰਮ_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  ਬਾਹਰਲਾ.resize(Src_ਬਾਈਟਾਂ.ਮਾਪ_2() * UNI_MAX_UTF8_ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਕੋਡ_ਬਿੰਦੂ + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&ਬਾਹਰਲਾ[0]);
  UTF8 *Dst_ਸਿਰਾ = Dst + ਬਾਹਰਲਾ.ਮਾਪ_2();

  Conversion_ਨਤੀਜਾ CR =
      ConvertUTF16toUTF8(&Src, Src_ਸਿਰਾ, &Dst, Dst_ਸਿਰਾ, strictConversion);
  ਪ੍ਰਤਿਜ੍ਞਾ(CR != ਨਿਸ਼ਾਨਾ_Exhausted);

  ਜੇ (CR != conversionOK) {
    ਬਾਹਰਲਾ.clear();
    ਵਾਪਸ ਝੂਠ;
  }

  ਬਾਹਰਲਾ.resize(reinterpret_cast<ਅੱਖਰ *>(Dst) - &ਬਾਹਰਲਾ[0]);
  ਬਾਹਰਲਾ.ਧੱਕੋ_ਪਿੱਠ(0);
  ਬਾਹਰਲਾ.ਕੱਢੋ_ਪਿੱਠ();
  ਵਾਪਸ ਸੱਚ;
}

ਤਾਰਕਿਕ convert_UTF16_ਮੰਜ਼ਿਲ_UTF8_String(ArrayRef<UTF16> Src, std::string &ਬਾਹਰਲਾ) {
  ਵਾਪਸ convert_UTF16_ਮੰਜ਼ਿਲ_UTF8_String(
      llvm::ArrayRef<ਅੱਖਰ>(reinterpret_cast<ਅਚਲ ਅੱਖਰ *>(Src.ਅੰਕੜੇ()),
                           Src.ਮਾਪ_2() * ਆਕਾਰ(UTF16)),
      ਬਾਹਰਲਾ);
}

ਤਾਰਕਿਕ convert_UTF32_ਮੰਜ਼ਿਲ_UTF8_String(ArrayRef<ਅੱਖਰ> Src_ਬਾਈਟਾਂ, std::string &ਬਾਹਰਲਾ) {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਬਾਹਰਲਾ.ਖਾਲੀ_ਥਾਵਾਂ());

  // Error out on an uneven byte count.
  ਜੇ (Src_ਬਾਈਟਾਂ.ਮਾਪ_2() % 4)
    ਵਾਪਸ ਝੂਠ;

  // Avoid OOB by returning early on empty input.
  ਜੇ (Src_ਬਾਈਟਾਂ.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ ਸੱਚ;

  ਅਚਲ UTF32 *Src = reinterpret_cast<ਅਚਲ UTF32 *>(Src_ਬਾਈਟਾਂ.begin());
  ਅਚਲ UTF32 *Src_ਸਿਰਾ = reinterpret_cast<ਅਚਲ UTF32 *>(Src_ਬਾਈਟਾਂ.ਸਿਰਾ());

  ਪ੍ਰਤਿਜ੍ਞਾ((uintptr_t)Src % ਆਕਾਰ(UTF32) == 0);

  // Byteswap if necessary.
  std::ਸਦਿਸ਼<UTF32> ByteSwapped;
  ਜੇ (Src[0] == UNI_UTF32_BYTE_ਕ੍ਰਮ_MARK_SWAPPED) {
    ByteSwapped.ਪਾਓ(ByteSwapped.ਸਿਰਾ(), Src, Src_ਸਿਰਾ);
    ਲਈ (UTF32 &I : ByteSwapped)
      I = llvm::byteswap<uint32_t>(I);
    Src = &ByteSwapped[0];
    Src_ਸਿਰਾ = &ByteSwapped[ByteSwapped.ਮਾਪ_2() - 1] + 1;
  }

  // Skip the BOM for conversion.
  ਜੇ (Src[0] == UNI_UTF32_BYTE_ਕ੍ਰਮ_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  ਬਾਹਰਲਾ.resize(Src_ਬਾਈਟਾਂ.ਮਾਪ_2() * UNI_MAX_UTF8_ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਕੋਡ_ਬਿੰਦੂ + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&ਬਾਹਰਲਾ[0]);
  UTF8 *Dst_ਸਿਰਾ = Dst + ਬਾਹਰਲਾ.ਮਾਪ_2();

  Conversion_ਨਤੀਜਾ CR =
      ConvertUTF32toUTF8(&Src, Src_ਸਿਰਾ, &Dst, Dst_ਸਿਰਾ, strictConversion);
  ਪ੍ਰਤਿਜ੍ਞਾ(CR != ਨਿਸ਼ਾਨਾ_Exhausted);

  ਜੇ (CR != conversionOK) {
    ਬਾਹਰਲਾ.clear();
    ਵਾਪਸ ਝੂਠ;
  }

  ਬਾਹਰਲਾ.resize(reinterpret_cast<ਅੱਖਰ *>(Dst) - &ਬਾਹਰਲਾ[0]);
  ਬਾਹਰਲਾ.ਧੱਕੋ_ਪਿੱਠ(0);
  ਬਾਹਰਲਾ.ਕੱਢੋ_ਪਿੱਠ();
  ਵਾਪਸ ਸੱਚ;
}

ਤਾਰਕਿਕ convert_UTF32_ਮੰਜ਼ਿਲ_UTF8_String(ArrayRef<UTF32> Src, std::string &ਬਾਹਰਲਾ) {
  ਵਾਪਸ convert_UTF32_ਮੰਜ਼ਿਲ_UTF8_String(
      llvm::ArrayRef<ਅੱਖਰ>(reinterpret_cast<ਅਚਲ ਅੱਖਰ *>(Src.ਅੰਕੜੇ()),
                           Src.ਮਾਪ_2() * ਆਕਾਰ(UTF32)),
      ਬਾਹਰਲਾ);
}

ਤਾਰਕਿਕ convert_UTF8_ਮੰਜ਼ਿਲ_UTF16_String(StringRef SrcUTF8,
                              Small_ਸਦਿਸ਼_Impl<UTF16> &DstUTF16) {
  ਪ੍ਰਤਿਜ੍ਞਾ(DstUTF16.ਖਾਲੀ_ਥਾਵਾਂ());

  // Avoid OOB by returning early on empty input.
  ਜੇ (SrcUTF8.ਖਾਲੀ_ਥਾਵਾਂ()) {
    DstUTF16.ਧੱਕੋ_ਪਿੱਠ(0);
    DstUTF16.ਕੱਢੋ_ਪਿੱਠ();
    ਵਾਪਸ ਸੱਚ;
  }

  ਅਚਲ UTF8 *Src = reinterpret_cast<ਅਚਲ UTF8 *>(SrcUTF8.begin());
  ਅਚਲ UTF8 *Src_ਸਿਰਾ = reinterpret_cast<ਅਚਲ UTF8 *>(SrcUTF8.ਸਿਰਾ());

  // Allocate the same number of UTF-16 code units as UTF-8 code units. Encoding
  // as UTF-16 should always require the same amount or less code units than the
  // UTF-8 encoding.  Allocate one extra byte for the null terminator though,
  // so that someone calling DstUTF16.data() gets a null terminated string.
  // We resize down later so we don't have to worry that this over allocates.
  DstUTF16.resize(SrcUTF8.ਮਾਪ_2()+1);
  UTF16 *Dst = &DstUTF16[0];
  UTF16 *Dst_ਸਿਰਾ = Dst + DstUTF16.ਮਾਪ_2();

  Conversion_ਨਤੀਜਾ CR =
      ConvertUTF8toUTF16(&Src, Src_ਸਿਰਾ, &Dst, Dst_ਸਿਰਾ, strictConversion);
  ਪ੍ਰਤਿਜ੍ਞਾ(CR != ਨਿਸ਼ਾਨਾ_Exhausted);

  ਜੇ (CR != conversionOK) {
    DstUTF16.clear();
    ਵਾਪਸ ਝੂਠ;
  }

  DstUTF16.resize(Dst - &DstUTF16[0]);
  DstUTF16.ਧੱਕੋ_ਪਿੱਠ(0);
  DstUTF16.ਕੱਢੋ_ਪਿੱਠ();
  ਵਾਪਸ ਸੱਚ;
}

static_assert(ਆਕਾਰ(wchar_t) == 1 || ਆਕਾਰ(wchar_t) == 2 ||
                  ਆਕਾਰ(wchar_t) == 4,
              "Expected wchar_t to be 1, 2, or 4 bytes");

ਸਾਂਚਾ <ਕਿਸਮ_ਨਾਂ TResult>
ਸਥਿਰ ਸਤਰਅੰਦਰ ਤਾਰਕਿਕ Convert_UTF8to_Wide_ਅੰਦਰੂਨੀ(llvm::StringRef ਸਰੋਤ,
                                             TResult &ਨਤੀਜਾ) {
  // Even in the case of UTF-16, the number of bytes in a UTF-8 string is
  // at least as large as the number of elements in the resulting wide
  // string, because surrogate pairs take at least 4 bytes in UTF-8.
  ਨਤੀਜਾ.resize(ਸਰੋਤ.ਮਾਪ_2() + 1);
  ਅੱਖਰ *ਨਤੀਜਾ_Ptr = reinterpret_cast<ਅੱਖਰ *>(&ਨਤੀਜਾ[0]);
  ਅਚਲ UTF8 *ErrorPtr;
  ਜੇ (!ConvertUTF8toWide(ਆਕਾਰ(wchar_t), ਸਰੋਤ, ਨਤੀਜਾ_Ptr, ErrorPtr)) {
    ਨਤੀਜਾ.clear();
    ਵਾਪਸ ਝੂਠ;
  }
  ਨਤੀਜਾ.resize(reinterpret_cast<wchar_t *>(ਨਤੀਜਾ_Ptr) - &ਨਤੀਜਾ[0]);
  ਵਾਪਸ ਸੱਚ;
}

ਤਾਰਕਿਕ ConvertUTF8toWide(llvm::StringRef ਸਰੋਤ, std::wstring &ਨਤੀਜਾ) {
  ਵਾਪਸ Convert_UTF8to_Wide_ਅੰਦਰੂਨੀ(ਸਰੋਤ, ਨਤੀਜਾ);
}

ਤਾਰਕਿਕ ConvertUTF8toWide(ਅਚਲ ਅੱਖਰ *ਸਰੋਤ, std::wstring &ਨਤੀਜਾ) {
  ਜੇ (!ਸਰੋਤ) {
    ਨਤੀਜਾ.clear();
    ਵਾਪਸ ਸੱਚ;
  }
  ਵਾਪਸ ConvertUTF8toWide(llvm::StringRef(ਸਰੋਤ), ਨਤੀਜਾ);
}

ਤਾਰਕਿਕ convert_Wide_ਮੰਜ਼ਿਲ_UTF8(std::wstring_view ਸਰੋਤ, std::string &ਨਤੀਜਾ) {
  ਜੇ (ਆਕਾਰ(wchar_t) == 1) {
    ਅਚਲ UTF8 *ਸ਼ੁਰੂ = reinterpret_cast<ਅਚਲ UTF8 *>(ਸਰੋਤ.ਅੰਕੜੇ());
    ਅਚਲ UTF8 *ਸਿਰਾ_2 =
        reinterpret_cast<ਅਚਲ UTF8 *>(ਸਰੋਤ.ਅੰਕੜੇ() + ਸਰੋਤ.ਮਾਪ_2());
    ਜੇ (!isLegalUTF8String(&ਸ਼ੁਰੂ, ਸਿਰਾ_2))
      ਵਾਪਸ ਝੂਠ;
    ਨਤੀਜਾ.resize(ਸਰੋਤ.ਮਾਪ_2());
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(&ਨਤੀਜਾ[0], ਸਰੋਤ.ਅੰਕੜੇ(), ਸਰੋਤ.ਮਾਪ_2());
    ਵਾਪਸ ਸੱਚ;
  } ਨਹੀਂਤਾਂ ਜੇ (ਆਕਾਰ(wchar_t) == 2) {
    ਵਾਪਸ convert_UTF16_ਮੰਜ਼ਿਲ_UTF8_String(
        llvm::ArrayRef<UTF16>(reinterpret_cast<ਅਚਲ UTF16 *>(ਸਰੋਤ.ਅੰਕੜੇ()),
                              ਸਰੋਤ.ਮਾਪ_2()),
        ਨਤੀਜਾ);
  } ਨਹੀਂਤਾਂ ਜੇ (ਆਕਾਰ(wchar_t) == 4) {
    ਅਚਲ UTF32 *ਸ਼ੁਰੂ = reinterpret_cast<ਅਚਲ UTF32 *>(ਸਰੋਤ.ਅੰਕੜੇ());
    ਅਚਲ UTF32 *ਸਿਰਾ_2 =
        reinterpret_cast<ਅਚਲ UTF32 *>(ਸਰੋਤ.ਅੰਕੜੇ() + ਸਰੋਤ.ਮਾਪ_2());
    ਨਤੀਜਾ.resize(UNI_MAX_UTF8_ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਕੋਡ_ਬਿੰਦੂ * ਸਰੋਤ.ਮਾਪ_2());
    UTF8 *ਨਤੀਜਾ_Ptr = reinterpret_cast<UTF8 *>(&ਨਤੀਜਾ[0]);
    UTF8 *ਨਤੀਜਾ_ਸਿਰਾ = reinterpret_cast<UTF8 *>(&ਨਤੀਜਾ[0] + ਨਤੀਜਾ.ਮਾਪ_2());
    ਜੇ (ConvertUTF32toUTF8(&ਸ਼ੁਰੂ, ਸਿਰਾ_2, &ਨਤੀਜਾ_Ptr, ਨਤੀਜਾ_ਸਿਰਾ,
                           strictConversion) == conversionOK) {
      ਨਤੀਜਾ.resize(reinterpret_cast<ਅੱਖਰ *>(ਨਤੀਜਾ_Ptr) - &ਨਤੀਜਾ[0]);
      ਵਾਪਸ ਸੱਚ;
    } ਨਹੀਂਤਾਂ {
      ਨਤੀਜਾ.clear();
      ਵਾਪਸ ਝੂਠ;
    }
  } ਨਹੀਂਤਾਂ {
    llvm_unreachable(
        "Control should never reach this point; see static_assert further up");
  }
}

ਤਾਰਕਿਕ Is_Single_ਕੋਡ_Unit_UTF8_Codepoint(ਬਿਨਾਨਿਸ਼ਾਨੀ V) { ਵਾਪਸ V <= 0x7F; }

ਤਾਰਕਿਕ Is_Single_ਕੋਡ_Unit_UTF16_Codepoint(ਬਿਨਾਨਿਸ਼ਾਨੀ V) {
  ਵਾਪਸ V <= 0xD7FF || (V >= 0xE000 && V <= 0xFFFF);
}

ਤਾਰਕਿਕ Is_Single_ਕੋਡ_Unit_UTF32_Codepoint(ਬਿਨਾਨਿਸ਼ਾਨੀ V) {
  ਵਾਪਸ V <= 0xD7FF || (V >= 0xE000 && V <= 0x10FFFF);
}

} // end namespace llvm

