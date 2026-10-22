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

तार्किक ConvertUTF8toWide(अचिह्नित Wide_Char_विस्तारः, llvm::StringRef Source,
                       वर्ण *&परिणाम_Ptr, ध्रुव UTF8 *&ErrorPtr) {
  प्रतिज्ञा(Wide_Char_विस्तारः == 1 || Wide_Char_विस्तारः == 2 || Wide_Char_विस्तारः == 4);
  Conversion_परिणाम परिणाम_2 = conversionOK;
  // Copy the character span over.
  यदि (Wide_Char_विस्तारः == 1) {
    ध्रुव UTF8 *Pos = reinterpret_cast<ध्रुव UTF8*>(Source.begin());
    यदि (!isLegalUTF8String(&Pos, reinterpret_cast<ध्रुव UTF8*>(Source.अन्तः()))) {
      परिणाम_2 = sourceIllegal;
      ErrorPtr = Pos;
    } अन्यथा {
      स्मृतिं_प्रतिलिख(परिणाम_Ptr, Source.दत्त(), Source.आकार_2());
      परिणाम_Ptr += Source.आकार_2();
    }
  } अन्यथा यदि (Wide_Char_विस्तारः == 2) {
    ध्रुव UTF8 *sourceStart = (ध्रुव UTF8*)Source.दत्त();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF16 *targetStart = reinterpret_cast<UTF16 *>(परिणाम_Ptr);
    ConversionFlags flags = strictConversion;
    परिणाम_2 =
        ConvertUTF8toUTF16(&sourceStart, sourceStart + Source.आकार_2(),
                           &targetStart, targetStart + Source.आकार_2(), flags);
    यदि (परिणाम_2 == conversionOK)
      परिणाम_Ptr = reinterpret_cast<वर्ण *>(targetStart);
    अन्यथा
      ErrorPtr = sourceStart;
  } अन्यथा यदि (Wide_Char_विस्तारः == 4) {
    ध्रुव UTF8 *sourceStart = (ध्रुव UTF8 *)Source.दत्त();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF32 *targetStart = reinterpret_cast<UTF32 *>(परिणाम_Ptr);
    ConversionFlags flags = strictConversion;
    परिणाम_2 =
        ConvertUTF8toUTF32(&sourceStart, sourceStart + Source.आकार_2(),
                           &targetStart, targetStart + Source.आकार_2(), flags);
    यदि (परिणाम_2 == conversionOK)
      परिणाम_Ptr = reinterpret_cast<वर्ण *>(targetStart);
    अन्यथा
      ErrorPtr = sourceStart;
  }
  प्रतिज्ञा((परिणाम_2 != targetExhausted) &&
         "ConvertUTF8toUTFXX exhausted target buffer");
  प्रत्यागम परिणाम_2 == conversionOK;
}

तार्किक Convert_संहिता_Point_गन्तव्यम्_UTF8(अचिह्नित Source, वर्ण *&परिणाम_Ptr) {
  ध्रुव UTF32 *SourceStart = &Source;
  ध्रुव UTF32 *Source_अन्तः = SourceStart + 1;
  UTF8 *TargetStart = reinterpret_cast<UTF8 *>(परिणाम_Ptr);
  UTF8 *Target_अन्तः = TargetStart + 4;
  Conversion_परिणाम CR = ConvertUTF32toUTF8(
      &SourceStart, Source_अन्तः, &TargetStart, Target_अन्तः, strictConversion);
  यदि (CR != conversionOK)
    प्रत्यागम असत्य;

  परिणाम_Ptr = reinterpret_cast<वर्ण *>(TargetStart);
  प्रत्यागम सत्य;
}

तार्किक has_UTF16_Byte_क्रमः_Mark(ArrayRef<वर्ण> S) {
  प्रत्यागम (S.आकार_2() >= 2 && ((S[0] == '\xff' && S[1] == '\xfe') ||
                            (S[0] == '\xfe' && S[1] == '\xff')));
}

तार्किक convert_UTF16_गन्तव्यम्_UTF8_String(ArrayRef<वर्ण> Src_अष्टकानि, std::string &बहिर्गमनम्) {
  प्रतिज्ञा(बहिर्गमनम्.रिक्तानि());

  // Error out on an uneven byte count.
  यदि (Src_अष्टकानि.आकार_2() % 2)
    प्रत्यागम असत्य;

  // Avoid OOB by returning early on empty input.
  यदि (Src_अष्टकानि.रिक्तानि())
    प्रत्यागम सत्य;

  ध्रुव UTF16 *Src = reinterpret_cast<ध्रुव UTF16 *>(Src_अष्टकानि.begin());
  ध्रुव UTF16 *Src_अन्तः = reinterpret_cast<ध्रुव UTF16 *>(Src_अष्टकानि.अन्तः());

  प्रतिज्ञा((uintptr_t)Src % परिमाण(UTF16) == 0);

  // Byteswap if necessary.
  std::सदिशम्<UTF16> ByteSwapped;
  यदि (Src[0] == UNI_UTF16_BYTE_क्रमः_MARK_SWAPPED) {
    ByteSwapped.निवेशय(ByteSwapped.अन्तः(), Src, Src_अन्तः);
    प्रति (UTF16 &I : ByteSwapped)
      I = llvm::byteswap<uint16_t>(I);
    Src = &ByteSwapped[0];
    Src_अन्तः = &ByteSwapped[ByteSwapped.आकार_2() - 1] + 1;
  }

  // Skip the BOM for conversion.
  यदि (Src[0] == UNI_UTF16_BYTE_क्रमः_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  बहिर्गमनम्.resize(Src_अष्टकानि.आकार_2() * UNI_MAX_UTF8_अष्टकानि_प्रत्येकम्_संहिता_POINT + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&बहिर्गमनम्[0]);
  UTF8 *Dst_अन्तः = Dst + बहिर्गमनम्.आकार_2();

  Conversion_परिणाम CR =
      ConvertUTF16toUTF8(&Src, Src_अन्तः, &Dst, Dst_अन्तः, strictConversion);
  प्रतिज्ञा(CR != targetExhausted);

  यदि (CR != conversionOK) {
    बहिर्गमनम्.clear();
    प्रत्यागम असत्य;
  }

  बहिर्गमनम्.resize(reinterpret_cast<वर्ण *>(Dst) - &बहिर्गमनम्[0]);
  बहिर्गमनम्.प्रक्षिप_पृष्ठभागः(0);
  बहिर्गमनम्.निष्कासय_पृष्ठभागः();
  प्रत्यागम सत्य;
}

तार्किक convert_UTF16_गन्तव्यम्_UTF8_String(ArrayRef<UTF16> Src, std::string &बहिर्गमनम्) {
  प्रत्यागम convert_UTF16_गन्तव्यम्_UTF8_String(
      llvm::ArrayRef<वर्ण>(reinterpret_cast<ध्रुव वर्ण *>(Src.दत्त()),
                           Src.आकार_2() * परिमाण(UTF16)),
      बहिर्गमनम्);
}

तार्किक convert_UTF32_गन्तव्यम्_UTF8_String(ArrayRef<वर्ण> Src_अष्टकानि, std::string &बहिर्गमनम्) {
  प्रतिज्ञा(बहिर्गमनम्.रिक्तानि());

  // Error out on an uneven byte count.
  यदि (Src_अष्टकानि.आकार_2() % 4)
    प्रत्यागम असत्य;

  // Avoid OOB by returning early on empty input.
  यदि (Src_अष्टकानि.रिक्तानि())
    प्रत्यागम सत्य;

  ध्रुव UTF32 *Src = reinterpret_cast<ध्रुव UTF32 *>(Src_अष्टकानि.begin());
  ध्रुव UTF32 *Src_अन्तः = reinterpret_cast<ध्रुव UTF32 *>(Src_अष्टकानि.अन्तः());

  प्रतिज्ञा((uintptr_t)Src % परिमाण(UTF32) == 0);

  // Byteswap if necessary.
  std::सदिशम्<UTF32> ByteSwapped;
  यदि (Src[0] == UNI_UTF32_BYTE_क्रमः_MARK_SWAPPED) {
    ByteSwapped.निवेशय(ByteSwapped.अन्तः(), Src, Src_अन्तः);
    प्रति (UTF32 &I : ByteSwapped)
      I = llvm::byteswap<uint32_t>(I);
    Src = &ByteSwapped[0];
    Src_अन्तः = &ByteSwapped[ByteSwapped.आकार_2() - 1] + 1;
  }

  // Skip the BOM for conversion.
  यदि (Src[0] == UNI_UTF32_BYTE_क्रमः_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  बहिर्गमनम्.resize(Src_अष्टकानि.आकार_2() * UNI_MAX_UTF8_अष्टकानि_प्रत्येकम्_संहिता_POINT + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&बहिर्गमनम्[0]);
  UTF8 *Dst_अन्तः = Dst + बहिर्गमनम्.आकार_2();

  Conversion_परिणाम CR =
      ConvertUTF32toUTF8(&Src, Src_अन्तः, &Dst, Dst_अन्तः, strictConversion);
  प्रतिज्ञा(CR != targetExhausted);

  यदि (CR != conversionOK) {
    बहिर्गमनम्.clear();
    प्रत्यागम असत्य;
  }

  बहिर्गमनम्.resize(reinterpret_cast<वर्ण *>(Dst) - &बहिर्गमनम्[0]);
  बहिर्गमनम्.प्रक्षिप_पृष्ठभागः(0);
  बहिर्गमनम्.निष्कासय_पृष्ठभागः();
  प्रत्यागम सत्य;
}

तार्किक convert_UTF32_गन्तव्यम्_UTF8_String(ArrayRef<UTF32> Src, std::string &बहिर्गमनम्) {
  प्रत्यागम convert_UTF32_गन्तव्यम्_UTF8_String(
      llvm::ArrayRef<वर्ण>(reinterpret_cast<ध्रुव वर्ण *>(Src.दत्त()),
                           Src.आकार_2() * परिमाण(UTF32)),
      बहिर्गमनम्);
}

तार्किक convert_UTF8_गन्तव्यम्_UTF16_String(StringRef SrcUTF8,
                              Small_सदिशम्_Impl<UTF16> &DstUTF16) {
  प्रतिज्ञा(DstUTF16.रिक्तानि());

  // Avoid OOB by returning early on empty input.
  यदि (SrcUTF8.रिक्तानि()) {
    DstUTF16.प्रक्षिप_पृष्ठभागः(0);
    DstUTF16.निष्कासय_पृष्ठभागः();
    प्रत्यागम सत्य;
  }

  ध्रुव UTF8 *Src = reinterpret_cast<ध्रुव UTF8 *>(SrcUTF8.begin());
  ध्रुव UTF8 *Src_अन्तः = reinterpret_cast<ध्रुव UTF8 *>(SrcUTF8.अन्तः());

  // Allocate the same number of UTF-16 code units as UTF-8 code units. Encoding
  // as UTF-16 should always require the same amount or less code units than the
  // UTF-8 encoding.  Allocate one extra byte for the null terminator though,
  // so that someone calling DstUTF16.data() gets a null terminated string.
  // We resize down later so we don't have to worry that this over allocates.
  DstUTF16.resize(SrcUTF8.आकार_2()+1);
  UTF16 *Dst = &DstUTF16[0];
  UTF16 *Dst_अन्तः = Dst + DstUTF16.आकार_2();

  Conversion_परिणाम CR =
      ConvertUTF8toUTF16(&Src, Src_अन्तः, &Dst, Dst_अन्तः, strictConversion);
  प्रतिज्ञा(CR != targetExhausted);

  यदि (CR != conversionOK) {
    DstUTF16.clear();
    प्रत्यागम असत्य;
  }

  DstUTF16.resize(Dst - &DstUTF16[0]);
  DstUTF16.प्रक्षिप_पृष्ठभागः(0);
  DstUTF16.निष्कासय_पृष्ठभागः();
  प्रत्यागम सत्य;
}

static_assert(परिमाण(wchar_t) == 1 || परिमाण(wchar_t) == 2 ||
                  परिमाण(wchar_t) == 4,
              "Expected wchar_t to be 1, 2, or 4 bytes");

प्रतिरूपम् <प्रकारनाम TResult>
स्थिर अन्तःस्थ तार्किक Convert_UTF8to_Wide_आन्तरिकम्(llvm::StringRef Source,
                                             TResult &परिणाम) {
  // Even in the case of UTF-16, the number of bytes in a UTF-8 string is
  // at least as large as the number of elements in the resulting wide
  // string, because surrogate pairs take at least 4 bytes in UTF-8.
  परिणाम.resize(Source.आकार_2() + 1);
  वर्ण *परिणाम_Ptr = reinterpret_cast<वर्ण *>(&परिणाम[0]);
  ध्रुव UTF8 *ErrorPtr;
  यदि (!ConvertUTF8toWide(परिमाण(wchar_t), Source, परिणाम_Ptr, ErrorPtr)) {
    परिणाम.clear();
    प्रत्यागम असत्य;
  }
  परिणाम.resize(reinterpret_cast<wchar_t *>(परिणाम_Ptr) - &परिणाम[0]);
  प्रत्यागम सत्य;
}

तार्किक ConvertUTF8toWide(llvm::StringRef Source, std::wstring &परिणाम) {
  प्रत्यागम Convert_UTF8to_Wide_आन्तरिकम्(Source, परिणाम);
}

तार्किक ConvertUTF8toWide(ध्रुव वर्ण *Source, std::wstring &परिणाम) {
  यदि (!Source) {
    परिणाम.clear();
    प्रत्यागम सत्य;
  }
  प्रत्यागम ConvertUTF8toWide(llvm::StringRef(Source), परिणाम);
}

तार्किक convert_Wide_गन्तव्यम्_UTF8(std::wstring_view Source, std::string &परिणाम) {
  यदि (परिमाण(wchar_t) == 1) {
    ध्रुव UTF8 *Start = reinterpret_cast<ध्रुव UTF8 *>(Source.दत्त());
    ध्रुव UTF8 *अन्तः_2 =
        reinterpret_cast<ध्रुव UTF8 *>(Source.दत्त() + Source.आकार_2());
    यदि (!isLegalUTF8String(&Start, अन्तः_2))
      प्रत्यागम असत्य;
    परिणाम.resize(Source.आकार_2());
    स्मृतिं_प्रतिलिख(&परिणाम[0], Source.दत्त(), Source.आकार_2());
    प्रत्यागम सत्य;
  } अन्यथा यदि (परिमाण(wchar_t) == 2) {
    प्रत्यागम convert_UTF16_गन्तव्यम्_UTF8_String(
        llvm::ArrayRef<UTF16>(reinterpret_cast<ध्रुव UTF16 *>(Source.दत्त()),
                              Source.आकार_2()),
        परिणाम);
  } अन्यथा यदि (परिमाण(wchar_t) == 4) {
    ध्रुव UTF32 *Start = reinterpret_cast<ध्रुव UTF32 *>(Source.दत्त());
    ध्रुव UTF32 *अन्तः_2 =
        reinterpret_cast<ध्रुव UTF32 *>(Source.दत्त() + Source.आकार_2());
    परिणाम.resize(UNI_MAX_UTF8_अष्टकानि_प्रत्येकम्_संहिता_POINT * Source.आकार_2());
    UTF8 *परिणाम_Ptr = reinterpret_cast<UTF8 *>(&परिणाम[0]);
    UTF8 *परिणाम_अन्तः = reinterpret_cast<UTF8 *>(&परिणाम[0] + परिणाम.आकार_2());
    यदि (ConvertUTF32toUTF8(&Start, अन्तः_2, &परिणाम_Ptr, परिणाम_अन्तः,
                           strictConversion) == conversionOK) {
      परिणाम.resize(reinterpret_cast<वर्ण *>(परिणाम_Ptr) - &परिणाम[0]);
      प्रत्यागम सत्य;
    } अन्यथा {
      परिणाम.clear();
      प्रत्यागम असत्य;
    }
  } अन्यथा {
    llvm_unreachable(
        "Control should never reach this point; see static_assert further up");
  }
}

तार्किक Is_Single_संहिता_Unit_UTF8_Codepoint(अचिह्नित V) { प्रत्यागम V <= 0x7F; }

तार्किक Is_Single_संहिता_Unit_UTF16_Codepoint(अचिह्नित V) {
  प्रत्यागम V <= 0xD7FF || (V >= 0xE000 && V <= 0xFFFF);
}

तार्किक Is_Single_संहिता_Unit_UTF32_Codepoint(अचिह्नित V) {
  प्रत्यागम V <= 0xD7FF || (V >= 0xE000 && V <= 0x10FFFF);
}

} // end namespace llvm

