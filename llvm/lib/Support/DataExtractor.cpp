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

using नामाकाश llvm;

तार्किक DataExtractor::prepareRead(uint64_t Offset, uint64_t Size,
                                Error *E) ध्रुव {
  सचे (isValidOffsetForDataOfSize(Offset, Size))
    पटिनिवत्त सच्च;
  सचे (E) {
    सचे (Offset <= Data.size())
      *E = createStringError(
          errc::illegal_byte_sequence,
          "unexpected end of data at offset 0x%zx while reading [0x%" PRIx64
          ", 0x%" PRIx64 ")",
          Data.size(), Offset, Offset + Size);
    अञ्ञथा
      *E = createStringError(errc::invalid_argument,
                             "offset 0x%" PRIx64
                             " is beyond the end of data at 0x%zx",
                             Offset, Data.size());
  }
  पटिनिवत्त मिच्छा;
}

स्थिर तार्किक isError(Error *E) { पटिनिवत्त E && *E; }

template <typename T>
T DataExtractor::getU(uint64_t *offset_ptr, Error *Err) ध्रुव {
  ErrorAsOutParameter ErrAsOut(Err);
  T val = 0;
  सचे (isError(Err))
    पटिनिवत्त val;

  uint64_t offset = *offset_ptr;
  सचे (!prepareRead(offset, परिमाण(T), Err))
    पटिनिवत्त val;
  std::memcpy(&val, &Data.data()[offset], परिमाण(val));
  सचे (sys::IsLittleEndianHost != IsLittleEndian)
    sys::swapByteOrder(val);

  // Advance the offset
  *offset_ptr += परिमाण(val);
  पटिनिवत्त val;
}

template <typename T>
T *DataExtractor::getUs(uint64_t *offset_ptr, T *dst, uint32_t गणनम्,
                        Error *Err) ध्रुव {
  ErrorAsOutParameter ErrAsOut(Err);
  सचे (isError(Err))
    पटिनिवत्त nullptr;

  uint64_t offset = *offset_ptr;

  सचे (!prepareRead(offset, परिमाण(*dst) * गणनम्, Err))
    पटिनिवत्त nullptr;
  अत्थाय (T *मानम्_ptr = dst, *end = dst + गणनम्; मानम्_ptr != end;
       ++मानम्_ptr, offset += परिमाण(*dst))
    *मानम्_ptr = getU<T>(offset_ptr, Err);
  // Advance the offset
  *offset_ptr = offset;
  // Return a non-NULL pointer to the converted data as an indicator of
  // success
  पटिनिवत्त dst;
}

uint8_t DataExtractor::getU8(uint64_t *offset_ptr, llvm::Error *Err) ध्रुव {
  पटिनिवत्त getU<uint8_t>(offset_ptr, Err);
}

uint8_t *DataExtractor::getU8(uint64_t *offset_ptr, uint8_t *dst,
                              uint32_t गणनम्) ध्रुव {
  पटिनिवत्त getUs<uint8_t>(offset_ptr, dst, गणनम्, nullptr);
}

uint8_t *DataExtractor::getU8(Cursor &C, uint8_t *Dst, uint32_t गणनम्_2) ध्रुव {
  पटिनिवत्त getUs<uint8_t>(&C.Offset, Dst, गणनम्_2, &C.Err);
}

uint16_t DataExtractor::getU16(uint64_t *offset_ptr, llvm::Error *Err) ध्रुव {
  पटिनिवत्त getU<uint16_t>(offset_ptr, Err);
}

uint16_t *DataExtractor::getU16(uint64_t *offset_ptr, uint16_t *dst,
                                uint32_t गणनम्) ध्रुव {
  पटिनिवत्त getUs<uint16_t>(offset_ptr, dst, गणनम्, nullptr);
}

uint32_t DataExtractor::getU24(uint64_t *OffsetPtr, Error *Err) ध्रुव {
  uint24_t ExtractedVal = getU<uint24_t>(OffsetPtr, Err);
  // The 3 bytes are in the correct byte order for the host.
  पटिनिवत्त ExtractedVal.getAsUint32(sys::IsLittleEndianHost);
}

uint32_t DataExtractor::getU32(uint64_t *offset_ptr, llvm::Error *Err) ध्रुव {
  पटिनिवत्त getU<uint32_t>(offset_ptr, Err);
}

uint32_t *DataExtractor::getU32(uint64_t *offset_ptr, uint32_t *dst,
                                uint32_t गणनम्) ध्रुव {
  पटिनिवत्त getUs<uint32_t>(offset_ptr, dst, गणनम्, nullptr);
}

uint64_t DataExtractor::getU64(uint64_t *offset_ptr, llvm::Error *Err) ध्रुव {
  पटिनिवत्त getU<uint64_t>(offset_ptr, Err);
}

uint64_t *DataExtractor::getU64(uint64_t *offset_ptr, uint64_t *dst,
                                uint32_t गणनम्) ध्रुव {
  पटिनिवत्त getUs<uint64_t>(offset_ptr, dst, गणनम्, nullptr);
}

uint64_t DataExtractor::getUnsigned(uint64_t *offset_ptr, uint32_t byte_size,
                                    llvm::Error *Err) ध्रुव {
  विकल्प (byte_size) {
  पक्ष 1:
    पटिनिवत्त getU8(offset_ptr, Err);
  पक्ष 2:
    पटिनिवत्त getU16(offset_ptr, Err);
  पक्ष 4:
    पटिनिवत्त getU32(offset_ptr, Err);
  पक्ष 8:
    पटिनिवत्त getU64(offset_ptr, Err);
  }

  // For any other byte size, read the bytes and swap/shift if necessary.
  ErrorAsOutParameter ErrAsOut(Err);
  uint64_t val = 0;
  सचे (isError(Err))
    पटिनिवत्त val;
  uint64_t offset = *offset_ptr;
  सचे (!prepareRead(offset, byte_size, Err))
    पटिनिवत्त val;
  // Copy into the least significant bytes of val regardless of host
  // endianness.
  std::memcpy(reinterpret_cast<वर्ण *>(&val) +
                  (sys::IsLittleEndianHost ? 0 : 8 - byte_size),
              &Data.data()[offset], byte_size);
  // Swap the least significant bytes of val if endianness doesn't match.
  सचे (sys::IsLittleEndianHost != IsLittleEndian)
    val = sys::getSwappedBytes(val) >> (8 * (8 - byte_size));

  *offset_ptr += byte_size;
  पटिनिवत्त val;
}

int64_t
DataExtractor::getSigned(uint64_t *offset_ptr, uint32_t byte_size) ध्रुव {
  विकल्प (byte_size) {
  पक्ष 1:
    पटिनिवत्त getS8(offset_ptr);
  पक्ष 2:
    पटिनिवत्त getS16(offset_ptr);
  पक्ष 4:
    पटिनिवत्त getS32(offset_ptr);
  पक्ष 8:
    पटिनिवत्त getS64(offset_ptr);
  }
  llvm_unreachable("getSigned unhandled case!");
}

StringRef DataExtractor::getCStrRef(uint64_t *OffsetPtr, Error *Err) ध्रुव {
  ErrorAsOutParameter ErrAsOut(Err);
  सचे (isError(Err))
    पटिनिवत्त StringRef();

  uint64_t Start = *OffsetPtr;
  StringRef::size_type Pos = Data.find('\0', Start);
  सचे (Pos != StringRef::npos) {
    *OffsetPtr = Pos + 1;
    पटिनिवत्त StringRef(Data.data() + Start, Pos - Start);
  }
  सचे (Err)
    *Err = createStringError(errc::illegal_byte_sequence,
                             "no null terminated string at offset 0x%" PRIx64,
                             Start);
  पटिनिवत्त StringRef();
}

StringRef DataExtractor::getFixedLengthString(uint64_t *OffsetPtr,
                                              uint64_t Length,
                                              StringRef TrimChars) ध्रुव {
  StringRef Bytes(getBytes(OffsetPtr, Length));
  पटिनिवत्त Bytes.trim(TrimChars);
}

StringRef DataExtractor::getBytes(uint64_t *OffsetPtr, uint64_t Length,
                                  Error *Err) ध्रुव {
  ErrorAsOutParameter ErrAsOut(Err);
  सचे (isError(Err))
    पटिनिवत्त StringRef();

  सचे (!prepareRead(*OffsetPtr, Length, Err))
    पटिनिवत्त StringRef();

  StringRef Result = Data.substr(*OffsetPtr, Length);
  *OffsetPtr += Length;
  पटिनिवत्त Result;
}

template <typename T>
स्थिर T getLEB128(StringRef Data, uint64_t *OffsetPtr, Error *Err,
                   T (&Decoder)(ध्रुव uint8_t *p, unsigned *n,
                                ध्रुव uint8_t *end, ध्रुव वर्ण **error)) {
  ArrayRef<uint8_t> Bytes = arrayRefFromStringRef(Data);
  प्रतिज्ञा(*OffsetPtr <= Bytes.size());
  ErrorAsOutParameter ErrAsOut(Err);
  सचे (isError(Err))
    पटिनिवत्त T();

  ध्रुव वर्ण *error = nullptr;
  unsigned bytes_read;
  T result =
      Decoder(Bytes.data() + *OffsetPtr, &bytes_read, Bytes.end(), &error);
  सचे (error) {
    सचे (Err)
      *Err = createStringError(errc::illegal_byte_sequence,
                               "unable to decode LEB128 at offset 0x%8.8" PRIx64
                               ": %s",
                               *OffsetPtr, error);
    पटिनिवत्त T();
  }
  *OffsetPtr += bytes_read;
  पटिनिवत्त result;
}

uint64_t DataExtractor::getULEB128(uint64_t *offset_ptr, Error *Err) ध्रुव {
  पटिनिवत्त getLEB128(Data, offset_ptr, Err, decodeULEB128);
}

int64_t DataExtractor::getSLEB128(uint64_t *offset_ptr, Error *Err) ध्रुव {
  पटिनिवत्त getLEB128(Data, offset_ptr, Err, decodeSLEB128);
}

APSInt DataExtractor::getSLEB128APSInt(uint64_t *OffsetPtr, Error *Err) ध्रुव {
  ErrorAsOutParameter ErrAsOut(Err);
  सचे (isError(Err))
    पटिनिवत्त APSInt();

  uint64_t Offset = *OffsetPtr;
  APInt मानम्;
  unsigned Shift = 0;
  uint8_t Byte;
  करोहि {
    सचे (!prepareRead(Offset, 1, Err))
      पटिनिवत्त APSInt();
    Byte = Data[Offset++];
    APInt Slice(7, Byte & 0x7f);
    सचे (Shift == 0)
      मानम् = Slice;
    अञ्ञथा {
      मानम् = मानम्.zext(Shift + 7);
      मानम्.निवेशय_Bits(Slice, Shift);
    }
    Shift += 7;
  } याव (Byte & 0x80);

  // Sign extend Value to at least 64 bits to match getSLEB128.
  सचे (Shift < 64)
    मानम् = मानम्.sext(64);

  *OffsetPtr = Offset;
  पटिनिवत्त APSInt(std::move(मानम्), /*isUnsigned=*/मिच्छा);
}

शून्य DataExtractor::skip(Cursor &C, uint64_t Length) ध्रुव {
  ErrorAsOutParameter ErrAsOut(C.Err);
  सचे (isError(&C.Err))
    पटिनिवत्त;

  सचे (prepareRead(C.Offset, Length, &C.Err))
    C.Offset += Length;
}
