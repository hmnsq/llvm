//===- BinaryStreamRef.cpp - ----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/BinaryStreamRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/BinaryByteStream.h"

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

ನಾಮಾಕಾಶ {

ವರ್ಗ ArrayRefImpl : ಸಾರ್ವಜನಿಕ ದ್ವಿಮಾನ_ಹರಿವು {
ಸಾರ್ವಜನಿಕ:
  ArrayRefImpl(ArrayRef<uint8_t> ದತ್ತಾಂಶ_2, endianness Endian) : BBS(ದತ್ತಾಂಶ_2, Endian) {}

  llvm::endianness getEndian() ಸ್ಥಿರಾಂಕ override { ಹಿಂತಿರುಗಿ BBS.getEndian(); }
  Error read_ಬೈಟುಗಳು(uint64_t ಸ್ಥಾನಾಂತರ, uint64_t ಗಾತ್ರ_2,
                  ArrayRef<uint8_t> &ಬಫರ್) override {
    ಹಿಂತಿರುಗಿ BBS.read_ಬೈಟುಗಳು(ಸ್ಥಾನಾಂತರ, ಗಾತ್ರ_2, ಬಫರ್);
  }
  Error readLongestContiguousChunk(uint64_t ಸ್ಥಾನಾಂತರ,
                                   ArrayRef<uint8_t> &ಬಫರ್) override {
    ಹಿಂತಿರುಗಿ BBS.readLongestContiguousChunk(ಸ್ಥಾನಾಂತರ, ಬಫರ್);
  }
  uint64_t get_ಉದ್ದಳತೆ() override { ಹಿಂತಿರುಗಿ BBS.get_ಉದ್ದಳತೆ(); }

ಖಾಸಗಿ:
  ದ್ವಿಮಾನ_Byte_ಹರಿವು BBS;
};

ವರ್ಗ MutableArrayRefImpl : ಸಾರ್ವಜನಿಕ Writable_ದ್ವಿಮಾನ_ಹರಿವು {
ಸಾರ್ವಜನಿಕ:
  MutableArrayRefImpl(MutableArrayRef<uint8_t> ದತ್ತಾಂಶ_2, endianness Endian)
      : BBS(ದತ್ತಾಂಶ_2, Endian) {}

  // Inherited via WritableBinaryStream
  llvm::endianness getEndian() ಸ್ಥಿರಾಂಕ override { ಹಿಂತಿರುಗಿ BBS.getEndian(); }
  Error read_ಬೈಟುಗಳು(uint64_t ಸ್ಥಾನಾಂತರ, uint64_t ಗಾತ್ರ_2,
                  ArrayRef<uint8_t> &ಬಫರ್) override {
    ಹಿಂತಿರುಗಿ BBS.read_ಬೈಟುಗಳು(ಸ್ಥಾನಾಂತರ, ಗಾತ್ರ_2, ಬಫರ್);
  }
  Error readLongestContiguousChunk(uint64_t ಸ್ಥಾನಾಂತರ,
                                   ArrayRef<uint8_t> &ಬಫರ್) override {
    ಹಿಂತಿರುಗಿ BBS.readLongestContiguousChunk(ಸ್ಥಾನಾಂತರ, ಬಫರ್);
  }
  uint64_t get_ಉದ್ದಳತೆ() override { ಹಿಂತಿರುಗಿ BBS.get_ಉದ್ದಳತೆ(); }

  Error write_ಬೈಟುಗಳು(uint64_t ಸ್ಥಾನಾಂತರ, ArrayRef<uint8_t> ದತ್ತಾಂಶ_2) override {
    ಹಿಂತಿರುಗಿ BBS.write_ಬೈಟುಗಳು(ಸ್ಥಾನಾಂತರ, ದತ್ತಾಂಶ_2);
  }
  Error commit() override { ಹಿಂತಿರುಗಿ BBS.commit(); }

ಖಾಸಗಿ:
  Mutable_ದ್ವಿಮಾನ_Byte_ಹರಿವು BBS;
};
} // namespace

ದ್ವಿಮಾನ_ಹರಿವು_Ref::ದ್ವಿಮಾನ_ಹರಿವು_Ref(ದ್ವಿಮಾನ_ಹರಿವು &ಹರಿವು)
    : ದ್ವಿಮಾನ_ಹರಿವು_Ref_ತಳ(ಹರಿವು) {}
ದ್ವಿಮಾನ_ಹರಿವು_Ref::ದ್ವಿಮಾನ_ಹರಿವು_Ref(ದ್ವಿಮಾನ_ಹರಿವು &ಹರಿವು, uint64_t ಸ್ಥಾನಾಂತರ,
                                 std::optional<uint64_t> ಉದ್ದಳತೆ)
    : ದ್ವಿಮಾನ_ಹರಿವು_Ref_ತಳ(ಹರಿವು, ಸ್ಥಾನಾಂತರ, ಉದ್ದಳತೆ) {}
ದ್ವಿಮಾನ_ಹರಿವು_Ref::ದ್ವಿಮಾನ_ಹರಿವು_Ref(ArrayRef<uint8_t> ದತ್ತಾಂಶ_2, endianness Endian)
    : ದ್ವಿಮಾನ_ಹರಿವು_Ref_ತಳ(std::make_shared<ArrayRefImpl>(ದತ್ತಾಂಶ_2, Endian), 0,
                          ದತ್ತಾಂಶ_2.ಗಾತ್ರ_3()) {}
ದ್ವಿಮಾನ_ಹರಿವು_Ref::ದ್ವಿಮಾನ_ಹರಿವು_Ref(StringRef ದತ್ತಾಂಶ_2, endianness Endian)
    : ದ್ವಿಮಾನ_ಹರಿವು_Ref(ArrayRef(ದತ್ತಾಂಶ_2.ಬೈಟುಗಳು_begin(), ದತ್ತಾಂಶ_2.ಬೈಟುಗಳು_ಕೊನೆ()), Endian) {}

Error ದ್ವಿಮಾನ_ಹರಿವು_Ref::read_ಬೈಟುಗಳು(uint64_t ಸ್ಥಾನಾಂತರ, uint64_t ಗಾತ್ರ_2,
                                 ArrayRef<uint8_t> &ಬಫರ್) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ EC = check_ಸ್ಥಾನಾಂತರ_For_Read(ಸ್ಥಾನಾಂತರ, ಗಾತ್ರ_2))
    ಹಿಂತಿರುಗಿ EC;
  ಹಿಂತಿರುಗಿ BorrowedImpl->read_ಬೈಟುಗಳು(View_ಸ್ಥಾನಾಂತರ + ಸ್ಥಾನಾಂತರ, ಗಾತ್ರ_2, ಬಫರ್);
}

Error ದ್ವಿಮಾನ_ಹರಿವು_Ref::readLongestContiguousChunk(
    uint64_t ಸ್ಥಾನಾಂತರ, ArrayRef<uint8_t> &ಬಫರ್) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ EC = check_ಸ್ಥಾನಾಂತರ_For_Read(ಸ್ಥಾನಾಂತರ, 1))
    ಹಿಂತಿರುಗಿ EC;

  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ EC =
          BorrowedImpl->readLongestContiguousChunk(View_ಸ್ಥಾನಾಂತರ + ಸ್ಥಾನಾಂತರ, ಬಫರ್))
    ಹಿಂತಿರುಗಿ EC;
  // This StreamRef might refer to a smaller window over a larger stream.  In
  // that case we will have read out more bytes than we should return, because
  // we should not read past the end of the current view.
  uint64_t Max_ಉದ್ದಳತೆ = get_ಉದ್ದಳತೆ() - ಸ್ಥಾನಾಂತರ;
  ಆದರೆ (ಬಫರ್.ಗಾತ್ರ_3() > Max_ಉದ್ದಳತೆ)
    ಬಫರ್ = ಬಫರ್.ತುಂಡು(0, Max_ಉದ್ದಳತೆ);
  ಹಿಂತಿರುಗಿ Error::success();
}

Writable_ದ್ವಿಮಾನ_ಹರಿವು_Ref::Writable_ದ್ವಿಮಾನ_ಹರಿವು_Ref(Writable_ದ್ವಿಮಾನ_ಹರಿವು &ಹರಿವು)
    : ದ್ವಿಮಾನ_ಹರಿವು_Ref_ತಳ(ಹರಿವು) {}

Writable_ದ್ವಿಮಾನ_ಹರಿವು_Ref::Writable_ದ್ವಿಮಾನ_ಹರಿವು_Ref(Writable_ದ್ವಿಮಾನ_ಹರಿವು &ಹರಿವು,
                                                 uint64_t ಸ್ಥಾನಾಂತರ,
                                                 std::optional<uint64_t> ಉದ್ದಳತೆ)
    : ದ್ವಿಮಾನ_ಹರಿವು_Ref_ತಳ(ಹರಿವು, ಸ್ಥಾನಾಂತರ, ಉದ್ದಳತೆ) {}

Writable_ದ್ವಿಮಾನ_ಹರಿವು_Ref::Writable_ದ್ವಿಮಾನ_ಹರಿವು_Ref(MutableArrayRef<uint8_t> ದತ್ತಾಂಶ_2,
                                                 endianness Endian)
    : ದ್ವಿಮಾನ_ಹರಿವು_Ref_ತಳ(std::make_shared<MutableArrayRefImpl>(ದತ್ತಾಂಶ_2, Endian),
                          0, ದತ್ತಾಂಶ_2.ಗಾತ್ರ_3()) {}

Error Writable_ದ್ವಿಮಾನ_ಹರಿವು_Ref::write_ಬೈಟುಗಳು(uint64_t ಸ್ಥಾನಾಂತರ,
                                          ArrayRef<uint8_t> ದತ್ತಾಂಶ_2) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ EC = check_ಸ್ಥಾನಾಂತರ_For_Write(ಸ್ಥಾನಾಂತರ, ದತ್ತಾಂಶ_2.ಗಾತ್ರ_3()))
    ಹಿಂತಿರುಗಿ EC;

  ಹಿಂತಿರುಗಿ BorrowedImpl->write_ಬೈಟುಗಳು(View_ಸ್ಥಾನಾಂತರ + ಸ್ಥಾನಾಂತರ, ದತ್ತಾಂಶ_2);
}

Writable_ದ್ವಿಮಾನ_ಹರಿವು_Ref::ಕ್ರಿಯಾಕಾರಕ ದ್ವಿಮಾನ_ಹರಿವು_Ref() ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ ದ್ವಿಮಾನ_ಹರಿವು_Ref(*BorrowedImpl, View_ಸ್ಥಾನಾಂತರ, ಉದ್ದಳತೆ);
}

/// For buffered streams, commits changes to the backing store.
Error Writable_ದ್ವಿಮಾನ_ಹರಿವು_Ref::commit() { ಹಿಂತಿರುಗಿ BorrowedImpl->commit(); }
