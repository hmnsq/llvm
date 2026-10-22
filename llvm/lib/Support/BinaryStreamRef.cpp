//===- BinaryStreamRef.cpp - ----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/BinaryStreamRef.h"
#include "llvm/Support/BinaryByteStream.h"

using namespace llvm;

namespace {

class ArrayRefImpl : public BinaryStream {
public:
  ArrayRefImpl(ArrayRef<uint8_t> Data, endianness Endian) : BBS(Data, Endian) {}

  llvm::endianness getEndian() const override { ᬯᬮᬶ BBS.getEndian(); }
  Error readBytes(uint64_t Offset, uint64_t Size,
                  ArrayRef<uint8_t> &Buffer) override {
    ᬯᬮᬶ BBS.readBytes(Offset, Size, Buffer);
  }
  Error readLongestContiguousChunk(uint64_t Offset,
                                   ArrayRef<uint8_t> &Buffer) override {
    ᬯᬮᬶ BBS.readLongestContiguousChunk(Offset, Buffer);
  }
  uint64_t getLength() override { ᬯᬮᬶ BBS.getLength(); }

private:
  BinaryByteStream BBS;
};

class MutableArrayRefImpl : public WritableBinaryStream {
public:
  MutableArrayRefImpl(MutableArrayRef<uint8_t> Data, endianness Endian)
      : BBS(Data, Endian) {}

  // Inherited via WritableBinaryStream
  llvm::endianness getEndian() const override { ᬯᬮᬶ BBS.getEndian(); }
  Error readBytes(uint64_t Offset, uint64_t Size,
                  ArrayRef<uint8_t> &Buffer) override {
    ᬯᬮᬶ BBS.readBytes(Offset, Size, Buffer);
  }
  Error readLongestContiguousChunk(uint64_t Offset,
                                   ArrayRef<uint8_t> &Buffer) override {
    ᬯᬮᬶ BBS.readLongestContiguousChunk(Offset, Buffer);
  }
  uint64_t getLength() override { ᬯᬮᬶ BBS.getLength(); }

  Error writeBytes(uint64_t Offset, ArrayRef<uint8_t> Data) override {
    ᬯᬮᬶ BBS.writeBytes(Offset, Data);
  }
  Error commit() override { ᬯᬮᬶ BBS.commit(); }

private:
  MutableBinaryByteStream BBS;
};
} // namespace

BinaryStreamRef::BinaryStreamRef(BinaryStream &Stream)
    : BinaryStreamRefBase(Stream) {}
BinaryStreamRef::BinaryStreamRef(BinaryStream &Stream, uint64_t Offset,
                                 std::optional<uint64_t> Length)
    : BinaryStreamRefBase(Stream, Offset, Length) {}
BinaryStreamRef::BinaryStreamRef(ArrayRef<uint8_t> Data, endianness Endian)
    : BinaryStreamRefBase(std::make_shared<ArrayRefImpl>(Data, Endian), 0,
                          Data.size()) {}
BinaryStreamRef::BinaryStreamRef(StringRef Data, endianness Endian)
    : BinaryStreamRef(ArrayRef(Data.bytes_begin(), Data.bytes_end()), Endian) {}

Error BinaryStreamRef::readBytes(uint64_t Offset, uint64_t Size,
                                 ArrayRef<uint8_t> &Buffer) const {
  ᬬᬾᬦ᭄ (auto EC = checkOffsetForRead(Offset, Size))
    ᬯᬮᬶ EC;
  ᬯᬮᬶ BorrowedImpl->readBytes(ViewOffset + Offset, Size, Buffer);
}

Error BinaryStreamRef::readLongestContiguousChunk(
    uint64_t Offset, ArrayRef<uint8_t> &Buffer) const {
  ᬬᬾᬦ᭄ (auto EC = checkOffsetForRead(Offset, 1))
    ᬯᬮᬶ EC;

  ᬬᬾᬦ᭄ (auto EC =
          BorrowedImpl->readLongestContiguousChunk(ViewOffset + Offset, Buffer))
    ᬯᬮᬶ EC;
  // This StreamRef might refer to a smaller window over a larger stream.  In
  // that case we will have read out more bytes than we should return, because
  // we should not read past the end of the current view.
  uint64_t MaxLength = getLength() - Offset;
  ᬬᬾᬦ᭄ (Buffer.size() > MaxLength)
    Buffer = Buffer.slice(0, MaxLength);
  ᬯᬮᬶ Error::success();
}

WritableBinaryStreamRef::WritableBinaryStreamRef(WritableBinaryStream &Stream)
    : BinaryStreamRefBase(Stream) {}

WritableBinaryStreamRef::WritableBinaryStreamRef(WritableBinaryStream &Stream,
                                                 uint64_t Offset,
                                                 std::optional<uint64_t> Length)
    : BinaryStreamRefBase(Stream, Offset, Length) {}

WritableBinaryStreamRef::WritableBinaryStreamRef(MutableArrayRef<uint8_t> Data,
                                                 endianness Endian)
    : BinaryStreamRefBase(std::make_shared<MutableArrayRefImpl>(Data, Endian),
                          0, Data.size()) {}

Error WritableBinaryStreamRef::writeBytes(uint64_t Offset,
                                          ArrayRef<uint8_t> Data) const {
  ᬬᬾᬦ᭄ (auto EC = checkOffsetForWrite(Offset, Data.size()))
    ᬯᬮᬶ EC;

  ᬯᬮᬶ BorrowedImpl->writeBytes(ViewOffset + Offset, Data);
}

WritableBinaryStreamRef::operator BinaryStreamRef() const {
  ᬯᬮᬶ BinaryStreamRef(*BorrowedImpl, ViewOffset, Length);
}

/// For buffered streams, commits changes to the backing store.
Error WritableBinaryStreamRef::commit() { ᬯᬮᬶ BorrowedImpl->commit(); }
