//===-- DataExtractor.cpp -------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/DataExtractor.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Errc.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ErrorHandling.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/LEB128.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/SwapByteOrder.h"

using ನಾಮಾಕಾಶ llvm;

ತಾರ್ಕಿಕ DataExtractor::prepareRead(uint64_t Offset, uint64_t Size,
                                Error *E) ಧ್ರುವ {
  ಯದಿ (isValidOffsetForDataOfSize(Offset, Size))
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  ಯದಿ (E) {
    ಯದಿ (Offset <= Data.size())
      *E = createStringError(
          errc::illegal_byte_sequence,
          "unexpected end of data at offset 0x%zx while reading [0x%" PRIx64
          ", 0x%" PRIx64 ")",
          Data.size(), Offset, Offset + Size);
    ಅನ್ಯಥಾ
      *E = createStringError(errc::invalid_argument,
                             "offset 0x%" PRIx64
                             " is beyond the end of data at 0x%zx",
                             Offset, Data.size());
  }
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ isError(Error *E) { ಪ್ರತ್ಯಾಗಮ E && *E; }

template <typename T>
T DataExtractor::getU(uint64_t *offset_ptr, Error *Err) ಧ್ರುವ {
  ErrorAsOutParameter ErrAsOut(Err);
  T val = 0;
  ಯದಿ (isError(Err))
    ಪ್ರತ್ಯಾಗಮ val;

  uint64_t offset = *offset_ptr;
  ಯದಿ (!prepareRead(offset, ಪರಿಮಾಣ(T), Err))
    ಪ್ರತ್ಯಾಗಮ val;
  std::memcpy(&val, &Data.data()[offset], ಪರಿಮಾಣ(val));
  ಯದಿ (sys::IsLittleEndianHost != IsLittleEndian)
    sys::swapByteOrder(val);

  // Advance the offset
  *offset_ptr += ಪರಿಮಾಣ(val);
  ಪ್ರತ್ಯಾಗಮ val;
}

template <typename T>
T *DataExtractor::getUs(uint64_t *offset_ptr, T *dst, uint32_t ಎಣಿಕೆ,
                        Error *Err) ಧ್ರುವ {
  ErrorAsOutParameter ErrAsOut(Err);
  ಯದಿ (isError(Err))
    ಪ್ರತ್ಯಾಗಮ nullptr;

  uint64_t offset = *offset_ptr;

  ಯದಿ (!prepareRead(offset, ಪರಿಮಾಣ(*dst) * ಎಣಿಕೆ, Err))
    ಪ್ರತ್ಯಾಗಮ nullptr;
  ಪ್ರತಿ (T *ಮೌಲ್ಯ_ptr = dst, *end = dst + ಎಣಿಕೆ; ಮೌಲ್ಯ_ptr != end;
       ++ಮೌಲ್ಯ_ptr, offset += ಪರಿಮಾಣ(*dst))
    *ಮೌಲ್ಯ_ptr = getU<T>(offset_ptr, Err);
  // Advance the offset
  *offset_ptr = offset;
  // Return a non-NULL pointer to the converted data as an indicator of
  // success
  ಪ್ರತ್ಯಾಗಮ dst;
}

uint8_t DataExtractor::getU8(uint64_t *offset_ptr, llvm::Error *Err) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ getU<uint8_t>(offset_ptr, Err);
}

uint8_t *DataExtractor::getU8(uint64_t *offset_ptr, uint8_t *dst,
                              uint32_t ಎಣಿಕೆ) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ getUs<uint8_t>(offset_ptr, dst, ಎಣಿಕೆ, nullptr);
}

uint8_t *DataExtractor::getU8(Cursor &C, uint8_t *Dst, uint32_t ಎಣಿಕೆ_2) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ getUs<uint8_t>(&C.Offset, Dst, ಎಣಿಕೆ_2, &C.Err);
}

uint16_t DataExtractor::getU16(uint64_t *offset_ptr, llvm::Error *Err) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ getU<uint16_t>(offset_ptr, Err);
}

uint16_t *DataExtractor::getU16(uint64_t *offset_ptr, uint16_t *dst,
                                uint32_t ಎಣಿಕೆ) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ getUs<uint16_t>(offset_ptr, dst, ಎಣಿಕೆ, nullptr);
}

uint32_t DataExtractor::getU24(uint64_t *OffsetPtr, Error *Err) ಧ್ರುವ {
  uint24_t ExtractedVal = getU<uint24_t>(OffsetPtr, Err);
  // The 3 bytes are in the correct byte order for the host.
  ಪ್ರತ್ಯಾಗಮ ExtractedVal.getAsUint32(sys::IsLittleEndianHost);
}

uint32_t DataExtractor::getU32(uint64_t *offset_ptr, llvm::Error *Err) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ getU<uint32_t>(offset_ptr, Err);
}

uint32_t *DataExtractor::getU32(uint64_t *offset_ptr, uint32_t *dst,
                                uint32_t ಎಣಿಕೆ) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ getUs<uint32_t>(offset_ptr, dst, ಎಣಿಕೆ, nullptr);
}

uint64_t DataExtractor::getU64(uint64_t *offset_ptr, llvm::Error *Err) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ getU<uint64_t>(offset_ptr, Err);
}

uint64_t *DataExtractor::getU64(uint64_t *offset_ptr, uint64_t *dst,
                                uint32_t ಎಣಿಕೆ) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ getUs<uint64_t>(offset_ptr, dst, ಎಣಿಕೆ, nullptr);
}

uint64_t DataExtractor::getUnsigned(uint64_t *offset_ptr, uint32_t byte_size,
                                    llvm::Error *Err) ಧ್ರುವ {
  ವಿಕಲ್ಪ (byte_size) {
  ಪಕ್ಷ 1:
    ಪ್ರತ್ಯಾಗಮ getU8(offset_ptr, Err);
  ಪಕ್ಷ 2:
    ಪ್ರತ್ಯಾಗಮ getU16(offset_ptr, Err);
  ಪಕ್ಷ 4:
    ಪ್ರತ್ಯಾಗಮ getU32(offset_ptr, Err);
  ಪಕ್ಷ 8:
    ಪ್ರತ್ಯಾಗಮ getU64(offset_ptr, Err);
  }

  // For any other byte size, read the bytes and swap/shift if necessary.
  ErrorAsOutParameter ErrAsOut(Err);
  uint64_t val = 0;
  ಯದಿ (isError(Err))
    ಪ್ರತ್ಯಾಗಮ val;
  uint64_t offset = *offset_ptr;
  ಯದಿ (!prepareRead(offset, byte_size, Err))
    ಪ್ರತ್ಯಾಗಮ val;
  // Copy into the least significant bytes of val regardless of host
  // endianness.
  std::memcpy(reinterpret_cast<ವರ್ಣ *>(&val) +
                  (sys::IsLittleEndianHost ? 0 : 8 - byte_size),
              &Data.data()[offset], byte_size);
  // Swap the least significant bytes of val if endianness doesn't match.
  ಯದಿ (sys::IsLittleEndianHost != IsLittleEndian)
    val = sys::getSwappedBytes(val) >> (8 * (8 - byte_size));

  *offset_ptr += byte_size;
  ಪ್ರತ್ಯಾಗಮ val;
}

int64_t
DataExtractor::getSigned(uint64_t *offset_ptr, uint32_t byte_size) ಧ್ರುವ {
  ವಿಕಲ್ಪ (byte_size) {
  ಪಕ್ಷ 1:
    ಪ್ರತ್ಯಾಗಮ getS8(offset_ptr);
  ಪಕ್ಷ 2:
    ಪ್ರತ್ಯಾಗಮ getS16(offset_ptr);
  ಪಕ್ಷ 4:
    ಪ್ರತ್ಯಾಗಮ getS32(offset_ptr);
  ಪಕ್ಷ 8:
    ಪ್ರತ್ಯಾಗಮ getS64(offset_ptr);
  }
  llvm_unreachable("getSigned unhandled case!");
}

StringRef DataExtractor::getCStrRef(uint64_t *OffsetPtr, Error *Err) ಧ್ರುವ {
  ErrorAsOutParameter ErrAsOut(Err);
  ಯದಿ (isError(Err))
    ಪ್ರತ್ಯಾಗಮ StringRef();

  uint64_t Start = *OffsetPtr;
  StringRef::size_type Pos = Data.find('\0', Start);
  ಯದಿ (Pos != StringRef::npos) {
    *OffsetPtr = Pos + 1;
    ಪ್ರತ್ಯಾಗಮ StringRef(Data.data() + Start, Pos - Start);
  }
  ಯದಿ (Err)
    *Err = createStringError(errc::illegal_byte_sequence,
                             "no null terminated string at offset 0x%" PRIx64,
                             Start);
  ಪ್ರತ್ಯಾಗಮ StringRef();
}

StringRef DataExtractor::getFixedLengthString(uint64_t *OffsetPtr,
                                              uint64_t Length,
                                              StringRef TrimChars) ಧ್ರುವ {
  StringRef Bytes(getBytes(OffsetPtr, Length));
  ಪ್ರತ್ಯಾಗಮ Bytes.trim(TrimChars);
}

StringRef DataExtractor::getBytes(uint64_t *OffsetPtr, uint64_t Length,
                                  Error *Err) ಧ್ರುವ {
  ErrorAsOutParameter ErrAsOut(Err);
  ಯದಿ (isError(Err))
    ಪ್ರತ್ಯಾಗಮ StringRef();

  ಯದಿ (!prepareRead(*OffsetPtr, Length, Err))
    ಪ್ರತ್ಯಾಗಮ StringRef();

  StringRef Result = Data.substr(*OffsetPtr, Length);
  *OffsetPtr += Length;
  ಪ್ರತ್ಯಾಗಮ Result;
}

template <typename T>
ಸ್ಥಿರ T getLEB128(StringRef Data, uint64_t *OffsetPtr, Error *Err,
                   T (&Decoder)(ಧ್ರುವ uint8_t *p, unsigned *n,
                                ಧ್ರುವ uint8_t *end, ಧ್ರುವ ವರ್ಣ **error)) {
  ArrayRef<uint8_t> Bytes = arrayRefFromStringRef(Data);
  ಪ್ರತಿಜ್ಞಾ(*OffsetPtr <= Bytes.size());
  ErrorAsOutParameter ErrAsOut(Err);
  ಯದಿ (isError(Err))
    ಪ್ರತ್ಯಾಗಮ T();

  ಧ್ರುವ ವರ್ಣ *error = nullptr;
  unsigned bytes_read;
  T result =
      Decoder(Bytes.data() + *OffsetPtr, &bytes_read, Bytes.end(), &error);
  ಯದಿ (error) {
    ಯದಿ (Err)
      *Err = createStringError(errc::illegal_byte_sequence,
                               "unable to decode LEB128 at offset 0x%8.8" PRIx64
                               ": %s",
                               *OffsetPtr, error);
    ಪ್ರತ್ಯಾಗಮ T();
  }
  *OffsetPtr += bytes_read;
  ಪ್ರತ್ಯಾಗಮ result;
}

uint64_t DataExtractor::getULEB128(uint64_t *offset_ptr, Error *Err) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ getLEB128(Data, offset_ptr, Err, decodeULEB128);
}

int64_t DataExtractor::getSLEB128(uint64_t *offset_ptr, Error *Err) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ getLEB128(Data, offset_ptr, Err, decodeSLEB128);
}

APSInt DataExtractor::getSLEB128APSInt(uint64_t *OffsetPtr, Error *Err) ಧ್ರುವ {
  ErrorAsOutParameter ErrAsOut(Err);
  ಯದಿ (isError(Err))
    ಪ್ರತ್ಯಾಗಮ APSInt();

  uint64_t Offset = *OffsetPtr;
  APInt ಮೌಲ್ಯ;
  unsigned Shift = 0;
  uint8_t Byte;
  ಕುರು {
    ಯದಿ (!prepareRead(Offset, 1, Err))
      ಪ್ರತ್ಯಾಗಮ APSInt();
    Byte = Data[Offset++];
    APInt Slice(7, Byte & 0x7f);
    ಯದಿ (Shift == 0)
      ಮೌಲ್ಯ = Slice;
    ಅನ್ಯಥಾ {
      ಮೌಲ್ಯ = ಮೌಲ್ಯ.zext(Shift + 7);
      ಮೌಲ್ಯ.ಸೇರಿಸು_Bits(Slice, Shift);
    }
    Shift += 7;
  } ಯಾವತ್ (Byte & 0x80);

  // Sign extend Value to at least 64 bits to match getSLEB128.
  ಯದಿ (Shift < 64)
    ಮೌಲ್ಯ = ಮೌಲ್ಯ.sext(64);

  *OffsetPtr = Offset;
  ಪ್ರತ್ಯಾಗಮ APSInt(std::move(ಮೌಲ್ಯ), /*isUnsigned=*/ಅಸತ್ಯ);
}

ಶೂನ್ಯ DataExtractor::skip(Cursor &C, uint64_t Length) ಧ್ರುವ {
  ErrorAsOutParameter ErrAsOut(C.Err);
  ಯದಿ (isError(&C.Err))
    ಪ್ರತ್ಯಾಗಮ;

  ಯದಿ (prepareRead(C.Offset, Length, &C.Err))
    C.Offset += Length;
}
