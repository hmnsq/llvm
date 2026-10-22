//===- BinaryStreamRef.cpp - ----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/BinaryStreamRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/BinaryByteStream.h"

using ನಾಮಾಕಾಶ llvm;

ನಾಮಾಕಾಶ {

ವರ್ಗ ArrayRefImpl : ಸಾರ್ವಜನಿಕ ದ್ವಿಮಾನ_Stream {
ಸಾರ್ವಜನಿಕ:
  ArrayRefImpl(ArrayRef<uint8_t> Data, endianness Endian) : BBS(Data, Endian) {}

  llvm::endianness getEndian() ಧ್ರುವ override { ಪ್ರತ್ಯಾಗಮ BBS.getEndian(); }
  Error readBytes(uint64_t Offset, uint64_t Size,
                  ArrayRef<uint8_t> &Buffer) override {
    ಪ್ರತ್ಯಾಗಮ BBS.readBytes(Offset, Size, Buffer);
  }
  Error readLongestContiguousChunk(uint64_t Offset,
                                   ArrayRef<uint8_t> &Buffer) override {
    ಪ್ರತ್ಯಾಗಮ BBS.readLongestContiguousChunk(Offset, Buffer);
  }
  uint64_t getLength() override { ಪ್ರತ್ಯಾಗಮ BBS.getLength(); }

ಗುಪ್ತ:
  ದ್ವಿಮಾನ_Byte_Stream BBS;
};

ವರ್ಗ MutableArrayRefImpl : ಸಾರ್ವಜನಿಕ Writable_ದ್ವಿಮಾನ_Stream {
ಸಾರ್ವಜನಿಕ:
  MutableArrayRefImpl(MutableArrayRef<uint8_t> Data, endianness Endian)
      : BBS(Data, Endian) {}

  // Inherited via WritableBinaryStream
  llvm::endianness getEndian() ಧ್ರುವ override { ಪ್ರತ್ಯಾಗಮ BBS.getEndian(); }
  Error readBytes(uint64_t Offset, uint64_t Size,
                  ArrayRef<uint8_t> &Buffer) override {
    ಪ್ರತ್ಯಾಗಮ BBS.readBytes(Offset, Size, Buffer);
  }
  Error readLongestContiguousChunk(uint64_t Offset,
                                   ArrayRef<uint8_t> &Buffer) override {
    ಪ್ರತ್ಯಾಗಮ BBS.readLongestContiguousChunk(Offset, Buffer);
  }
  uint64_t getLength() override { ಪ್ರತ್ಯಾಗಮ BBS.getLength(); }

  Error writeBytes(uint64_t Offset, ArrayRef<uint8_t> Data) override {
    ಪ್ರತ್ಯಾಗಮ BBS.writeBytes(Offset, Data);
  }
  Error commit() override { ಪ್ರತ್ಯಾಗಮ BBS.commit(); }

ಗುಪ್ತ:
  Mutable_ದ್ವಿಮಾನ_Byte_Stream BBS;
};
} // namespace

ದ್ವಿಮಾನ_Stream_Ref::ದ್ವಿಮಾನ_Stream_Ref(ದ್ವಿಮಾನ_Stream &Stream)
    : ದ್ವಿಮಾನ_Stream_Ref_Base(Stream) {}
ದ್ವಿಮಾನ_Stream_Ref::ದ್ವಿಮಾನ_Stream_Ref(ದ್ವಿಮಾನ_Stream &Stream, uint64_t Offset,
                                 std::optional<uint64_t> Length)
    : ದ್ವಿಮಾನ_Stream_Ref_Base(Stream, Offset, Length) {}
ದ್ವಿಮಾನ_Stream_Ref::ದ್ವಿಮಾನ_Stream_Ref(ArrayRef<uint8_t> Data, endianness Endian)
    : ದ್ವಿಮಾನ_Stream_Ref_Base(std::make_shared<ArrayRefImpl>(Data, Endian), 0,
                          Data.size()) {}
ದ್ವಿಮಾನ_Stream_Ref::ದ್ವಿಮಾನ_Stream_Ref(StringRef Data, endianness Endian)
    : ದ್ವಿಮಾನ_Stream_Ref(ArrayRef(Data.bytes_begin(), Data.bytes_end()), Endian) {}

Error ದ್ವಿಮಾನ_Stream_Ref::readBytes(uint64_t Offset, uint64_t Size,
                                 ArrayRef<uint8_t> &Buffer) ಧ್ರುವ {
  ಯದಿ (auto EC = checkOffsetForRead(Offset, Size))
    ಪ್ರತ್ಯಾಗಮ EC;
  ಪ್ರತ್ಯಾಗಮ BorrowedImpl->readBytes(ViewOffset + Offset, Size, Buffer);
}

Error ದ್ವಿಮಾನ_Stream_Ref::readLongestContiguousChunk(
    uint64_t Offset, ArrayRef<uint8_t> &Buffer) ಧ್ರುವ {
  ಯದಿ (auto EC = checkOffsetForRead(Offset, 1))
    ಪ್ರತ್ಯಾಗಮ EC;

  ಯದಿ (auto EC =
          BorrowedImpl->readLongestContiguousChunk(ViewOffset + Offset, Buffer))
    ಪ್ರತ್ಯಾಗಮ EC;
  // This StreamRef might refer to a smaller window over a larger stream.  In
  // that case we will have read out more bytes than we should return, because
  // we should not read past the end of the current view.
  uint64_t MaxLength = getLength() - Offset;
  ಯದಿ (Buffer.size() > MaxLength)
    Buffer = Buffer.slice(0, MaxLength);
  ಪ್ರತ್ಯಾಗಮ Error::success();
}

Writable_ದ್ವಿಮಾನ_Stream_Ref::Writable_ದ್ವಿಮಾನ_Stream_Ref(Writable_ದ್ವಿಮಾನ_Stream &Stream)
    : ದ್ವಿಮಾನ_Stream_Ref_Base(Stream) {}

Writable_ದ್ವಿಮಾನ_Stream_Ref::Writable_ದ್ವಿಮಾನ_Stream_Ref(Writable_ದ್ವಿಮಾನ_Stream &Stream,
                                                 uint64_t Offset,
                                                 std::optional<uint64_t> Length)
    : ದ್ವಿಮಾನ_Stream_Ref_Base(Stream, Offset, Length) {}

Writable_ದ್ವಿಮಾನ_Stream_Ref::Writable_ದ್ವಿಮಾನ_Stream_Ref(MutableArrayRef<uint8_t> Data,
                                                 endianness Endian)
    : ದ್ವಿಮಾನ_Stream_Ref_Base(std::make_shared<MutableArrayRefImpl>(Data, Endian),
                          0, Data.size()) {}

Error Writable_ದ್ವಿಮಾನ_Stream_Ref::writeBytes(uint64_t Offset,
                                          ArrayRef<uint8_t> Data) ಧ್ರುವ {
  ಯದಿ (auto EC = checkOffsetForWrite(Offset, Data.size()))
    ಪ್ರತ್ಯಾಗಮ EC;

  ಪ್ರತ್ಯಾಗಮ BorrowedImpl->writeBytes(ViewOffset + Offset, Data);
}

Writable_ದ್ವಿಮಾನ_Stream_Ref::operator ದ್ವಿಮಾನ_Stream_Ref() ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ ದ್ವಿಮಾನ_Stream_Ref(*BorrowedImpl, ViewOffset, Length);
}

/// For buffered streams, commits changes to the backing store.
Error Writable_ದ್ವಿಮಾನ_Stream_Ref::commit() { ಪ್ರತ್ಯಾಗಮ BorrowedImpl->commit(); }
