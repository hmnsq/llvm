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
                                Error *E) स्थिरांक {
  जर (isValidOffsetForDataOfSize(Offset, Size))
    परत सत्य;
  जर (E) {
    जर (Offset <= Data.size())
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
  परत असत्य;
}

स्थिर तार्किक isError(Error *E) { परत E && *E; }

ढाँचा <typename T>
T DataExtractor::getU(uint64_t *offset_ptr, Error *Err) स्थिरांक {
  ErrorAsOutParameter ErrAsOut(Err);
  T val = 0;
  जर (isError(Err))
    परत val;

  uint64_t offset = *offset_ptr;
  जर (!prepareRead(offset, आकार(T), Err))
    परत val;
  std::स_नकल(&val, &Data.data()[offset], आकार(val));
  जर (sys::IsLittleEndianHost != IsLittleEndian)
    sys::swapByteOrder(val);

  // Advance the offset
  *offset_ptr += आकार(val);
  परत val;
}

ढाँचा <typename T>
T *DataExtractor::getUs(uint64_t *offset_ptr, T *dst, uint32_t मोजणी,
                        Error *Err) स्थिरांक {
  ErrorAsOutParameter ErrAsOut(Err);
  जर (isError(Err))
    परत nullptr;

  uint64_t offset = *offset_ptr;

  जर (!prepareRead(offset, आकार(*dst) * मोजणी, Err))
    परत nullptr;
  साठी (T *मूल्य_ptr = dst, *end = dst + मोजणी; मूल्य_ptr != end;
       ++मूल्य_ptr, offset += आकार(*dst))
    *मूल्य_ptr = getU<T>(offset_ptr, Err);
  // Advance the offset
  *offset_ptr = offset;
  // Return a non-NULL pointer to the converted data as an indicator of
  // success
  परत dst;
}

uint8_t DataExtractor::getU8(uint64_t *offset_ptr, llvm::Error *Err) स्थिरांक {
  परत getU<uint8_t>(offset_ptr, Err);
}

uint8_t *DataExtractor::getU8(uint64_t *offset_ptr, uint8_t *dst,
                              uint32_t मोजणी) स्थिरांक {
  परत getUs<uint8_t>(offset_ptr, dst, मोजणी, nullptr);
}

uint8_t *DataExtractor::getU8(Cursor &C, uint8_t *Dst, uint32_t मोजणी_2) स्थिरांक {
  परत getUs<uint8_t>(&C.Offset, Dst, मोजणी_2, &C.Err);
}

uint16_t DataExtractor::getU16(uint64_t *offset_ptr, llvm::Error *Err) स्थिरांक {
  परत getU<uint16_t>(offset_ptr, Err);
}

uint16_t *DataExtractor::getU16(uint64_t *offset_ptr, uint16_t *dst,
                                uint32_t मोजणी) स्थिरांक {
  परत getUs<uint16_t>(offset_ptr, dst, मोजणी, nullptr);
}

uint32_t DataExtractor::getU24(uint64_t *OffsetPtr, Error *Err) स्थिरांक {
  uint24_t ExtractedVal = getU<uint24_t>(OffsetPtr, Err);
  // The 3 bytes are in the correct byte order for the host.
  परत ExtractedVal.getAsUint32(sys::IsLittleEndianHost);
}

uint32_t DataExtractor::getU32(uint64_t *offset_ptr, llvm::Error *Err) स्थिरांक {
  परत getU<uint32_t>(offset_ptr, Err);
}

uint32_t *DataExtractor::getU32(uint64_t *offset_ptr, uint32_t *dst,
                                uint32_t मोजणी) स्थिरांक {
  परत getUs<uint32_t>(offset_ptr, dst, मोजणी, nullptr);
}

uint64_t DataExtractor::getU64(uint64_t *offset_ptr, llvm::Error *Err) स्थिरांक {
  परत getU<uint64_t>(offset_ptr, Err);
}

uint64_t *DataExtractor::getU64(uint64_t *offset_ptr, uint64_t *dst,
                                uint32_t मोजणी) स्थिरांक {
  परत getUs<uint64_t>(offset_ptr, dst, मोजणी, nullptr);
}

uint64_t DataExtractor::getUnsigned(uint64_t *offset_ptr, uint32_t byte_size,
                                    llvm::Error *Err) स्थिरांक {
  निवड (byte_size) {
  स्थिती 1:
    परत getU8(offset_ptr, Err);
  स्थिती 2:
    परत getU16(offset_ptr, Err);
  स्थिती 4:
    परत getU32(offset_ptr, Err);
  स्थिती 8:
    परत getU64(offset_ptr, Err);
  }

  // For any other byte size, read the bytes and swap/shift if necessary.
  ErrorAsOutParameter ErrAsOut(Err);
  uint64_t val = 0;
  जर (isError(Err))
    परत val;
  uint64_t offset = *offset_ptr;
  जर (!prepareRead(offset, byte_size, Err))
    परत val;
  // Copy into the least significant bytes of val regardless of host
  // endianness.
  std::स_नकल(reinterpret_cast<अक्षर *>(&val) +
                  (sys::IsLittleEndianHost ? 0 : 8 - byte_size),
              &Data.data()[offset], byte_size);
  // Swap the least significant bytes of val if endianness doesn't match.
  जर (sys::IsLittleEndianHost != IsLittleEndian)
    val = sys::getSwappedBytes(val) >> (8 * (8 - byte_size));

  *offset_ptr += byte_size;
  परत val;
}

int64_t
DataExtractor::getSigned(uint64_t *offset_ptr, uint32_t byte_size) स्थिरांक {
  निवड (byte_size) {
  स्थिती 1:
    परत getS8(offset_ptr);
  स्थिती 2:
    परत getS16(offset_ptr);
  स्थिती 4:
    परत getS32(offset_ptr);
  स्थिती 8:
    परत getS64(offset_ptr);
  }
  llvm_unreachable("getSigned unhandled case!");
}

StringRef DataExtractor::getCStrRef(uint64_t *OffsetPtr, Error *Err) स्थिरांक {
  ErrorAsOutParameter ErrAsOut(Err);
  जर (isError(Err))
    परत StringRef();

  uint64_t Start = *OffsetPtr;
  StringRef::size_type Pos = Data.find('\0', Start);
  जर (Pos != StringRef::npos) {
    *OffsetPtr = Pos + 1;
    परत StringRef(Data.data() + Start, Pos - Start);
  }
  जर (Err)
    *Err = createStringError(errc::illegal_byte_sequence,
                             "no null terminated string at offset 0x%" PRIx64,
                             Start);
  परत StringRef();
}

StringRef DataExtractor::getFixedLengthString(uint64_t *OffsetPtr,
                                              uint64_t Length,
                                              StringRef TrimChars) स्थिरांक {
  StringRef Bytes(getBytes(OffsetPtr, Length));
  परत Bytes.trim(TrimChars);
}

StringRef DataExtractor::getBytes(uint64_t *OffsetPtr, uint64_t Length,
                                  Error *Err) स्थिरांक {
  ErrorAsOutParameter ErrAsOut(Err);
  जर (isError(Err))
    परत StringRef();

  जर (!prepareRead(*OffsetPtr, Length, Err))
    परत StringRef();

  StringRef Result = Data.substr(*OffsetPtr, Length);
  *OffsetPtr += Length;
  परत Result;
}

ढाँचा <typename T>
स्थिर T getLEB128(StringRef Data, uint64_t *OffsetPtr, Error *Err,
                   T (&Decoder)(स्थिरांक uint8_t *p, अचिन्हित *n,
                                स्थिरांक uint8_t *end, स्थिरांक अक्षर **error)) {
  ArrayRef<uint8_t> Bytes = arrayRefFromStringRef(Data);
  प्रतिज्ञा(*OffsetPtr <= Bytes.size());
  ErrorAsOutParameter ErrAsOut(Err);
  जर (isError(Err))
    परत T();

  स्थिरांक अक्षर *error = nullptr;
  अचिन्हित bytes_read;
  T result =
      Decoder(Bytes.data() + *OffsetPtr, &bytes_read, Bytes.end(), &error);
  जर (error) {
    जर (Err)
      *Err = createStringError(errc::illegal_byte_sequence,
                               "unable to decode LEB128 at offset 0x%8.8" PRIx64
                               ": %s",
                               *OffsetPtr, error);
    परत T();
  }
  *OffsetPtr += bytes_read;
  परत result;
}

uint64_t DataExtractor::getULEB128(uint64_t *offset_ptr, Error *Err) स्थिरांक {
  परत getLEB128(Data, offset_ptr, Err, decodeULEB128);
}

int64_t DataExtractor::getSLEB128(uint64_t *offset_ptr, Error *Err) स्थिरांक {
  परत getLEB128(Data, offset_ptr, Err, decodeSLEB128);
}

APSInt DataExtractor::getSLEB128APSInt(uint64_t *OffsetPtr, Error *Err) स्थिरांक {
  ErrorAsOutParameter ErrAsOut(Err);
  जर (isError(Err))
    परत APSInt();

  uint64_t Offset = *OffsetPtr;
  APInt मूल्य;
  अचिन्हित Shift = 0;
  uint8_t Byte;
  करा {
    जर (!prepareRead(Offset, 1, Err))
      परत APSInt();
    Byte = Data[Offset++];
    APInt Slice(7, Byte & 0x7f);
    जर (Shift == 0)
      मूल्य = Slice;
    अन्यथा {
      मूल्य = मूल्य.zext(Shift + 7);
      मूल्य.घाला_Bits(Slice, Shift);
    }
    Shift += 7;
  } पर्यंत (Byte & 0x80);

  // Sign extend Value to at least 64 bits to match getSLEB128.
  जर (Shift < 64)
    मूल्य = मूल्य.sext(64);

  *OffsetPtr = Offset;
  परत APSInt(std::move(मूल्य), /*isUnsigned=*/असत्य);
}

शून्य DataExtractor::skip(Cursor &C, uint64_t Length) स्थिरांक {
  ErrorAsOutParameter ErrAsOut(C.Err);
  जर (isError(&C.Err))
    परत;

  जर (prepareRead(C.Offset, Length, &C.Err))
    C.Offset += Length;
}
