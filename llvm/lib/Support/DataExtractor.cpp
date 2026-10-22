//===-- DataExtractor.cpp -------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/Support/DataExtractor.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Errc.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ErrorHandling.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/LEB128.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/SwapByteOrder.h"

using ନାମାକାଶ llvm;

ତାର୍କିକ DataExtractor::prepareRead(uint64_t Offset, uint64_t Size,
                                Error *E) ଅଚଳ {
  ଯଦି (isValidOffsetForDataOfSize(Offset, Size))
    ଫେରନ୍ତୁ ସତ୍ୟ;
  ଯଦି (E) {
    ଯଦି (Offset <= Data.size())
      *E = createStringError(
          errc::illegal_byte_sequence,
          "unexpected end of data at offset 0x%zx while reading [0x%" PRIx64
          ", 0x%" PRIx64 ")",
          Data.size(), Offset, Offset + Size);
    ଅନ୍ୟଥା
      *E = createStringError(errc::invalid_argument,
                             "offset 0x%" PRIx64
                             " is beyond the end of data at 0x%zx",
                             Offset, Data.size());
  }
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

ସ୍ଥିର ତାର୍କିକ isError(Error *E) { ଫେରନ୍ତୁ E && *E; }

template <typename T>
T DataExtractor::getU(uint64_t *offset_ptr, Error *Err) ଅଚଳ {
  ErrorAsOutParameter ErrAsOut(Err);
  T val = 0;
  ଯଦି (isError(Err))
    ଫେରନ୍ତୁ val;

  uint64_t offset = *offset_ptr;
  ଯଦି (!prepareRead(offset, ଆକାର(T), Err))
    ଫେରନ୍ତୁ val;
  std::memcpy(&val, &Data.data()[offset], ଆକାର(val));
  ଯଦି (sys::IsLittleEndianHost != IsLittleEndian)
    sys::swapByteOrder(val);

  // Advance the offset
  *offset_ptr += ଆକାର(val);
  ଫେରନ୍ତୁ val;
}

template <typename T>
T *DataExtractor::getUs(uint64_t *offset_ptr, T *dst, uint32_t ଗଣନା_2,
                        Error *Err) ଅଚଳ {
  ErrorAsOutParameter ErrAsOut(Err);
  ଯଦି (isError(Err))
    ଫେରନ୍ତୁ nullptr;

  uint64_t offset = *offset_ptr;

  ଯଦି (!prepareRead(offset, ଆକାର(*dst) * ଗଣନା_2, Err))
    ଫେରନ୍ତୁ nullptr;
  ପାଇଁ (T *ମୂଲ୍ୟ_ptr = dst, *end = dst + ଗଣନା_2; ମୂଲ୍ୟ_ptr != end;
       ++ମୂଲ୍ୟ_ptr, offset += ଆକାର(*dst))
    *ମୂଲ୍ୟ_ptr = getU<T>(offset_ptr, Err);
  // Advance the offset
  *offset_ptr = offset;
  // Return a non-NULL pointer to the converted data as an indicator of
  // success
  ଫେରନ୍ତୁ dst;
}

uint8_t DataExtractor::getU8(uint64_t *offset_ptr, llvm::Error *Err) ଅଚଳ {
  ଫେରନ୍ତୁ getU<uint8_t>(offset_ptr, Err);
}

uint8_t *DataExtractor::getU8(uint64_t *offset_ptr, uint8_t *dst,
                              uint32_t ଗଣନା_2) ଅଚଳ {
  ଫେରନ୍ତୁ getUs<uint8_t>(offset_ptr, dst, ଗଣନା_2, nullptr);
}

uint8_t *DataExtractor::getU8(Cursor &C, uint8_t *Dst, uint32_t ଗଣନା_3) ଅଚଳ {
  ଫେରନ୍ତୁ getUs<uint8_t>(&C.Offset, Dst, ଗଣନା_3, &C.Err);
}

uint16_t DataExtractor::getU16(uint64_t *offset_ptr, llvm::Error *Err) ଅଚଳ {
  ଫେରନ୍ତୁ getU<uint16_t>(offset_ptr, Err);
}

uint16_t *DataExtractor::getU16(uint64_t *offset_ptr, uint16_t *dst,
                                uint32_t ଗଣନା_2) ଅଚଳ {
  ଫେରନ୍ତୁ getUs<uint16_t>(offset_ptr, dst, ଗଣନା_2, nullptr);
}

uint32_t DataExtractor::getU24(uint64_t *OffsetPtr, Error *Err) ଅଚଳ {
  uint24_t ExtractedVal = getU<uint24_t>(OffsetPtr, Err);
  // The 3 bytes are in the correct byte order for the host.
  ଫେରନ୍ତୁ ExtractedVal.getAsUint32(sys::IsLittleEndianHost);
}

uint32_t DataExtractor::getU32(uint64_t *offset_ptr, llvm::Error *Err) ଅଚଳ {
  ଫେରନ୍ତୁ getU<uint32_t>(offset_ptr, Err);
}

uint32_t *DataExtractor::getU32(uint64_t *offset_ptr, uint32_t *dst,
                                uint32_t ଗଣନା_2) ଅଚଳ {
  ଫେରନ୍ତୁ getUs<uint32_t>(offset_ptr, dst, ଗଣନା_2, nullptr);
}

uint64_t DataExtractor::getU64(uint64_t *offset_ptr, llvm::Error *Err) ଅଚଳ {
  ଫେରନ୍ତୁ getU<uint64_t>(offset_ptr, Err);
}

uint64_t *DataExtractor::getU64(uint64_t *offset_ptr, uint64_t *dst,
                                uint32_t ଗଣନା_2) ଅଚଳ {
  ଫେରନ୍ତୁ getUs<uint64_t>(offset_ptr, dst, ଗଣନା_2, nullptr);
}

uint64_t DataExtractor::getUnsigned(uint64_t *offset_ptr, uint32_t byte_size,
                                    llvm::Error *Err) ଅଚଳ {
  ବାଛ (byte_size) {
  ସ୍ଥିତି 1:
    ଫେରନ୍ତୁ getU8(offset_ptr, Err);
  ସ୍ଥିତି 2:
    ଫେରନ୍ତୁ getU16(offset_ptr, Err);
  ସ୍ଥିତି 4:
    ଫେରନ୍ତୁ getU32(offset_ptr, Err);
  ସ୍ଥିତି 8:
    ଫେରନ୍ତୁ getU64(offset_ptr, Err);
  }

  // For any other byte size, read the bytes and swap/shift if necessary.
  ErrorAsOutParameter ErrAsOut(Err);
  uint64_t val = 0;
  ଯଦି (isError(Err))
    ଫେରନ୍ତୁ val;
  uint64_t offset = *offset_ptr;
  ଯଦି (!prepareRead(offset, byte_size, Err))
    ଫେରନ୍ତୁ val;
  // Copy into the least significant bytes of val regardless of host
  // endianness.
  std::memcpy(reinterpret_cast<ଅକ୍ଷର *>(&val) +
                  (sys::IsLittleEndianHost ? 0 : 8 - byte_size),
              &Data.data()[offset], byte_size);
  // Swap the least significant bytes of val if endianness doesn't match.
  ଯଦି (sys::IsLittleEndianHost != IsLittleEndian)
    val = sys::getSwappedBytes(val) >> (8 * (8 - byte_size));

  *offset_ptr += byte_size;
  ଫେରନ୍ତୁ val;
}

int64_t
DataExtractor::getSigned(uint64_t *offset_ptr, uint32_t byte_size) ଅଚଳ {
  ବାଛ (byte_size) {
  ସ୍ଥିତି 1:
    ଫେରନ୍ତୁ getS8(offset_ptr);
  ସ୍ଥିତି 2:
    ଫେରନ୍ତୁ getS16(offset_ptr);
  ସ୍ଥିତି 4:
    ଫେରନ୍ତୁ getS32(offset_ptr);
  ସ୍ଥିତି 8:
    ଫେରନ୍ତୁ getS64(offset_ptr);
  }
  llvm_unreachable("getSigned unhandled case!");
}

StringRef DataExtractor::getCStrRef(uint64_t *OffsetPtr, Error *Err) ଅଚଳ {
  ErrorAsOutParameter ErrAsOut(Err);
  ଯଦି (isError(Err))
    ଫେରନ୍ତୁ StringRef();

  uint64_t Start = *OffsetPtr;
  StringRef::size_type Pos = Data.find('\0', Start);
  ଯଦି (Pos != StringRef::npos) {
    *OffsetPtr = Pos + 1;
    ଫେରନ୍ତୁ StringRef(Data.data() + Start, Pos - Start);
  }
  ଯଦି (Err)
    *Err = createStringError(errc::illegal_byte_sequence,
                             "no null terminated string at offset 0x%" PRIx64,
                             Start);
  ଫେରନ୍ତୁ StringRef();
}

StringRef DataExtractor::getFixedLengthString(uint64_t *OffsetPtr,
                                              uint64_t Length,
                                              StringRef TrimChars) ଅଚଳ {
  StringRef Bytes(getBytes(OffsetPtr, Length));
  ଫେରନ୍ତୁ Bytes.trim(TrimChars);
}

StringRef DataExtractor::getBytes(uint64_t *OffsetPtr, uint64_t Length,
                                  Error *Err) ଅଚଳ {
  ErrorAsOutParameter ErrAsOut(Err);
  ଯଦି (isError(Err))
    ଫେରନ୍ତୁ StringRef();

  ଯଦି (!prepareRead(*OffsetPtr, Length, Err))
    ଫେରନ୍ତୁ StringRef();

  StringRef Result = Data.substr(*OffsetPtr, Length);
  *OffsetPtr += Length;
  ଫେରନ୍ତୁ Result;
}

template <typename T>
ସ୍ଥିର T getLEB128(StringRef Data, uint64_t *OffsetPtr, Error *Err,
                   T (&Decoder)(ଅଚଳ uint8_t *p, unsigned *n,
                                ଅଚଳ uint8_t *end, ଅଚଳ ଅକ୍ଷର **error)) {
  ArrayRef<uint8_t> Bytes = arrayRefFromStringRef(Data);
  ପ୍ରତିଜ୍ଞା(*OffsetPtr <= Bytes.size());
  ErrorAsOutParameter ErrAsOut(Err);
  ଯଦି (isError(Err))
    ଫେରନ୍ତୁ T();

  ଅଚଳ ଅକ୍ଷର *error = nullptr;
  unsigned bytes_read;
  T result =
      Decoder(Bytes.data() + *OffsetPtr, &bytes_read, Bytes.end(), &error);
  ଯଦି (error) {
    ଯଦି (Err)
      *Err = createStringError(errc::illegal_byte_sequence,
                               "unable to decode LEB128 at offset 0x%8.8" PRIx64
                               ": %s",
                               *OffsetPtr, error);
    ଫେରନ୍ତୁ T();
  }
  *OffsetPtr += bytes_read;
  ଫେରନ୍ତୁ result;
}

uint64_t DataExtractor::getULEB128(uint64_t *offset_ptr, Error *Err) ଅଚଳ {
  ଫେରନ୍ତୁ getLEB128(Data, offset_ptr, Err, decodeULEB128);
}

int64_t DataExtractor::getSLEB128(uint64_t *offset_ptr, Error *Err) ଅଚଳ {
  ଫେରନ୍ତୁ getLEB128(Data, offset_ptr, Err, decodeSLEB128);
}

APSInt DataExtractor::getSLEB128APSInt(uint64_t *OffsetPtr, Error *Err) ଅଚଳ {
  ErrorAsOutParameter ErrAsOut(Err);
  ଯଦି (isError(Err))
    ଫେରନ୍ତୁ APSInt();

  uint64_t Offset = *OffsetPtr;
  APInt ମୂଲ୍ୟ;
  unsigned Shift = 0;
  uint8_t Byte;
  କର {
    ଯଦି (!prepareRead(Offset, 1, Err))
      ଫେରନ୍ତୁ APSInt();
    Byte = Data[Offset++];
    APInt Slice(7, Byte & 0x7f);
    ଯଦି (Shift == 0)
      ମୂଲ୍ୟ = Slice;
    ଅନ୍ୟଥା {
      ମୂଲ୍ୟ = ମୂଲ୍ୟ.zext(Shift + 7);
      ମୂଲ୍ୟ.ଭର୍ତ୍ତି_Bits(Slice, Shift);
    }
    Shift += 7;
  } ଯେତେବେଳେ (Byte & 0x80);

  // Sign extend Value to at least 64 bits to match getSLEB128.
  ଯଦି (Shift < 64)
    ମୂଲ୍ୟ = ମୂଲ୍ୟ.sext(64);

  *OffsetPtr = Offset;
  ଫେରନ୍ତୁ APSInt(std::move(ମୂଲ୍ୟ), /*isUnsigned=*/ମିଥ୍ୟା);
}

ଶୂନ୍ୟ DataExtractor::skip(Cursor &C, uint64_t Length) ଅଚଳ {
  ErrorAsOutParameter ErrAsOut(C.Err);
  ଯଦି (isError(&C.Err))
    ଫେରନ୍ତୁ;

  ଯଦି (prepareRead(C.Offset, Length, &C.Err))
    C.Offset += Length;
}
