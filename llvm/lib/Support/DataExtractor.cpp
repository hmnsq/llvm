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

যৌক্তিক তথ্য_Extractor::prepareRead(uint64_t Offset, uint64_t আকার_2,
                                Error *E) ধ্রুবক {
  যদি (is_Valid_Offset_For_তথ্য_Of_আকার(Offset, আকার_2))
    ফেরত সত্য;
  যদি (E) {
    যদি (Offset <= তথ্য_2.আকার_3())
      *E = createStringError(
          errc::illegal_byte_sequence,
          "unexpected end of data at offset 0x%zx while reading [0x%" PRIx64
          ", 0x%" PRIx64 ")",
          তথ্য_2.আকার_3(), Offset, Offset + আকার_2);
    অন্যথা
      *E = createStringError(errc::invalid_argument,
                             "offset 0x%" PRIx64
                             " is beyond the end of data at 0x%zx",
                             Offset, তথ্য_2.আকার_3());
  }
  ফেরত মিথ্যা;
}

স্থির যৌক্তিক isError(Error *E) { ফেরত E && *E; }

template <typename T>
T তথ্য_Extractor::getU(uint64_t *offset_ptr, Error *Err) ধ্রুবক {
  ErrorAsOutParameter ErrAsOut(Err);
  T val = 0;
  যদি (isError(Err))
    ফেরত val;

  uint64_t offset = *offset_ptr;
  যদি (!prepareRead(offset, আকার(T), Err))
    ফেরত val;
  std::memcpy(&val, &তথ্য_2.তথ্য()[offset], আকার(val));
  যদি (sys::IsLittleEndianHost != IsLittleEndian)
    sys::swapByteOrder(val);

  // Advance the offset
  *offset_ptr += আকার(val);
  ফেরত val;
}

template <typename T>
T *তথ্য_Extractor::getUs(uint64_t *offset_ptr, T *dst, uint32_t গুনতি,
                        Error *Err) ধ্রুবক {
  ErrorAsOutParameter ErrAsOut(Err);
  যদি (isError(Err))
    ফেরত nullptr;

  uint64_t offset = *offset_ptr;

  যদি (!prepareRead(offset, আকার(*dst) * গুনতি, Err))
    ফেরত nullptr;
  জন্য (T *মান_ptr = dst, *end = dst + গুনতি; মান_ptr != end;
       ++মান_ptr, offset += আকার(*dst))
    *মান_ptr = getU<T>(offset_ptr, Err);
  // Advance the offset
  *offset_ptr = offset;
  // Return a non-NULL pointer to the converted data as an indicator of
  // success
  ফেরত dst;
}

uint8_t তথ্য_Extractor::getU8(uint64_t *offset_ptr, llvm::Error *Err) ধ্রুবক {
  ফেরত getU<uint8_t>(offset_ptr, Err);
}

uint8_t *তথ্য_Extractor::getU8(uint64_t *offset_ptr, uint8_t *dst,
                              uint32_t গুনতি) ধ্রুবক {
  ফেরত getUs<uint8_t>(offset_ptr, dst, গুনতি, nullptr);
}

uint8_t *তথ্য_Extractor::getU8(Cursor &C, uint8_t *Dst, uint32_t গুনতি_2) ধ্রুবক {
  ফেরত getUs<uint8_t>(&C.Offset, Dst, গুনতি_2, &C.Err);
}

uint16_t তথ্য_Extractor::getU16(uint64_t *offset_ptr, llvm::Error *Err) ধ্রুবক {
  ফেরত getU<uint16_t>(offset_ptr, Err);
}

uint16_t *তথ্য_Extractor::getU16(uint64_t *offset_ptr, uint16_t *dst,
                                uint32_t গুনতি) ধ্রুবক {
  ফেরত getUs<uint16_t>(offset_ptr, dst, গুনতি, nullptr);
}

uint32_t তথ্য_Extractor::getU24(uint64_t *OffsetPtr, Error *Err) ধ্রুবক {
  uint24_t ExtractedVal = getU<uint24_t>(OffsetPtr, Err);
  // The 3 bytes are in the correct byte order for the host.
  ফেরত ExtractedVal.getAsUint32(sys::IsLittleEndianHost);
}

uint32_t তথ্য_Extractor::getU32(uint64_t *offset_ptr, llvm::Error *Err) ধ্রুবক {
  ফেরত getU<uint32_t>(offset_ptr, Err);
}

uint32_t *তথ্য_Extractor::getU32(uint64_t *offset_ptr, uint32_t *dst,
                                uint32_t গুনতি) ধ্রুবক {
  ফেরত getUs<uint32_t>(offset_ptr, dst, গুনতি, nullptr);
}

uint64_t তথ্য_Extractor::getU64(uint64_t *offset_ptr, llvm::Error *Err) ধ্রুবক {
  ফেরত getU<uint64_t>(offset_ptr, Err);
}

uint64_t *তথ্য_Extractor::getU64(uint64_t *offset_ptr, uint64_t *dst,
                                uint32_t গুনতি) ধ্রুবক {
  ফেরত getUs<uint64_t>(offset_ptr, dst, গুনতি, nullptr);
}

uint64_t তথ্য_Extractor::getUnsigned(uint64_t *offset_ptr, uint32_t byte_আকার,
                                    llvm::Error *Err) ধ্রুবক {
  নির্বাচন (byte_আকার) {
  ক্ষেত্র 1:
    ফেরত getU8(offset_ptr, Err);
  ক্ষেত্র 2:
    ফেরত getU16(offset_ptr, Err);
  ক্ষেত্র 4:
    ফেরত getU32(offset_ptr, Err);
  ক্ষেত্র 8:
    ফেরত getU64(offset_ptr, Err);
  }

  // For any other byte size, read the bytes and swap/shift if necessary.
  ErrorAsOutParameter ErrAsOut(Err);
  uint64_t val = 0;
  যদি (isError(Err))
    ফেরত val;
  uint64_t offset = *offset_ptr;
  যদি (!prepareRead(offset, byte_আকার, Err))
    ফেরত val;
  // Copy into the least significant bytes of val regardless of host
  // endianness.
  std::memcpy(reinterpret_cast<অক্ষর *>(&val) +
                  (sys::IsLittleEndianHost ? 0 : 8 - byte_আকার),
              &তথ্য_2.তথ্য()[offset], byte_আকার);
  // Swap the least significant bytes of val if endianness doesn't match.
  যদি (sys::IsLittleEndianHost != IsLittleEndian)
    val = sys::getSwappedBytes(val) >> (8 * (8 - byte_আকার));

  *offset_ptr += byte_আকার;
  ফেরত val;
}

int64_t
তথ্য_Extractor::getSigned(uint64_t *offset_ptr, uint32_t byte_আকার) ধ্রুবক {
  নির্বাচন (byte_আকার) {
  ক্ষেত্র 1:
    ফেরত getS8(offset_ptr);
  ক্ষেত্র 2:
    ফেরত getS16(offset_ptr);
  ক্ষেত্র 4:
    ফেরত getS32(offset_ptr);
  ক্ষেত্র 8:
    ফেরত getS64(offset_ptr);
  }
  llvm_unreachable("getSigned unhandled case!");
}

StringRef তথ্য_Extractor::getCStrRef(uint64_t *OffsetPtr, Error *Err) ধ্রুবক {
  ErrorAsOutParameter ErrAsOut(Err);
  যদি (isError(Err))
    ফেরত StringRef();

  uint64_t Start = *OffsetPtr;
  StringRef::আকার_type Pos = তথ্য_2.find('\0', Start);
  যদি (Pos != StringRef::npos) {
    *OffsetPtr = Pos + 1;
    ফেরত StringRef(তথ্য_2.তথ্য() + Start, Pos - Start);
  }
  যদি (Err)
    *Err = createStringError(errc::illegal_byte_sequence,
                             "no null terminated string at offset 0x%" PRIx64,
                             Start);
  ফেরত StringRef();
}

StringRef তথ্য_Extractor::get_Fixed_দৈর্ঘ্য_String(uint64_t *OffsetPtr,
                                              uint64_t দৈর্ঘ্য,
                                              StringRef TrimChars) ধ্রুবক {
  StringRef Bytes(getBytes(OffsetPtr, দৈর্ঘ্য));
  ফেরত Bytes.trim(TrimChars);
}

StringRef তথ্য_Extractor::getBytes(uint64_t *OffsetPtr, uint64_t দৈর্ঘ্য,
                                  Error *Err) ধ্রুবক {
  ErrorAsOutParameter ErrAsOut(Err);
  যদি (isError(Err))
    ফেরত StringRef();

  যদি (!prepareRead(*OffsetPtr, দৈর্ঘ্য, Err))
    ফেরত StringRef();

  StringRef ফলাফল = তথ্য_2.substr(*OffsetPtr, দৈর্ঘ্য);
  *OffsetPtr += দৈর্ঘ্য;
  ফেরত ফলাফল;
}

template <typename T>
স্থির T getLEB128(StringRef তথ্য_2, uint64_t *OffsetPtr, Error *Err,
                   T (&Decoder)(ধ্রুবক uint8_t *p, unsigned *n,
                                ধ্রুবক uint8_t *end, ধ্রুবক অক্ষর **error)) {
  ArrayRef<uint8_t> Bytes = arrayRefFromStringRef(তথ্য_2);
  প্রতিজ্ঞা(*OffsetPtr <= Bytes.আকার_3());
  ErrorAsOutParameter ErrAsOut(Err);
  যদি (isError(Err))
    ফেরত T();

  ধ্রুবক অক্ষর *error = nullptr;
  unsigned bytes_read;
  T ফলাফল_2 =
      Decoder(Bytes.তথ্য() + *OffsetPtr, &bytes_read, Bytes.end(), &error);
  যদি (error) {
    যদি (Err)
      *Err = createStringError(errc::illegal_byte_sequence,
                               "unable to decode LEB128 at offset 0x%8.8" PRIx64
                               ": %s",
                               *OffsetPtr, error);
    ফেরত T();
  }
  *OffsetPtr += bytes_read;
  ফেরত ফলাফল_2;
}

uint64_t তথ্য_Extractor::getULEB128(uint64_t *offset_ptr, Error *Err) ধ্রুবক {
  ফেরত getLEB128(তথ্য_2, offset_ptr, Err, decodeULEB128);
}

int64_t তথ্য_Extractor::getSLEB128(uint64_t *offset_ptr, Error *Err) ধ্রুবক {
  ফেরত getLEB128(তথ্য_2, offset_ptr, Err, decodeSLEB128);
}

APSInt তথ্য_Extractor::getSLEB128APSInt(uint64_t *OffsetPtr, Error *Err) ধ্রুবক {
  ErrorAsOutParameter ErrAsOut(Err);
  যদি (isError(Err))
    ফেরত APSInt();

  uint64_t Offset = *OffsetPtr;
  APInt মান;
  unsigned Shift = 0;
  uint8_t Byte;
  করো {
    যদি (!prepareRead(Offset, 1, Err))
      ফেরত APSInt();
    Byte = তথ্য_2[Offset++];
    APInt Slice(7, Byte & 0x7f);
    যদি (Shift == 0)
      মান = Slice;
    অন্যথা {
      মান = মান.zext(Shift + 7);
      মান.ঢোকাও_Bits(Slice, Shift);
    }
    Shift += 7;
  } যখন (Byte & 0x80);

  // Sign extend Value to at least 64 bits to match getSLEB128.
  যদি (Shift < 64)
    মান = মান.sext(64);

  *OffsetPtr = Offset;
  ফেরত APSInt(std::move(মান), /*isUnsigned=*/মিথ্যা);
}

শূন্য তথ্য_Extractor::skip(Cursor &C, uint64_t দৈর্ঘ্য) ধ্রুবক {
  ErrorAsOutParameter ErrAsOut(C.Err);
  যদি (isError(&C.Err))
    ফেরত;

  যদি (prepareRead(C.Offset, দৈর্ঘ্য, &C.Err))
    C.Offset += দৈর্ঘ্য;
}
