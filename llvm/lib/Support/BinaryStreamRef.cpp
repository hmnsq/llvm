//===- BinaryStreamRef.cpp - ----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/BinaryStreamRef.h"
#شامل "llvm/Support/BinaryByteStream.h"

using فضاینام llvm;

فضاینام {

رده ArrayRefImpl : عمومی دودویی_Stream {
عمومی:
  ArrayRefImpl(ArrayRef<uint8_t> Data, endianness Endian) : BBS(Data, Endian) {}

  llvm::endianness getEndian() ثابت override { بازگشت BBS.getEndian(); }
  Error readBytes(uint64_t Offset, uint64_t Size,
                  ArrayRef<uint8_t> &Buffer) override {
    بازگشت BBS.readBytes(Offset, Size, Buffer);
  }
  Error readLongestContiguousChunk(uint64_t Offset,
                                   ArrayRef<uint8_t> &Buffer) override {
    بازگشت BBS.readLongestContiguousChunk(Offset, Buffer);
  }
  uint64_t getLength() override { بازگشت BBS.getLength(); }

خصوصی:
  دودویی_Byte_Stream BBS;
};

رده MutableArrayRefImpl : عمومی Writable_دودویی_Stream {
عمومی:
  MutableArrayRefImpl(MutableArrayRef<uint8_t> Data, endianness Endian)
      : BBS(Data, Endian) {}

  // Inherited via WritableBinaryStream
  llvm::endianness getEndian() ثابت override { بازگشت BBS.getEndian(); }
  Error readBytes(uint64_t Offset, uint64_t Size,
                  ArrayRef<uint8_t> &Buffer) override {
    بازگشت BBS.readBytes(Offset, Size, Buffer);
  }
  Error readLongestContiguousChunk(uint64_t Offset,
                                   ArrayRef<uint8_t> &Buffer) override {
    بازگشت BBS.readLongestContiguousChunk(Offset, Buffer);
  }
  uint64_t getLength() override { بازگشت BBS.getLength(); }

  Error writeBytes(uint64_t Offset, ArrayRef<uint8_t> Data) override {
    بازگشت BBS.writeBytes(Offset, Data);
  }
  Error commit() override { بازگشت BBS.commit(); }

خصوصی:
  Mutable_دودویی_Byte_Stream BBS;
};
} // namespace

دودویی_Stream_Ref::دودویی_Stream_Ref(دودویی_Stream &Stream)
    : دودویی_Stream_Ref_Base(Stream) {}
دودویی_Stream_Ref::دودویی_Stream_Ref(دودویی_Stream &Stream, uint64_t Offset,
                                 std::optional<uint64_t> Length)
    : دودویی_Stream_Ref_Base(Stream, Offset, Length) {}
دودویی_Stream_Ref::دودویی_Stream_Ref(ArrayRef<uint8_t> Data, endianness Endian)
    : دودویی_Stream_Ref_Base(std::make_shared<ArrayRefImpl>(Data, Endian), 0,
                          Data.size()) {}
دودویی_Stream_Ref::دودویی_Stream_Ref(StringRef Data, endianness Endian)
    : دودویی_Stream_Ref(ArrayRef(Data.bytes_begin(), Data.bytes_end()), Endian) {}

Error دودویی_Stream_Ref::readBytes(uint64_t Offset, uint64_t Size,
                                 ArrayRef<uint8_t> &Buffer) ثابت {
  اگر (auto EC = checkOffsetForRead(Offset, Size))
    بازگشت EC;
  بازگشت BorrowedImpl->readBytes(ViewOffset + Offset, Size, Buffer);
}

Error دودویی_Stream_Ref::readLongestContiguousChunk(
    uint64_t Offset, ArrayRef<uint8_t> &Buffer) ثابت {
  اگر (auto EC = checkOffsetForRead(Offset, 1))
    بازگشت EC;

  اگر (auto EC =
          BorrowedImpl->readLongestContiguousChunk(ViewOffset + Offset, Buffer))
    بازگشت EC;
  // This StreamRef might refer to a smaller window over a larger stream.  In
  // that case we will have read out more bytes than we should return, because
  // we should not read past the end of the current view.
  uint64_t MaxLength = getLength() - Offset;
  اگر (Buffer.size() > MaxLength)
    Buffer = Buffer.slice(0, MaxLength);
  بازگشت Error::success();
}

Writable_دودویی_Stream_Ref::Writable_دودویی_Stream_Ref(Writable_دودویی_Stream &Stream)
    : دودویی_Stream_Ref_Base(Stream) {}

Writable_دودویی_Stream_Ref::Writable_دودویی_Stream_Ref(Writable_دودویی_Stream &Stream,
                                                 uint64_t Offset,
                                                 std::optional<uint64_t> Length)
    : دودویی_Stream_Ref_Base(Stream, Offset, Length) {}

Writable_دودویی_Stream_Ref::Writable_دودویی_Stream_Ref(MutableArrayRef<uint8_t> Data,
                                                 endianness Endian)
    : دودویی_Stream_Ref_Base(std::make_shared<MutableArrayRefImpl>(Data, Endian),
                          0, Data.size()) {}

Error Writable_دودویی_Stream_Ref::writeBytes(uint64_t Offset,
                                          ArrayRef<uint8_t> Data) ثابت {
  اگر (auto EC = checkOffsetForWrite(Offset, Data.size()))
    بازگشت EC;

  بازگشت BorrowedImpl->writeBytes(ViewOffset + Offset, Data);
}

Writable_دودویی_Stream_Ref::operator دودویی_Stream_Ref() ثابت {
  بازگشت دودویی_Stream_Ref(*BorrowedImpl, ViewOffset, Length);
}

/// For buffered streams, commits changes to the backing store.
Error Writable_دودویی_Stream_Ref::commit() { بازگشت BorrowedImpl->commit(); }
