//===- BinaryStreamRef.cpp - ----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/BinaryStreamRef.h"
#ضمن "llvm/Support/BinaryByteStream.h"

using نطاق llvm;

نطاق {

صنف ArrayRefImpl : عام BinaryStream {
عام:
  ArrayRefImpl(ArrayRef<uint8_t> Data, endianness Endian) : BBS(Data, Endian) {}

  llvm::endianness getEndian() ثابت override { أرجع BBS.getEndian(); }
  Error readBytes(uint64_t Offset, uint64_t Size,
                  ArrayRef<uint8_t> &Buffer) override {
    أرجع BBS.readBytes(Offset, Size, Buffer);
  }
  Error readLongestContiguousChunk(uint64_t Offset,
                                   ArrayRef<uint8_t> &Buffer) override {
    أرجع BBS.readLongestContiguousChunk(Offset, Buffer);
  }
  uint64_t getLength() override { أرجع BBS.getLength(); }

خاص:
  BinaryByteStream BBS;
};

صنف MutableArrayRefImpl : عام WritableBinaryStream {
عام:
  MutableArrayRefImpl(MutableArrayRef<uint8_t> Data, endianness Endian)
      : BBS(Data, Endian) {}

  // Inherited via WritableBinaryStream
  llvm::endianness getEndian() ثابت override { أرجع BBS.getEndian(); }
  Error readBytes(uint64_t Offset, uint64_t Size,
                  ArrayRef<uint8_t> &Buffer) override {
    أرجع BBS.readBytes(Offset, Size, Buffer);
  }
  Error readLongestContiguousChunk(uint64_t Offset,
                                   ArrayRef<uint8_t> &Buffer) override {
    أرجع BBS.readLongestContiguousChunk(Offset, Buffer);
  }
  uint64_t getLength() override { أرجع BBS.getLength(); }

  Error writeBytes(uint64_t Offset, ArrayRef<uint8_t> Data) override {
    أرجع BBS.writeBytes(Offset, Data);
  }
  Error commit() override { أرجع BBS.commit(); }

خاص:
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
                                 ArrayRef<uint8_t> &Buffer) ثابت {
  𐡄𐡍 (auto EC = checkOffsetForRead(Offset, Size))
    أرجع EC;
  أرجع BorrowedImpl->readBytes(ViewOffset + Offset, Size, Buffer);
}

Error BinaryStreamRef::readLongestContiguousChunk(
    uint64_t Offset, ArrayRef<uint8_t> &Buffer) ثابت {
  𐡄𐡍 (auto EC = checkOffsetForRead(Offset, 1))
    أرجع EC;

  𐡄𐡍 (auto EC =
          BorrowedImpl->readLongestContiguousChunk(ViewOffset + Offset, Buffer))
    أرجع EC;
  // This StreamRef might refer to a smaller window over a larger stream.  In
  // that case we will have read out more bytes than we should return, because
  // we should not read past the end of the current view.
  uint64_t MaxLength = getLength() - Offset;
  𐡄𐡍 (Buffer.size() > MaxLength)
    Buffer = Buffer.slice(0, MaxLength);
  أرجع Error::success();
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
                                          ArrayRef<uint8_t> Data) ثابت {
  𐡄𐡍 (auto EC = checkOffsetForWrite(Offset, Data.size()))
    أرجع EC;

  أرجع BorrowedImpl->writeBytes(ViewOffset + Offset, Data);
}

WritableBinaryStreamRef::operator BinaryStreamRef() ثابت {
  أرجع BinaryStreamRef(*BorrowedImpl, ViewOffset, Length);
}

/// For buffered streams, commits changes to the backing store.
Error WritableBinaryStreamRef::commit() { أرجع BorrowedImpl->commit(); }
