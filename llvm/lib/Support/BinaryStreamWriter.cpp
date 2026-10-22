//===- BinaryStreamWriter.cpp - Writes objects to a BinaryStream ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/BinaryStreamWriter.h"

#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/BinaryStreamReader.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/BinaryStreamRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/LEB128.h"

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

ದ್ವಿಮಾನ_ಹರಿವು_Writer::ದ್ವಿಮಾನ_ಹರಿವು_Writer(Writable_ದ್ವಿಮಾನ_ಹರಿವು_Ref Ref)
    : ಹರಿವು(Ref) {}

ದ್ವಿಮಾನ_ಹರಿವು_Writer::ದ್ವಿಮಾನ_ಹರಿವು_Writer(Writable_ದ್ವಿಮಾನ_ಹರಿವು &ಹರಿವು)
    : ಹರಿವು(ಹರಿವು) {}

ದ್ವಿಮಾನ_ಹರಿವು_Writer::ದ್ವಿಮಾನ_ಹರಿವು_Writer(MutableArrayRef<uint8_t> ದತ್ತಾಂಶ_2,
                                       llvm::endianness Endian)
    : ಹರಿವು(ದತ್ತಾಂಶ_2, Endian) {}

Error ದ್ವಿಮಾನ_ಹರಿವು_Writer::write_ಬೈಟುಗಳು(ArrayRef<uint8_t> ಬಫರ್) {
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ EC = ಹರಿವು.write_ಬೈಟುಗಳು(ಸ್ಥಾನಾಂತರ, ಬಫರ್))
    ಹಿಂತಿರುಗಿ EC;
  ಸ್ಥಾನಾಂತರ += ಬಫರ್.ಗಾತ್ರ_3();
  ಹಿಂತಿರುಗಿ Error::success();
}

Error ದ್ವಿಮಾನ_ಹರಿವು_Writer::writeULEB128(uint64_t ಮೌಲ್ಯ) {
  uint8_t Encoded_ಬೈಟುಗಳು[10] = {0};
  ಚಿಹ್ನೆರಹಿತ ಗಾತ್ರ_2 = encodeULEB128(ಮೌಲ್ಯ, &Encoded_ಬೈಟುಗಳು[0]);
  ಹಿಂತಿರುಗಿ write_ಬೈಟುಗಳು({Encoded_ಬೈಟುಗಳು, ಗಾತ್ರ_2});
}

Error ದ್ವಿಮಾನ_ಹರಿವು_Writer::writeSLEB128(int64_t ಮೌಲ್ಯ) {
  uint8_t Encoded_ಬೈಟುಗಳು[10] = {0};
  ಚಿಹ್ನೆರಹಿತ ಗಾತ್ರ_2 = encodeSLEB128(ಮೌಲ್ಯ, &Encoded_ಬೈಟುಗಳು[0]);
  ಹಿಂತಿರುಗಿ write_ಬೈಟುಗಳು({Encoded_ಬೈಟುಗಳು, ಗಾತ್ರ_2});
}

Error ದ್ವಿಮಾನ_ಹರಿವು_Writer::writeCString(StringRef Str) {
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ EC = writeFixedString(Str))
    ಹಿಂತಿರುಗಿ EC;
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ EC = writeObject('\0'))
    ಹಿಂತಿರುಗಿ EC;

  ಹಿಂತಿರುಗಿ Error::success();
}

Error ದ್ವಿಮಾನ_ಹರಿವು_Writer::writeFixedString(StringRef Str) {

  ಹಿಂತಿರುಗಿ write_ಬೈಟುಗಳು(arrayRefFromStringRef(Str));
}

Error ದ್ವಿಮಾನ_ಹರಿವು_Writer::write_ಹರಿವು_Ref(ದ್ವಿಮಾನ_ಹರಿವು_Ref Ref) {
  ಹಿಂತಿರುಗಿ write_ಹರಿವು_Ref(Ref, Ref.get_ಉದ್ದಳತೆ());
}

Error ದ್ವಿಮಾನ_ಹರಿವು_Writer::write_ಹರಿವು_Ref(ದ್ವಿಮಾನ_ಹರಿವು_Ref Ref, uint64_t ಉದ್ದಳತೆ) {
  ದ್ವಿಮಾನ_ಹರಿವು_Reader SrcReader(Ref.ತುಂಡು(0, ಉದ್ದಳತೆ));
  // This is a bit tricky.  If we just call readBytes, we are requiring that it
  // return us the entire stream as a contiguous buffer.  There is no guarantee
  // this can be satisfied by returning a reference straight from the buffer, as
  // an implementation may not store all data in a single contiguous buffer.  So
  // we iterate over each contiguous chunk, writing each one in succession.
  ಆಗ (SrcReader.ಬೈಟುಗಳು_ಉಳಿದ() > 0) {
    ArrayRef<uint8_t> Chunk;
    ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ EC = SrcReader.readLongestContiguousChunk(Chunk))
      ಹಿಂತಿರುಗಿ EC;
    ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ EC = write_ಬೈಟುಗಳು(Chunk))
      ಹಿಂತಿರುಗಿ EC;
  }
  ಹಿಂತಿರುಗಿ Error::success();
}

std::pair<ದ್ವಿಮಾನ_ಹರಿವು_Writer, ದ್ವಿಮಾನ_ಹರಿವು_Writer>
ದ್ವಿಮಾನ_ಹರಿವು_Writer::ವಿಭಜಿಸು(uint64_t Off) ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(get_ಉದ್ದಳತೆ() >= Off);

  Writable_ದ್ವಿಮಾನ_ಹರಿವು_Ref ಮೊದಲ = ಹರಿವು.drop_front(ಸ್ಥಾನಾಂತರ);

  Writable_ದ್ವಿಮಾನ_ಹರಿವು_Ref ಎರಡನೇ = ಮೊದಲ.drop_front(Off);
  ಮೊದಲ = ಮೊದಲ.keep_front(Off);
  ದ್ವಿಮಾನ_ಹರಿವು_Writer W1{ಮೊದಲ};
  ದ್ವಿಮಾನ_ಹರಿವು_Writer W2{ಎರಡನೇ};
  ಹಿಂತಿರುಗಿ {W1, W2};
}

Error ದ್ವಿಮಾನ_ಹರಿವು_Writer::pad_ಗಮ್ಯ_Alignment(uint32_t Align) {
  uint64_t New_ಸ್ಥಾನಾಂತರ = align_ಗಮ್ಯ(ಸ್ಥಾನಾಂತರ, Align);
  ಸ್ಥಿರಾಂಕ uint64_t Zeros_ಗಾತ್ರ = 64;
  ಸ್ಥಿರ constexpr ಅಕ್ಷರ Zeros[Zeros_ಗಾತ್ರ] = {};
  ಆಗ (ಸ್ಥಾನಾಂತರ < New_ಸ್ಥಾನಾಂತರ)
    ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ E = writeArray(
            ArrayRef<ಅಕ್ಷರ>(Zeros, std::min(Zeros_ಗಾತ್ರ, New_ಸ್ಥಾನಾಂತರ - ಸ್ಥಾನಾಂತರ))))
      ಹಿಂತಿರುಗಿ E;
  ಹಿಂತಿರುಗಿ Error::success();
}
