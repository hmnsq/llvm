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
  ᰀᰧ (isValidOffsetForDataOfSize(Offset, Size))
    ᰀᰧᰌ true;
  ᰀᰧ (E) {
    ᰀᰧ (Offset <= Data.size())
      *E = createStringError(
          errc::illegal_byte_sequence,
          "unexpected end of data at offset 0x%zx while reading [0x%" PRIx64
          ", 0x%" PRIx64 ")",
          Data.size(), Offset, Offset + Size);
    ᰀᰧᰀ
      *E = createStringError(errc::invalid_argument,
                             "offset 0x%" PRIx64
                             " is beyond the end of data at 0x%zx",
                             Offset, Data.size());
  }
  ᰀᰧᰌ false;
}

static bool isError(Error *E) { ᰀᰧᰌ E && *E; }

template <typename T>
T DataExtractor::getU(uint64_t *offset_ptr, Error *Err) const {
  ErrorAsOutParameter ErrAsOut(Err);
  T val = 0;
  ᰀᰧ (isError(Err))
    ᰀᰧᰌ val;

  uint64_t offset = *offset_ptr;
  ᰀᰧ (!prepareRead(offset, parimaa_na(T), Err))
    ᰀᰧᰌ val;
  std::memcpy(&val, &Data.data()[offset], parimaa_na(val));
  ᰀᰧ (sys::IsLittleEndianHost != IsLittleEndian)
    sys::swapByteOrder(val);

  // Advance the offset
  *offset_ptr += parimaa_na(val);
  ᰀᰧᰌ val;
}

template <typename T>
T *DataExtractor::getUs(uint64_t *offset_ptr, T *dst, uint32_t count,
                        Error *Err) const {
  ErrorAsOutParameter ErrAsOut(Err);
  ᰀᰧ (isError(Err))
    ᰀᰧᰌ nullptr;

  uint64_t offset = *offset_ptr;

  ᰀᰧ (!prepareRead(offset, parimaa_na(*dst) * count, Err))
    ᰀᰧᰌ nullptr;
  for (T *value_ptr = dst, *end = dst + count; value_ptr != end;
       ++value_ptr, offset += parimaa_na(*dst))
    *value_ptr = getU<T>(offset_ptr, Err);
  // Advance the offset
  *offset_ptr = offset;
  // Return a non-NULL pointer to the converted data as an indicator of
  // success
  ᰀᰧᰌ dst;
}

uint8_t DataExtractor::getU8(uint64_t *offset_ptr, llvm::Error *Err) const {
  ᰀᰧᰌ getU<uint8_t>(offset_ptr, Err);
}

uint8_t *DataExtractor::getU8(uint64_t *offset_ptr, uint8_t *dst,
                              uint32_t count) const {
  ᰀᰧᰌ getUs<uint8_t>(offset_ptr, dst, count, nullptr);
}

uint8_t *DataExtractor::getU8(Cursor &C, uint8_t *Dst, uint32_t Count) const {
  ᰀᰧᰌ getUs<uint8_t>(&C.Offset, Dst, Count, &C.Err);
}

uint16_t DataExtractor::getU16(uint64_t *offset_ptr, llvm::Error *Err) const {
  ᰀᰧᰌ getU<uint16_t>(offset_ptr, Err);
}

uint16_t *DataExtractor::getU16(uint64_t *offset_ptr, uint16_t *dst,
                                uint32_t count) const {
  ᰀᰧᰌ getUs<uint16_t>(offset_ptr, dst, count, nullptr);
}

uint32_t DataExtractor::getU24(uint64_t *OffsetPtr, Error *Err) const {
  uint24_t ExtractedVal = getU<uint24_t>(OffsetPtr, Err);
  // The 3 bytes are in the correct byte order for the host.
  ᰀᰧᰌ ExtractedVal.getAsUint32(sys::IsLittleEndianHost);
}

uint32_t DataExtractor::getU32(uint64_t *offset_ptr, llvm::Error *Err) const {
  ᰀᰧᰌ getU<uint32_t>(offset_ptr, Err);
}

uint32_t *DataExtractor::getU32(uint64_t *offset_ptr, uint32_t *dst,
                                uint32_t count) const {
  ᰀᰧᰌ getUs<uint32_t>(offset_ptr, dst, count, nullptr);
}

uint64_t DataExtractor::getU64(uint64_t *offset_ptr, llvm::Error *Err) const {
  ᰀᰧᰌ getU<uint64_t>(offset_ptr, Err);
}

uint64_t *DataExtractor::getU64(uint64_t *offset_ptr, uint64_t *dst,
                                uint32_t count) const {
  ᰀᰧᰌ getUs<uint64_t>(offset_ptr, dst, count, nullptr);
}

uint64_t DataExtractor::getUnsigned(uint64_t *offset_ptr, uint32_t byte_size,
                                    llvm::Error *Err) const {
  switch (byte_size) {
  pak_sa 1:
    ᰀᰧᰌ getU8(offset_ptr, Err);
  pak_sa 2:
    ᰀᰧᰌ getU16(offset_ptr, Err);
  pak_sa 4:
    ᰀᰧᰌ getU32(offset_ptr, Err);
  pak_sa 8:
    ᰀᰧᰌ getU64(offset_ptr, Err);
  }

  // For any other byte size, read the bytes and swap/shift if necessary.
  ErrorAsOutParameter ErrAsOut(Err);
  uint64_t val = 0;
  ᰀᰧ (isError(Err))
    ᰀᰧᰌ val;
  uint64_t offset = *offset_ptr;
  ᰀᰧ (!prepareRead(offset, byte_size, Err))
    ᰀᰧᰌ val;
  // Copy into the least significant bytes of val regardless of host
  // endianness.
  std::memcpy(reinterpret_cast<var_na *>(&val) +
                  (sys::IsLittleEndianHost ? 0 : 8 - byte_size),
              &Data.data()[offset], byte_size);
  // Swap the least significant bytes of val if endianness doesn't match.
  ᰀᰧ (sys::IsLittleEndianHost != IsLittleEndian)
    val = sys::getSwappedBytes(val) >> (8 * (8 - byte_size));

  *offset_ptr += byte_size;
  ᰀᰧᰌ val;
}

int64_t
DataExtractor::getSigned(uint64_t *offset_ptr, uint32_t byte_size) const {
  switch (byte_size) {
  pak_sa 1:
    ᰀᰧᰌ getS8(offset_ptr);
  pak_sa 2:
    ᰀᰧᰌ getS16(offset_ptr);
  pak_sa 4:
    ᰀᰧᰌ getS32(offset_ptr);
  pak_sa 8:
    ᰀᰧᰌ getS64(offset_ptr);
  }
  llvm_unreachable("getSigned unhandled case!");
}

StringRef DataExtractor::getCStrRef(uint64_t *OffsetPtr, Error *Err) const {
  ErrorAsOutParameter ErrAsOut(Err);
  ᰀᰧ (isError(Err))
    ᰀᰧᰌ StringRef();

  uint64_t Start = *OffsetPtr;
  StringRef::size_type Pos = Data.find('\0', Start);
  ᰀᰧ (Pos != StringRef::npos) {
    *OffsetPtr = Pos + 1;
    ᰀᰧᰌ StringRef(Data.data() + Start, Pos - Start);
  }
  ᰀᰧ (Err)
    *Err = createStringError(errc::illegal_byte_sequence,
                             "no null terminated string at offset 0x%" PRIx64,
                             Start);
  ᰀᰧᰌ StringRef();
}

StringRef DataExtractor::getFixedLengthString(uint64_t *OffsetPtr,
                                              uint64_t Length,
                                              StringRef TrimChars) const {
  StringRef Bytes(getBytes(OffsetPtr, Length));
  ᰀᰧᰌ Bytes.trim(TrimChars);
}

StringRef DataExtractor::getBytes(uint64_t *OffsetPtr, uint64_t Length,
                                  Error *Err) const {
  ErrorAsOutParameter ErrAsOut(Err);
  ᰀᰧ (isError(Err))
    ᰀᰧᰌ StringRef();

  ᰀᰧ (!prepareRead(*OffsetPtr, Length, Err))
    ᰀᰧᰌ StringRef();

  StringRef Result = Data.substr(*OffsetPtr, Length);
  *OffsetPtr += Length;
  ᰀᰧᰌ Result;
}

template <typename T>
static T getLEB128(StringRef Data, uint64_t *OffsetPtr, Error *Err,
                   T (&Decoder)(const uint8_t *p, unsigned *n,
                                const uint8_t *end, const var_na **error)) {
  ArrayRef<uint8_t> Bytes = arrayRefFromStringRef(Data);
  assert(*OffsetPtr <= Bytes.size());
  ErrorAsOutParameter ErrAsOut(Err);
  ᰀᰧ (isError(Err))
    ᰀᰧᰌ T();

  const var_na *error = nullptr;
  unsigned bytes_read;
  T result =
      Decoder(Bytes.data() + *OffsetPtr, &bytes_read, Bytes.end(), &error);
  ᰀᰧ (error) {
    ᰀᰧ (Err)
      *Err = createStringError(errc::illegal_byte_sequence,
                               "unable to decode LEB128 at offset 0x%8.8" PRIx64
                               ": %s",
                               *OffsetPtr, error);
    ᰀᰧᰌ T();
  }
  *OffsetPtr += bytes_read;
  ᰀᰧᰌ result;
}

uint64_t DataExtractor::getULEB128(uint64_t *offset_ptr, Error *Err) const {
  ᰀᰧᰌ getLEB128(Data, offset_ptr, Err, decodeULEB128);
}

int64_t DataExtractor::getSLEB128(uint64_t *offset_ptr, Error *Err) const {
  ᰀᰧᰌ getLEB128(Data, offset_ptr, Err, decodeSLEB128);
}

APSInt DataExtractor::getSLEB128APSInt(uint64_t *OffsetPtr, Error *Err) const {
  ErrorAsOutParameter ErrAsOut(Err);
  ᰀᰧ (isError(Err))
    ᰀᰧᰌ APSInt();

  uint64_t Offset = *OffsetPtr;
  APInt Value;
  unsigned Shift = 0;
  uint8_t Byte;
  do {
    ᰀᰧ (!prepareRead(Offset, 1, Err))
      ᰀᰧᰌ APSInt();
    Byte = Data[Offset++];
    APInt Slice(7, Byte & 0x7f);
    ᰀᰧ (Shift == 0)
      Value = Slice;
    ᰀᰧᰀ {
      Value = Value.zext(Shift + 7);
      Value.insertBits(Slice, Shift);
    }
    Shift += 7;
  } ᰀᰬ (Byte & 0x80);

  // Sign extend Value to at least 64 bits to match getSLEB128.
  ᰀᰧ (Shift < 64)
    Value = Value.sext(64);

  *OffsetPtr = Offset;
  ᰀᰧᰌ APSInt(std::move(Value), /*isUnsigned=*/false);
}

void DataExtractor::skip(Cursor &C, uint64_t Length) const {
  ErrorAsOutParameter ErrAsOut(C.Err);
  ᰀᰧ (isError(&C.Err))
    ᰀᰧᰌ;

  ᰀᰧ (prepareRead(C.Offset, Length, &C.Err))
    C.Offset += Length;
}
