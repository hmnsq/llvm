//===- BinaryStreamRef.cpp - ----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/BinaryStreamRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/BinaryByteStream.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਨਾਮਾਕਾਸ਼ {

ਵਰਗ ArrayRefImpl : ਜਨਤਕ ਦੁਆਧਾਰੀ_ਧਾਰਾ {
ਜਨਤਕ:
  ArrayRefImpl(ArrayRef<uint8_t> ਅੰਕੜੇ_2, endianness Endian) : BBS(ਅੰਕੜੇ_2, Endian) {}

  llvm::endianness getEndian() ਅਚਲ override { ਵਾਪਸ BBS.getEndian(); }
  Error read_ਬਾਈਟਾਂ(uint64_t ਵਿੱਥ, uint64_t ਮਾਪ,
                  ArrayRef<uint8_t> &ਬਫਰ) override {
    ਵਾਪਸ BBS.read_ਬਾਈਟਾਂ(ਵਿੱਥ, ਮਾਪ, ਬਫਰ);
  }
  Error readLongestContiguousChunk(uint64_t ਵਿੱਥ,
                                   ArrayRef<uint8_t> &ਬਫਰ) override {
    ਵਾਪਸ BBS.readLongestContiguousChunk(ਵਿੱਥ, ਬਫਰ);
  }
  uint64_t get_ਲੰਮਾਈ() override { ਵਾਪਸ BBS.get_ਲੰਮਾਈ(); }

ਨਿਜੀ:
  ਦੁਆਧਾਰੀ_Byte_ਧਾਰਾ BBS;
};

ਵਰਗ MutableArrayRefImpl : ਜਨਤਕ Writable_ਦੁਆਧਾਰੀ_ਧਾਰਾ {
ਜਨਤਕ:
  MutableArrayRefImpl(MutableArrayRef<uint8_t> ਅੰਕੜੇ_2, endianness Endian)
      : BBS(ਅੰਕੜੇ_2, Endian) {}

  // Inherited via WritableBinaryStream
  llvm::endianness getEndian() ਅਚਲ override { ਵਾਪਸ BBS.getEndian(); }
  Error read_ਬਾਈਟਾਂ(uint64_t ਵਿੱਥ, uint64_t ਮਾਪ,
                  ArrayRef<uint8_t> &ਬਫਰ) override {
    ਵਾਪਸ BBS.read_ਬਾਈਟਾਂ(ਵਿੱਥ, ਮਾਪ, ਬਫਰ);
  }
  Error readLongestContiguousChunk(uint64_t ਵਿੱਥ,
                                   ArrayRef<uint8_t> &ਬਫਰ) override {
    ਵਾਪਸ BBS.readLongestContiguousChunk(ਵਿੱਥ, ਬਫਰ);
  }
  uint64_t get_ਲੰਮਾਈ() override { ਵਾਪਸ BBS.get_ਲੰਮਾਈ(); }

  Error write_ਬਾਈਟਾਂ(uint64_t ਵਿੱਥ, ArrayRef<uint8_t> ਅੰਕੜੇ_2) override {
    ਵਾਪਸ BBS.write_ਬਾਈਟਾਂ(ਵਿੱਥ, ਅੰਕੜੇ_2);
  }
  Error commit() override { ਵਾਪਸ BBS.commit(); }

ਨਿਜੀ:
  Mutable_ਦੁਆਧਾਰੀ_Byte_ਧਾਰਾ BBS;
};
} // namespace

ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref::ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref(ਦੁਆਧਾਰੀ_ਧਾਰਾ &ਧਾਰਾ)
    : ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref_ਅਧਾਰ(ਧਾਰਾ) {}
ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref::ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref(ਦੁਆਧਾਰੀ_ਧਾਰਾ &ਧਾਰਾ, uint64_t ਵਿੱਥ,
                                 std::optional<uint64_t> ਲੰਮਾਈ)
    : ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref_ਅਧਾਰ(ਧਾਰਾ, ਵਿੱਥ, ਲੰਮਾਈ) {}
ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref::ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref(ArrayRef<uint8_t> ਅੰਕੜੇ_2, endianness Endian)
    : ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref_ਅਧਾਰ(std::make_shared<ArrayRefImpl>(ਅੰਕੜੇ_2, Endian), 0,
                          ਅੰਕੜੇ_2.ਮਾਪ_2()) {}
ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref::ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref(StringRef ਅੰਕੜੇ_2, endianness Endian)
    : ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref(ArrayRef(ਅੰਕੜੇ_2.ਬਾਈਟਾਂ_begin(), ਅੰਕੜੇ_2.ਬਾਈਟਾਂ_ਸਿਰਾ()), Endian) {}

Error ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref::read_ਬਾਈਟਾਂ(uint64_t ਵਿੱਥ, uint64_t ਮਾਪ,
                                 ArrayRef<uint8_t> &ਬਫਰ) ਅਚਲ {
  ਜੇ (ਸਵੈਚਾਲਿਤ EC = check_ਵਿੱਥ_For_Read(ਵਿੱਥ, ਮਾਪ))
    ਵਾਪਸ EC;
  ਵਾਪਸ BorrowedImpl->read_ਬਾਈਟਾਂ(View_ਵਿੱਥ + ਵਿੱਥ, ਮਾਪ, ਬਫਰ);
}

Error ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref::readLongestContiguousChunk(
    uint64_t ਵਿੱਥ, ArrayRef<uint8_t> &ਬਫਰ) ਅਚਲ {
  ਜੇ (ਸਵੈਚਾਲਿਤ EC = check_ਵਿੱਥ_For_Read(ਵਿੱਥ, 1))
    ਵਾਪਸ EC;

  ਜੇ (ਸਵੈਚਾਲਿਤ EC =
          BorrowedImpl->readLongestContiguousChunk(View_ਵਿੱਥ + ਵਿੱਥ, ਬਫਰ))
    ਵਾਪਸ EC;
  // This StreamRef might refer to a smaller window over a larger stream.  In
  // that case we will have read out more bytes than we should return, because
  // we should not read past the end of the current view.
  uint64_t Max_ਲੰਮਾਈ = get_ਲੰਮਾਈ() - ਵਿੱਥ;
  ਜੇ (ਬਫਰ.ਮਾਪ_2() > Max_ਲੰਮਾਈ)
    ਬਫਰ = ਬਫਰ.ਟੁਕੜਾ(0, Max_ਲੰਮਾਈ);
  ਵਾਪਸ Error::success();
}

Writable_ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref::Writable_ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref(Writable_ਦੁਆਧਾਰੀ_ਧਾਰਾ &ਧਾਰਾ)
    : ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref_ਅਧਾਰ(ਧਾਰਾ) {}

Writable_ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref::Writable_ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref(Writable_ਦੁਆਧਾਰੀ_ਧਾਰਾ &ਧਾਰਾ,
                                                 uint64_t ਵਿੱਥ,
                                                 std::optional<uint64_t> ਲੰਮਾਈ)
    : ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref_ਅਧਾਰ(ਧਾਰਾ, ਵਿੱਥ, ਲੰਮਾਈ) {}

Writable_ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref::Writable_ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref(MutableArrayRef<uint8_t> ਅੰਕੜੇ_2,
                                                 endianness Endian)
    : ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref_ਅਧਾਰ(std::make_shared<MutableArrayRefImpl>(ਅੰਕੜੇ_2, Endian),
                          0, ਅੰਕੜੇ_2.ਮਾਪ_2()) {}

Error Writable_ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref::write_ਬਾਈਟਾਂ(uint64_t ਵਿੱਥ,
                                          ArrayRef<uint8_t> ਅੰਕੜੇ_2) ਅਚਲ {
  ਜੇ (ਸਵੈਚਾਲਿਤ EC = check_ਵਿੱਥ_For_Write(ਵਿੱਥ, ਅੰਕੜੇ_2.ਮਾਪ_2()))
    ਵਾਪਸ EC;

  ਵਾਪਸ BorrowedImpl->write_ਬਾਈਟਾਂ(View_ਵਿੱਥ + ਵਿੱਥ, ਅੰਕੜੇ_2);
}

Writable_ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref::ਸੰਚਾਲਕ ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref() ਅਚਲ {
  ਵਾਪਸ ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref(*BorrowedImpl, View_ਵਿੱਥ, ਲੰਮਾਈ);
}

/// For buffered streams, commits changes to the backing store.
Error Writable_ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref::commit() { ਵਾਪਸ BorrowedImpl->commit(); }
