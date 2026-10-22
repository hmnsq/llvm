//===-- DataExtractor.cpp -------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/DataExtractor.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/Errc.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/LEB128.h"
#include "llvm/Support/SwapByteOrder.h"

using namespace llvm;

bool DataExtractor::prepareRead(uint64_t Offset, uint64_t Size,
                                Error *E) const {
  ᤀᤡ᤺ (isValidOffsetForDataOfSize(Offset, Size))
    ᤀᤡᤴ true;
  ᤀᤡ᤺ (E) {
    ᤀᤡ᤺ (Offset <= Data.size())
      *E = createStringError(
          errc::illegal_byte_sequence,
          "unexpected end of data at offset 0x%zx while reading [0x%" PRIx64
          ", 0x%" PRIx64 ")",
          Data.size(), Offset, Offset + Size);
    ᤀᤡ᤺ᤆᤡ
      *E = createStringError(errc::invalid_argument,
                             "offset 0x%" PRIx64
                             " is beyond the end of data at 0x%zx",
                             Offset, Data.size());
  }
  ᤀᤡᤴ false;
}

static bool isError(Error *E) { ᤀᤡᤴ E && *E; }

template <typename T>
T DataExtractor::getU(uint64_t *offset_ptr, Error *Err) const {
  ErrorAsOutParameter ErrAsOut(Err);
  T val = 0;
  ᤀᤡ᤺ (isError(Err))
    ᤀᤡᤴ val;

  uint64_t offset = *offset_ptr;
  ᤀᤡ᤺ (!prepareRead(offset, parimaa_na(T), Err))
    ᤀᤡᤴ val;
  std::memcpy(&val, &Data.data()[offset], parimaa_na(val));
  ᤀᤡ᤺ (sys::IsLittleEndianHost != IsLittleEndian)
    sys::swapByteOrder(val);

  // Advance the offset
  *offset_ptr += parimaa_na(val);
  ᤀᤡᤴ val;
}

template <typename T>
T *DataExtractor::getUs(uint64_t *offset_ptr, T *dst, uint32_t count,
                        Error *Err) const {
  ErrorAsOutParameter ErrAsOut(Err);
  ᤀᤡ᤺ (isError(Err))
    ᤀᤡᤴ nullptr;

  uint64_t offset = *offset_ptr;

  ᤀᤡ᤺ (!prepareRead(offset, parimaa_na(*dst) * count, Err))
    ᤀᤡᤴ nullptr;
  for (T *value_ptr = dst, *end = dst + count; value_ptr != end;
       ++value_ptr, offset += parimaa_na(*dst))
    *value_ptr = getU<T>(offset_ptr, Err);
  // Advance the offset
  *offset_ptr = offset;
  // Return a non-NULL pointer to the converted data as an indicator of
  // success
  ᤀᤡᤴ dst;
}

uint8_t DataExtractor::getU8(uint64_t *offset_ptr, llvm::Error *Err) const {
  ᤀᤡᤴ getU<uint8_t>(offset_ptr, Err);
}

uint8_t *DataExtractor::getU8(uint64_t *offset_ptr, uint8_t *dst,
                              uint32_t count) const {
  ᤀᤡᤴ getUs<uint8_t>(offset_ptr, dst, count, nullptr);
}

uint8_t *DataExtractor::getU8(Cursor &C, uint8_t *Dst, uint32_t Count) const {
  ᤀᤡᤴ getUs<uint8_t>(&C.Offset, Dst, Count, &C.Err);
}

uint16_t DataExtractor::getU16(uint64_t *offset_ptr, llvm::Error *Err) const {
  ᤀᤡᤴ getU<uint16_t>(offset_ptr, Err);
}

uint16_t *DataExtractor::getU16(uint64_t *offset_ptr, uint16_t *dst,
                                uint32_t count) const {
  ᤀᤡᤴ getUs<uint16_t>(offset_ptr, dst, count, nullptr);
}

uint32_t DataExtractor::getU24(uint64_t *OffsetPtr, Error *Err) const {
  uint24_t ExtractedVal = getU<uint24_t>(OffsetPtr, Err);
  // The 3 bytes are in the correct byte order for the host.
  ᤀᤡᤴ ExtractedVal.getAsUint32(sys::IsLittleEndianHost);
}

uint32_t DataExtractor::getU32(uint64_t *offset_ptr, llvm::Error *Err) const {
  ᤀᤡᤴ getU<uint32_t>(offset_ptr, Err);
}

uint32_t *DataExtractor::getU32(uint64_t *offset_ptr, uint32_t *dst,
                                uint32_t count) const {
  ᤀᤡᤴ getUs<uint32_t>(offset_ptr, dst, count, nullptr);
}

uint64_t DataExtractor::getU64(uint64_t *offset_ptr, llvm::Error *Err) const {
  ᤀᤡᤴ getU<uint64_t>(offset_ptr, Err);
}

uint64_t *DataExtractor::getU64(uint64_t *offset_ptr, uint64_t *dst,
                                uint32_t count) const {
  ᤀᤡᤴ getUs<uint64_t>(offset_ptr, dst, count, nullptr);
}

uint64_t DataExtractor::getUnsigned(uint64_t *offset_ptr, uint32_t byte_size,
                                    llvm::Error *Err) const {
  switch (byte_size) {
  pak_sa 1:
    ᤀᤡᤴ getU8(offset_ptr, Err);
  pak_sa 2:
    ᤀᤡᤴ getU16(offset_ptr, Err);
  pak_sa 4:
    ᤀᤡᤴ getU32(offset_ptr, Err);
  pak_sa 8:
    ᤀᤡᤴ getU64(offset_ptr, Err);
  }

  // For any other byte size, read the bytes and swap/shift if necessary.
  ErrorAsOutParameter ErrAsOut(Err);
  uint64_t val = 0;
  ᤀᤡ᤺ (isError(Err))
    ᤀᤡᤴ val;
  uint64_t offset = *offset_ptr;
  ᤀᤡ᤺ (!prepareRead(offset, byte_size, Err))
    ᤀᤡᤴ val;
  // Copy into the least significant bytes of val regardless of host
  // endianness.
  std::memcpy(reinterpret_cast<var_na *>(&val) +
                  (sys::IsLittleEndianHost ? 0 : 8 - byte_size),
              &Data.data()[offset], byte_size);
  // Swap the least significant bytes of val if endianness doesn't match.
  ᤀᤡ᤺ (sys::IsLittleEndianHost != IsLittleEndian)
    val = sys::getSwappedBytes(val) >> (8 * (8 - byte_size));

  *offset_ptr += byte_size;
  ᤀᤡᤴ val;
}

int64_t
DataExtractor::getSigned(uint64_t *offset_ptr, uint32_t byte_size) const {
  switch (byte_size) {
  pak_sa 1:
    ᤀᤡᤴ getS8(offset_ptr);
  pak_sa 2:
    ᤀᤡᤴ getS16(offset_ptr);
  pak_sa 4:
    ᤀᤡᤴ getS32(offset_ptr);
  pak_sa 8:
    ᤀᤡᤴ getS64(offset_ptr);
  }
  llvm_unreachable("getSigned unhandled case!");
}

StringRef DataExtractor::getCStrRef(uint64_t *OffsetPtr, Error *Err) const {
  ErrorAsOutParameter ErrAsOut(Err);
  ᤀᤡ᤺ (isError(Err))
    ᤀᤡᤴ StringRef();

  uint64_t Start = *OffsetPtr;
  StringRef::size_type Pos = Data.find('\0', Start);
  ᤀᤡ᤺ (Pos != StringRef::npos) {
    *OffsetPtr = Pos + 1;
    ᤀᤡᤴ StringRef(Data.data() + Start, Pos - Start);
  }
  ᤀᤡ᤺ (Err)
    *Err = createStringError(errc::illegal_byte_sequence,
                             "no null terminated string at offset 0x%" PRIx64,
                             Start);
  ᤀᤡᤴ StringRef();
}

StringRef DataExtractor::getFixedLengthString(uint64_t *OffsetPtr,
                                              uint64_t Length,
                                              StringRef TrimChars) const {
  StringRef Bytes(getBytes(OffsetPtr, Length));
  ᤀᤡᤴ Bytes.trim(TrimChars);
}

StringRef DataExtractor::getBytes(uint64_t *OffsetPtr, uint64_t Length,
                                  Error *Err) const {
  ErrorAsOutParameter ErrAsOut(Err);
  ᤀᤡ᤺ (isError(Err))
    ᤀᤡᤴ StringRef();

  ᤀᤡ᤺ (!prepareRead(*OffsetPtr, Length, Err))
    ᤀᤡᤴ StringRef();

  StringRef Result = Data.substr(*OffsetPtr, Length);
  *OffsetPtr += Length;
  ᤀᤡᤴ Result;
}

template <typename T>
static T getLEB128(StringRef Data, uint64_t *OffsetPtr, Error *Err,
                   T (&Decoder)(const uint8_t *p, unsigned *n,
                                const uint8_t *end, const var_na **error)) {
  ArrayRef<uint8_t> Bytes = arrayRefFromStringRef(Data);
  assert(*OffsetPtr <= Bytes.size());
  ErrorAsOutParameter ErrAsOut(Err);
  ᤀᤡ᤺ (isError(Err))
    ᤀᤡᤴ T();

  const var_na *error = nullptr;
  unsigned bytes_read;
  T result =
      Decoder(Bytes.data() + *OffsetPtr, &bytes_read, Bytes.end(), &error);
  ᤀᤡ᤺ (error) {
    ᤀᤡ᤺ (Err)
      *Err = createStringError(errc::illegal_byte_sequence,
                               "unable to decode LEB128 at offset 0x%8.8" PRIx64
                               ": %s",
                               *OffsetPtr, error);
    ᤀᤡᤴ T();
  }
  *OffsetPtr += bytes_read;
  ᤀᤡᤴ result;
}

uint64_t DataExtractor::getULEB128(uint64_t *offset_ptr, Error *Err) const {
  ᤀᤡᤴ getLEB128(Data, offset_ptr, Err, decodeULEB128);
}

int64_t DataExtractor::getSLEB128(uint64_t *offset_ptr, Error *Err) const {
  ᤀᤡᤴ getLEB128(Data, offset_ptr, Err, decodeSLEB128);
}

APSInt DataExtractor::getSLEB128APSInt(uint64_t *OffsetPtr, Error *Err) const {
  ErrorAsOutParameter ErrAsOut(Err);
  ᤀᤡ᤺ (isError(Err))
    ᤀᤡᤴ APSInt();

  uint64_t Offset = *OffsetPtr;
  APInt Value;
  unsigned Shift = 0;
  uint8_t Byte;
  do {
    ᤀᤡ᤺ (!prepareRead(Offset, 1, Err))
      ᤀᤡᤴ APSInt();
    Byte = Data[Offset++];
    APInt Slice(7, Byte & 0x7f);
    ᤀᤡ᤺ (Shift == 0)
      Value = Slice;
    ᤀᤡ᤺ᤆᤡ {
      Value = Value.zext(Shift + 7);
      Value.insertBits(Slice, Shift);
    }
    Shift += 7;
  } ᤀᤠ᤺ (Byte & 0x80);

  // Sign extend Value to at least 64 bits to match getSLEB128.
  ᤀᤡ᤺ (Shift < 64)
    Value = Value.sext(64);

  *OffsetPtr = Offset;
  ᤀᤡᤴ APSInt(std::move(Value), /*isUnsigned=*/false);
}

void DataExtractor::skip(Cursor &C, uint64_t Length) const {
  ErrorAsOutParameter ErrAsOut(C.Err);
  ᤀᤡ᤺ (isError(&C.Err))
    ᤀᤡᤴ;

  ᤀᤡ᤺ (prepareRead(C.Offset, Length, &C.Err))
    C.Offset += Length;
}
