//===- BinaryStreamRef.cpp - ----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/Support/BinaryStreamRef.h"
#অন্তর্ভাব "llvm/Support/BinaryByteStream.h"

using নামাকাশ llvm;

নামাকাশ {

বর্গ ArrayRefImpl : সার্বজনিক দ্বিমিক_Stream {
সার্বজনিক:
  ArrayRefImpl(ArrayRef<uint8_t> Data, endianness Endian) : BBS(Data, Endian) {}

  llvm::endianness getEndian() ধ্রুব override { প্রত্যাগম BBS.getEndian(); }
  Error readBytes(uint64_t Offset, uint64_t Size,
                  ArrayRef<uint8_t> &Buffer) override {
    প্রত্যাগম BBS.readBytes(Offset, Size, Buffer);
  }
  Error readLongestContiguousChunk(uint64_t Offset,
                                   ArrayRef<uint8_t> &Buffer) override {
    প্রত্যাগম BBS.readLongestContiguousChunk(Offset, Buffer);
  }
  uint64_t getLength() override { প্রত্যাগম BBS.getLength(); }

গুপ্ত:
  দ্বিমিক_Byte_Stream BBS;
};

বর্গ MutableArrayRefImpl : সার্বজনিক Writable_দ্বিমিক_Stream {
সার্বজনিক:
  MutableArrayRefImpl(MutableArrayRef<uint8_t> Data, endianness Endian)
      : BBS(Data, Endian) {}

  // Inherited via WritableBinaryStream
  llvm::endianness getEndian() ধ্রুব override { প্রত্যাগম BBS.getEndian(); }
  Error readBytes(uint64_t Offset, uint64_t Size,
                  ArrayRef<uint8_t> &Buffer) override {
    প্রত্যাগম BBS.readBytes(Offset, Size, Buffer);
  }
  Error readLongestContiguousChunk(uint64_t Offset,
                                   ArrayRef<uint8_t> &Buffer) override {
    প্রত্যাগম BBS.readLongestContiguousChunk(Offset, Buffer);
  }
  uint64_t getLength() override { প্রত্যাগম BBS.getLength(); }

  Error writeBytes(uint64_t Offset, ArrayRef<uint8_t> Data) override {
    প্রত্যাগম BBS.writeBytes(Offset, Data);
  }
  Error commit() override { প্রত্যাগম BBS.commit(); }

গুপ্ত:
  Mutable_দ্বিমিক_Byte_Stream BBS;
};
} // namespace

দ্বিমিক_Stream_Ref::দ্বিমিক_Stream_Ref(দ্বিমিক_Stream &Stream)
    : দ্বিমিক_Stream_Ref_Base(Stream) {}
দ্বিমিক_Stream_Ref::দ্বিমিক_Stream_Ref(দ্বিমিক_Stream &Stream, uint64_t Offset,
                                 std::optional<uint64_t> Length)
    : দ্বিমিক_Stream_Ref_Base(Stream, Offset, Length) {}
দ্বিমিক_Stream_Ref::দ্বিমিক_Stream_Ref(ArrayRef<uint8_t> Data, endianness Endian)
    : দ্বিমিক_Stream_Ref_Base(std::make_shared<ArrayRefImpl>(Data, Endian), 0,
                          Data.size()) {}
দ্বিমিক_Stream_Ref::দ্বিমিক_Stream_Ref(StringRef Data, endianness Endian)
    : দ্বিমিক_Stream_Ref(ArrayRef(Data.bytes_begin(), Data.bytes_end()), Endian) {}

Error দ্বিমিক_Stream_Ref::readBytes(uint64_t Offset, uint64_t Size,
                                 ArrayRef<uint8_t> &Buffer) ধ্রুব {
  যদি (auto EC = checkOffsetForRead(Offset, Size))
    প্রত্যাগম EC;
  প্রত্যাগম BorrowedImpl->readBytes(ViewOffset + Offset, Size, Buffer);
}

Error দ্বিমিক_Stream_Ref::readLongestContiguousChunk(
    uint64_t Offset, ArrayRef<uint8_t> &Buffer) ধ্রুব {
  যদি (auto EC = checkOffsetForRead(Offset, 1))
    প্রত্যাগম EC;

  যদি (auto EC =
          BorrowedImpl->readLongestContiguousChunk(ViewOffset + Offset, Buffer))
    প্রত্যাগম EC;
  // This StreamRef might refer to a smaller window over a larger stream.  In
  // that case we will have read out more bytes than we should return, because
  // we should not read past the end of the current view.
  uint64_t MaxLength = getLength() - Offset;
  যদি (Buffer.size() > MaxLength)
    Buffer = Buffer.slice(0, MaxLength);
  প্রত্যাগম Error::success();
}

Writable_দ্বিমিক_Stream_Ref::Writable_দ্বিমিক_Stream_Ref(Writable_দ্বিমিক_Stream &Stream)
    : দ্বিমিক_Stream_Ref_Base(Stream) {}

Writable_দ্বিমিক_Stream_Ref::Writable_দ্বিমিক_Stream_Ref(Writable_দ্বিমিক_Stream &Stream,
                                                 uint64_t Offset,
                                                 std::optional<uint64_t> Length)
    : দ্বিমিক_Stream_Ref_Base(Stream, Offset, Length) {}

Writable_দ্বিমিক_Stream_Ref::Writable_দ্বিমিক_Stream_Ref(MutableArrayRef<uint8_t> Data,
                                                 endianness Endian)
    : দ্বিমিক_Stream_Ref_Base(std::make_shared<MutableArrayRefImpl>(Data, Endian),
                          0, Data.size()) {}

Error Writable_দ্বিমিক_Stream_Ref::writeBytes(uint64_t Offset,
                                          ArrayRef<uint8_t> Data) ধ্রুব {
  যদি (auto EC = checkOffsetForWrite(Offset, Data.size()))
    প্রত্যাগম EC;

  প্রত্যাগম BorrowedImpl->writeBytes(ViewOffset + Offset, Data);
}

Writable_দ্বিমিক_Stream_Ref::operator দ্বিমিক_Stream_Ref() ধ্রুব {
  প্রত্যাগম দ্বিমিক_Stream_Ref(*BorrowedImpl, ViewOffset, Length);
}

/// For buffered streams, commits changes to the backing store.
Error Writable_দ্বিমিক_Stream_Ref::commit() { প্রত্যাগম BorrowedImpl->commit(); }
