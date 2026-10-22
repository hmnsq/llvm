//===-- DataExtractor.cpp -------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/DataExtractor.h"
#समावेश "llvm/ADT/StringExtras.h"
#समावेश "llvm/Support/Errc.h"
#समावेश "llvm/Support/ErrorHandling.h"
#समावेश "llvm/Support/LEB128.h"
#समावेश "llvm/Support/SwapByteOrder.h"

using namespace llvm;

bool तथ्य_Extractor::prepareRead(uint64_t Offset, uint64_t आकार,
                                Error *E) स्थिर {
  अगर (is_Valid_Offset_For_तथ्य_Of_आकार(Offset, आकार))
    वापस true;
  अगर (E) {
    अगर (Offset <= तथ्य_2.आकार_2())
      *E = createStringError(
          errc::illegal_byte_sequence,
          "unexpected end of data at offset 0x%zx while reading [0x%" PRIx64
          ", 0x%" PRIx64 ")",
          तथ्य_2.आकार_2(), Offset, Offset + आकार);
    अन्यथा
      *E = createStringError(errc::invalid_argument,
                             "offset 0x%" PRIx64
                             " is beyond the end of data at 0x%zx",
                             Offset, तथ्य_2.आकार_2());
  }
  वापस false;
}

अटल bool isError(Error *E) { वापस E && *E; }

ढाँचा <typename T>
T तथ्य_Extractor::getU(uint64_t *offset_ptr, Error *Err) स्थिर {
  ErrorAsOutParameter ErrAsOut(Err);
  T val = 0;
  अगर (isError(Err))
    वापस val;

  uint64_t offset = *offset_ptr;
  अगर (!prepareRead(offset, माप(T), Err))
    वापस val;
  std::स_नकल(&val, &तथ्य_2.तथ्य()[offset], माप(val));
  अगर (sys::IsLittleEndianHost != IsLittleEndian)
    sys::swapByteOrder(val);

  // Advance the offset
  *offset_ptr += माप(val);
  वापस val;
}

ढाँचा <typename T>
T *तथ्य_Extractor::getUs(uint64_t *offset_ptr, T *dst, uint32_t गिनती,
                        Error *Err) स्थिर {
  ErrorAsOutParameter ErrAsOut(Err);
  अगर (isError(Err))
    वापस nullptr;

  uint64_t offset = *offset_ptr;

  अगर (!prepareRead(offset, माप(*dst) * गिनती, Err))
    वापस nullptr;
  क्रम (T *मूल्य_ptr = dst, *end = dst + गिनती; मूल्य_ptr != end;
       ++मूल्य_ptr, offset += माप(*dst))
    *मूल्य_ptr = getU<T>(offset_ptr, Err);
  // Advance the offset
  *offset_ptr = offset;
  // Return a non-NULL pointer to the converted data as an indicator of
  // success
  वापस dst;
}

uint8_t तथ्य_Extractor::getU8(uint64_t *offset_ptr, llvm::Error *Err) स्थिर {
  वापस getU<uint8_t>(offset_ptr, Err);
}

uint8_t *तथ्य_Extractor::getU8(uint64_t *offset_ptr, uint8_t *dst,
                              uint32_t गिनती) स्थिर {
  वापस getUs<uint8_t>(offset_ptr, dst, गिनती, nullptr);
}

uint8_t *तथ्य_Extractor::getU8(Cursor &C, uint8_t *Dst, uint32_t गिनती_2) स्थिर {
  वापस getUs<uint8_t>(&C.Offset, Dst, गिनती_2, &C.Err);
}

uint16_t तथ्य_Extractor::getU16(uint64_t *offset_ptr, llvm::Error *Err) स्थिर {
  वापस getU<uint16_t>(offset_ptr, Err);
}

uint16_t *तथ्य_Extractor::getU16(uint64_t *offset_ptr, uint16_t *dst,
                                uint32_t गिनती) स्थिर {
  वापस getUs<uint16_t>(offset_ptr, dst, गिनती, nullptr);
}

uint32_t तथ्य_Extractor::getU24(uint64_t *OffsetPtr, Error *Err) स्थिर {
  uint24_t ExtractedVal = getU<uint24_t>(OffsetPtr, Err);
  // The 3 bytes are in the correct byte order for the host.
  वापस ExtractedVal.getAsUint32(sys::IsLittleEndianHost);
}

uint32_t तथ्य_Extractor::getU32(uint64_t *offset_ptr, llvm::Error *Err) स्थिर {
  वापस getU<uint32_t>(offset_ptr, Err);
}

uint32_t *तथ्य_Extractor::getU32(uint64_t *offset_ptr, uint32_t *dst,
                                uint32_t गिनती) स्थिर {
  वापस getUs<uint32_t>(offset_ptr, dst, गिनती, nullptr);
}

uint64_t तथ्य_Extractor::getU64(uint64_t *offset_ptr, llvm::Error *Err) स्थिर {
  वापस getU<uint64_t>(offset_ptr, Err);
}

uint64_t *तथ्य_Extractor::getU64(uint64_t *offset_ptr, uint64_t *dst,
                                uint32_t गिनती) स्थिर {
  वापस getUs<uint64_t>(offset_ptr, dst, गिनती, nullptr);
}

uint64_t तथ्य_Extractor::getUnsigned(uint64_t *offset_ptr, uint32_t byte_आकार,
                                    llvm::Error *Err) स्थिर {
  चयन (byte_आकार) {
  हाल 1:
    वापस getU8(offset_ptr, Err);
  हाल 2:
    वापस getU16(offset_ptr, Err);
  हाल 4:
    वापस getU32(offset_ptr, Err);
  हाल 8:
    वापस getU64(offset_ptr, Err);
  }

  // For any other byte size, read the bytes and swap/shift if necessary.
  ErrorAsOutParameter ErrAsOut(Err);
  uint64_t val = 0;
  अगर (isError(Err))
    वापस val;
  uint64_t offset = *offset_ptr;
  अगर (!prepareRead(offset, byte_आकार, Err))
    वापस val;
  // Copy into the least significant bytes of val regardless of host
  // endianness.
  std::स_नकल(reinterpret_cast<अक्षर *>(&val) +
                  (sys::IsLittleEndianHost ? 0 : 8 - byte_आकार),
              &तथ्य_2.तथ्य()[offset], byte_आकार);
  // Swap the least significant bytes of val if endianness doesn't match.
  अगर (sys::IsLittleEndianHost != IsLittleEndian)
    val = sys::getSwappedBytes(val) >> (8 * (8 - byte_आकार));

  *offset_ptr += byte_आकार;
  वापस val;
}

int64_t
तथ्य_Extractor::getSigned(uint64_t *offset_ptr, uint32_t byte_आकार) स्थिर {
  चयन (byte_आकार) {
  हाल 1:
    वापस getS8(offset_ptr);
  हाल 2:
    वापस getS16(offset_ptr);
  हाल 4:
    वापस getS32(offset_ptr);
  हाल 8:
    वापस getS64(offset_ptr);
  }
  llvm_unreachable("getSigned unhandled case!");
}

StringRef तथ्य_Extractor::getCStrRef(uint64_t *OffsetPtr, Error *Err) स्थिर {
  ErrorAsOutParameter ErrAsOut(Err);
  अगर (isError(Err))
    वापस StringRef();

  uint64_t Start = *OffsetPtr;
  StringRef::आकार_type Pos = तथ्य_2.find('\0', Start);
  अगर (Pos != StringRef::npos) {
    *OffsetPtr = Pos + 1;
    वापस StringRef(तथ्य_2.तथ्य() + Start, Pos - Start);
  }
  अगर (Err)
    *Err = createStringError(errc::illegal_byte_sequence,
                             "no null terminated string at offset 0x%" PRIx64,
                             Start);
  वापस StringRef();
}

StringRef तथ्य_Extractor::get_Fixed_लंबाई_String(uint64_t *OffsetPtr,
                                              uint64_t लंबाई,
                                              StringRef TrimChars) स्थिर {
  StringRef Bytes(getBytes(OffsetPtr, लंबाई));
  वापस Bytes.trim(TrimChars);
}

StringRef तथ्य_Extractor::getBytes(uint64_t *OffsetPtr, uint64_t लंबाई,
                                  Error *Err) स्थिर {
  ErrorAsOutParameter ErrAsOut(Err);
  अगर (isError(Err))
    वापस StringRef();

  अगर (!prepareRead(*OffsetPtr, लंबाई, Err))
    वापस StringRef();

  StringRef परिणाम = तथ्य_2.substr(*OffsetPtr, लंबाई);
  *OffsetPtr += लंबाई;
  वापस परिणाम;
}

ढाँचा <typename T>
अटल T getLEB128(StringRef तथ्य_2, uint64_t *OffsetPtr, Error *Err,
                   T (&Decoder)(स्थिर uint8_t *p, अचिन्हित *n,
                                स्थिर uint8_t *end, स्थिर अक्षर **error)) {
  ArrayRef<uint8_t> Bytes = arrayRefFromStringRef(तथ्य_2);
  निश्चित(*OffsetPtr <= Bytes.आकार_2());
  ErrorAsOutParameter ErrAsOut(Err);
  अगर (isError(Err))
    वापस T();

  स्थिर अक्षर *error = nullptr;
  अचिन्हित bytes_read;
  T परिणाम_2 =
      Decoder(Bytes.तथ्य() + *OffsetPtr, &bytes_read, Bytes.end(), &error);
  अगर (error) {
    अगर (Err)
      *Err = createStringError(errc::illegal_byte_sequence,
                               "unable to decode LEB128 at offset 0x%8.8" PRIx64
                               ": %s",
                               *OffsetPtr, error);
    वापस T();
  }
  *OffsetPtr += bytes_read;
  वापस परिणाम_2;
}

uint64_t तथ्य_Extractor::getULEB128(uint64_t *offset_ptr, Error *Err) स्थिर {
  वापस getLEB128(तथ्य_2, offset_ptr, Err, decodeULEB128);
}

int64_t तथ्य_Extractor::getSLEB128(uint64_t *offset_ptr, Error *Err) स्थिर {
  वापस getLEB128(तथ्य_2, offset_ptr, Err, decodeSLEB128);
}

APSInt तथ्य_Extractor::getSLEB128APSInt(uint64_t *OffsetPtr, Error *Err) स्थिर {
  ErrorAsOutParameter ErrAsOut(Err);
  अगर (isError(Err))
    वापस APSInt();

  uint64_t Offset = *OffsetPtr;
  APInt मूल्य;
  अचिन्हित Shift = 0;
  uint8_t Byte;
  करो {
    अगर (!prepareRead(Offset, 1, Err))
      वापस APSInt();
    Byte = तथ्य_2[Offset++];
    APInt Slice(7, Byte & 0x7f);
    अगर (Shift == 0)
      मूल्य = Slice;
    अन्यथा {
      मूल्य = मूल्य.zext(Shift + 7);
      मूल्य.जोड़ो_Bits(Slice, Shift);
    }
    Shift += 7;
  } जबतक (Byte & 0x80);

  // Sign extend Value to at least 64 bits to match getSLEB128.
  अगर (Shift < 64)
    मूल्य = मूल्य.sext(64);

  *OffsetPtr = Offset;
  वापस APSInt(std::move(मूल्य), /*isUnsigned=*/false);
}

व्योम तथ्य_Extractor::skip(Cursor &C, uint64_t लंबाई) स्थिर {
  ErrorAsOutParameter ErrAsOut(C.Err);
  अगर (isError(&C.Err))
    वापस;

  अगर (prepareRead(C.Offset, लंबाई, &C.Err))
    C.Offset += लंबाई;
}
