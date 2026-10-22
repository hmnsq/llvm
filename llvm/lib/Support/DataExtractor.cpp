//===-- DataExtractor.cpp -------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/Support/DataExtractor.h"
#അന്തര്ഭാവ "llvm/ADT/StringExtras.h"
#അന്തര്ഭാവ "llvm/Support/Errc.h"
#അന്തര്ഭാവ "llvm/Support/ErrorHandling.h"
#അന്തര്ഭാവ "llvm/Support/LEB128.h"
#അന്തര്ഭാവ "llvm/Support/SwapByteOrder.h"

using നാമാകാശ llvm;

തർക്കം DataExtractor::prepareRead(uint64_t Offset, uint64_t Size,
                                Error *E) സ്ഥിരം {
  എങ്കിൽ (isValidOffsetForDataOfSize(Offset, Size))
    തിരികെ സത്യം;
  എങ്കിൽ (E) {
    എങ്കിൽ (Offset <= Data.size())
      *E = createStringError(
          errc::illegal_byte_sequence,
          "unexpected end of data at offset 0x%zx while reading [0x%" PRIx64
          ", 0x%" PRIx64 ")",
          Data.size(), Offset, Offset + Size);
    അല്ലെങ്കിൽ
      *E = createStringError(errc::invalid_argument,
                             "offset 0x%" PRIx64
                             " is beyond the end of data at 0x%zx",
                             Offset, Data.size());
  }
  തിരികെ അസത്യം;
}

സ്ഥിതം തർക്കം isError(Error *E) { തിരികെ E && *E; }

template <typename T>
T DataExtractor::getU(uint64_t *offset_ptr, Error *Err) സ്ഥിരം {
  ErrorAsOutParameter ErrAsOut(Err);
  T val = 0;
  എങ്കിൽ (isError(Err))
    തിരികെ val;

  uint64_t offset = *offset_ptr;
  എങ്കിൽ (!prepareRead(offset, വലുപ്പം(T), Err))
    തിരികെ val;
  std::memcpy(&val, &Data.data()[offset], വലുപ്പം(val));
  എങ്കിൽ (sys::IsLittleEndianHost != IsLittleEndian)
    sys::swapByteOrder(val);

  // Advance the offset
  *offset_ptr += വലുപ്പം(val);
  തിരികെ val;
}

template <typename T>
T *DataExtractor::getUs(uint64_t *offset_ptr, T *dst, uint32_t എണ്ണം,
                        Error *Err) സ്ഥിരം {
  ErrorAsOutParameter ErrAsOut(Err);
  എങ്കിൽ (isError(Err))
    തിരികെ nullptr;

  uint64_t offset = *offset_ptr;

  എങ്കിൽ (!prepareRead(offset, വലുപ്പം(*dst) * എണ്ണം, Err))
    തിരികെ nullptr;
  വേണ്ടി (T *മൂല്യം_ptr = dst, *end = dst + എണ്ണം; മൂല്യം_ptr != end;
       ++മൂല്യം_ptr, offset += വലുപ്പം(*dst))
    *മൂല്യം_ptr = getU<T>(offset_ptr, Err);
  // Advance the offset
  *offset_ptr = offset;
  // Return a non-NULL pointer to the converted data as an indicator of
  // success
  തിരികെ dst;
}

uint8_t DataExtractor::getU8(uint64_t *offset_ptr, llvm::Error *Err) സ്ഥിരം {
  തിരികെ getU<uint8_t>(offset_ptr, Err);
}

uint8_t *DataExtractor::getU8(uint64_t *offset_ptr, uint8_t *dst,
                              uint32_t എണ്ണം) സ്ഥിരം {
  തിരികെ getUs<uint8_t>(offset_ptr, dst, എണ്ണം, nullptr);
}

uint8_t *DataExtractor::getU8(Cursor &C, uint8_t *Dst, uint32_t എണ്ണം_2) സ്ഥിരം {
  തിരികെ getUs<uint8_t>(&C.Offset, Dst, എണ്ണം_2, &C.Err);
}

uint16_t DataExtractor::getU16(uint64_t *offset_ptr, llvm::Error *Err) സ്ഥിരം {
  തിരികെ getU<uint16_t>(offset_ptr, Err);
}

uint16_t *DataExtractor::getU16(uint64_t *offset_ptr, uint16_t *dst,
                                uint32_t എണ്ണം) സ്ഥിരം {
  തിരികെ getUs<uint16_t>(offset_ptr, dst, എണ്ണം, nullptr);
}

uint32_t DataExtractor::getU24(uint64_t *OffsetPtr, Error *Err) സ്ഥിരം {
  uint24_t ExtractedVal = getU<uint24_t>(OffsetPtr, Err);
  // The 3 bytes are in the correct byte order for the host.
  തിരികെ ExtractedVal.getAsUint32(sys::IsLittleEndianHost);
}

uint32_t DataExtractor::getU32(uint64_t *offset_ptr, llvm::Error *Err) സ്ഥിരം {
  തിരികെ getU<uint32_t>(offset_ptr, Err);
}

uint32_t *DataExtractor::getU32(uint64_t *offset_ptr, uint32_t *dst,
                                uint32_t എണ്ണം) സ്ഥിരം {
  തിരികെ getUs<uint32_t>(offset_ptr, dst, എണ്ണം, nullptr);
}

uint64_t DataExtractor::getU64(uint64_t *offset_ptr, llvm::Error *Err) സ്ഥിരം {
  തിരികെ getU<uint64_t>(offset_ptr, Err);
}

uint64_t *DataExtractor::getU64(uint64_t *offset_ptr, uint64_t *dst,
                                uint32_t എണ്ണം) സ്ഥിരം {
  തിരികെ getUs<uint64_t>(offset_ptr, dst, എണ്ണം, nullptr);
}

uint64_t DataExtractor::getUnsigned(uint64_t *offset_ptr, uint32_t byte_size,
                                    llvm::Error *Err) സ്ഥിരം {
  തിരഞ്ഞെടുപ്പ് (byte_size) {
  അവസ്ഥ 1:
    തിരികെ getU8(offset_ptr, Err);
  അവസ്ഥ 2:
    തിരികെ getU16(offset_ptr, Err);
  അവസ്ഥ 4:
    തിരികെ getU32(offset_ptr, Err);
  അവസ്ഥ 8:
    തിരികെ getU64(offset_ptr, Err);
  }

  // For any other byte size, read the bytes and swap/shift if necessary.
  ErrorAsOutParameter ErrAsOut(Err);
  uint64_t val = 0;
  എങ്കിൽ (isError(Err))
    തിരികെ val;
  uint64_t offset = *offset_ptr;
  എങ്കിൽ (!prepareRead(offset, byte_size, Err))
    തിരികെ val;
  // Copy into the least significant bytes of val regardless of host
  // endianness.
  std::memcpy(reinterpret_cast<അക്ഷരം *>(&val) +
                  (sys::IsLittleEndianHost ? 0 : 8 - byte_size),
              &Data.data()[offset], byte_size);
  // Swap the least significant bytes of val if endianness doesn't match.
  എങ്കിൽ (sys::IsLittleEndianHost != IsLittleEndian)
    val = sys::getSwappedBytes(val) >> (8 * (8 - byte_size));

  *offset_ptr += byte_size;
  തിരികെ val;
}

int64_t
DataExtractor::getSigned(uint64_t *offset_ptr, uint32_t byte_size) സ്ഥിരം {
  തിരഞ്ഞെടുപ്പ് (byte_size) {
  അവസ്ഥ 1:
    തിരികെ getS8(offset_ptr);
  അവസ്ഥ 2:
    തിരികെ getS16(offset_ptr);
  അവസ്ഥ 4:
    തിരികെ getS32(offset_ptr);
  അവസ്ഥ 8:
    തിരികെ getS64(offset_ptr);
  }
  llvm_unreachable("getSigned unhandled case!");
}

StringRef DataExtractor::getCStrRef(uint64_t *OffsetPtr, Error *Err) സ്ഥിരം {
  ErrorAsOutParameter ErrAsOut(Err);
  എങ്കിൽ (isError(Err))
    തിരികെ StringRef();

  uint64_t Start = *OffsetPtr;
  StringRef::size_type Pos = Data.find('\0', Start);
  എങ്കിൽ (Pos != StringRef::npos) {
    *OffsetPtr = Pos + 1;
    തിരികെ StringRef(Data.data() + Start, Pos - Start);
  }
  എങ്കിൽ (Err)
    *Err = createStringError(errc::illegal_byte_sequence,
                             "no null terminated string at offset 0x%" PRIx64,
                             Start);
  തിരികെ StringRef();
}

StringRef DataExtractor::getFixedLengthString(uint64_t *OffsetPtr,
                                              uint64_t Length,
                                              StringRef TrimChars) സ്ഥിരം {
  StringRef Bytes(getBytes(OffsetPtr, Length));
  തിരികെ Bytes.trim(TrimChars);
}

StringRef DataExtractor::getBytes(uint64_t *OffsetPtr, uint64_t Length,
                                  Error *Err) സ്ഥിരം {
  ErrorAsOutParameter ErrAsOut(Err);
  എങ്കിൽ (isError(Err))
    തിരികെ StringRef();

  എങ്കിൽ (!prepareRead(*OffsetPtr, Length, Err))
    തിരികെ StringRef();

  StringRef Result = Data.substr(*OffsetPtr, Length);
  *OffsetPtr += Length;
  തിരികെ Result;
}

template <typename T>
സ്ഥിതം T getLEB128(StringRef Data, uint64_t *OffsetPtr, Error *Err,
                   T (&Decoder)(സ്ഥിരം uint8_t *p, unsigned *n,
                                സ്ഥിരം uint8_t *end, സ്ഥിരം അക്ഷരം **error)) {
  ArrayRef<uint8_t> Bytes = arrayRefFromStringRef(Data);
  പ്രതിജ്ഞാ(*OffsetPtr <= Bytes.size());
  ErrorAsOutParameter ErrAsOut(Err);
  എങ്കിൽ (isError(Err))
    തിരികെ T();

  സ്ഥിരം അക്ഷരം *error = nullptr;
  unsigned bytes_read;
  T result =
      Decoder(Bytes.data() + *OffsetPtr, &bytes_read, Bytes.end(), &error);
  എങ്കിൽ (error) {
    എങ്കിൽ (Err)
      *Err = createStringError(errc::illegal_byte_sequence,
                               "unable to decode LEB128 at offset 0x%8.8" PRIx64
                               ": %s",
                               *OffsetPtr, error);
    തിരികെ T();
  }
  *OffsetPtr += bytes_read;
  തിരികെ result;
}

uint64_t DataExtractor::getULEB128(uint64_t *offset_ptr, Error *Err) സ്ഥിരം {
  തിരികെ getLEB128(Data, offset_ptr, Err, decodeULEB128);
}

int64_t DataExtractor::getSLEB128(uint64_t *offset_ptr, Error *Err) സ്ഥിരം {
  തിരികെ getLEB128(Data, offset_ptr, Err, decodeSLEB128);
}

APSInt DataExtractor::getSLEB128APSInt(uint64_t *OffsetPtr, Error *Err) സ്ഥിരം {
  ErrorAsOutParameter ErrAsOut(Err);
  എങ്കിൽ (isError(Err))
    തിരികെ APSInt();

  uint64_t Offset = *OffsetPtr;
  APInt മൂല്യം;
  unsigned Shift = 0;
  uint8_t Byte;
  ചെയ്യുക {
    എങ്കിൽ (!prepareRead(Offset, 1, Err))
      തിരികെ APSInt();
    Byte = Data[Offset++];
    APInt Slice(7, Byte & 0x7f);
    എങ്കിൽ (Shift == 0)
      മൂല്യം = Slice;
    അല്ലെങ്കിൽ {
      മൂല്യം = മൂല്യം.zext(Shift + 7);
      മൂല്യം.തിരുകുക_Bits(Slice, Shift);
    }
    Shift += 7;
  } ആകുമ്പോൾ (Byte & 0x80);

  // Sign extend Value to at least 64 bits to match getSLEB128.
  എങ്കിൽ (Shift < 64)
    മൂല്യം = മൂല്യം.sext(64);

  *OffsetPtr = Offset;
  തിരികെ APSInt(std::move(മൂല്യം), /*isUnsigned=*/അസത്യം);
}

ശൂന്യം DataExtractor::skip(Cursor &C, uint64_t Length) സ്ഥിരം {
  ErrorAsOutParameter ErrAsOut(C.Err);
  എങ്കിൽ (isError(&C.Err))
    തിരികെ;

  എങ്കിൽ (prepareRead(C.Offset, Length, &C.Err))
    C.Offset += Length;
}
