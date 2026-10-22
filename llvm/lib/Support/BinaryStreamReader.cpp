//===- BinaryStreamReader.cpp - Reads objects from a binary stream --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/BinaryStreamReader.h"

#ಅನ್ತರ್ಭಾವ "llvm/Support/BinaryStreamError.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/BinaryStreamRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/LEB128.h"

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

ದ್ವಿಮಾನ_ಹರಿವು_Reader::ದ್ವಿಮಾನ_ಹರಿವು_Reader(ದ್ವಿಮಾನ_ಹರಿವು_Ref Ref) : ಹರಿವು(Ref) {}

ದ್ವಿಮಾನ_ಹರಿವು_Reader::ದ್ವಿಮಾನ_ಹರಿವು_Reader(ದ್ವಿಮಾನ_ಹರಿವು &ಹರಿವು) : ಹರಿವು(ಹರಿವು) {}

ದ್ವಿಮಾನ_ಹರಿವು_Reader::ದ್ವಿಮಾನ_ಹರಿವು_Reader(ArrayRef<uint8_t> ದತ್ತಾಂಶ_2,
                                       endianness Endian)
    : ಹರಿವು(ದತ್ತಾಂಶ_2, Endian) {}

ದ್ವಿಮಾನ_ಹರಿವು_Reader::ದ್ವಿಮಾನ_ಹರಿವು_Reader(StringRef ದತ್ತಾಂಶ_2, endianness Endian)
    : ಹರಿವು(ದತ್ತಾಂಶ_2, Endian) {}

Error ದ್ವಿಮಾನ_ಹರಿವು_Reader::readLongestContiguousChunk(
    ArrayRef<uint8_t> &ಬಫರ್) {
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ EC = ಹರಿವು.readLongestContiguousChunk(ಸ್ಥಾನಾಂತರ, ಬಫರ್))
    ಹಿಂತಿರುಗಿ EC;
  ಸ್ಥಾನಾಂತರ += ಬಫರ್.ಗಾತ್ರ_3();
  ಹಿಂತಿರುಗಿ Error::success();
}

Error ದ್ವಿಮಾನ_ಹರಿವು_Reader::read_ಬೈಟುಗಳು(ArrayRef<uint8_t> &ಬಫರ್, uint32_t ಗಾತ್ರ_2) {
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ EC = ಹರಿವು.read_ಬೈಟುಗಳು(ಸ್ಥಾನಾಂತರ, ಗಾತ್ರ_2, ಬಫರ್))
    ಹಿಂತಿರುಗಿ EC;
  ಸ್ಥಾನಾಂತರ += ಗಾತ್ರ_2;
  ಹಿಂತಿರುಗಿ Error::success();
}

Error ದ್ವಿಮಾನ_ಹರಿವು_Reader::readULEB128(uint64_t &Dest) {
  Small_ಸದಿಶ<uint8_t, 10> Encoded_ಬೈಟುಗಳು;
  ArrayRef<uint8_t> ಮುಂದಿನ_Byte;

  // Copy the encoded ULEB into the buffer.
  ಮಾಡು {
    ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ Err = read_ಬೈಟುಗಳು(ಮುಂದಿನ_Byte, 1))
      ಹಿಂತಿರುಗಿ Err;
    Encoded_ಬೈಟುಗಳು.ತಳ್ಳು_ಹಿಂಭಾಗ(ಮುಂದಿನ_Byte[0]);
  } ಆಗ (ಮುಂದಿನ_Byte[0] & 0x80);

  Dest = decodeULEB128(Encoded_ಬೈಟುಗಳು.begin(), ಶೂನ್ಯಸೂಚಕ, Encoded_ಬೈಟುಗಳು.ಕೊನೆ());
  ಹಿಂತಿರುಗಿ Error::success();
}

Error ದ್ವಿಮಾನ_ಹರಿವು_Reader::readSLEB128(int64_t &Dest) {
  Small_ಸದಿಶ<uint8_t, 10> Encoded_ಬೈಟುಗಳು;
  ArrayRef<uint8_t> ಮುಂದಿನ_Byte;

  // Copy the encoded ULEB into the buffer.
  ಮಾಡು {
    ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ Err = read_ಬೈಟುಗಳು(ಮುಂದಿನ_Byte, 1))
      ಹಿಂತಿರುಗಿ Err;
    Encoded_ಬೈಟುಗಳು.ತಳ್ಳು_ಹಿಂಭಾಗ(ಮುಂದಿನ_Byte[0]);
  } ಆಗ (ಮುಂದಿನ_Byte[0] & 0x80);

  Dest = decodeSLEB128(Encoded_ಬೈಟುಗಳು.begin(), ಶೂನ್ಯಸೂಚಕ, Encoded_ಬೈಟುಗಳು.ಕೊನೆ());
  ಹಿಂತಿರುಗಿ Error::success();
}

Error ದ್ವಿಮಾನ_ಹರಿವು_Reader::readCString(StringRef &Dest) {
  uint64_t Original_ಸ್ಥಾನಾಂತರ = get_ಸ್ಥಾನಾಂತರ();
  uint64_t ಸಿಕ್ಕಿತು_ಸ್ಥಾನಾಂತರ = 0;
  ಆಗ (ನಿಜ) {
    uint64_t This_ಸ್ಥಾನಾಂತರ = get_ಸ್ಥಾನಾಂತರ();
    ArrayRef<uint8_t> ಬಫರ್;
    ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ EC = readLongestContiguousChunk(ಬಫರ್))
      ಹಿಂತಿರುಗಿ EC;
    StringRef S(reinterpret_cast<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *>(ಬಫರ್.begin()), ಬಫರ್.ಗಾತ್ರ_3());
    size_t Pos = S.ಕಂಡುಹಿಡಿ_ಮೊದಲ_ದ('\0');
    ಆದರೆ (LLVM_LIKELY(Pos != StringRef::npos)) {
      ಸಿಕ್ಕಿತು_ಸ್ಥಾನಾಂತರ = Pos + This_ಸ್ಥಾನಾಂತರ;
      ನಿಲ್ಲಿಸು;
    }
  }
  ಪ್ರತಿಜ್ಞಾ(ಸಿಕ್ಕಿತು_ಸ್ಥಾನಾಂತರ >= Original_ಸ್ಥಾನಾಂತರ);

  ಗಣ_ಸ್ಥಾನಾಂತರ(Original_ಸ್ಥಾನಾಂತರ);
  size_t ಉದ್ದಳತೆ = ಸಿಕ್ಕಿತು_ಸ್ಥಾನಾಂತರ - Original_ಸ್ಥಾನಾಂತರ;

  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ EC = readFixedString(Dest, ಉದ್ದಳತೆ))
    ಹಿಂತಿರುಗಿ EC;

  // Now set the offset back to after the null terminator.
  ಗಣ_ಸ್ಥಾನಾಂತರ(ಸಿಕ್ಕಿತು_ಸ್ಥಾನಾಂತರ + 1);
  ಹಿಂತಿರುಗಿ Error::success();
}

Error ದ್ವಿಮಾನ_ಹರಿವು_Reader::readWideString(ArrayRef<UTF16> &Dest) {
  uint64_t ಉದ್ದಳತೆ = 0;
  uint64_t Original_ಸ್ಥಾನಾಂತರ = get_ಸ್ಥಾನಾಂತರ();
  ಸ್ಥಿರಾಂಕ UTF16 *C;
  ಆಗ (ನಿಜ) {
    ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ EC = readObject(C))
      ಹಿಂತಿರುಗಿ EC;
    ಆದರೆ (*C == 0x0000)
      ನಿಲ್ಲಿಸು;
    ++ಉದ್ದಳತೆ;
  }
  uint64_t New_ಸ್ಥಾನಾಂತರ = get_ಸ್ಥಾನಾಂತರ();
  ಗಣ_ಸ್ಥಾನಾಂತರ(Original_ಸ್ಥಾನಾಂತರ);

  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ EC = readArray(Dest, ಉದ್ದಳತೆ))
    ಹಿಂತಿರುಗಿ EC;
  ಗಣ_ಸ್ಥಾನಾಂತರ(New_ಸ್ಥಾನಾಂತರ);
  ಹಿಂತಿರುಗಿ Error::success();
}

Error ದ್ವಿಮಾನ_ಹರಿವು_Reader::readFixedString(StringRef &Dest, uint32_t ಉದ್ದಳತೆ) {
  ArrayRef<uint8_t> ಬೈಟುಗಳು_2;
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ EC = read_ಬೈಟುಗಳು(ಬೈಟುಗಳು_2, ಉದ್ದಳತೆ))
    ಹಿಂತಿರುಗಿ EC;
  Dest = StringRef(reinterpret_cast<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *>(ಬೈಟುಗಳು_2.begin()), ಬೈಟುಗಳು_2.ಗಾತ್ರ_3());
  ಹಿಂತಿರುಗಿ Error::success();
}

Error ದ್ವಿಮಾನ_ಹರಿವು_Reader::read_ಹರಿವು_Ref(ದ್ವಿಮಾನ_ಹರಿವು_Ref &Ref) {
  ಹಿಂತಿರುಗಿ read_ಹರಿವು_Ref(Ref, ಬೈಟುಗಳು_ಉಳಿದ());
}

Error ದ್ವಿಮಾನ_ಹರಿವು_Reader::read_ಹರಿವು_Ref(ದ್ವಿಮಾನ_ಹರಿವು_Ref &Ref, uint32_t ಉದ್ದಳತೆ) {
  ಆದರೆ (ಬೈಟುಗಳು_ಉಳಿದ() < ಉದ್ದಳತೆ)
    ಹಿಂತಿರುಗಿ make_error<ದ್ವಿಮಾನ_ಹರಿವು_Error>(ಹರಿವು_error_ಸಂಕೇತ::ಹರಿವು_too_short);
  Ref = ಹರಿವು.ತುಂಡು(ಸ್ಥಾನಾಂತರ, ಉದ್ದಳತೆ);
  ಸ್ಥಾನಾಂತರ += ಉದ್ದಳತೆ;
  ಹಿಂತಿರುಗಿ Error::success();
}

Error ದ್ವಿಮಾನ_ಹರಿವು_Reader::readSubstream(ದ್ವಿಮಾನ_Substream_Ref &Ref,
                                        uint32_t ಉದ್ದಳತೆ) {
  Ref.ಸ್ಥಾನಾಂತರ = get_ಸ್ಥಾನಾಂತರ();
  ಹಿಂತಿರುಗಿ read_ಹರಿವು_Ref(Ref.ಹರಿವು_ದತ್ತಾಂಶ, ಉದ್ದಳತೆ);
}

Error ದ್ವಿಮಾನ_ಹರಿವು_Reader::skip(uint64_t Amount) {
  ಆದರೆ (Amount > ಬೈಟುಗಳು_ಉಳಿದ())
    ಹಿಂತಿರುಗಿ make_error<ದ್ವಿಮಾನ_ಹರಿವು_Error>(ಹರಿವು_error_ಸಂಕೇತ::ಹರಿವು_too_short);
  ಸ್ಥಾನಾಂತರ += Amount;
  ಹಿಂತಿರುಗಿ Error::success();
}

Error ದ್ವಿಮಾನ_ಹರಿವು_Reader::pad_ಗಮ್ಯ_Alignment(uint32_t Align) {
  uint32_t New_ಸ್ಥಾನಾಂತರ = align_ಗಮ್ಯ(ಸ್ಥಾನಾಂತರ, Align);
  ಹಿಂತಿರುಗಿ skip(New_ಸ್ಥಾನಾಂತರ - ಸ್ಥಾನಾಂತರ);
}

uint8_t ದ್ವಿಮಾನ_ಹರಿವು_Reader::peek() ಸ್ಥಿರಾಂಕ {
  ArrayRef<uint8_t> ಬಫರ್;
  ಸ್ವಯಂಚಾಲಿತ EC = ಹರಿವು.read_ಬೈಟುಗಳು(ಸ್ಥಾನಾಂತರ, 1, ಬಫರ್);
  ಪ್ರತಿಜ್ಞಾ(!EC && "Cannot peek an empty buffer!");
  llvm::consumeError(std::move(EC));
  ಹಿಂತಿರುಗಿ ಬಫರ್[0];
}

std::pair<ದ್ವಿಮಾನ_ಹರಿವು_Reader, ದ್ವಿಮಾನ_ಹರಿವು_Reader>
ದ್ವಿಮಾನ_ಹರಿವು_Reader::ವಿಭಜಿಸು(uint64_t Off) ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(get_ಉದ್ದಳತೆ() >= Off);

  ದ್ವಿಮಾನ_ಹರಿವು_Ref ಮೊದಲ = ಹರಿವು.drop_front(ಸ್ಥಾನಾಂತರ);

  ದ್ವಿಮಾನ_ಹರಿವು_Ref ಎರಡನೇ = ಮೊದಲ.drop_front(Off);
  ಮೊದಲ = ಮೊದಲ.keep_front(Off);
  ದ್ವಿಮಾನ_ಹರಿವು_Reader W1{ಮೊದಲ};
  ದ್ವಿಮಾನ_ಹರಿವು_Reader W2{ಎರಡನೇ};
  ಹಿಂತಿರುಗಿ {W1, W2};
}
