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

صنف ArrayRefImpl : عام نبيو_Stream {
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
  نبيو_Byte_Stream BBS;
};

صنف MutableArrayRefImpl : عام Writable_نبيو_Stream {
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
  Mutable_نبيو_Byte_Stream BBS;
};
} // namespace

نبيو_Stream_Ref::نبيو_Stream_Ref(نبيو_Stream &Stream)
    : نبيو_Stream_Ref_Base(Stream) {}
نبيو_Stream_Ref::نبيو_Stream_Ref(نبيو_Stream &Stream, uint64_t Offset,
                                 std::optional<uint64_t> Length)
    : نبيو_Stream_Ref_Base(Stream, Offset, Length) {}
نبيو_Stream_Ref::نبيو_Stream_Ref(ArrayRef<uint8_t> Data, endianness Endian)
    : نبيو_Stream_Ref_Base(std::make_shared<ArrayRefImpl>(Data, Endian), 0,
                          Data.size()) {}
نبيو_Stream_Ref::نبيو_Stream_Ref(StringRef Data, endianness Endian)
    : نبيو_Stream_Ref(ArrayRef(Data.bytes_begin(), Data.bytes_end()), Endian) {}

Error نبيو_Stream_Ref::readBytes(uint64_t Offset, uint64_t Size,
                                 ArrayRef<uint8_t> &Buffer) ثابت {
  إذا (auto EC = checkOffsetForRead(Offset, Size))
    أرجع EC;
  أرجع BorrowedImpl->readBytes(ViewOffset + Offset, Size, Buffer);
}

Error نبيو_Stream_Ref::readLongestContiguousChunk(
    uint64_t Offset, ArrayRef<uint8_t> &Buffer) ثابت {
  إذا (auto EC = checkOffsetForRead(Offset, 1))
    أرجع EC;

  إذا (auto EC =
          BorrowedImpl->readLongestContiguousChunk(ViewOffset + Offset, Buffer))
    أرجع EC;
  // This StreamRef might refer to a smaller window over a larger stream.  In
  // that case we will have read out more bytes than we should return, because
  // we should not read past the end of the current view.
  uint64_t MaxLength = getLength() - Offset;
  إذا (Buffer.size() > MaxLength)
    Buffer = Buffer.slice(0, MaxLength);
  أرجع Error::success();
}

Writable_نبيو_Stream_Ref::Writable_نبيو_Stream_Ref(Writable_نبيو_Stream &Stream)
    : نبيو_Stream_Ref_Base(Stream) {}

Writable_نبيو_Stream_Ref::Writable_نبيو_Stream_Ref(Writable_نبيو_Stream &Stream,
                                                 uint64_t Offset,
                                                 std::optional<uint64_t> Length)
    : نبيو_Stream_Ref_Base(Stream, Offset, Length) {}

Writable_نبيو_Stream_Ref::Writable_نبيو_Stream_Ref(MutableArrayRef<uint8_t> Data,
                                                 endianness Endian)
    : نبيو_Stream_Ref_Base(std::make_shared<MutableArrayRefImpl>(Data, Endian),
                          0, Data.size()) {}

Error Writable_نبيو_Stream_Ref::writeBytes(uint64_t Offset,
                                          ArrayRef<uint8_t> Data) ثابت {
  إذا (auto EC = checkOffsetForWrite(Offset, Data.size()))
    أرجع EC;

  أرجع BorrowedImpl->writeBytes(ViewOffset + Offset, Data);
}

Writable_نبيو_Stream_Ref::operator نبيو_Stream_Ref() ثابت {
  أرجع نبيو_Stream_Ref(*BorrowedImpl, ViewOffset, Length);
}

/// For buffered streams, commits changes to the backing store.
Error Writable_نبيو_Stream_Ref::commit() { أرجع BorrowedImpl->commit(); }
