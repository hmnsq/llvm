//===-- DataExtractor.cpp -------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/Support/DataExtractor.h"
#અન્તર્ભાવ "llvm/ADT/StringExtras.h"
#અન્તર્ભાવ "llvm/Support/Errc.h"
#અન્તર્ભાવ "llvm/Support/ErrorHandling.h"
#અન્તર્ભાવ "llvm/Support/LEB128.h"
#અન્તર્ભાવ "llvm/Support/SwapByteOrder.h"

using નામાકાશ llvm;

તાર્કિક DataExtractor::prepareRead(uint64_t Offset, uint64_t Size,
                                Error *E) અચલ {
  જો (isValidOffsetForDataOfSize(Offset, Size))
    પાછા સાચું;
  જો (E) {
    જો (Offset <= Data.size())
      *E = createStringError(
          errc::illegal_byte_sequence,
          "unexpected end of data at offset 0x%zx while reading [0x%" PRIx64
          ", 0x%" PRIx64 ")",
          Data.size(), Offset, Offset + Size);
    નહીંતર
      *E = createStringError(errc::invalid_argument,
                             "offset 0x%" PRIx64
                             " is beyond the end of data at 0x%zx",
                             Offset, Data.size());
  }
  પાછા ખોટું;
}

સ્થિર તાર્કિક isError(Error *E) { પાછા E && *E; }

template <typename T>
T DataExtractor::getU(uint64_t *offset_ptr, Error *Err) અચલ {
  ErrorAsOutParameter ErrAsOut(Err);
  T val = 0;
  જો (isError(Err))
    પાછા val;

  uint64_t offset = *offset_ptr;
  જો (!prepareRead(offset, કદ(T), Err))
    પાછા val;
  std::memcpy(&val, &Data.data()[offset], કદ(val));
  જો (sys::IsLittleEndianHost != IsLittleEndian)
    sys::swapByteOrder(val);

  // Advance the offset
  *offset_ptr += કદ(val);
  પાછા val;
}

template <typename T>
T *DataExtractor::getUs(uint64_t *offset_ptr, T *dst, uint32_t ગણતરી,
                        Error *Err) અચલ {
  ErrorAsOutParameter ErrAsOut(Err);
  જો (isError(Err))
    પાછા nullptr;

  uint64_t offset = *offset_ptr;

  જો (!prepareRead(offset, કદ(*dst) * ગણતરી, Err))
    પાછા nullptr;
  માટે (T *કિંમત_ptr = dst, *end = dst + ગણતરી; કિંમત_ptr != end;
       ++કિંમત_ptr, offset += કદ(*dst))
    *કિંમત_ptr = getU<T>(offset_ptr, Err);
  // Advance the offset
  *offset_ptr = offset;
  // Return a non-NULL pointer to the converted data as an indicator of
  // success
  પાછા dst;
}

uint8_t DataExtractor::getU8(uint64_t *offset_ptr, llvm::Error *Err) અચલ {
  પાછા getU<uint8_t>(offset_ptr, Err);
}

uint8_t *DataExtractor::getU8(uint64_t *offset_ptr, uint8_t *dst,
                              uint32_t ગણતરી) અચલ {
  પાછા getUs<uint8_t>(offset_ptr, dst, ગણતરી, nullptr);
}

uint8_t *DataExtractor::getU8(Cursor &C, uint8_t *Dst, uint32_t ગણતરી_2) અચલ {
  પાછા getUs<uint8_t>(&C.Offset, Dst, ગણતરી_2, &C.Err);
}

uint16_t DataExtractor::getU16(uint64_t *offset_ptr, llvm::Error *Err) અચલ {
  પાછા getU<uint16_t>(offset_ptr, Err);
}

uint16_t *DataExtractor::getU16(uint64_t *offset_ptr, uint16_t *dst,
                                uint32_t ગણતરી) અચલ {
  પાછા getUs<uint16_t>(offset_ptr, dst, ગણતરી, nullptr);
}

uint32_t DataExtractor::getU24(uint64_t *OffsetPtr, Error *Err) અચલ {
  uint24_t ExtractedVal = getU<uint24_t>(OffsetPtr, Err);
  // The 3 bytes are in the correct byte order for the host.
  પાછા ExtractedVal.getAsUint32(sys::IsLittleEndianHost);
}

uint32_t DataExtractor::getU32(uint64_t *offset_ptr, llvm::Error *Err) અચલ {
  પાછા getU<uint32_t>(offset_ptr, Err);
}

uint32_t *DataExtractor::getU32(uint64_t *offset_ptr, uint32_t *dst,
                                uint32_t ગણતરી) અચલ {
  પાછા getUs<uint32_t>(offset_ptr, dst, ગણતરી, nullptr);
}

uint64_t DataExtractor::getU64(uint64_t *offset_ptr, llvm::Error *Err) અચલ {
  પાછા getU<uint64_t>(offset_ptr, Err);
}

uint64_t *DataExtractor::getU64(uint64_t *offset_ptr, uint64_t *dst,
                                uint32_t ગણતરી) અચલ {
  પાછા getUs<uint64_t>(offset_ptr, dst, ગણતરી, nullptr);
}

uint64_t DataExtractor::getUnsigned(uint64_t *offset_ptr, uint32_t byte_size,
                                    llvm::Error *Err) અચલ {
  પસંદગી (byte_size) {
  સ્થિતિ 1:
    પાછા getU8(offset_ptr, Err);
  સ્થિતિ 2:
    પાછા getU16(offset_ptr, Err);
  સ્થિતિ 4:
    પાછા getU32(offset_ptr, Err);
  સ્થિતિ 8:
    પાછા getU64(offset_ptr, Err);
  }

  // For any other byte size, read the bytes and swap/shift if necessary.
  ErrorAsOutParameter ErrAsOut(Err);
  uint64_t val = 0;
  જો (isError(Err))
    પાછા val;
  uint64_t offset = *offset_ptr;
  જો (!prepareRead(offset, byte_size, Err))
    પાછા val;
  // Copy into the least significant bytes of val regardless of host
  // endianness.
  std::memcpy(reinterpret_cast<અક્ષર *>(&val) +
                  (sys::IsLittleEndianHost ? 0 : 8 - byte_size),
              &Data.data()[offset], byte_size);
  // Swap the least significant bytes of val if endianness doesn't match.
  જો (sys::IsLittleEndianHost != IsLittleEndian)
    val = sys::getSwappedBytes(val) >> (8 * (8 - byte_size));

  *offset_ptr += byte_size;
  પાછા val;
}

int64_t
DataExtractor::getSigned(uint64_t *offset_ptr, uint32_t byte_size) અચલ {
  પસંદગી (byte_size) {
  સ્થિતિ 1:
    પાછા getS8(offset_ptr);
  સ્થિતિ 2:
    પાછા getS16(offset_ptr);
  સ્થિતિ 4:
    પાછા getS32(offset_ptr);
  સ્થિતિ 8:
    પાછા getS64(offset_ptr);
  }
  llvm_unreachable("getSigned unhandled case!");
}

StringRef DataExtractor::getCStrRef(uint64_t *OffsetPtr, Error *Err) અચલ {
  ErrorAsOutParameter ErrAsOut(Err);
  જો (isError(Err))
    પાછા StringRef();

  uint64_t Start = *OffsetPtr;
  StringRef::size_type Pos = Data.find('\0', Start);
  જો (Pos != StringRef::npos) {
    *OffsetPtr = Pos + 1;
    પાછા StringRef(Data.data() + Start, Pos - Start);
  }
  જો (Err)
    *Err = createStringError(errc::illegal_byte_sequence,
                             "no null terminated string at offset 0x%" PRIx64,
                             Start);
  પાછા StringRef();
}

StringRef DataExtractor::getFixedLengthString(uint64_t *OffsetPtr,
                                              uint64_t Length,
                                              StringRef TrimChars) અચલ {
  StringRef Bytes(getBytes(OffsetPtr, Length));
  પાછા Bytes.trim(TrimChars);
}

StringRef DataExtractor::getBytes(uint64_t *OffsetPtr, uint64_t Length,
                                  Error *Err) અચલ {
  ErrorAsOutParameter ErrAsOut(Err);
  જો (isError(Err))
    પાછા StringRef();

  જો (!prepareRead(*OffsetPtr, Length, Err))
    પાછા StringRef();

  StringRef Result = Data.substr(*OffsetPtr, Length);
  *OffsetPtr += Length;
  પાછા Result;
}

template <typename T>
સ્થિર T getLEB128(StringRef Data, uint64_t *OffsetPtr, Error *Err,
                   T (&Decoder)(અચલ uint8_t *p, unsigned *n,
                                અચલ uint8_t *end, અચલ અક્ષર **error)) {
  ArrayRef<uint8_t> Bytes = arrayRefFromStringRef(Data);
  પ્રતિજ્ઞા(*OffsetPtr <= Bytes.size());
  ErrorAsOutParameter ErrAsOut(Err);
  જો (isError(Err))
    પાછા T();

  અચલ અક્ષર *error = nullptr;
  unsigned bytes_read;
  T result =
      Decoder(Bytes.data() + *OffsetPtr, &bytes_read, Bytes.end(), &error);
  જો (error) {
    જો (Err)
      *Err = createStringError(errc::illegal_byte_sequence,
                               "unable to decode LEB128 at offset 0x%8.8" PRIx64
                               ": %s",
                               *OffsetPtr, error);
    પાછા T();
  }
  *OffsetPtr += bytes_read;
  પાછા result;
}

uint64_t DataExtractor::getULEB128(uint64_t *offset_ptr, Error *Err) અચલ {
  પાછા getLEB128(Data, offset_ptr, Err, decodeULEB128);
}

int64_t DataExtractor::getSLEB128(uint64_t *offset_ptr, Error *Err) અચલ {
  પાછા getLEB128(Data, offset_ptr, Err, decodeSLEB128);
}

APSInt DataExtractor::getSLEB128APSInt(uint64_t *OffsetPtr, Error *Err) અચલ {
  ErrorAsOutParameter ErrAsOut(Err);
  જો (isError(Err))
    પાછા APSInt();

  uint64_t Offset = *OffsetPtr;
  APInt કિંમત;
  unsigned Shift = 0;
  uint8_t Byte;
  કરો {
    જો (!prepareRead(Offset, 1, Err))
      પાછા APSInt();
    Byte = Data[Offset++];
    APInt Slice(7, Byte & 0x7f);
    જો (Shift == 0)
      કિંમત = Slice;
    નહીંતર {
      કિંમત = કિંમત.zext(Shift + 7);
      કિંમત.ઉમેરો_Bits(Slice, Shift);
    }
    Shift += 7;
  } જ્યારે (Byte & 0x80);

  // Sign extend Value to at least 64 bits to match getSLEB128.
  જો (Shift < 64)
    કિંમત = કિંમત.sext(64);

  *OffsetPtr = Offset;
  પાછા APSInt(std::move(કિંમત), /*isUnsigned=*/ખોટું);
}

શૂન્ય DataExtractor::skip(Cursor &C, uint64_t Length) અચલ {
  ErrorAsOutParameter ErrAsOut(C.Err);
  જો (isError(&C.Err))
    પાછા;

  જો (prepareRead(C.Offset, Length, &C.Err))
    C.Offset += Length;
}
