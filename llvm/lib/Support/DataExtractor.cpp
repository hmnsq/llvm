//===-- DataExtractor.cpp -------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/Support/DataExtractor.h"
#அந்தர்பாவ "llvm/ADT/StringExtras.h"
#அந்தர்பாவ "llvm/Support/Errc.h"
#அந்தர்பாவ "llvm/Support/ErrorHandling.h"
#அந்தர்பாவ "llvm/Support/LEB128.h"
#அந்தர்பாவ "llvm/Support/SwapByteOrder.h"

using நாமாகாஶ llvm;

தருக்க DataExtractor::prepareRead(uint64_t Offset, uint64_t Size,
                                Error *E) மாறிலி {
  எனில் (isValidOffsetForDataOfSize(Offset, Size))
    திரும்பு உண்மை;
  எனில் (E) {
    எனில் (Offset <= Data.size())
      *E = createStringError(
          errc::illegal_byte_sequence,
          "unexpected end of data at offset 0x%zx while reading [0x%" PRIx64
          ", 0x%" PRIx64 ")",
          Data.size(), Offset, Offset + Size);
    இல்லையெனில்
      *E = createStringError(errc::invalid_argument,
                             "offset 0x%" PRIx64
                             " is beyond the end of data at 0x%zx",
                             Offset, Data.size());
  }
  திரும்பு பொய்;
}

நிலையான தருக்க isError(Error *E) { திரும்பு E && *E; }

template <typename T>
T DataExtractor::getU(uint64_t *offset_ptr, Error *Err) மாறிலி {
  ErrorAsOutParameter ErrAsOut(Err);
  T val = 0;
  எனில் (isError(Err))
    திரும்பு val;

  uint64_t offset = *offset_ptr;
  எனில் (!prepareRead(offset, அளவு(T), Err))
    திரும்பு val;
  std::memcpy(&val, &Data.data()[offset], அளவு(val));
  எனில் (sys::IsLittleEndianHost != IsLittleEndian)
    sys::swapByteOrder(val);

  // Advance the offset
  *offset_ptr += அளவு(val);
  திரும்பு val;
}

template <typename T>
T *DataExtractor::getUs(uint64_t *offset_ptr, T *dst, uint32_t மொத்தம்,
                        Error *Err) மாறிலி {
  ErrorAsOutParameter ErrAsOut(Err);
  எனில் (isError(Err))
    திரும்பு nullptr;

  uint64_t offset = *offset_ptr;

  எனில் (!prepareRead(offset, அளவு(*dst) * மொத்தம், Err))
    திரும்பு nullptr;
  க்கு (T *மதிப்பு_ptr = dst, *end = dst + மொத்தம்; மதிப்பு_ptr != end;
       ++மதிப்பு_ptr, offset += அளவு(*dst))
    *மதிப்பு_ptr = getU<T>(offset_ptr, Err);
  // Advance the offset
  *offset_ptr = offset;
  // Return a non-NULL pointer to the converted data as an indicator of
  // success
  திரும்பு dst;
}

uint8_t DataExtractor::getU8(uint64_t *offset_ptr, llvm::Error *Err) மாறிலி {
  திரும்பு getU<uint8_t>(offset_ptr, Err);
}

uint8_t *DataExtractor::getU8(uint64_t *offset_ptr, uint8_t *dst,
                              uint32_t மொத்தம்) மாறிலி {
  திரும்பு getUs<uint8_t>(offset_ptr, dst, மொத்தம், nullptr);
}

uint8_t *DataExtractor::getU8(Cursor &C, uint8_t *Dst, uint32_t மொத்தம்_2) மாறிலி {
  திரும்பு getUs<uint8_t>(&C.Offset, Dst, மொத்தம்_2, &C.Err);
}

uint16_t DataExtractor::getU16(uint64_t *offset_ptr, llvm::Error *Err) மாறிலி {
  திரும்பு getU<uint16_t>(offset_ptr, Err);
}

uint16_t *DataExtractor::getU16(uint64_t *offset_ptr, uint16_t *dst,
                                uint32_t மொத்தம்) மாறிலி {
  திரும்பு getUs<uint16_t>(offset_ptr, dst, மொத்தம், nullptr);
}

uint32_t DataExtractor::getU24(uint64_t *OffsetPtr, Error *Err) மாறிலி {
  uint24_t ExtractedVal = getU<uint24_t>(OffsetPtr, Err);
  // The 3 bytes are in the correct byte order for the host.
  திரும்பு ExtractedVal.getAsUint32(sys::IsLittleEndianHost);
}

uint32_t DataExtractor::getU32(uint64_t *offset_ptr, llvm::Error *Err) மாறிலி {
  திரும்பு getU<uint32_t>(offset_ptr, Err);
}

uint32_t *DataExtractor::getU32(uint64_t *offset_ptr, uint32_t *dst,
                                uint32_t மொத்தம்) மாறிலி {
  திரும்பு getUs<uint32_t>(offset_ptr, dst, மொத்தம், nullptr);
}

uint64_t DataExtractor::getU64(uint64_t *offset_ptr, llvm::Error *Err) மாறிலி {
  திரும்பு getU<uint64_t>(offset_ptr, Err);
}

uint64_t *DataExtractor::getU64(uint64_t *offset_ptr, uint64_t *dst,
                                uint32_t மொத்தம்) மாறிலி {
  திரும்பு getUs<uint64_t>(offset_ptr, dst, மொத்தம், nullptr);
}

uint64_t DataExtractor::getUnsigned(uint64_t *offset_ptr, uint32_t byte_size,
                                    llvm::Error *Err) மாறிலி {
  தேர்வு (byte_size) {
  நிலை 1:
    திரும்பு getU8(offset_ptr, Err);
  நிலை 2:
    திரும்பு getU16(offset_ptr, Err);
  நிலை 4:
    திரும்பு getU32(offset_ptr, Err);
  நிலை 8:
    திரும்பு getU64(offset_ptr, Err);
  }

  // For any other byte size, read the bytes and swap/shift if necessary.
  ErrorAsOutParameter ErrAsOut(Err);
  uint64_t val = 0;
  எனில் (isError(Err))
    திரும்பு val;
  uint64_t offset = *offset_ptr;
  எனில் (!prepareRead(offset, byte_size, Err))
    திரும்பு val;
  // Copy into the least significant bytes of val regardless of host
  // endianness.
  std::memcpy(reinterpret_cast<எழுத்து *>(&val) +
                  (sys::IsLittleEndianHost ? 0 : 8 - byte_size),
              &Data.data()[offset], byte_size);
  // Swap the least significant bytes of val if endianness doesn't match.
  எனில் (sys::IsLittleEndianHost != IsLittleEndian)
    val = sys::getSwappedBytes(val) >> (8 * (8 - byte_size));

  *offset_ptr += byte_size;
  திரும்பு val;
}

int64_t
DataExtractor::getSigned(uint64_t *offset_ptr, uint32_t byte_size) மாறிலி {
  தேர்வு (byte_size) {
  நிலை 1:
    திரும்பு getS8(offset_ptr);
  நிலை 2:
    திரும்பு getS16(offset_ptr);
  நிலை 4:
    திரும்பு getS32(offset_ptr);
  நிலை 8:
    திரும்பு getS64(offset_ptr);
  }
  llvm_unreachable("getSigned unhandled case!");
}

StringRef DataExtractor::getCStrRef(uint64_t *OffsetPtr, Error *Err) மாறிலி {
  ErrorAsOutParameter ErrAsOut(Err);
  எனில் (isError(Err))
    திரும்பு StringRef();

  uint64_t Start = *OffsetPtr;
  StringRef::size_type Pos = Data.find('\0', Start);
  எனில் (Pos != StringRef::npos) {
    *OffsetPtr = Pos + 1;
    திரும்பு StringRef(Data.data() + Start, Pos - Start);
  }
  எனில் (Err)
    *Err = createStringError(errc::illegal_byte_sequence,
                             "no null terminated string at offset 0x%" PRIx64,
                             Start);
  திரும்பு StringRef();
}

StringRef DataExtractor::getFixedLengthString(uint64_t *OffsetPtr,
                                              uint64_t Length,
                                              StringRef TrimChars) மாறிலி {
  StringRef Bytes(getBytes(OffsetPtr, Length));
  திரும்பு Bytes.trim(TrimChars);
}

StringRef DataExtractor::getBytes(uint64_t *OffsetPtr, uint64_t Length,
                                  Error *Err) மாறிலி {
  ErrorAsOutParameter ErrAsOut(Err);
  எனில் (isError(Err))
    திரும்பு StringRef();

  எனில் (!prepareRead(*OffsetPtr, Length, Err))
    திரும்பு StringRef();

  StringRef Result = Data.substr(*OffsetPtr, Length);
  *OffsetPtr += Length;
  திரும்பு Result;
}

template <typename T>
நிலையான T getLEB128(StringRef Data, uint64_t *OffsetPtr, Error *Err,
                   T (&Decoder)(மாறிலி uint8_t *p, unsigned *n,
                                மாறிலி uint8_t *end, மாறிலி எழுத்து **error)) {
  ArrayRef<uint8_t> Bytes = arrayRefFromStringRef(Data);
  ப்ரதிஜ்ஞா(*OffsetPtr <= Bytes.size());
  ErrorAsOutParameter ErrAsOut(Err);
  எனில் (isError(Err))
    திரும்பு T();

  மாறிலி எழுத்து *error = nullptr;
  unsigned bytes_read;
  T result =
      Decoder(Bytes.data() + *OffsetPtr, &bytes_read, Bytes.end(), &error);
  எனில் (error) {
    எனில் (Err)
      *Err = createStringError(errc::illegal_byte_sequence,
                               "unable to decode LEB128 at offset 0x%8.8" PRIx64
                               ": %s",
                               *OffsetPtr, error);
    திரும்பு T();
  }
  *OffsetPtr += bytes_read;
  திரும்பு result;
}

uint64_t DataExtractor::getULEB128(uint64_t *offset_ptr, Error *Err) மாறிலி {
  திரும்பு getLEB128(Data, offset_ptr, Err, decodeULEB128);
}

int64_t DataExtractor::getSLEB128(uint64_t *offset_ptr, Error *Err) மாறிலி {
  திரும்பு getLEB128(Data, offset_ptr, Err, decodeSLEB128);
}

APSInt DataExtractor::getSLEB128APSInt(uint64_t *OffsetPtr, Error *Err) மாறிலி {
  ErrorAsOutParameter ErrAsOut(Err);
  எனில் (isError(Err))
    திரும்பு APSInt();

  uint64_t Offset = *OffsetPtr;
  APInt மதிப்பு;
  unsigned Shift = 0;
  uint8_t Byte;
  செய் {
    எனில் (!prepareRead(Offset, 1, Err))
      திரும்பு APSInt();
    Byte = Data[Offset++];
    APInt Slice(7, Byte & 0x7f);
    எனில் (Shift == 0)
      மதிப்பு = Slice;
    இல்லையெனில் {
      மதிப்பு = மதிப்பு.zext(Shift + 7);
      மதிப்பு.செருகு_Bits(Slice, Shift);
    }
    Shift += 7;
  } போது (Byte & 0x80);

  // Sign extend Value to at least 64 bits to match getSLEB128.
  எனில் (Shift < 64)
    மதிப்பு = மதிப்பு.sext(64);

  *OffsetPtr = Offset;
  திரும்பு APSInt(std::move(மதிப்பு), /*isUnsigned=*/பொய்);
}

வெறுமை DataExtractor::skip(Cursor &C, uint64_t Length) மாறிலி {
  ErrorAsOutParameter ErrAsOut(C.Err);
  எனில் (isError(&C.Err))
    திரும்பு;

  எனில் (prepareRead(C.Offset, Length, &C.Err))
    C.Offset += Length;
}
