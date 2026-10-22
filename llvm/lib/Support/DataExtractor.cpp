//===-- DataExtractor.cpp -------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/Support/DataExtractor.h"
#অন্তর্ভাব "llvm/ADT/StringExtras.h"
#অন্তর্ভাব "llvm/Support/Errc.h"
#অন্তর্ভাব "llvm/Support/ErrorHandling.h"
#অন্তর্ভাব "llvm/Support/LEB128.h"
#অন্তর্ভাব "llvm/Support/SwapByteOrder.h"

using নামাকাশ llvm;

তার্কিক DataExtractor::prepareRead(uint64_t Offset, uint64_t Size,
                                Error *E) ধ্রুব {
  যদি (isValidOffsetForDataOfSize(Offset, Size))
    প্রত্যাগম সত্য;
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
  প্রত্যাগম অসত্য;
}

স্থির তার্কিক isError(Error *E) { প্রত্যাগম E && *E; }

template <typename T>
T DataExtractor::getU(uint64_t *offset_ptr, Error *Err) ধ্রুব {
  ErrorAsOutParameter ErrAsOut(Err);
  T val = 0;
  যদি (isError(Err))
    প্রত্যাগম val;

  uint64_t offset = *offset_ptr;
  যদি (!prepareRead(offset, পরিমাণ(T), Err))
    প্রত্যাগম val;
  std::memcpy(&val, &Data.data()[offset], পরিমাণ(val));
  যদি (sys::IsLittleEndianHost != IsLittleEndian)
    sys::swapByteOrder(val);

  // Advance the offset
  *offset_ptr += পরিমাণ(val);
  প্রত্যাগম val;
}

template <typename T>
T *DataExtractor::getUs(uint64_t *offset_ptr, T *dst, uint32_t গুনতি,
                        Error *Err) ধ্রুব {
  ErrorAsOutParameter ErrAsOut(Err);
  যদি (isError(Err))
    প্রত্যাগম nullptr;

  uint64_t offset = *offset_ptr;

  যদি (!prepareRead(offset, পরিমাণ(*dst) * গুনতি, Err))
    প্রত্যাগম nullptr;
  প্রতি (T *মান_ptr = dst, *end = dst + গুনতি; মান_ptr != end;
       ++মান_ptr, offset += পরিমাণ(*dst))
    *মান_ptr = getU<T>(offset_ptr, Err);
  // Advance the offset
  *offset_ptr = offset;
  // Return a non-NULL pointer to the converted data as an indicator of
  // success
  প্রত্যাগম dst;
}

uint8_t DataExtractor::getU8(uint64_t *offset_ptr, llvm::Error *Err) ধ্রুব {
  প্রত্যাগম getU<uint8_t>(offset_ptr, Err);
}

uint8_t *DataExtractor::getU8(uint64_t *offset_ptr, uint8_t *dst,
                              uint32_t গুনতি) ধ্রুব {
  প্রত্যাগম getUs<uint8_t>(offset_ptr, dst, গুনতি, nullptr);
}

uint8_t *DataExtractor::getU8(Cursor &C, uint8_t *Dst, uint32_t গুনতি_2) ধ্রুব {
  প্রত্যাগম getUs<uint8_t>(&C.Offset, Dst, গুনতি_2, &C.Err);
}

uint16_t DataExtractor::getU16(uint64_t *offset_ptr, llvm::Error *Err) ধ্রুব {
  প্রত্যাগম getU<uint16_t>(offset_ptr, Err);
}

uint16_t *DataExtractor::getU16(uint64_t *offset_ptr, uint16_t *dst,
                                uint32_t গুনতি) ধ্রুব {
  প্রত্যাগম getUs<uint16_t>(offset_ptr, dst, গুনতি, nullptr);
}

uint32_t DataExtractor::getU24(uint64_t *OffsetPtr, Error *Err) ধ্রুব {
  uint24_t ExtractedVal = getU<uint24_t>(OffsetPtr, Err);
  // The 3 bytes are in the correct byte order for the host.
  প্রত্যাগম ExtractedVal.getAsUint32(sys::IsLittleEndianHost);
}

uint32_t DataExtractor::getU32(uint64_t *offset_ptr, llvm::Error *Err) ধ্রুব {
  প্রত্যাগম getU<uint32_t>(offset_ptr, Err);
}

uint32_t *DataExtractor::getU32(uint64_t *offset_ptr, uint32_t *dst,
                                uint32_t গুনতি) ধ্রুব {
  প্রত্যাগম getUs<uint32_t>(offset_ptr, dst, গুনতি, nullptr);
}

uint64_t DataExtractor::getU64(uint64_t *offset_ptr, llvm::Error *Err) ধ্রুব {
  প্রত্যাগম getU<uint64_t>(offset_ptr, Err);
}

uint64_t *DataExtractor::getU64(uint64_t *offset_ptr, uint64_t *dst,
                                uint32_t গুনতি) ধ্রুব {
  প্রত্যাগম getUs<uint64_t>(offset_ptr, dst, গুনতি, nullptr);
}

uint64_t DataExtractor::getUnsigned(uint64_t *offset_ptr, uint32_t byte_size,
                                    llvm::Error *Err) ধ্রুব {
  বিকল্প (byte_size) {
  পক্ষ 1:
    প্রত্যাগম getU8(offset_ptr, Err);
  পক্ষ 2:
    প্রত্যাগম getU16(offset_ptr, Err);
  পক্ষ 4:
    প্রত্যাগম getU32(offset_ptr, Err);
  পক্ষ 8:
    প্রত্যাগম getU64(offset_ptr, Err);
  }

  // For any other byte size, read the bytes and swap/shift if necessary.
  ErrorAsOutParameter ErrAsOut(Err);
  uint64_t val = 0;
  যদি (isError(Err))
    প্রত্যাগম val;
  uint64_t offset = *offset_ptr;
  যদি (!prepareRead(offset, byte_size, Err))
    প্রত্যাগম val;
  // Copy into the least significant bytes of val regardless of host
  // endianness.
  std::memcpy(reinterpret_cast<বর্ণ *>(&val) +
                  (sys::IsLittleEndianHost ? 0 : 8 - byte_size),
              &Data.data()[offset], byte_size);
  // Swap the least significant bytes of val if endianness doesn't match.
  যদি (sys::IsLittleEndianHost != IsLittleEndian)
    val = sys::getSwappedBytes(val) >> (8 * (8 - byte_size));

  *offset_ptr += byte_size;
  প্রত্যাগম val;
}

int64_t
DataExtractor::getSigned(uint64_t *offset_ptr, uint32_t byte_size) ধ্রুব {
  বিকল্প (byte_size) {
  পক্ষ 1:
    প্রত্যাগম getS8(offset_ptr);
  পক্ষ 2:
    প্রত্যাগম getS16(offset_ptr);
  পক্ষ 4:
    প্রত্যাগম getS32(offset_ptr);
  পক্ষ 8:
    প্রত্যাগম getS64(offset_ptr);
  }
  llvm_unreachable("getSigned unhandled case!");
}

StringRef DataExtractor::getCStrRef(uint64_t *OffsetPtr, Error *Err) ধ্রুব {
  ErrorAsOutParameter ErrAsOut(Err);
  যদি (isError(Err))
    প্রত্যাগম StringRef();

  uint64_t Start = *OffsetPtr;
  StringRef::size_type Pos = Data.find('\0', Start);
  যদি (Pos != StringRef::npos) {
    *OffsetPtr = Pos + 1;
    প্রত্যাগম StringRef(Data.data() + Start, Pos - Start);
  }
  যদি (Err)
    *Err = createStringError(errc::illegal_byte_sequence,
                             "no null terminated string at offset 0x%" PRIx64,
                             Start);
  প্রত্যাগম StringRef();
}

StringRef DataExtractor::getFixedLengthString(uint64_t *OffsetPtr,
                                              uint64_t Length,
                                              StringRef TrimChars) ধ্রুব {
  StringRef Bytes(getBytes(OffsetPtr, Length));
  প্রত্যাগম Bytes.trim(TrimChars);
}

StringRef DataExtractor::getBytes(uint64_t *OffsetPtr, uint64_t Length,
                                  Error *Err) ধ্রুব {
  ErrorAsOutParameter ErrAsOut(Err);
  যদি (isError(Err))
    প্রত্যাগম StringRef();

  যদি (!prepareRead(*OffsetPtr, Length, Err))
    প্রত্যাগম StringRef();

  StringRef Result = Data.substr(*OffsetPtr, Length);
  *OffsetPtr += Length;
  প্রত্যাগম Result;
}

template <typename T>
স্থির T getLEB128(StringRef Data, uint64_t *OffsetPtr, Error *Err,
                   T (&Decoder)(ধ্রুব uint8_t *p, unsigned *n,
                                ধ্রুব uint8_t *end, ধ্রুব বর্ণ **error)) {
  ArrayRef<uint8_t> Bytes = arrayRefFromStringRef(Data);
  প্রতিজ্ঞা(*OffsetPtr <= Bytes.size());
  ErrorAsOutParameter ErrAsOut(Err);
  যদি (isError(Err))
    প্রত্যাগম T();

  ধ্রুব বর্ণ *error = nullptr;
  unsigned bytes_read;
  T result =
      Decoder(Bytes.data() + *OffsetPtr, &bytes_read, Bytes.end(), &error);
  যদি (error) {
    যদি (Err)
      *Err = createStringError(errc::illegal_byte_sequence,
                               "unable to decode LEB128 at offset 0x%8.8" PRIx64
                               ": %s",
                               *OffsetPtr, error);
    প্রত্যাগম T();
  }
  *OffsetPtr += bytes_read;
  প্রত্যাগম result;
}

uint64_t DataExtractor::getULEB128(uint64_t *offset_ptr, Error *Err) ধ্রুব {
  প্রত্যাগম getLEB128(Data, offset_ptr, Err, decodeULEB128);
}

int64_t DataExtractor::getSLEB128(uint64_t *offset_ptr, Error *Err) ধ্রুব {
  প্রত্যাগম getLEB128(Data, offset_ptr, Err, decodeSLEB128);
}

APSInt DataExtractor::getSLEB128APSInt(uint64_t *OffsetPtr, Error *Err) ধ্রুব {
  ErrorAsOutParameter ErrAsOut(Err);
  যদি (isError(Err))
    প্রত্যাগম APSInt();

  uint64_t Offset = *OffsetPtr;
  APInt মান;
  unsigned Shift = 0;
  uint8_t Byte;
  কুরু {
    যদি (!prepareRead(Offset, 1, Err))
      প্রত্যাগম APSInt();
    Byte = Data[Offset++];
    APInt Slice(7, Byte & 0x7f);
    যদি (Shift == 0)
      মান = Slice;
    অন্যথা {
      মান = মান.zext(Shift + 7);
      মান.ঢোকাও_Bits(Slice, Shift);
    }
    Shift += 7;
  } যাবত্ (Byte & 0x80);

  // Sign extend Value to at least 64 bits to match getSLEB128.
  যদি (Shift < 64)
    মান = মান.sext(64);

  *OffsetPtr = Offset;
  প্রত্যাগম APSInt(std::move(মান), /*isUnsigned=*/অসত্য);
}

শূন্য DataExtractor::skip(Cursor &C, uint64_t Length) ধ্রুব {
  ErrorAsOutParameter ErrAsOut(C.Err);
  যদি (isError(&C.Err))
    প্রত্যাগম;

  যদি (prepareRead(C.Offset, Length, &C.Err))
    C.Offset += Length;
}
