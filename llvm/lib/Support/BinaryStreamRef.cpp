//===- BinaryStreamRef.cpp - ----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/Support/BinaryStreamRef.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/BinaryByteStream.h"

using ନାମାକାଶ llvm;

ନାମାକାଶ {

ଶ୍ରେଣୀ ArrayRefImpl : ସର୍ବସାଧାରଣ ଦ୍ୱିମିକ_Stream {
ସର୍ବସାଧାରଣ:
  ArrayRefImpl(ArrayRef<uint8_t> Data, endianness Endian) : BBS(Data, Endian) {}

  llvm::endianness getEndian() ଅଚଳ override { ଫେରନ୍ତୁ BBS.getEndian(); }
  Error readBytes(uint64_t Offset, uint64_t Size,
                  ArrayRef<uint8_t> &Buffer) override {
    ଫେରନ୍ତୁ BBS.readBytes(Offset, Size, Buffer);
  }
  Error readLongestContiguousChunk(uint64_t Offset,
                                   ArrayRef<uint8_t> &Buffer) override {
    ଫେରନ୍ତୁ BBS.readLongestContiguousChunk(Offset, Buffer);
  }
  uint64_t getLength() override { ଫେରନ୍ତୁ BBS.getLength(); }

ବ୍ୟକ୍ତିଗତ:
  ଦ୍ୱିମିକ_Byte_Stream BBS;
};

ଶ୍ରେଣୀ MutableArrayRefImpl : ସର୍ବସାଧାରଣ Writable_ଦ୍ୱିମିକ_Stream {
ସର୍ବସାଧାରଣ:
  MutableArrayRefImpl(MutableArrayRef<uint8_t> Data, endianness Endian)
      : BBS(Data, Endian) {}

  // Inherited via WritableBinaryStream
  llvm::endianness getEndian() ଅଚଳ override { ଫେରନ୍ତୁ BBS.getEndian(); }
  Error readBytes(uint64_t Offset, uint64_t Size,
                  ArrayRef<uint8_t> &Buffer) override {
    ଫେରନ୍ତୁ BBS.readBytes(Offset, Size, Buffer);
  }
  Error readLongestContiguousChunk(uint64_t Offset,
                                   ArrayRef<uint8_t> &Buffer) override {
    ଫେରନ୍ତୁ BBS.readLongestContiguousChunk(Offset, Buffer);
  }
  uint64_t getLength() override { ଫେରନ୍ତୁ BBS.getLength(); }

  Error writeBytes(uint64_t Offset, ArrayRef<uint8_t> Data) override {
    ଫେରନ୍ତୁ BBS.writeBytes(Offset, Data);
  }
  Error commit() override { ଫେରନ୍ତୁ BBS.commit(); }

ବ୍ୟକ୍ତିଗତ:
  Mutable_ଦ୍ୱିମିକ_Byte_Stream BBS;
};
} // namespace

ଦ୍ୱିମିକ_Stream_Ref::ଦ୍ୱିମିକ_Stream_Ref(ଦ୍ୱିମିକ_Stream &Stream)
    : ଦ୍ୱିମିକ_Stream_Ref_Base(Stream) {}
ଦ୍ୱିମିକ_Stream_Ref::ଦ୍ୱିମିକ_Stream_Ref(ଦ୍ୱିମିକ_Stream &Stream, uint64_t Offset,
                                 std::optional<uint64_t> Length)
    : ଦ୍ୱିମିକ_Stream_Ref_Base(Stream, Offset, Length) {}
ଦ୍ୱିମିକ_Stream_Ref::ଦ୍ୱିମିକ_Stream_Ref(ArrayRef<uint8_t> Data, endianness Endian)
    : ଦ୍ୱିମିକ_Stream_Ref_Base(std::make_shared<ArrayRefImpl>(Data, Endian), 0,
                          Data.size()) {}
ଦ୍ୱିମିକ_Stream_Ref::ଦ୍ୱିମିକ_Stream_Ref(StringRef Data, endianness Endian)
    : ଦ୍ୱିମିକ_Stream_Ref(ArrayRef(Data.bytes_begin(), Data.bytes_end()), Endian) {}

Error ଦ୍ୱିମିକ_Stream_Ref::readBytes(uint64_t Offset, uint64_t Size,
                                 ArrayRef<uint8_t> &Buffer) ଅଚଳ {
  ଯଦି (auto EC = checkOffsetForRead(Offset, Size))
    ଫେରନ୍ତୁ EC;
  ଫେରନ୍ତୁ BorrowedImpl->readBytes(ViewOffset + Offset, Size, Buffer);
}

Error ଦ୍ୱିମିକ_Stream_Ref::readLongestContiguousChunk(
    uint64_t Offset, ArrayRef<uint8_t> &Buffer) ଅଚଳ {
  ଯଦି (auto EC = checkOffsetForRead(Offset, 1))
    ଫେରନ୍ତୁ EC;

  ଯଦି (auto EC =
          BorrowedImpl->readLongestContiguousChunk(ViewOffset + Offset, Buffer))
    ଫେରନ୍ତୁ EC;
  // This StreamRef might refer to a smaller window over a larger stream.  In
  // that case we will have read out more bytes than we should return, because
  // we should not read past the end of the current view.
  uint64_t MaxLength = getLength() - Offset;
  ଯଦି (Buffer.size() > MaxLength)
    Buffer = Buffer.slice(0, MaxLength);
  ଫେରନ୍ତୁ Error::success();
}

Writable_ଦ୍ୱିମିକ_Stream_Ref::Writable_ଦ୍ୱିମିକ_Stream_Ref(Writable_ଦ୍ୱିମିକ_Stream &Stream)
    : ଦ୍ୱିମିକ_Stream_Ref_Base(Stream) {}

Writable_ଦ୍ୱିମିକ_Stream_Ref::Writable_ଦ୍ୱିମିକ_Stream_Ref(Writable_ଦ୍ୱିମିକ_Stream &Stream,
                                                 uint64_t Offset,
                                                 std::optional<uint64_t> Length)
    : ଦ୍ୱିମିକ_Stream_Ref_Base(Stream, Offset, Length) {}

Writable_ଦ୍ୱିମିକ_Stream_Ref::Writable_ଦ୍ୱିମିକ_Stream_Ref(MutableArrayRef<uint8_t> Data,
                                                 endianness Endian)
    : ଦ୍ୱିମିକ_Stream_Ref_Base(std::make_shared<MutableArrayRefImpl>(Data, Endian),
                          0, Data.size()) {}

Error Writable_ଦ୍ୱିମିକ_Stream_Ref::writeBytes(uint64_t Offset,
                                          ArrayRef<uint8_t> Data) ଅଚଳ {
  ଯଦି (auto EC = checkOffsetForWrite(Offset, Data.size()))
    ଫେରନ୍ତୁ EC;

  ଫେରନ୍ତୁ BorrowedImpl->writeBytes(ViewOffset + Offset, Data);
}

Writable_ଦ୍ୱିମିକ_Stream_Ref::operator ଦ୍ୱିମିକ_Stream_Ref() ଅଚଳ {
  ଫେରନ୍ତୁ ଦ୍ୱିମିକ_Stream_Ref(*BorrowedImpl, ViewOffset, Length);
}

/// For buffered streams, commits changes to the backing store.
Error Writable_ଦ୍ୱିମିକ_Stream_Ref::commit() { ଫେରନ୍ତୁ BorrowedImpl->commit(); }
