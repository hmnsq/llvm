//===-- DataExtractor.cpp -------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/DataExtractor.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/Support/Errc.h"
#अन्तर्भाव "llvm/Support/ErrorHandling.h"
#अन्तर्भाव "llvm/Support/LEB128.h"
#अन्तर्भाव "llvm/Support/SwapByteOrder.h"

उपयुज्य नामाकाश llvm;

तार्किक दत्त_Extractor::prepareRead(uint64_t विस्थापनम्, uint64_t आकार,
                                Error *E) ध्रुव {
  यदि (is_Valid_विस्थापनम्_For_दत्त_सम्बन्धि_आकार(विस्थापनम्, आकार))
    प्रत्यागम सत्य;
  यदि (E) {
    यदि (विस्थापनम् <= दत्त_2.आकार_2())
      *E = createStringError(
          errc::illegal_byte_sequence,
          "unexpected end of data at offset 0x%zx while reading [0x%" PRIx64
          ", 0x%" PRIx64 ")",
          दत्त_2.आकार_2(), विस्थापनम्, विस्थापनम् + आकार);
    अन्यथा
      *E = createStringError(errc::invalid_तर्कः,
                             "offset 0x%" PRIx64
                             " is beyond the end of data at 0x%zx",
                             विस्थापनम्, दत्त_2.आकार_2());
  }
  प्रत्यागम असत्य;
}

स्थिर तार्किक isError(Error *E) { प्रत्यागम E && *E; }

प्रतिरूपम् <प्रकारनाम T>
T दत्त_Extractor::getU(uint64_t *विस्थापनम्_ptr, Error *Err) ध्रुव {
  Error_As_बहिर्गमनम्_Parameter Err_As_बहिर्गमनम्(Err);
  T val = 0;
  यदि (isError(Err))
    प्रत्यागम val;

  uint64_t विस्थापनम्_2 = *विस्थापनम्_ptr;
  यदि (!prepareRead(विस्थापनम्_2, परिमाण(T), Err))
    प्रत्यागम val;
  std::स्मृतिं_प्रतिलिख(&val, &दत्त_2.दत्त()[विस्थापनम्_2], परिमाण(val));
  यदि (sys::IsLittleEndianHost != IsLittleEndian)
    sys::swap_Byte_क्रमः(val);

  // Advance the offset
  *विस्थापनम्_ptr += परिमाण(val);
  प्रत्यागम val;
}

प्रतिरूपम् <प्रकारनाम T>
T *दत्त_Extractor::getUs(uint64_t *विस्थापनम्_ptr, T *dst, uint32_t गणनम्,
                        Error *Err) ध्रुव {
  Error_As_बहिर्गमनम्_Parameter Err_As_बहिर्गमनम्(Err);
  यदि (isError(Err))
    प्रत्यागम शून्यसूचकः;

  uint64_t विस्थापनम्_2 = *विस्थापनम्_ptr;

  यदि (!prepareRead(विस्थापनम्_2, परिमाण(*dst) * गणनम्, Err))
    प्रत्यागम शून्यसूचकः;
  प्रति (T *मानम्_ptr = dst, *अन्तः = dst + गणनम्; मानम्_ptr != अन्तः;
       ++मानम्_ptr, विस्थापनम्_2 += परिमाण(*dst))
    *मानम्_ptr = getU<T>(विस्थापनम्_ptr, Err);
  // Advance the offset
  *विस्थापनम्_ptr = विस्थापनम्_2;
  // Return a non-NULL pointer to the converted data as an indicator of
  // success
  प्रत्यागम dst;
}

uint8_t दत्त_Extractor::getU8(uint64_t *विस्थापनम्_ptr, llvm::Error *Err) ध्रुव {
  प्रत्यागम getU<uint8_t>(विस्थापनम्_ptr, Err);
}

uint8_t *दत्त_Extractor::getU8(uint64_t *विस्थापनम्_ptr, uint8_t *dst,
                              uint32_t गणनम्) ध्रुव {
  प्रत्यागम getUs<uint8_t>(विस्थापनम्_ptr, dst, गणनम्, शून्यसूचकः);
}

uint8_t *दत्त_Extractor::getU8(Cursor &C, uint8_t *Dst, uint32_t गणनम्_2) ध्रुव {
  प्रत्यागम getUs<uint8_t>(&C.विस्थापनम्, Dst, गणनम्_2, &C.Err);
}

uint16_t दत्त_Extractor::getU16(uint64_t *विस्थापनम्_ptr, llvm::Error *Err) ध्रुव {
  प्रत्यागम getU<uint16_t>(विस्थापनम्_ptr, Err);
}

uint16_t *दत्त_Extractor::getU16(uint64_t *विस्थापनम्_ptr, uint16_t *dst,
                                uint32_t गणनम्) ध्रुव {
  प्रत्यागम getUs<uint16_t>(विस्थापनम्_ptr, dst, गणनम्, शून्यसूचकः);
}

uint32_t दत्त_Extractor::getU24(uint64_t *विस्थापनम्_Ptr, Error *Err) ध्रुव {
  uint24_t ExtractedVal = getU<uint24_t>(विस्थापनम्_Ptr, Err);
  // The 3 bytes are in the correct byte order for the host.
  प्रत्यागम ExtractedVal.getAsUint32(sys::IsLittleEndianHost);
}

uint32_t दत्त_Extractor::getU32(uint64_t *विस्थापनम्_ptr, llvm::Error *Err) ध्रुव {
  प्रत्यागम getU<uint32_t>(विस्थापनम्_ptr, Err);
}

uint32_t *दत्त_Extractor::getU32(uint64_t *विस्थापनम्_ptr, uint32_t *dst,
                                uint32_t गणनम्) ध्रुव {
  प्रत्यागम getUs<uint32_t>(विस्थापनम्_ptr, dst, गणनम्, शून्यसूचकः);
}

uint64_t दत्त_Extractor::getU64(uint64_t *विस्थापनम्_ptr, llvm::Error *Err) ध्रुव {
  प्रत्यागम getU<uint64_t>(विस्थापनम्_ptr, Err);
}

uint64_t *दत्त_Extractor::getU64(uint64_t *विस्थापनम्_ptr, uint64_t *dst,
                                uint32_t गणनम्) ध्रुव {
  प्रत्यागम getUs<uint64_t>(विस्थापनम्_ptr, dst, गणनम्, शून्यसूचकः);
}

uint64_t दत्त_Extractor::getUnsigned(uint64_t *विस्थापनम्_ptr, uint32_t byte_आकार,
                                    llvm::Error *Err) ध्रुव {
  विकल्प (byte_आकार) {
  पक्ष 1:
    प्रत्यागम getU8(विस्थापनम्_ptr, Err);
  पक्ष 2:
    प्रत्यागम getU16(विस्थापनम्_ptr, Err);
  पक्ष 4:
    प्रत्यागम getU32(विस्थापनम्_ptr, Err);
  पक्ष 8:
    प्रत्यागम getU64(विस्थापनम्_ptr, Err);
  }

  // For any other byte size, read the bytes and swap/shift if necessary.
  Error_As_बहिर्गमनम्_Parameter Err_As_बहिर्गमनम्(Err);
  uint64_t val = 0;
  यदि (isError(Err))
    प्रत्यागम val;
  uint64_t विस्थापनम्_2 = *विस्थापनम्_ptr;
  यदि (!prepareRead(विस्थापनम्_2, byte_आकार, Err))
    प्रत्यागम val;
  // Copy into the least significant bytes of val regardless of host
  // endianness.
  std::स्मृतिं_प्रतिलिख(reinterpret_cast<वर्ण *>(&val) +
                  (sys::IsLittleEndianHost ? 0 : 8 - byte_आकार),
              &दत्त_2.दत्त()[विस्थापनम्_2], byte_आकार);
  // Swap the least significant bytes of val if endianness doesn't match.
  यदि (sys::IsLittleEndianHost != IsLittleEndian)
    val = sys::get_Swapped_अष्टकानि(val) >> (8 * (8 - byte_आकार));

  *विस्थापनम्_ptr += byte_आकार;
  प्रत्यागम val;
}

int64_t
दत्त_Extractor::getSigned(uint64_t *विस्थापनम्_ptr, uint32_t byte_आकार) ध्रुव {
  विकल्प (byte_आकार) {
  पक्ष 1:
    प्रत्यागम getS8(विस्थापनम्_ptr);
  पक्ष 2:
    प्रत्यागम getS16(विस्थापनम्_ptr);
  पक्ष 4:
    प्रत्यागम getS32(विस्थापनम्_ptr);
  पक्ष 8:
    प्रत्यागम getS64(विस्थापनम्_ptr);
  }
  llvm_unreachable("getSigned unhandled case!");
}

StringRef दत्त_Extractor::getCStrRef(uint64_t *विस्थापनम्_Ptr, Error *Err) ध्रुव {
  Error_As_बहिर्गमनम्_Parameter Err_As_बहिर्गमनम्(Err);
  यदि (isError(Err))
    प्रत्यागम StringRef();

  uint64_t Start = *विस्थापनम्_Ptr;
  StringRef::आकार_type Pos = दत्त_2.find('\0', Start);
  यदि (Pos != StringRef::npos) {
    *विस्थापनम्_Ptr = Pos + 1;
    प्रत्यागम StringRef(दत्त_2.दत्त() + Start, Pos - Start);
  }
  यदि (Err)
    *Err = createStringError(errc::illegal_byte_sequence,
                             "no null terminated string at offset 0x%" PRIx64,
                             Start);
  प्रत्यागम StringRef();
}

StringRef दत्त_Extractor::get_Fixed_दीर्घता_String(uint64_t *विस्थापनम्_Ptr,
                                              uint64_t दीर्घता,
                                              StringRef TrimChars) ध्रुव {
  StringRef अष्टकानि_2(get_अष्टकानि(विस्थापनम्_Ptr, दीर्घता));
  प्रत्यागम अष्टकानि_2.trim(TrimChars);
}

StringRef दत्त_Extractor::get_अष्टकानि(uint64_t *विस्थापनम्_Ptr, uint64_t दीर्घता,
                                  Error *Err) ध्रुव {
  Error_As_बहिर्गमनम्_Parameter Err_As_बहिर्गमनम्(Err);
  यदि (isError(Err))
    प्रत्यागम StringRef();

  यदि (!prepareRead(*विस्थापनम्_Ptr, दीर्घता, Err))
    प्रत्यागम StringRef();

  StringRef परिणाम = दत्त_2.substr(*विस्थापनम्_Ptr, दीर्घता);
  *विस्थापनम्_Ptr += दीर्घता;
  प्रत्यागम परिणाम;
}

प्रतिरूपम् <प्रकारनाम T>
स्थिर T getLEB128(StringRef दत्त_2, uint64_t *विस्थापनम्_Ptr, Error *Err,
                   T (&Decoder)(ध्रुव uint8_t *p, अचिह्नित *n,
                                ध्रुव uint8_t *अन्तः, ध्रुव वर्ण **error)) {
  ArrayRef<uint8_t> अष्टकानि_2 = arrayRefFromStringRef(दत्त_2);
  प्रतिज्ञा(*विस्थापनम्_Ptr <= अष्टकानि_2.आकार_2());
  Error_As_बहिर्गमनम्_Parameter Err_As_बहिर्गमनम्(Err);
  यदि (isError(Err))
    प्रत्यागम T();

  ध्रुव वर्ण *error = शून्यसूचकः;
  अचिह्नित अष्टकानि_read;
  T परिणाम_2 =
      Decoder(अष्टकानि_2.दत्त() + *विस्थापनम्_Ptr, &अष्टकानि_read, अष्टकानि_2.अन्तः(), &error);
  यदि (error) {
    यदि (Err)
      *Err = createStringError(errc::illegal_byte_sequence,
                               "unable to decode LEB128 at offset 0x%8.8" PRIx64
                               ": %s",
                               *विस्थापनम्_Ptr, error);
    प्रत्यागम T();
  }
  *विस्थापनम्_Ptr += अष्टकानि_read;
  प्रत्यागम परिणाम_2;
}

uint64_t दत्त_Extractor::getULEB128(uint64_t *विस्थापनम्_ptr, Error *Err) ध्रुव {
  प्रत्यागम getLEB128(दत्त_2, विस्थापनम्_ptr, Err, decodeULEB128);
}

int64_t दत्त_Extractor::getSLEB128(uint64_t *विस्थापनम्_ptr, Error *Err) ध्रुव {
  प्रत्यागम getLEB128(दत्त_2, विस्थापनम्_ptr, Err, decodeSLEB128);
}

APSInt दत्त_Extractor::getSLEB128APSInt(uint64_t *विस्थापनम्_Ptr, Error *Err) ध्रुव {
  Error_As_बहिर्गमनम्_Parameter Err_As_बहिर्गमनम्(Err);
  यदि (isError(Err))
    प्रत्यागम APSInt();

  uint64_t विस्थापनम् = *विस्थापनम्_Ptr;
  APInt मानम्;
  अचिह्नित सरणम्_2 = 0;
  uint8_t Byte;
  कुरु {
    यदि (!prepareRead(विस्थापनम्, 1, Err))
      प्रत्यागम APSInt();
    Byte = दत्त_2[विस्थापनम्++];
    APInt खण्डिका_2(7, Byte & 0x7f);
    यदि (सरणम्_2 == 0)
      मानम् = खण्डिका_2;
    अन्यथा {
      मानम् = मानम्.zext(सरणम्_2 + 7);
      मानम्.निवेशय_द्वयङ्काः(खण्डिका_2, सरणम्_2);
    }
    सरणम्_2 += 7;
  } यावत् (Byte & 0x80);

  // Sign extend Value to at least 64 bits to match getSLEB128.
  यदि (सरणम्_2 < 64)
    मानम् = मानम्.sext(64);

  *विस्थापनम्_Ptr = विस्थापनम्;
  प्रत्यागम APSInt(std::move(मानम्), /*isUnsigned=*/असत्य);
}

शून्य दत्त_Extractor::skip(Cursor &C, uint64_t दीर्घता) ध्रुव {
  Error_As_बहिर्गमनम्_Parameter Err_As_बहिर्गमनम्(C.Err);
  यदि (isError(&C.Err))
    प्रत्यागम;

  यदि (prepareRead(C.विस्थापनम्, दीर्घता, &C.Err))
    C.विस्थापनम् += दीर्घता;
}
