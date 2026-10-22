//===- BinaryStreamRef.cpp - ----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/Support/BinaryStreamRef.h"
#અન્તર્ભાવ "llvm/Support/BinaryByteStream.h"

using નામાકાશ llvm;

નામાકાશ {

વર્ગ ArrayRefImpl : જાહેર દ્વિઅંકી_Stream {
જાહેર:
  ArrayRefImpl(ArrayRef<uint8_t> Data, endianness Endian) : BBS(Data, Endian) {}

  llvm::endianness getEndian() અચલ override { પાછા BBS.getEndian(); }
  Error readBytes(uint64_t Offset, uint64_t Size,
                  ArrayRef<uint8_t> &Buffer) override {
    પાછા BBS.readBytes(Offset, Size, Buffer);
  }
  Error readLongestContiguousChunk(uint64_t Offset,
                                   ArrayRef<uint8_t> &Buffer) override {
    પાછા BBS.readLongestContiguousChunk(Offset, Buffer);
  }
  uint64_t getLength() override { પાછા BBS.getLength(); }

ખાનગી:
  દ્વિઅંકી_Byte_Stream BBS;
};

વર્ગ MutableArrayRefImpl : જાહેર Writable_દ્વિઅંકી_Stream {
જાહેર:
  MutableArrayRefImpl(MutableArrayRef<uint8_t> Data, endianness Endian)
      : BBS(Data, Endian) {}

  // Inherited via WritableBinaryStream
  llvm::endianness getEndian() અચલ override { પાછા BBS.getEndian(); }
  Error readBytes(uint64_t Offset, uint64_t Size,
                  ArrayRef<uint8_t> &Buffer) override {
    પાછા BBS.readBytes(Offset, Size, Buffer);
  }
  Error readLongestContiguousChunk(uint64_t Offset,
                                   ArrayRef<uint8_t> &Buffer) override {
    પાછા BBS.readLongestContiguousChunk(Offset, Buffer);
  }
  uint64_t getLength() override { પાછા BBS.getLength(); }

  Error writeBytes(uint64_t Offset, ArrayRef<uint8_t> Data) override {
    પાછા BBS.writeBytes(Offset, Data);
  }
  Error commit() override { પાછા BBS.commit(); }

ખાનગી:
  Mutable_દ્વિઅંકી_Byte_Stream BBS;
};
} // namespace

દ્વિઅંકી_Stream_Ref::દ્વિઅંકી_Stream_Ref(દ્વિઅંકી_Stream &Stream)
    : દ્વિઅંકી_Stream_Ref_Base(Stream) {}
દ્વિઅંકી_Stream_Ref::દ્વિઅંકી_Stream_Ref(દ્વિઅંકી_Stream &Stream, uint64_t Offset,
                                 std::optional<uint64_t> Length)
    : દ્વિઅંકી_Stream_Ref_Base(Stream, Offset, Length) {}
દ્વિઅંકી_Stream_Ref::દ્વિઅંકી_Stream_Ref(ArrayRef<uint8_t> Data, endianness Endian)
    : દ્વિઅંકી_Stream_Ref_Base(std::make_shared<ArrayRefImpl>(Data, Endian), 0,
                          Data.size()) {}
દ્વિઅંકી_Stream_Ref::દ્વિઅંકી_Stream_Ref(StringRef Data, endianness Endian)
    : દ્વિઅંકી_Stream_Ref(ArrayRef(Data.bytes_begin(), Data.bytes_end()), Endian) {}

Error દ્વિઅંકી_Stream_Ref::readBytes(uint64_t Offset, uint64_t Size,
                                 ArrayRef<uint8_t> &Buffer) અચલ {
  જો (auto EC = checkOffsetForRead(Offset, Size))
    પાછા EC;
  પાછા BorrowedImpl->readBytes(ViewOffset + Offset, Size, Buffer);
}

Error દ્વિઅંકી_Stream_Ref::readLongestContiguousChunk(
    uint64_t Offset, ArrayRef<uint8_t> &Buffer) અચલ {
  જો (auto EC = checkOffsetForRead(Offset, 1))
    પાછા EC;

  જો (auto EC =
          BorrowedImpl->readLongestContiguousChunk(ViewOffset + Offset, Buffer))
    પાછા EC;
  // This StreamRef might refer to a smaller window over a larger stream.  In
  // that case we will have read out more bytes than we should return, because
  // we should not read past the end of the current view.
  uint64_t MaxLength = getLength() - Offset;
  જો (Buffer.size() > MaxLength)
    Buffer = Buffer.slice(0, MaxLength);
  પાછા Error::success();
}

Writable_દ્વિઅંકી_Stream_Ref::Writable_દ્વિઅંકી_Stream_Ref(Writable_દ્વિઅંકી_Stream &Stream)
    : દ્વિઅંકી_Stream_Ref_Base(Stream) {}

Writable_દ્વિઅંકી_Stream_Ref::Writable_દ્વિઅંકી_Stream_Ref(Writable_દ્વિઅંકી_Stream &Stream,
                                                 uint64_t Offset,
                                                 std::optional<uint64_t> Length)
    : દ્વિઅંકી_Stream_Ref_Base(Stream, Offset, Length) {}

Writable_દ્વિઅંકી_Stream_Ref::Writable_દ્વિઅંકી_Stream_Ref(MutableArrayRef<uint8_t> Data,
                                                 endianness Endian)
    : દ્વિઅંકી_Stream_Ref_Base(std::make_shared<MutableArrayRefImpl>(Data, Endian),
                          0, Data.size()) {}

Error Writable_દ્વિઅંકી_Stream_Ref::writeBytes(uint64_t Offset,
                                          ArrayRef<uint8_t> Data) અચલ {
  જો (auto EC = checkOffsetForWrite(Offset, Data.size()))
    પાછા EC;

  પાછા BorrowedImpl->writeBytes(ViewOffset + Offset, Data);
}

Writable_દ્વિઅંકી_Stream_Ref::operator દ્વિઅંકી_Stream_Ref() અચલ {
  પાછા દ્વિઅંકી_Stream_Ref(*BorrowedImpl, ViewOffset, Length);
}

/// For buffered streams, commits changes to the backing store.
Error Writable_દ્વિઅંકી_Stream_Ref::commit() { પાછા BorrowedImpl->commit(); }
