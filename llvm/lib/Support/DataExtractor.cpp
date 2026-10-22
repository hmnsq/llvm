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
  यदि (isValidOffsetForDataOfSize(Offset, Size))
    प्रत्यागम सत्य;
  यदि (E) {
    यदि (Offset <= Data.size())
      *E = createStringError(
          errc::illegal_byte_sequence,
          "unexpected end of data at offset 0x%zx while reading [0x%" PRIx64
          ", 0x%" PRIx64 ")",
          Data.size(), Offset, Offset + Size);
    अन्यथा
      *E = createStringError(errc::invalid_argument,
                             "offset 0x%" PRIx64
                             " is beyond the end of data at 0x%zx",
                             Offset, Data.size());
  }
  प्रत्यागम असत्य;
}

स्थिर तार्किक isError(Error *E) { प्रत्यागम E && *E; }

template <typename T>
T DataExtractor::getU(uint64_t *offset_ptr, Error *Err) ध्रुव {
  ErrorAsOutParameter ErrAsOut(Err);
  T val = 0;
  यदि (isError(Err))
    प्रत्यागम val;

  uint64_t offset = *offset_ptr;
  यदि (!prepareRead(offset, परिमाण(T), Err))
    प्रत्यागम val;
  std::memcpy(&val, &Data.data()[offset], परिमाण(val));
  यदि (sys::IsLittleEndianHost != IsLittleEndian)
    sys::swapByteOrder(val);

  // Advance the offset
  *offset_ptr += परिमाण(val);
  प्रत्यागम val;
}

template <typename T>
T *DataExtractor::getUs(uint64_t *offset_ptr, T *dst, uint32_t गिनती,
                        Error *Err) ध्रुव {
  ErrorAsOutParameter ErrAsOut(Err);
  यदि (isError(Err))
    प्रत्यागम nullptr;

  uint64_t offset = *offset_ptr;

  यदि (!prepareRead(offset, परिमाण(*dst) * गिनती, Err))
    प्रत्यागम nullptr;
  प्रति (T *मूल्य_ptr = dst, *end = dst + गिनती; मूल्य_ptr != end;
       ++मूल्य_ptr, offset += परिमाण(*dst))
    *मूल्य_ptr = getU<T>(offset_ptr, Err);
  // Advance the offset
  *offset_ptr = offset;
  // Return a non-NULL pointer to the converted data as an indicator of
  // success
  प्रत्यागम dst;
}

uint8_t DataExtractor::getU8(uint64_t *offset_ptr, llvm::Error *Err) ध्रुव {
  प्रत्यागम getU<uint8_t>(offset_ptr, Err);
}

uint8_t *DataExtractor::getU8(uint64_t *offset_ptr, uint8_t *dst,
                              uint32_t गिनती) ध्रुव {
  प्रत्यागम getUs<uint8_t>(offset_ptr, dst, गिनती, nullptr);
}

uint8_t *DataExtractor::getU8(Cursor &C, uint8_t *Dst, uint32_t गिनती_2) ध्रुव {
  प्रत्यागम getUs<uint8_t>(&C.Offset, Dst, गिनती_2, &C.Err);
}

uint16_t DataExtractor::getU16(uint64_t *offset_ptr, llvm::Error *Err) ध्रुव {
  प्रत्यागम getU<uint16_t>(offset_ptr, Err);
}

uint16_t *DataExtractor::getU16(uint64_t *offset_ptr, uint16_t *dst,
                                uint32_t गिनती) ध्रुव {
  प्रत्यागम getUs<uint16_t>(offset_ptr, dst, गिनती, nullptr);
}

uint32_t DataExtractor::getU24(uint64_t *OffsetPtr, Error *Err) ध्रुव {
  uint24_t ExtractedVal = getU<uint24_t>(OffsetPtr, Err);
  // The 3 bytes are in the correct byte order for the host.
  प्रत्यागम ExtractedVal.getAsUint32(sys::IsLittleEndianHost);
}

uint32_t DataExtractor::getU32(uint64_t *offset_ptr, llvm::Error *Err) ध्रुव {
  प्रत्यागम getU<uint32_t>(offset_ptr, Err);
}

uint32_t *DataExtractor::getU32(uint64_t *offset_ptr, uint32_t *dst,
                                uint32_t गिनती) ध्रुव {
  प्रत्यागम getUs<uint32_t>(offset_ptr, dst, गिनती, nullptr);
}

uint64_t DataExtractor::getU64(uint64_t *offset_ptr, llvm::Error *Err) ध्रुव {
  प्रत्यागम getU<uint64_t>(offset_ptr, Err);
}

uint64_t *DataExtractor::getU64(uint64_t *offset_ptr, uint64_t *dst,
                                uint32_t गिनती) ध्रुव {
  प्रत्यागम getUs<uint64_t>(offset_ptr, dst, गिनती, nullptr);
}

uint64_t DataExtractor::getUnsigned(uint64_t *offset_ptr, uint32_t byte_size,
                                    llvm::Error *Err) ध्रुव {
  विकल्प (byte_size) {
  पक्ष 1:
    प्रत्यागम getU8(offset_ptr, Err);
  पक्ष 2:
    प्रत्यागम getU16(offset_ptr, Err);
  पक्ष 4:
    प्रत्यागम getU32(offset_ptr, Err);
  पक्ष 8:
    प्रत्यागम getU64(offset_ptr, Err);
  }

  // For any other byte size, read the bytes and swap/shift if necessary.
  ErrorAsOutParameter ErrAsOut(Err);
  uint64_t val = 0;
  यदि (isError(Err))
    प्रत्यागम val;
  uint64_t offset = *offset_ptr;
  यदि (!prepareRead(offset, byte_size, Err))
    प्रत्यागम val;
  // Copy into the least significant bytes of val regardless of host
  // endianness.
  std::memcpy(reinterpret_cast<वर्ण *>(&val) +
                  (sys::IsLittleEndianHost ? 0 : 8 - byte_size),
              &Data.data()[offset], byte_size);
  // Swap the least significant bytes of val if endianness doesn't match.
  यदि (sys::IsLittleEndianHost != IsLittleEndian)
    val = sys::getSwappedBytes(val) >> (8 * (8 - byte_size));

  *offset_ptr += byte_size;
  प्रत्यागम val;
}

int64_t
DataExtractor::getSigned(uint64_t *offset_ptr, uint32_t byte_size) ध्रुव {
  विकल्प (byte_size) {
  पक्ष 1:
    प्रत्यागम getS8(offset_ptr);
  पक्ष 2:
    प्रत्यागम getS16(offset_ptr);
  पक्ष 4:
    प्रत्यागम getS32(offset_ptr);
  पक्ष 8:
    प्रत्यागम getS64(offset_ptr);
  }
  llvm_unreachable("getSigned unhandled case!");
}

StringRef DataExtractor::getCStrRef(uint64_t *OffsetPtr, Error *Err) ध्रुव {
  ErrorAsOutParameter ErrAsOut(Err);
  यदि (isError(Err))
    प्रत्यागम StringRef();

  uint64_t Start = *OffsetPtr;
  StringRef::size_type Pos = Data.find('\0', Start);
  यदि (Pos != StringRef::npos) {
    *OffsetPtr = Pos + 1;
    प्रत्यागम StringRef(Data.data() + Start, Pos - Start);
  }
  यदि (Err)
    *Err = createStringError(errc::illegal_byte_sequence,
                             "no null terminated string at offset 0x%" PRIx64,
                             Start);
  प्रत्यागम StringRef();
}

StringRef DataExtractor::getFixedLengthString(uint64_t *OffsetPtr,
                                              uint64_t Length,
                                              StringRef TrimChars) ध्रुव {
  StringRef Bytes(getBytes(OffsetPtr, Length));
  प्रत्यागम Bytes.trim(TrimChars);
}

StringRef DataExtractor::getBytes(uint64_t *OffsetPtr, uint64_t Length,
                                  Error *Err) ध्रुव {
  ErrorAsOutParameter ErrAsOut(Err);
  यदि (isError(Err))
    प्रत्यागम StringRef();

  यदि (!prepareRead(*OffsetPtr, Length, Err))
    प्रत्यागम StringRef();

  StringRef Result = Data.substr(*OffsetPtr, Length);
  *OffsetPtr += Length;
  प्रत्यागम Result;
}

template <typename T>
स्थिर T getLEB128(StringRef Data, uint64_t *OffsetPtr, Error *Err,
                   T (&Decoder)(ध्रुव uint8_t *p, unsigned *n,
                                ध्रुव uint8_t *end, ध्रुव वर्ण **error)) {
  ArrayRef<uint8_t> Bytes = arrayRefFromStringRef(Data);
  प्रतिज्ञा(*OffsetPtr <= Bytes.size());
  ErrorAsOutParameter ErrAsOut(Err);
  यदि (isError(Err))
    प्रत्यागम T();

  ध्रुव वर्ण *error = nullptr;
  unsigned bytes_read;
  T result =
      Decoder(Bytes.data() + *OffsetPtr, &bytes_read, Bytes.end(), &error);
  यदि (error) {
    यदि (Err)
      *Err = createStringError(errc::illegal_byte_sequence,
                               "unable to decode LEB128 at offset 0x%8.8" PRIx64
                               ": %s",
                               *OffsetPtr, error);
    प्रत्यागम T();
  }
  *OffsetPtr += bytes_read;
  प्रत्यागम result;
}

uint64_t DataExtractor::getULEB128(uint64_t *offset_ptr, Error *Err) ध्रुव {
  प्रत्यागम getLEB128(Data, offset_ptr, Err, decodeULEB128);
}

int64_t DataExtractor::getSLEB128(uint64_t *offset_ptr, Error *Err) ध्रुव {
  प्रत्यागम getLEB128(Data, offset_ptr, Err, decodeSLEB128);
}

APSInt DataExtractor::getSLEB128APSInt(uint64_t *OffsetPtr, Error *Err) ध्रुव {
  ErrorAsOutParameter ErrAsOut(Err);
  यदि (isError(Err))
    प्रत्यागम APSInt();

  uint64_t Offset = *OffsetPtr;
  APInt मूल्य;
  unsigned Shift = 0;
  uint8_t Byte;
  कुरु {
    यदि (!prepareRead(Offset, 1, Err))
      प्रत्यागम APSInt();
    Byte = Data[Offset++];
    APInt Slice(7, Byte & 0x7f);
    यदि (Shift == 0)
      मूल्य = Slice;
    अन्यथा {
      मूल्य = मूल्य.zext(Shift + 7);
      मूल्य.जोड़ो_Bits(Slice, Shift);
    }
    Shift += 7;
  } यावत् (Byte & 0x80);

  // Sign extend Value to at least 64 bits to match getSLEB128.
  यदि (Shift < 64)
    मूल्य = मूल्य.sext(64);

  *OffsetPtr = Offset;
  प्रत्यागम APSInt(std::move(मूल्य), /*isUnsigned=*/असत्य);
}

शून्य DataExtractor::skip(Cursor &C, uint64_t Length) ध्रुव {
  ErrorAsOutParameter ErrAsOut(C.Err);
  यदि (isError(&C.Err))
    प्रत्यागम;

  यदि (prepareRead(C.Offset, Length, &C.Err))
    C.Offset += Length;
}
