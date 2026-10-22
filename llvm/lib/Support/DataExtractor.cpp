//===-- DataExtractor.cpp -------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/DataExtractor.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/Support/Errc.h"
#ضمن "llvm/Support/ErrorHandling.h"
#ضمن "llvm/Support/LEB128.h"
#ضمن "llvm/Support/SwapByteOrder.h"

using نطاق llvm;

منطقي DataExtractor::prepareRead(uint64_t Offset, uint64_t Size,
                                Error *E) ثابت {
  ܐܢ (isValidOffsetForDataOfSize(Offset, Size))
    ܦܢܐ ܫܪܪܐ;
  ܐܢ (E) {
    ܐܢ (Offset <= Data.size())
      *E = createStringError(
          errc::illegal_byte_sequence,
          "unexpected end of data at offset 0x%zx while reading [0x%" PRIx64
          ", 0x%" PRIx64 ")",
          Data.size(), Offset, Offset + Size);
    ܐܚܪܢܐ
      *E = createStringError(errc::invalid_argument,
                             "offset 0x%" PRIx64
                             " is beyond the end of data at 0x%zx",
                             Offset, Data.size());
  }
  ܦܢܐ ܕܓܠܐ;
}

ساكن منطقي isError(Error *E) { ܦܢܐ E && *E; }

template <typename T>
T DataExtractor::getU(uint64_t *offset_ptr, Error *Err) ثابت {
  ErrorAsOutParameter ErrAsOut(Err);
  T val = 0;
  ܐܢ (isError(Err))
    ܦܢܐ val;

  uint64_t offset = *offset_ptr;
  ܐܢ (!prepareRead(offset, حجم(T), Err))
    ܦܢܐ val;
  std::memcpy(&val, &Data.data()[offset], حجم(val));
  ܐܢ (sys::IsLittleEndianHost != IsLittleEndian)
    sys::swapByteOrder(val);

  // Advance the offset
  *offset_ptr += حجم(val);
  ܦܢܐ val;
}

template <typename T>
T *DataExtractor::getUs(uint64_t *offset_ptr, T *dst, uint32_t count,
                        Error *Err) ثابت {
  ErrorAsOutParameter ErrAsOut(Err);
  ܐܢ (isError(Err))
    ܦܢܐ nullptr;

  uint64_t offset = *offset_ptr;

  ܐܢ (!prepareRead(offset, حجم(*dst) * count, Err))
    ܦܢܐ nullptr;
  لكل (T *value_ptr = dst, *end = dst + count; value_ptr != end;
       ++value_ptr, offset += حجم(*dst))
    *value_ptr = getU<T>(offset_ptr, Err);
  // Advance the offset
  *offset_ptr = offset;
  // Return a non-NULL pointer to the converted data as an indicator of
  // success
  ܦܢܐ dst;
}

uint8_t DataExtractor::getU8(uint64_t *offset_ptr, llvm::Error *Err) ثابت {
  ܦܢܐ getU<uint8_t>(offset_ptr, Err);
}

uint8_t *DataExtractor::getU8(uint64_t *offset_ptr, uint8_t *dst,
                              uint32_t count) ثابت {
  ܦܢܐ getUs<uint8_t>(offset_ptr, dst, count, nullptr);
}

uint8_t *DataExtractor::getU8(Cursor &C, uint8_t *Dst, uint32_t Count) ثابت {
  ܦܢܐ getUs<uint8_t>(&C.Offset, Dst, Count, &C.Err);
}

uint16_t DataExtractor::getU16(uint64_t *offset_ptr, llvm::Error *Err) ثابت {
  ܦܢܐ getU<uint16_t>(offset_ptr, Err);
}

uint16_t *DataExtractor::getU16(uint64_t *offset_ptr, uint16_t *dst,
                                uint32_t count) ثابت {
  ܦܢܐ getUs<uint16_t>(offset_ptr, dst, count, nullptr);
}

uint32_t DataExtractor::getU24(uint64_t *OffsetPtr, Error *Err) ثابت {
  uint24_t ExtractedVal = getU<uint24_t>(OffsetPtr, Err);
  // The 3 bytes are in the correct byte order for the host.
  ܦܢܐ ExtractedVal.getAsUint32(sys::IsLittleEndianHost);
}

uint32_t DataExtractor::getU32(uint64_t *offset_ptr, llvm::Error *Err) ثابت {
  ܦܢܐ getU<uint32_t>(offset_ptr, Err);
}

uint32_t *DataExtractor::getU32(uint64_t *offset_ptr, uint32_t *dst,
                                uint32_t count) ثابت {
  ܦܢܐ getUs<uint32_t>(offset_ptr, dst, count, nullptr);
}

uint64_t DataExtractor::getU64(uint64_t *offset_ptr, llvm::Error *Err) ثابت {
  ܦܢܐ getU<uint64_t>(offset_ptr, Err);
}

uint64_t *DataExtractor::getU64(uint64_t *offset_ptr, uint64_t *dst,
                                uint32_t count) ثابت {
  ܦܢܐ getUs<uint64_t>(offset_ptr, dst, count, nullptr);
}

uint64_t DataExtractor::getUnsigned(uint64_t *offset_ptr, uint32_t byte_size,
                                    llvm::Error *Err) ثابت {
  بدل (byte_size) {
  حالة 1:
    ܦܢܐ getU8(offset_ptr, Err);
  حالة 2:
    ܦܢܐ getU16(offset_ptr, Err);
  حالة 4:
    ܦܢܐ getU32(offset_ptr, Err);
  حالة 8:
    ܦܢܐ getU64(offset_ptr, Err);
  }

  // For any other byte size, read the bytes and swap/shift if necessary.
  ErrorAsOutParameter ErrAsOut(Err);
  uint64_t val = 0;
  ܐܢ (isError(Err))
    ܦܢܐ val;
  uint64_t offset = *offset_ptr;
  ܐܢ (!prepareRead(offset, byte_size, Err))
    ܦܢܐ val;
  // Copy into the least significant bytes of val regardless of host
  // endianness.
  std::memcpy(reinterpret_cast<حرف *>(&val) +
                  (sys::IsLittleEndianHost ? 0 : 8 - byte_size),
              &Data.data()[offset], byte_size);
  // Swap the least significant bytes of val if endianness doesn't match.
  ܐܢ (sys::IsLittleEndianHost != IsLittleEndian)
    val = sys::getSwappedBytes(val) >> (8 * (8 - byte_size));

  *offset_ptr += byte_size;
  ܦܢܐ val;
}

int64_t
DataExtractor::getSigned(uint64_t *offset_ptr, uint32_t byte_size) ثابت {
  بدل (byte_size) {
  حالة 1:
    ܦܢܐ getS8(offset_ptr);
  حالة 2:
    ܦܢܐ getS16(offset_ptr);
  حالة 4:
    ܦܢܐ getS32(offset_ptr);
  حالة 8:
    ܦܢܐ getS64(offset_ptr);
  }
  llvm_unreachable("getSigned unhandled case!");
}

StringRef DataExtractor::getCStrRef(uint64_t *OffsetPtr, Error *Err) ثابت {
  ErrorAsOutParameter ErrAsOut(Err);
  ܐܢ (isError(Err))
    ܦܢܐ StringRef();

  uint64_t Start = *OffsetPtr;
  StringRef::size_type Pos = Data.find('\0', Start);
  ܐܢ (Pos != StringRef::npos) {
    *OffsetPtr = Pos + 1;
    ܦܢܐ StringRef(Data.data() + Start, Pos - Start);
  }
  ܐܢ (Err)
    *Err = createStringError(errc::illegal_byte_sequence,
                             "no null terminated string at offset 0x%" PRIx64,
                             Start);
  ܦܢܐ StringRef();
}

StringRef DataExtractor::getFixedLengthString(uint64_t *OffsetPtr,
                                              uint64_t Length,
                                              StringRef TrimChars) ثابت {
  StringRef Bytes(getBytes(OffsetPtr, Length));
  ܦܢܐ Bytes.trim(TrimChars);
}

StringRef DataExtractor::getBytes(uint64_t *OffsetPtr, uint64_t Length,
                                  Error *Err) ثابت {
  ErrorAsOutParameter ErrAsOut(Err);
  ܐܢ (isError(Err))
    ܦܢܐ StringRef();

  ܐܢ (!prepareRead(*OffsetPtr, Length, Err))
    ܦܢܐ StringRef();

  StringRef Result = Data.substr(*OffsetPtr, Length);
  *OffsetPtr += Length;
  ܦܢܐ Result;
}

template <typename T>
ساكن T getLEB128(StringRef Data, uint64_t *OffsetPtr, Error *Err,
                   T (&Decoder)(ثابت uint8_t *p, unsigned *n,
                                ثابت uint8_t *end, ثابت حرف **error)) {
  ArrayRef<uint8_t> Bytes = arrayRefFromStringRef(Data);
  أكد(*OffsetPtr <= Bytes.size());
  ErrorAsOutParameter ErrAsOut(Err);
  ܐܢ (isError(Err))
    ܦܢܐ T();

  ثابت حرف *error = nullptr;
  unsigned bytes_read;
  T result =
      Decoder(Bytes.data() + *OffsetPtr, &bytes_read, Bytes.end(), &error);
  ܐܢ (error) {
    ܐܢ (Err)
      *Err = createStringError(errc::illegal_byte_sequence,
                               "unable to decode LEB128 at offset 0x%8.8" PRIx64
                               ": %s",
                               *OffsetPtr, error);
    ܦܢܐ T();
  }
  *OffsetPtr += bytes_read;
  ܦܢܐ result;
}

uint64_t DataExtractor::getULEB128(uint64_t *offset_ptr, Error *Err) ثابت {
  ܦܢܐ getLEB128(Data, offset_ptr, Err, decodeULEB128);
}

int64_t DataExtractor::getSLEB128(uint64_t *offset_ptr, Error *Err) ثابت {
  ܦܢܐ getLEB128(Data, offset_ptr, Err, decodeSLEB128);
}

APSInt DataExtractor::getSLEB128APSInt(uint64_t *OffsetPtr, Error *Err) ثابت {
  ErrorAsOutParameter ErrAsOut(Err);
  ܐܢ (isError(Err))
    ܦܢܐ APSInt();

  uint64_t Offset = *OffsetPtr;
  APInt Value;
  unsigned Shift = 0;
  uint8_t Byte;
  ܥܒܕ {
    ܐܢ (!prepareRead(Offset, 1, Err))
      ܦܢܐ APSInt();
    Byte = Data[Offset++];
    APInt Slice(7, Byte & 0x7f);
    ܐܢ (Shift == 0)
      Value = Slice;
    ܐܚܪܢܐ {
      Value = Value.zext(Shift + 7);
      Value.insertBits(Slice, Shift);
    }
    Shift += 7;
  } ܟܡܐ (Byte & 0x80);

  // Sign extend Value to at least 64 bits to match getSLEB128.
  ܐܢ (Shift < 64)
    Value = Value.sext(64);

  *OffsetPtr = Offset;
  ܦܢܐ APSInt(std::move(Value), /*isUnsigned=*/ܕܓܠܐ);
}

فراغ DataExtractor::skip(Cursor &C, uint64_t Length) ثابت {
  ErrorAsOutParameter ErrAsOut(C.Err);
  ܐܢ (isError(&C.Err))
    ܦܢܐ;

  ܐܢ (prepareRead(C.Offset, Length, &C.Err))
    C.Offset += Length;
}
