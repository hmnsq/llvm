//===-- DataExtractor.cpp -------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাৱ "llvm/Support/DataExtractor.h"
#অন্তর্ভাৱ "llvm/ADT/StringExtras.h"
#অন্তর্ভাৱ "llvm/Support/Errc.h"
#অন্তর্ভাৱ "llvm/Support/ErrorHandling.h"
#অন্তর্ভাৱ "llvm/Support/LEB128.h"
#অন্তর্ভাৱ "llvm/Support/SwapByteOrder.h"

using নামাকাশ llvm;

তার্কিক DataExtractor::prepareRead(uint64_t Offset, uint64_t Size,
                                Error *E) ধ্রুৱ {
  যদি (isValidOffsetForDataOfSize(Offset, Size))
    ঘূৰাই সঁচা;
  যদি (E) {
    যদি (Offset <= Data.size())
      *E = createStringError(
          errc::illegal_byte_sequence,
          "unexpected end of data at offset 0x%zx while reading [0x%" PRIx64
          ", 0x%" PRIx64 ")",
          Data.size(), Offset, Offset + Size);
    অন্যথা
      *E = createStringError(errc::invalid_argument,
                             "offset 0x%" PRIx64
                             " is beyond the end of data at 0x%zx",
                             Offset, Data.size());
  }
  ঘূৰাই মিছা;
}

স্থির তার্কিক isError(Error *E) { ঘূৰাই E && *E; }

template <typename T>
T DataExtractor::getU(uint64_t *offset_ptr, Error *Err) ধ্রুৱ {
  ErrorAsOutParameter ErrAsOut(Err);
  T val = 0;
  যদি (isError(Err))
    ঘূৰাই val;

  uint64_t offset = *offset_ptr;
  যদি (!prepareRead(offset, পরিমাণ(T), Err))
    ঘূৰাই val;
  std::memcpy(&val, &Data.data()[offset], পরিমাণ(val));
  যদি (sys::IsLittleEndianHost != IsLittleEndian)
    sys::swapByteOrder(val);

  // Advance the offset
  *offset_ptr += পরিমাণ(val);
  ঘূৰাই val;
}

template <typename T>
T *DataExtractor::getUs(uint64_t *offset_ptr, T *dst, uint32_t গুনতি,
                        Error *Err) ধ্রুৱ {
  ErrorAsOutParameter ErrAsOut(Err);
  যদি (isError(Err))
    ঘূৰাই nullptr;

  uint64_t offset = *offset_ptr;

  যদি (!prepareRead(offset, পরিমাণ(*dst) * গুনতি, Err))
    ঘূৰাই nullptr;
  বাবে (T *মান_ptr = dst, *end = dst + গুনতি; মান_ptr != end;
       ++মান_ptr, offset += পরিমাণ(*dst))
    *মান_ptr = getU<T>(offset_ptr, Err);
  // Advance the offset
  *offset_ptr = offset;
  // Return a non-NULL pointer to the converted data as an indicator of
  // success
  ঘূৰাই dst;
}

uint8_t DataExtractor::getU8(uint64_t *offset_ptr, llvm::Error *Err) ধ্রুৱ {
  ঘূৰাই getU<uint8_t>(offset_ptr, Err);
}

uint8_t *DataExtractor::getU8(uint64_t *offset_ptr, uint8_t *dst,
                              uint32_t গুনতি) ধ্রুৱ {
  ঘূৰাই getUs<uint8_t>(offset_ptr, dst, গুনতি, nullptr);
}

uint8_t *DataExtractor::getU8(Cursor &C, uint8_t *Dst, uint32_t গুনতি_2) ধ্রুৱ {
  ঘূৰাই getUs<uint8_t>(&C.Offset, Dst, গুনতি_2, &C.Err);
}

uint16_t DataExtractor::getU16(uint64_t *offset_ptr, llvm::Error *Err) ধ্রুৱ {
  ঘূৰাই getU<uint16_t>(offset_ptr, Err);
}

uint16_t *DataExtractor::getU16(uint64_t *offset_ptr, uint16_t *dst,
                                uint32_t গুনতি) ধ্রুৱ {
  ঘূৰাই getUs<uint16_t>(offset_ptr, dst, গুনতি, nullptr);
}

uint32_t DataExtractor::getU24(uint64_t *OffsetPtr, Error *Err) ধ্রুৱ {
  uint24_t ExtractedVal = getU<uint24_t>(OffsetPtr, Err);
  // The 3 bytes are in the correct byte order for the host.
  ঘূৰাই ExtractedVal.getAsUint32(sys::IsLittleEndianHost);
}

uint32_t DataExtractor::getU32(uint64_t *offset_ptr, llvm::Error *Err) ধ্রুৱ {
  ঘূৰাই getU<uint32_t>(offset_ptr, Err);
}

uint32_t *DataExtractor::getU32(uint64_t *offset_ptr, uint32_t *dst,
                                uint32_t গুনতি) ধ্রুৱ {
  ঘূৰাই getUs<uint32_t>(offset_ptr, dst, গুনতি, nullptr);
}

uint64_t DataExtractor::getU64(uint64_t *offset_ptr, llvm::Error *Err) ধ্রুৱ {
  ঘূৰাই getU<uint64_t>(offset_ptr, Err);
}

uint64_t *DataExtractor::getU64(uint64_t *offset_ptr, uint64_t *dst,
                                uint32_t গুনতি) ধ্রুৱ {
  ঘূৰাই getUs<uint64_t>(offset_ptr, dst, গুনতি, nullptr);
}

uint64_t DataExtractor::getUnsigned(uint64_t *offset_ptr, uint32_t byte_size,
                                    llvm::Error *Err) ধ্রুৱ {
  ৱিকল্প (byte_size) {
  পক্ষ 1:
    ঘূৰাই getU8(offset_ptr, Err);
  পক্ষ 2:
    ঘূৰাই getU16(offset_ptr, Err);
  পক্ষ 4:
    ঘূৰাই getU32(offset_ptr, Err);
  পক্ষ 8:
    ঘূৰাই getU64(offset_ptr, Err);
  }

  // For any other byte size, read the bytes and swap/shift if necessary.
  ErrorAsOutParameter ErrAsOut(Err);
  uint64_t val = 0;
  যদি (isError(Err))
    ঘূৰাই val;
  uint64_t offset = *offset_ptr;
  যদি (!prepareRead(offset, byte_size, Err))
    ঘূৰাই val;
  // Copy into the least significant bytes of val regardless of host
  // endianness.
  std::memcpy(reinterpret_cast<ৱর্ণ *>(&val) +
                  (sys::IsLittleEndianHost ? 0 : 8 - byte_size),
              &Data.data()[offset], byte_size);
  // Swap the least significant bytes of val if endianness doesn't match.
  যদি (sys::IsLittleEndianHost != IsLittleEndian)
    val = sys::getSwappedBytes(val) >> (8 * (8 - byte_size));

  *offset_ptr += byte_size;
  ঘূৰাই val;
}

int64_t
DataExtractor::getSigned(uint64_t *offset_ptr, uint32_t byte_size) ধ্রুৱ {
  ৱিকল্প (byte_size) {
  পক্ষ 1:
    ঘূৰাই getS8(offset_ptr);
  পক্ষ 2:
    ঘূৰাই getS16(offset_ptr);
  পক্ষ 4:
    ঘূৰাই getS32(offset_ptr);
  পক্ষ 8:
    ঘূৰাই getS64(offset_ptr);
  }
  llvm_unreachable("getSigned unhandled case!");
}

StringRef DataExtractor::getCStrRef(uint64_t *OffsetPtr, Error *Err) ধ্রুৱ {
  ErrorAsOutParameter ErrAsOut(Err);
  যদি (isError(Err))
    ঘূৰাই StringRef();

  uint64_t Start = *OffsetPtr;
  StringRef::size_type Pos = Data.find('\0', Start);
  যদি (Pos != StringRef::npos) {
    *OffsetPtr = Pos + 1;
    ঘূৰাই StringRef(Data.data() + Start, Pos - Start);
  }
  যদি (Err)
    *Err = createStringError(errc::illegal_byte_sequence,
                             "no null terminated string at offset 0x%" PRIx64,
                             Start);
  ঘূৰাই StringRef();
}

StringRef DataExtractor::getFixedLengthString(uint64_t *OffsetPtr,
                                              uint64_t Length,
                                              StringRef TrimChars) ধ্রুৱ {
  StringRef Bytes(getBytes(OffsetPtr, Length));
  ঘূৰাই Bytes.trim(TrimChars);
}

StringRef DataExtractor::getBytes(uint64_t *OffsetPtr, uint64_t Length,
                                  Error *Err) ধ্রুৱ {
  ErrorAsOutParameter ErrAsOut(Err);
  যদি (isError(Err))
    ঘূৰাই StringRef();

  যদি (!prepareRead(*OffsetPtr, Length, Err))
    ঘূৰাই StringRef();

  StringRef Result = Data.substr(*OffsetPtr, Length);
  *OffsetPtr += Length;
  ঘূৰাই Result;
}

template <typename T>
স্থির T getLEB128(StringRef Data, uint64_t *OffsetPtr, Error *Err,
                   T (&Decoder)(ধ্রুৱ uint8_t *p, unsigned *n,
                                ধ্রুৱ uint8_t *end, ধ্রুৱ ৱর্ণ **error)) {
  ArrayRef<uint8_t> Bytes = arrayRefFromStringRef(Data);
  প্রতিজ্ঞা(*OffsetPtr <= Bytes.size());
  ErrorAsOutParameter ErrAsOut(Err);
  যদি (isError(Err))
    ঘূৰাই T();

  ধ্রুৱ ৱর্ণ *error = nullptr;
  unsigned bytes_read;
  T result =
      Decoder(Bytes.data() + *OffsetPtr, &bytes_read, Bytes.end(), &error);
  যদি (error) {
    যদি (Err)
      *Err = createStringError(errc::illegal_byte_sequence,
                               "unable to decode LEB128 at offset 0x%8.8" PRIx64
                               ": %s",
                               *OffsetPtr, error);
    ঘূৰাই T();
  }
  *OffsetPtr += bytes_read;
  ঘূৰাই result;
}

uint64_t DataExtractor::getULEB128(uint64_t *offset_ptr, Error *Err) ধ্রুৱ {
  ঘূৰাই getLEB128(Data, offset_ptr, Err, decodeULEB128);
}

int64_t DataExtractor::getSLEB128(uint64_t *offset_ptr, Error *Err) ধ্রুৱ {
  ঘূৰাই getLEB128(Data, offset_ptr, Err, decodeSLEB128);
}

APSInt DataExtractor::getSLEB128APSInt(uint64_t *OffsetPtr, Error *Err) ধ্রুৱ {
  ErrorAsOutParameter ErrAsOut(Err);
  যদি (isError(Err))
    ঘূৰাই APSInt();

  uint64_t Offset = *OffsetPtr;
  APInt মান;
  unsigned Shift = 0;
  uint8_t Byte;
  কৰা {
    যদি (!prepareRead(Offset, 1, Err))
      ঘূৰাই APSInt();
    Byte = Data[Offset++];
    APInt Slice(7, Byte & 0x7f);
    যদি (Shift == 0)
      মান = Slice;
    অন্যথা {
      মান = মান.zext(Shift + 7);
      মান.ঢোকাও_Bits(Slice, Shift);
    }
    Shift += 7;
  } যেতিয়ালৈ (Byte & 0x80);

  // Sign extend Value to at least 64 bits to match getSLEB128.
  যদি (Shift < 64)
    মান = মান.sext(64);

  *OffsetPtr = Offset;
  ঘূৰাই APSInt(std::move(মান), /*isUnsigned=*/মিছা);
}

শূন্য DataExtractor::skip(Cursor &C, uint64_t Length) ধ্রুৱ {
  ErrorAsOutParameter ErrAsOut(C.Err);
  যদি (isError(&C.Err))
    ঘূৰাই;

  যদি (prepareRead(C.Offset, Length, &C.Err))
    C.Offset += Length;
}
