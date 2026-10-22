//===-- DataExtractor.cpp -------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/DataExtractor.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/Support/Errc.h"
#شامل "llvm/Support/ErrorHandling.h"
#شامل "llvm/Support/LEB128.h"
#شامل "llvm/Support/SwapByteOrder.h"

using فضاینام llvm;

منطقی DataExtractor::prepareRead(uint64_t Offset, uint64_t Size,
                                Error *E) ثابت {
  اگر (isValidOffsetForDataOfSize(Offset, Size))
    بازگشت درست;
  اگر (E) {
    اگر (Offset <= Data.size())
      *E = createStringError(
          errc::illegal_byte_sequence,
          "unexpected end of data at offset 0x%zx while reading [0x%" PRIx64
          ", 0x%" PRIx64 ")",
          Data.size(), Offset, Offset + Size);
    وگرنه
      *E = createStringError(errc::invalid_argument,
                             "offset 0x%" PRIx64
                             " is beyond the end of data at 0x%zx",
                             Offset, Data.size());
  }
  بازگشت نادرست;
}

ایستا منطقی isError(Error *E) { بازگشت E && *E; }

template <typename T>
T DataExtractor::getU(uint64_t *offset_ptr, Error *Err) ثابت {
  ErrorAsOutParameter ErrAsOut(Err);
  T val = 0;
  اگر (isError(Err))
    بازگشت val;

  uint64_t offset = *offset_ptr;
  اگر (!prepareRead(offset, اندازه(T), Err))
    بازگشت val;
  std::memcpy(&val, &Data.data()[offset], اندازه(val));
  اگر (sys::IsLittleEndianHost != IsLittleEndian)
    sys::swapByteOrder(val);

  // Advance the offset
  *offset_ptr += اندازه(val);
  بازگشت val;
}

template <typename T>
T *DataExtractor::getUs(uint64_t *offset_ptr, T *dst, uint32_t شمار,
                        Error *Err) ثابت {
  ErrorAsOutParameter ErrAsOut(Err);
  اگر (isError(Err))
    بازگشت nullptr;

  uint64_t offset = *offset_ptr;

  اگر (!prepareRead(offset, اندازه(*dst) * شمار, Err))
    بازگشت nullptr;
  برای (T *مقدار_ptr = dst, *end = dst + شمار; مقدار_ptr != end;
       ++مقدار_ptr, offset += اندازه(*dst))
    *مقدار_ptr = getU<T>(offset_ptr, Err);
  // Advance the offset
  *offset_ptr = offset;
  // Return a non-NULL pointer to the converted data as an indicator of
  // success
  بازگشت dst;
}

uint8_t DataExtractor::getU8(uint64_t *offset_ptr, llvm::Error *Err) ثابت {
  بازگشت getU<uint8_t>(offset_ptr, Err);
}

uint8_t *DataExtractor::getU8(uint64_t *offset_ptr, uint8_t *dst,
                              uint32_t شمار) ثابت {
  بازگشت getUs<uint8_t>(offset_ptr, dst, شمار, nullptr);
}

uint8_t *DataExtractor::getU8(Cursor &C, uint8_t *Dst, uint32_t شمار_2) ثابت {
  بازگشت getUs<uint8_t>(&C.Offset, Dst, شمار_2, &C.Err);
}

uint16_t DataExtractor::getU16(uint64_t *offset_ptr, llvm::Error *Err) ثابت {
  بازگشت getU<uint16_t>(offset_ptr, Err);
}

uint16_t *DataExtractor::getU16(uint64_t *offset_ptr, uint16_t *dst,
                                uint32_t شمار) ثابت {
  بازگشت getUs<uint16_t>(offset_ptr, dst, شمار, nullptr);
}

uint32_t DataExtractor::getU24(uint64_t *OffsetPtr, Error *Err) ثابت {
  uint24_t ExtractedVal = getU<uint24_t>(OffsetPtr, Err);
  // The 3 bytes are in the correct byte order for the host.
  بازگشت ExtractedVal.getAsUint32(sys::IsLittleEndianHost);
}

uint32_t DataExtractor::getU32(uint64_t *offset_ptr, llvm::Error *Err) ثابت {
  بازگشت getU<uint32_t>(offset_ptr, Err);
}

uint32_t *DataExtractor::getU32(uint64_t *offset_ptr, uint32_t *dst,
                                uint32_t شمار) ثابت {
  بازگشت getUs<uint32_t>(offset_ptr, dst, شمار, nullptr);
}

uint64_t DataExtractor::getU64(uint64_t *offset_ptr, llvm::Error *Err) ثابت {
  بازگشت getU<uint64_t>(offset_ptr, Err);
}

uint64_t *DataExtractor::getU64(uint64_t *offset_ptr, uint64_t *dst,
                                uint32_t شمار) ثابت {
  بازگشت getUs<uint64_t>(offset_ptr, dst, شمار, nullptr);
}

uint64_t DataExtractor::getUnsigned(uint64_t *offset_ptr, uint32_t byte_size,
                                    llvm::Error *Err) ثابت {
  انتخاب (byte_size) {
  حالت 1:
    بازگشت getU8(offset_ptr, Err);
  حالت 2:
    بازگشت getU16(offset_ptr, Err);
  حالت 4:
    بازگشت getU32(offset_ptr, Err);
  حالت 8:
    بازگشت getU64(offset_ptr, Err);
  }

  // For any other byte size, read the bytes and swap/shift if necessary.
  ErrorAsOutParameter ErrAsOut(Err);
  uint64_t val = 0;
  اگر (isError(Err))
    بازگشت val;
  uint64_t offset = *offset_ptr;
  اگر (!prepareRead(offset, byte_size, Err))
    بازگشت val;
  // Copy into the least significant bytes of val regardless of host
  // endianness.
  std::memcpy(reinterpret_cast<نویسه *>(&val) +
                  (sys::IsLittleEndianHost ? 0 : 8 - byte_size),
              &Data.data()[offset], byte_size);
  // Swap the least significant bytes of val if endianness doesn't match.
  اگر (sys::IsLittleEndianHost != IsLittleEndian)
    val = sys::getSwappedBytes(val) >> (8 * (8 - byte_size));

  *offset_ptr += byte_size;
  بازگشت val;
}

int64_t
DataExtractor::getSigned(uint64_t *offset_ptr, uint32_t byte_size) ثابت {
  انتخاب (byte_size) {
  حالت 1:
    بازگشت getS8(offset_ptr);
  حالت 2:
    بازگشت getS16(offset_ptr);
  حالت 4:
    بازگشت getS32(offset_ptr);
  حالت 8:
    بازگشت getS64(offset_ptr);
  }
  llvm_unreachable("getSigned unhandled case!");
}

StringRef DataExtractor::getCStrRef(uint64_t *OffsetPtr, Error *Err) ثابت {
  ErrorAsOutParameter ErrAsOut(Err);
  اگر (isError(Err))
    بازگشت StringRef();

  uint64_t Start = *OffsetPtr;
  StringRef::size_type Pos = Data.find('\0', Start);
  اگر (Pos != StringRef::npos) {
    *OffsetPtr = Pos + 1;
    بازگشت StringRef(Data.data() + Start, Pos - Start);
  }
  اگر (Err)
    *Err = createStringError(errc::illegal_byte_sequence,
                             "no null terminated string at offset 0x%" PRIx64,
                             Start);
  بازگشت StringRef();
}

StringRef DataExtractor::getFixedLengthString(uint64_t *OffsetPtr,
                                              uint64_t Length,
                                              StringRef TrimChars) ثابت {
  StringRef Bytes(getBytes(OffsetPtr, Length));
  بازگشت Bytes.trim(TrimChars);
}

StringRef DataExtractor::getBytes(uint64_t *OffsetPtr, uint64_t Length,
                                  Error *Err) ثابت {
  ErrorAsOutParameter ErrAsOut(Err);
  اگر (isError(Err))
    بازگشت StringRef();

  اگر (!prepareRead(*OffsetPtr, Length, Err))
    بازگشت StringRef();

  StringRef Result = Data.substr(*OffsetPtr, Length);
  *OffsetPtr += Length;
  بازگشت Result;
}

template <typename T>
ایستا T getLEB128(StringRef Data, uint64_t *OffsetPtr, Error *Err,
                   T (&Decoder)(ثابت uint8_t *p, unsigned *n,
                                ثابت uint8_t *end, ثابت نویسه **error)) {
  ArrayRef<uint8_t> Bytes = arrayRefFromStringRef(Data);
  أكد(*OffsetPtr <= Bytes.size());
  ErrorAsOutParameter ErrAsOut(Err);
  اگر (isError(Err))
    بازگشت T();

  ثابت نویسه *error = nullptr;
  unsigned bytes_read;
  T result =
      Decoder(Bytes.data() + *OffsetPtr, &bytes_read, Bytes.end(), &error);
  اگر (error) {
    اگر (Err)
      *Err = createStringError(errc::illegal_byte_sequence,
                               "unable to decode LEB128 at offset 0x%8.8" PRIx64
                               ": %s",
                               *OffsetPtr, error);
    بازگشت T();
  }
  *OffsetPtr += bytes_read;
  بازگشت result;
}

uint64_t DataExtractor::getULEB128(uint64_t *offset_ptr, Error *Err) ثابت {
  بازگشت getLEB128(Data, offset_ptr, Err, decodeULEB128);
}

int64_t DataExtractor::getSLEB128(uint64_t *offset_ptr, Error *Err) ثابت {
  بازگشت getLEB128(Data, offset_ptr, Err, decodeSLEB128);
}

APSInt DataExtractor::getSLEB128APSInt(uint64_t *OffsetPtr, Error *Err) ثابت {
  ErrorAsOutParameter ErrAsOut(Err);
  اگر (isError(Err))
    بازگشت APSInt();

  uint64_t Offset = *OffsetPtr;
  APInt مقدار;
  unsigned Shift = 0;
  uint8_t Byte;
  انجامبده {
    اگر (!prepareRead(Offset, 1, Err))
      بازگشت APSInt();
    Byte = Data[Offset++];
    APInt Slice(7, Byte & 0x7f);
    اگر (Shift == 0)
      مقدار = Slice;
    وگرنه {
      مقدار = مقدار.zext(Shift + 7);
      مقدار.درج_Bits(Slice, Shift);
    }
    Shift += 7;
  } وقتی (Byte & 0x80);

  // Sign extend Value to at least 64 bits to match getSLEB128.
  اگر (Shift < 64)
    مقدار = مقدار.sext(64);

  *OffsetPtr = Offset;
  بازگشت APSInt(std::move(مقدار), /*isUnsigned=*/نادرست);
}

تهی DataExtractor::skip(Cursor &C, uint64_t Length) ثابت {
  ErrorAsOutParameter ErrAsOut(C.Err);
  اگر (isError(&C.Err))
    بازگشت;

  اگر (prepareRead(C.Offset, Length, &C.Err))
    C.Offset += Length;
}
