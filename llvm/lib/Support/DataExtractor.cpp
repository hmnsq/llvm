//===-- DataExtractor.cpp -------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/DataExtractor.h"
#అన్తర్భావ "llvm/ADT/StringExtras.h"
#అన్తర్భావ "llvm/Support/Errc.h"
#అన్తర్భావ "llvm/Support/ErrorHandling.h"
#అన్తర్భావ "llvm/Support/LEB128.h"
#అన్తర్భావ "llvm/Support/SwapByteOrder.h"

ఉపయోగించి నామాకాశ llvm;

తార్కిక దత్తాంశం_Extractor::prepareRead(uint64_t స్థానభ్రంశం, uint64_t కొలత,
                                Error *E) స్థిరరాశి {
  ఒకవేళ (is_Valid_స్థానభ్రంశం_For_దత్తాంశం_యొక్క_కొలత(స్థానభ్రంశం, కొలత))
    తిరిగి నిజం;
  ఒకవేళ (E) {
    ఒకవేళ (స్థానభ్రంశం <= దత్తాంశం_2.కొలత_2())
      *E = createStringError(
          errc::illegal_byte_sequence,
          "unexpected end of data at offset 0x%zx while reading [0x%" PRIx64
          ", 0x%" PRIx64 ")",
          దత్తాంశం_2.కొలత_2(), స్థానభ్రంశం, స్థానభ్రంశం + కొలత);
    లేకపోతే
      *E = createStringError(errc::invalid_వాదం,
                             "offset 0x%" PRIx64
                             " is beyond the end of data at 0x%zx",
                             స్థానభ్రంశం, దత్తాంశం_2.కొలత_2());
  }
  తిరిగి అబద్ధం;
}

స్థిర తార్కిక isError(Error *E) { తిరిగి E && *E; }

మూస <రకం_పేరు T>
T దత్తాంశం_Extractor::getU(uint64_t *స్థానభ్రంశం_ptr, Error *Err) స్థిరరాశి {
  Error_As_బహిర్గతం_Parameter Err_As_బహిర్గతం(Err);
  T val = 0;
  ఒకవేళ (isError(Err))
    తిరిగి val;

  uint64_t స్థానభ్రంశం_2 = *స్థానభ్రంశం_ptr;
  ఒకవేళ (!prepareRead(స్థానభ్రంశం_2, పరిమాణం(T), Err))
    తిరిగి val;
  std::జ్ఞాపకం_నకలు(&val, &దత్తాంశం_2.దత్తాంశం()[స్థానభ్రంశం_2], పరిమాణం(val));
  ఒకవేళ (sys::IsLittleEndianHost != IsLittleEndian)
    sys::మార్పిడి_Byte_క్రమం(val);

  // Advance the offset
  *స్థానభ్రంశం_ptr += పరిమాణం(val);
  తిరిగి val;
}

మూస <రకం_పేరు T>
T *దత్తాంశం_Extractor::getUs(uint64_t *స్థానభ్రంశం_ptr, T *dst, uint32_t లెక్క,
                        Error *Err) స్థిరరాశి {
  Error_As_బహిర్గతం_Parameter Err_As_బహిర్గతం(Err);
  ఒకవేళ (isError(Err))
    తిరిగి శూన్యసూచిక;

  uint64_t స్థానభ్రంశం_2 = *స్థానభ్రంశం_ptr;

  ఒకవేళ (!prepareRead(స్థానభ్రంశం_2, పరిమాణం(*dst) * లెక్క, Err))
    తిరిగి శూన్యసూచిక;
  కొరకు (T *విలువ_ptr = dst, *చివర = dst + లెక్క; విలువ_ptr != చివర;
       ++విలువ_ptr, స్థానభ్రంశం_2 += పరిమాణం(*dst))
    *విలువ_ptr = getU<T>(స్థానభ్రంశం_ptr, Err);
  // Advance the offset
  *స్థానభ్రంశం_ptr = స్థానభ్రంశం_2;
  // Return a non-NULL pointer to the converted data as an indicator of
  // success
  తిరిగి dst;
}

uint8_t దత్తాంశం_Extractor::getU8(uint64_t *స్థానభ్రంశం_ptr, llvm::Error *Err) స్థిరరాశి {
  తిరిగి getU<uint8_t>(స్థానభ్రంశం_ptr, Err);
}

uint8_t *దత్తాంశం_Extractor::getU8(uint64_t *స్థానభ్రంశం_ptr, uint8_t *dst,
                              uint32_t లెక్క) స్థిరరాశి {
  తిరిగి getUs<uint8_t>(స్థానభ్రంశం_ptr, dst, లెక్క, శూన్యసూచిక);
}

uint8_t *దత్తాంశం_Extractor::getU8(Cursor &C, uint8_t *Dst, uint32_t లెక్క_2) స్థిరరాశి {
  తిరిగి getUs<uint8_t>(&C.స్థానభ్రంశం, Dst, లెక్క_2, &C.Err);
}

uint16_t దత్తాంశం_Extractor::getU16(uint64_t *స్థానభ్రంశం_ptr, llvm::Error *Err) స్థిరరాశి {
  తిరిగి getU<uint16_t>(స్థానభ్రంశం_ptr, Err);
}

uint16_t *దత్తాంశం_Extractor::getU16(uint64_t *స్థానభ్రంశం_ptr, uint16_t *dst,
                                uint32_t లెక్క) స్థిరరాశి {
  తిరిగి getUs<uint16_t>(స్థానభ్రంశం_ptr, dst, లెక్క, శూన్యసూచిక);
}

uint32_t దత్తాంశం_Extractor::getU24(uint64_t *స్థానభ్రంశం_Ptr, Error *Err) స్థిరరాశి {
  uint24_t ExtractedVal = getU<uint24_t>(స్థానభ్రంశం_Ptr, Err);
  // The 3 bytes are in the correct byte order for the host.
  తిరిగి ExtractedVal.getAsUint32(sys::IsLittleEndianHost);
}

uint32_t దత్తాంశం_Extractor::getU32(uint64_t *స్థానభ్రంశం_ptr, llvm::Error *Err) స్థిరరాశి {
  తిరిగి getU<uint32_t>(స్థానభ్రంశం_ptr, Err);
}

uint32_t *దత్తాంశం_Extractor::getU32(uint64_t *స్థానభ్రంశం_ptr, uint32_t *dst,
                                uint32_t లెక్క) స్థిరరాశి {
  తిరిగి getUs<uint32_t>(స్థానభ్రంశం_ptr, dst, లెక్క, శూన్యసూచిక);
}

uint64_t దత్తాంశం_Extractor::getU64(uint64_t *స్థానభ్రంశం_ptr, llvm::Error *Err) స్థిరరాశి {
  తిరిగి getU<uint64_t>(స్థానభ్రంశం_ptr, Err);
}

uint64_t *దత్తాంశం_Extractor::getU64(uint64_t *స్థానభ్రంశం_ptr, uint64_t *dst,
                                uint32_t లెక్క) స్థిరరాశి {
  తిరిగి getUs<uint64_t>(స్థానభ్రంశం_ptr, dst, లెక్క, శూన్యసూచిక);
}

uint64_t దత్తాంశం_Extractor::getUnsigned(uint64_t *స్థానభ్రంశం_ptr, uint32_t byte_కొలత,
                                    llvm::Error *Err) స్థిరరాశి {
  ఎంపిక (byte_కొలత) {
  స్థితి 1:
    తిరిగి getU8(స్థానభ్రంశం_ptr, Err);
  స్థితి 2:
    తిరిగి getU16(స్థానభ్రంశం_ptr, Err);
  స్థితి 4:
    తిరిగి getU32(స్థానభ్రంశం_ptr, Err);
  స్థితి 8:
    తిరిగి getU64(స్థానభ్రంశం_ptr, Err);
  }

  // For any other byte size, read the bytes and swap/shift if necessary.
  Error_As_బహిర్గతం_Parameter Err_As_బహిర్గతం(Err);
  uint64_t val = 0;
  ఒకవేళ (isError(Err))
    తిరిగి val;
  uint64_t స్థానభ్రంశం_2 = *స్థానభ్రంశం_ptr;
  ఒకవేళ (!prepareRead(స్థానభ్రంశం_2, byte_కొలత, Err))
    తిరిగి val;
  // Copy into the least significant bytes of val regardless of host
  // endianness.
  std::జ్ఞాపకం_నకలు(reinterpret_cast<అక్షరం *>(&val) +
                  (sys::IsLittleEndianHost ? 0 : 8 - byte_కొలత),
              &దత్తాంశం_2.దత్తాంశం()[స్థానభ్రంశం_2], byte_కొలత);
  // Swap the least significant bytes of val if endianness doesn't match.
  ఒకవేళ (sys::IsLittleEndianHost != IsLittleEndian)
    val = sys::get_Swapped_బైట్లు(val) >> (8 * (8 - byte_కొలత));

  *స్థానభ్రంశం_ptr += byte_కొలత;
  తిరిగి val;
}

int64_t
దత్తాంశం_Extractor::getSigned(uint64_t *స్థానభ్రంశం_ptr, uint32_t byte_కొలత) స్థిరరాశి {
  ఎంపిక (byte_కొలత) {
  స్థితి 1:
    తిరిగి getS8(స్థానభ్రంశం_ptr);
  స్థితి 2:
    తిరిగి getS16(స్థానభ్రంశం_ptr);
  స్థితి 4:
    తిరిగి getS32(స్థానభ్రంశం_ptr);
  స్థితి 8:
    తిరిగి getS64(స్థానభ్రంశం_ptr);
  }
  llvm_unreachable("getSigned unhandled case!");
}

StringRef దత్తాంశం_Extractor::getCStrRef(uint64_t *స్థానభ్రంశం_Ptr, Error *Err) స్థిరరాశి {
  Error_As_బహిర్గతం_Parameter Err_As_బహిర్గతం(Err);
  ఒకవేళ (isError(Err))
    తిరిగి StringRef();

  uint64_t ప్రారంభం = *స్థానభ్రంశం_Ptr;
  StringRef::కొలత_type Pos = దత్తాంశం_2.కనుగొను('\0', ప్రారంభం);
  ఒకవేళ (Pos != StringRef::npos) {
    *స్థానభ్రంశం_Ptr = Pos + 1;
    తిరిగి StringRef(దత్తాంశం_2.దత్తాంశం() + ప్రారంభం, Pos - ప్రారంభం);
  }
  ఒకవేళ (Err)
    *Err = createStringError(errc::illegal_byte_sequence,
                             "no null terminated string at offset 0x%" PRIx64,
                             ప్రారంభం);
  తిరిగి StringRef();
}

StringRef దత్తాంశం_Extractor::get_Fixed_నిడివి_String(uint64_t *స్థానభ్రంశం_Ptr,
                                              uint64_t నిడివి,
                                              StringRef TrimChars) స్థిరరాశి {
  StringRef బైట్లు_2(get_బైట్లు(స్థానభ్రంశం_Ptr, నిడివి));
  తిరిగి బైట్లు_2.trim(TrimChars);
}

StringRef దత్తాంశం_Extractor::get_బైట్లు(uint64_t *స్థానభ్రంశం_Ptr, uint64_t నిడివి,
                                  Error *Err) స్థిరరాశి {
  Error_As_బహిర్గతం_Parameter Err_As_బహిర్గతం(Err);
  ఒకవేళ (isError(Err))
    తిరిగి StringRef();

  ఒకవేళ (!prepareRead(*స్థానభ్రంశం_Ptr, నిడివి, Err))
    తిరిగి StringRef();

  StringRef ఫలితం = దత్తాంశం_2.substr(*స్థానభ్రంశం_Ptr, నిడివి);
  *స్థానభ్రంశం_Ptr += నిడివి;
  తిరిగి ఫలితం;
}

మూస <రకం_పేరు T>
స్థిర T getLEB128(StringRef దత్తాంశం_2, uint64_t *స్థానభ్రంశం_Ptr, Error *Err,
                   T (&Decoder)(స్థిరరాశి uint8_t *p, గుర్తులేని *n,
                                స్థిరరాశి uint8_t *చివర, స్థిరరాశి అక్షరం **error)) {
  ArrayRef<uint8_t> బైట్లు_2 = arrayRefFromStringRef(దత్తాంశం_2);
  ప్రతిజ్ఞా(*స్థానభ్రంశం_Ptr <= బైట్లు_2.కొలత_2());
  Error_As_బహిర్గతం_Parameter Err_As_బహిర్గతం(Err);
  ఒకవేళ (isError(Err))
    తిరిగి T();

  స్థిరరాశి అక్షరం *error = శూన్యసూచిక;
  గుర్తులేని బైట్లు_read;
  T ఫలితం_2 =
      Decoder(బైట్లు_2.దత్తాంశం() + *స్థానభ్రంశం_Ptr, &బైట్లు_read, బైట్లు_2.చివర(), &error);
  ఒకవేళ (error) {
    ఒకవేళ (Err)
      *Err = createStringError(errc::illegal_byte_sequence,
                               "unable to decode LEB128 at offset 0x%8.8" PRIx64
                               ": %s",
                               *స్థానభ్రంశం_Ptr, error);
    తిరిగి T();
  }
  *స్థానభ్రంశం_Ptr += బైట్లు_read;
  తిరిగి ఫలితం_2;
}

uint64_t దత్తాంశం_Extractor::getULEB128(uint64_t *స్థానభ్రంశం_ptr, Error *Err) స్థిరరాశి {
  తిరిగి getLEB128(దత్తాంశం_2, స్థానభ్రంశం_ptr, Err, decodeULEB128);
}

int64_t దత్తాంశం_Extractor::getSLEB128(uint64_t *స్థానభ్రంశం_ptr, Error *Err) స్థిరరాశి {
  తిరిగి getLEB128(దత్తాంశం_2, స్థానభ్రంశం_ptr, Err, decodeSLEB128);
}

APSInt దత్తాంశం_Extractor::getSLEB128APSInt(uint64_t *స్థానభ్రంశం_Ptr, Error *Err) స్థిరరాశి {
  Error_As_బహిర్గతం_Parameter Err_As_బహిర్గతం(Err);
  ఒకవేళ (isError(Err))
    తిరిగి APSInt();

  uint64_t స్థానభ్రంశం = *స్థానభ్రంశం_Ptr;
  APInt విలువ;
  గుర్తులేని జరుపు_2 = 0;
  uint8_t Byte;
  చేయి {
    ఒకవేళ (!prepareRead(స్థానభ్రంశం, 1, Err))
      తిరిగి APSInt();
    Byte = దత్తాంశం_2[స్థానభ్రంశం++];
    APInt ముక్క_2(7, Byte & 0x7f);
    ఒకవేళ (జరుపు_2 == 0)
      విలువ = ముక్క_2;
    లేకపోతే {
      విలువ = విలువ.zext(జరుపు_2 + 7);
      విలువ.చొప్పించు_బిట్లు(ముక్క_2, జరుపు_2);
    }
    జరుపు_2 += 7;
  } అయితే (Byte & 0x80);

  // Sign extend Value to at least 64 bits to match getSLEB128.
  ఒకవేళ (జరుపు_2 < 64)
    విలువ = విలువ.sext(64);

  *స్థానభ్రంశం_Ptr = స్థానభ్రంశం;
  తిరిగి APSInt(std::move(విలువ), /*isUnsigned=*/అబద్ధం);
}

శూన్య దత్తాంశం_Extractor::skip(Cursor &C, uint64_t నిడివి) స్థిరరాశి {
  Error_As_బహిర్గతం_Parameter Err_As_బహిర్గతం(C.Err);
  ఒకవేళ (isError(&C.Err))
    తిరిగి;

  ఒకవేళ (prepareRead(C.స్థానభ్రంశం, నిడివి, &C.Err))
    C.స్థానభ్రంశం += నిడివి;
}
