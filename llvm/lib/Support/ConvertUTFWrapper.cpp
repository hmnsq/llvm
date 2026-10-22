//===-- ConvertUTFWrapper.cpp - Wrap ConvertUTF.h with clang data types -----===
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/ADT/ArrayRef.h"
#అన్తర్భావ "llvm/ADT/StringRef.h"
#అన్తర్భావ "llvm/ADT/bit.h"
#అన్తర్భావ "llvm/Support/ConvertUTF.h"
#అన్తర్భావ "llvm/Support/ErrorHandling.h"
#అన్తర్భావ <string>
#అన్తర్భావ <vector>

నామాకాశ llvm {

తార్కిక ConvertUTF8toWide(గుర్తులేని Wide_Char_వెడల్పు, llvm::StringRef ఆధారం,
                       అక్షరం *&ఫలితం_Ptr, స్థిరరాశి UTF8 *&ErrorPtr) {
  ప్రతిజ్ఞా(Wide_Char_వెడల్పు == 1 || Wide_Char_వెడల్పు == 2 || Wide_Char_వెడల్పు == 4);
  Conversion_ఫలితం ఫలితం_2 = conversionOK;
  // Copy the character span over.
  ఒకవేళ (Wide_Char_వెడల్పు == 1) {
    స్థిరరాశి UTF8 *Pos = reinterpret_cast<స్థిరరాశి UTF8*>(ఆధారం.begin());
    ఒకవేళ (!isLegalUTF8String(&Pos, reinterpret_cast<స్థిరరాశి UTF8*>(ఆధారం.చివర()))) {
      ఫలితం_2 = ఆధారం_Illegal;
      ErrorPtr = Pos;
    } లేకపోతే {
      జ్ఞాపకం_నకలు(ఫలితం_Ptr, ఆధారం.దత్తాంశం(), ఆధారం.కొలత_2());
      ఫలితం_Ptr += ఆధారం.కొలత_2();
    }
  } లేకపోతే ఒకవేళ (Wide_Char_వెడల్పు == 2) {
    స్థిరరాశి UTF8 *ఆధారం_ప్రారంభం = (స్థిరరాశి UTF8*)ఆధారం.దత్తాంశం();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF16 *లక్ష్యం_ప్రారంభం = reinterpret_cast<UTF16 *>(ఫలితం_Ptr);
    ConversionFlags flags = strictConversion;
    ఫలితం_2 =
        ConvertUTF8toUTF16(&ఆధారం_ప్రారంభం, ఆధారం_ప్రారంభం + ఆధారం.కొలత_2(),
                           &లక్ష్యం_ప్రారంభం, లక్ష్యం_ప్రారంభం + ఆధారం.కొలత_2(), flags);
    ఒకవేళ (ఫలితం_2 == conversionOK)
      ఫలితం_Ptr = reinterpret_cast<అక్షరం *>(లక్ష్యం_ప్రారంభం);
    లేకపోతే
      ErrorPtr = ఆధారం_ప్రారంభం;
  } లేకపోతే ఒకవేళ (Wide_Char_వెడల్పు == 4) {
    స్థిరరాశి UTF8 *ఆధారం_ప్రారంభం = (స్థిరరాశి UTF8 *)ఆధారం.దత్తాంశం();
    // FIXME: Make the type of the result buffer correct instead of
    // using reinterpret_cast.
    UTF32 *లక్ష్యం_ప్రారంభం = reinterpret_cast<UTF32 *>(ఫలితం_Ptr);
    ConversionFlags flags = strictConversion;
    ఫలితం_2 =
        ConvertUTF8toUTF32(&ఆధారం_ప్రారంభం, ఆధారం_ప్రారంభం + ఆధారం.కొలత_2(),
                           &లక్ష్యం_ప్రారంభం, లక్ష్యం_ప్రారంభం + ఆధారం.కొలత_2(), flags);
    ఒకవేళ (ఫలితం_2 == conversionOK)
      ఫలితం_Ptr = reinterpret_cast<అక్షరం *>(లక్ష్యం_ప్రారంభం);
    లేకపోతే
      ErrorPtr = ఆధారం_ప్రారంభం;
  }
  ప్రతిజ్ఞా((ఫలితం_2 != లక్ష్యం_Exhausted) &&
         "ConvertUTF8toUTFXX exhausted target buffer");
  తిరిగి ఫలితం_2 == conversionOK;
}

తార్కిక Convert_సంకేతం_బిందువు_గమ్యం_UTF8(గుర్తులేని ఆధారం, అక్షరం *&ఫలితం_Ptr) {
  స్థిరరాశి UTF32 *ఆధారం_ప్రారంభం_2 = &ఆధారం;
  స్థిరరాశి UTF32 *ఆధారం_చివర_2 = ఆధారం_ప్రారంభం_2 + 1;
  UTF8 *లక్ష్యం_ప్రారంభం_2 = reinterpret_cast<UTF8 *>(ఫలితం_Ptr);
  UTF8 *లక్ష్యం_చివర_2 = లక్ష్యం_ప్రారంభం_2 + 4;
  Conversion_ఫలితం CR = ConvertUTF32toUTF8(
      &ఆధారం_ప్రారంభం_2, ఆధారం_చివర_2, &లక్ష్యం_ప్రారంభం_2, లక్ష్యం_చివర_2, strictConversion);
  ఒకవేళ (CR != conversionOK)
    తిరిగి అబద్ధం;

  ఫలితం_Ptr = reinterpret_cast<అక్షరం *>(లక్ష్యం_ప్రారంభం_2);
  తిరిగి నిజం;
}

తార్కిక has_UTF16_Byte_క్రమం_Mark(ArrayRef<అక్షరం> S) {
  తిరిగి (S.కొలత_2() >= 2 && ((S[0] == '\xff' && S[1] == '\xfe') ||
                            (S[0] == '\xfe' && S[1] == '\xff')));
}

తార్కిక convert_UTF16_గమ్యం_UTF8_String(ArrayRef<అక్షరం> Src_బైట్లు, std::string &బహిర్గతం) {
  ప్రతిజ్ఞా(బహిర్గతం.ఖాళీలు());

  // Error out on an uneven byte count.
  ఒకవేళ (Src_బైట్లు.కొలత_2() % 2)
    తిరిగి అబద్ధం;

  // Avoid OOB by returning early on empty input.
  ఒకవేళ (Src_బైట్లు.ఖాళీలు())
    తిరిగి నిజం;

  స్థిరరాశి UTF16 *Src = reinterpret_cast<స్థిరరాశి UTF16 *>(Src_బైట్లు.begin());
  స్థిరరాశి UTF16 *Src_చివర = reinterpret_cast<స్థిరరాశి UTF16 *>(Src_బైట్లు.చివర());

  ప్రతిజ్ఞా((uintptr_t)Src % పరిమాణం(UTF16) == 0);

  // Byteswap if necessary.
  std::సదిశ<UTF16> ByteSwapped;
  ఒకవేళ (Src[0] == UNI_UTF16_BYTE_క్రమం_MARK_SWAPPED) {
    ByteSwapped.చొప్పించు(ByteSwapped.చివర(), Src, Src_చివర);
    కొరకు (UTF16 &I : ByteSwapped)
      I = llvm::byteswap<uint16_t>(I);
    Src = &ByteSwapped[0];
    Src_చివర = &ByteSwapped[ByteSwapped.కొలత_2() - 1] + 1;
  }

  // Skip the BOM for conversion.
  ఒకవేళ (Src[0] == UNI_UTF16_BYTE_క్రమం_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  బహిర్గతం.resize(Src_బైట్లు.కొలత_2() * UNI_MAX_UTF8_బైట్లు_చొప్పున_సంకేతం_బిందువు + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&బహిర్గతం[0]);
  UTF8 *Dst_చివర = Dst + బహిర్గతం.కొలత_2();

  Conversion_ఫలితం CR =
      ConvertUTF16toUTF8(&Src, Src_చివర, &Dst, Dst_చివర, strictConversion);
  ప్రతిజ్ఞా(CR != లక్ష్యం_Exhausted);

  ఒకవేళ (CR != conversionOK) {
    బహిర్గతం.clear();
    తిరిగి అబద్ధం;
  }

  బహిర్గతం.resize(reinterpret_cast<అక్షరం *>(Dst) - &బహిర్గతం[0]);
  బహిర్గతం.నెట్టు_వెనుక(0);
  బహిర్గతం.తీయి_వెనుక();
  తిరిగి నిజం;
}

తార్కిక convert_UTF16_గమ్యం_UTF8_String(ArrayRef<UTF16> Src, std::string &బహిర్గతం) {
  తిరిగి convert_UTF16_గమ్యం_UTF8_String(
      llvm::ArrayRef<అక్షరం>(reinterpret_cast<స్థిరరాశి అక్షరం *>(Src.దత్తాంశం()),
                           Src.కొలత_2() * పరిమాణం(UTF16)),
      బహిర్గతం);
}

తార్కిక convert_UTF32_గమ్యం_UTF8_String(ArrayRef<అక్షరం> Src_బైట్లు, std::string &బహిర్గతం) {
  ప్రతిజ్ఞా(బహిర్గతం.ఖాళీలు());

  // Error out on an uneven byte count.
  ఒకవేళ (Src_బైట్లు.కొలత_2() % 4)
    తిరిగి అబద్ధం;

  // Avoid OOB by returning early on empty input.
  ఒకవేళ (Src_బైట్లు.ఖాళీలు())
    తిరిగి నిజం;

  స్థిరరాశి UTF32 *Src = reinterpret_cast<స్థిరరాశి UTF32 *>(Src_బైట్లు.begin());
  స్థిరరాశి UTF32 *Src_చివర = reinterpret_cast<స్థిరరాశి UTF32 *>(Src_బైట్లు.చివర());

  ప్రతిజ్ఞా((uintptr_t)Src % పరిమాణం(UTF32) == 0);

  // Byteswap if necessary.
  std::సదిశ<UTF32> ByteSwapped;
  ఒకవేళ (Src[0] == UNI_UTF32_BYTE_క్రమం_MARK_SWAPPED) {
    ByteSwapped.చొప్పించు(ByteSwapped.చివర(), Src, Src_చివర);
    కొరకు (UTF32 &I : ByteSwapped)
      I = llvm::byteswap<uint32_t>(I);
    Src = &ByteSwapped[0];
    Src_చివర = &ByteSwapped[ByteSwapped.కొలత_2() - 1] + 1;
  }

  // Skip the BOM for conversion.
  ఒకవేళ (Src[0] == UNI_UTF32_BYTE_క్రమం_MARK_NATIVE)
    Src++;

  // Just allocate enough space up front.  We'll shrink it later.  Allocate
  // enough that we can fit a null terminator without reallocating.
  బహిర్గతం.resize(Src_బైట్లు.కొలత_2() * UNI_MAX_UTF8_బైట్లు_చొప్పున_సంకేతం_బిందువు + 1);
  UTF8 *Dst = reinterpret_cast<UTF8 *>(&బహిర్గతం[0]);
  UTF8 *Dst_చివర = Dst + బహిర్గతం.కొలత_2();

  Conversion_ఫలితం CR =
      ConvertUTF32toUTF8(&Src, Src_చివర, &Dst, Dst_చివర, strictConversion);
  ప్రతిజ్ఞా(CR != లక్ష్యం_Exhausted);

  ఒకవేళ (CR != conversionOK) {
    బహిర్గతం.clear();
    తిరిగి అబద్ధం;
  }

  బహిర్గతం.resize(reinterpret_cast<అక్షరం *>(Dst) - &బహిర్గతం[0]);
  బహిర్గతం.నెట్టు_వెనుక(0);
  బహిర్గతం.తీయి_వెనుక();
  తిరిగి నిజం;
}

తార్కిక convert_UTF32_గమ్యం_UTF8_String(ArrayRef<UTF32> Src, std::string &బహిర్గతం) {
  తిరిగి convert_UTF32_గమ్యం_UTF8_String(
      llvm::ArrayRef<అక్షరం>(reinterpret_cast<స్థిరరాశి అక్షరం *>(Src.దత్తాంశం()),
                           Src.కొలత_2() * పరిమాణం(UTF32)),
      బహిర్గతం);
}

తార్కిక convert_UTF8_గమ్యం_UTF16_String(StringRef SrcUTF8,
                              Small_సదిశ_Impl<UTF16> &DstUTF16) {
  ప్రతిజ్ఞా(DstUTF16.ఖాళీలు());

  // Avoid OOB by returning early on empty input.
  ఒకవేళ (SrcUTF8.ఖాళీలు()) {
    DstUTF16.నెట్టు_వెనుక(0);
    DstUTF16.తీయి_వెనుక();
    తిరిగి నిజం;
  }

  స్థిరరాశి UTF8 *Src = reinterpret_cast<స్థిరరాశి UTF8 *>(SrcUTF8.begin());
  స్థిరరాశి UTF8 *Src_చివర = reinterpret_cast<స్థిరరాశి UTF8 *>(SrcUTF8.చివర());

  // Allocate the same number of UTF-16 code units as UTF-8 code units. Encoding
  // as UTF-16 should always require the same amount or less code units than the
  // UTF-8 encoding.  Allocate one extra byte for the null terminator though,
  // so that someone calling DstUTF16.data() gets a null terminated string.
  // We resize down later so we don't have to worry that this over allocates.
  DstUTF16.resize(SrcUTF8.కొలత_2()+1);
  UTF16 *Dst = &DstUTF16[0];
  UTF16 *Dst_చివర = Dst + DstUTF16.కొలత_2();

  Conversion_ఫలితం CR =
      ConvertUTF8toUTF16(&Src, Src_చివర, &Dst, Dst_చివర, strictConversion);
  ప్రతిజ్ఞా(CR != లక్ష్యం_Exhausted);

  ఒకవేళ (CR != conversionOK) {
    DstUTF16.clear();
    తిరిగి అబద్ధం;
  }

  DstUTF16.resize(Dst - &DstUTF16[0]);
  DstUTF16.నెట్టు_వెనుక(0);
  DstUTF16.తీయి_వెనుక();
  తిరిగి నిజం;
}

static_assert(పరిమాణం(wchar_t) == 1 || పరిమాణం(wchar_t) == 2 ||
                  పరిమాణం(wchar_t) == 4,
              "Expected wchar_t to be 1, 2, or 4 bytes");

మూస <రకం_పేరు TResult>
స్థిర అంతఃవరుస తార్కిక Convert_UTF8to_Wide_అంతర్గత(llvm::StringRef ఆధారం,
                                             TResult &ఫలితం) {
  // Even in the case of UTF-16, the number of bytes in a UTF-8 string is
  // at least as large as the number of elements in the resulting wide
  // string, because surrogate pairs take at least 4 bytes in UTF-8.
  ఫలితం.resize(ఆధారం.కొలత_2() + 1);
  అక్షరం *ఫలితం_Ptr = reinterpret_cast<అక్షరం *>(&ఫలితం[0]);
  స్థిరరాశి UTF8 *ErrorPtr;
  ఒకవేళ (!ConvertUTF8toWide(పరిమాణం(wchar_t), ఆధారం, ఫలితం_Ptr, ErrorPtr)) {
    ఫలితం.clear();
    తిరిగి అబద్ధం;
  }
  ఫలితం.resize(reinterpret_cast<wchar_t *>(ఫలితం_Ptr) - &ఫలితం[0]);
  తిరిగి నిజం;
}

తార్కిక ConvertUTF8toWide(llvm::StringRef ఆధారం, std::wstring &ఫలితం) {
  తిరిగి Convert_UTF8to_Wide_అంతర్గత(ఆధారం, ఫలితం);
}

తార్కిక ConvertUTF8toWide(స్థిరరాశి అక్షరం *ఆధారం, std::wstring &ఫలితం) {
  ఒకవేళ (!ఆధారం) {
    ఫలితం.clear();
    తిరిగి నిజం;
  }
  తిరిగి ConvertUTF8toWide(llvm::StringRef(ఆధారం), ఫలితం);
}

తార్కిక convert_Wide_గమ్యం_UTF8(std::wstring_view ఆధారం, std::string &ఫలితం) {
  ఒకవేళ (పరిమాణం(wchar_t) == 1) {
    స్థిరరాశి UTF8 *ప్రారంభం = reinterpret_cast<స్థిరరాశి UTF8 *>(ఆధారం.దత్తాంశం());
    స్థిరరాశి UTF8 *చివర_2 =
        reinterpret_cast<స్థిరరాశి UTF8 *>(ఆధారం.దత్తాంశం() + ఆధారం.కొలత_2());
    ఒకవేళ (!isLegalUTF8String(&ప్రారంభం, చివర_2))
      తిరిగి అబద్ధం;
    ఫలితం.resize(ఆధారం.కొలత_2());
    జ్ఞాపకం_నకలు(&ఫలితం[0], ఆధారం.దత్తాంశం(), ఆధారం.కొలత_2());
    తిరిగి నిజం;
  } లేకపోతే ఒకవేళ (పరిమాణం(wchar_t) == 2) {
    తిరిగి convert_UTF16_గమ్యం_UTF8_String(
        llvm::ArrayRef<UTF16>(reinterpret_cast<స్థిరరాశి UTF16 *>(ఆధారం.దత్తాంశం()),
                              ఆధారం.కొలత_2()),
        ఫలితం);
  } లేకపోతే ఒకవేళ (పరిమాణం(wchar_t) == 4) {
    స్థిరరాశి UTF32 *ప్రారంభం = reinterpret_cast<స్థిరరాశి UTF32 *>(ఆధారం.దత్తాంశం());
    స్థిరరాశి UTF32 *చివర_2 =
        reinterpret_cast<స్థిరరాశి UTF32 *>(ఆధారం.దత్తాంశం() + ఆధారం.కొలత_2());
    ఫలితం.resize(UNI_MAX_UTF8_బైట్లు_చొప్పున_సంకేతం_బిందువు * ఆధారం.కొలత_2());
    UTF8 *ఫలితం_Ptr = reinterpret_cast<UTF8 *>(&ఫలితం[0]);
    UTF8 *ఫలితం_చివర = reinterpret_cast<UTF8 *>(&ఫలితం[0] + ఫలితం.కొలత_2());
    ఒకవేళ (ConvertUTF32toUTF8(&ప్రారంభం, చివర_2, &ఫలితం_Ptr, ఫలితం_చివర,
                           strictConversion) == conversionOK) {
      ఫలితం.resize(reinterpret_cast<అక్షరం *>(ఫలితం_Ptr) - &ఫలితం[0]);
      తిరిగి నిజం;
    } లేకపోతే {
      ఫలితం.clear();
      తిరిగి అబద్ధం;
    }
  } లేకపోతే {
    llvm_unreachable(
        "Control should never reach this point; see static_assert further up");
  }
}

తార్కిక Is_Single_సంకేతం_Unit_UTF8_Codepoint(గుర్తులేని V) { తిరిగి V <= 0x7F; }

తార్కిక Is_Single_సంకేతం_Unit_UTF16_Codepoint(గుర్తులేని V) {
  తిరిగి V <= 0xD7FF || (V >= 0xE000 && V <= 0xFFFF);
}

తార్కిక Is_Single_సంకేతం_Unit_UTF32_Codepoint(గుర్తులేని V) {
  తిరిగి V <= 0xD7FF || (V >= 0xE000 && V <= 0x10FFFF);
}

} // end namespace llvm

