//===- BinaryStreamRef.cpp - ----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/Support/BinaryStreamRef.h"
#அந்தர்பாவ "llvm/Support/BinaryByteStream.h"

using நாமாகாஶ llvm;

நாமாகாஶ {

வகுப்பு ArrayRefImpl : பொது இருமம்_Stream {
பொது:
  ArrayRefImpl(ArrayRef<uint8_t> Data, endianness Endian) : BBS(Data, Endian) {}

  llvm::endianness getEndian() மாறிலி override { திரும்பு BBS.getEndian(); }
  Error readBytes(uint64_t Offset, uint64_t Size,
                  ArrayRef<uint8_t> &Buffer) override {
    திரும்பு BBS.readBytes(Offset, Size, Buffer);
  }
  Error readLongestContiguousChunk(uint64_t Offset,
                                   ArrayRef<uint8_t> &Buffer) override {
    திரும்பு BBS.readLongestContiguousChunk(Offset, Buffer);
  }
  uint64_t getLength() override { திரும்பு BBS.getLength(); }

தனிப்பட்ட:
  இருமம்_Byte_Stream BBS;
};

வகுப்பு MutableArrayRefImpl : பொது Writable_இருமம்_Stream {
பொது:
  MutableArrayRefImpl(MutableArrayRef<uint8_t> Data, endianness Endian)
      : BBS(Data, Endian) {}

  // Inherited via WritableBinaryStream
  llvm::endianness getEndian() மாறிலி override { திரும்பு BBS.getEndian(); }
  Error readBytes(uint64_t Offset, uint64_t Size,
                  ArrayRef<uint8_t> &Buffer) override {
    திரும்பு BBS.readBytes(Offset, Size, Buffer);
  }
  Error readLongestContiguousChunk(uint64_t Offset,
                                   ArrayRef<uint8_t> &Buffer) override {
    திரும்பு BBS.readLongestContiguousChunk(Offset, Buffer);
  }
  uint64_t getLength() override { திரும்பு BBS.getLength(); }

  Error writeBytes(uint64_t Offset, ArrayRef<uint8_t> Data) override {
    திரும்பு BBS.writeBytes(Offset, Data);
  }
  Error commit() override { திரும்பு BBS.commit(); }

தனிப்பட்ட:
  Mutable_இருமம்_Byte_Stream BBS;
};
} // namespace

இருமம்_Stream_Ref::இருமம்_Stream_Ref(இருமம்_Stream &Stream)
    : இருமம்_Stream_Ref_Base(Stream) {}
இருமம்_Stream_Ref::இருமம்_Stream_Ref(இருமம்_Stream &Stream, uint64_t Offset,
                                 std::optional<uint64_t> Length)
    : இருமம்_Stream_Ref_Base(Stream, Offset, Length) {}
இருமம்_Stream_Ref::இருமம்_Stream_Ref(ArrayRef<uint8_t> Data, endianness Endian)
    : இருமம்_Stream_Ref_Base(std::make_shared<ArrayRefImpl>(Data, Endian), 0,
                          Data.size()) {}
இருமம்_Stream_Ref::இருமம்_Stream_Ref(StringRef Data, endianness Endian)
    : இருமம்_Stream_Ref(ArrayRef(Data.bytes_begin(), Data.bytes_end()), Endian) {}

Error இருமம்_Stream_Ref::readBytes(uint64_t Offset, uint64_t Size,
                                 ArrayRef<uint8_t> &Buffer) மாறிலி {
  எனில் (auto EC = checkOffsetForRead(Offset, Size))
    திரும்பு EC;
  திரும்பு BorrowedImpl->readBytes(ViewOffset + Offset, Size, Buffer);
}

Error இருமம்_Stream_Ref::readLongestContiguousChunk(
    uint64_t Offset, ArrayRef<uint8_t> &Buffer) மாறிலி {
  எனில் (auto EC = checkOffsetForRead(Offset, 1))
    திரும்பு EC;

  எனில் (auto EC =
          BorrowedImpl->readLongestContiguousChunk(ViewOffset + Offset, Buffer))
    திரும்பு EC;
  // This StreamRef might refer to a smaller window over a larger stream.  In
  // that case we will have read out more bytes than we should return, because
  // we should not read past the end of the current view.
  uint64_t MaxLength = getLength() - Offset;
  எனில் (Buffer.size() > MaxLength)
    Buffer = Buffer.slice(0, MaxLength);
  திரும்பு Error::success();
}

Writable_இருமம்_Stream_Ref::Writable_இருமம்_Stream_Ref(Writable_இருமம்_Stream &Stream)
    : இருமம்_Stream_Ref_Base(Stream) {}

Writable_இருமம்_Stream_Ref::Writable_இருமம்_Stream_Ref(Writable_இருமம்_Stream &Stream,
                                                 uint64_t Offset,
                                                 std::optional<uint64_t> Length)
    : இருமம்_Stream_Ref_Base(Stream, Offset, Length) {}

Writable_இருமம்_Stream_Ref::Writable_இருமம்_Stream_Ref(MutableArrayRef<uint8_t> Data,
                                                 endianness Endian)
    : இருமம்_Stream_Ref_Base(std::make_shared<MutableArrayRefImpl>(Data, Endian),
                          0, Data.size()) {}

Error Writable_இருமம்_Stream_Ref::writeBytes(uint64_t Offset,
                                          ArrayRef<uint8_t> Data) மாறிலி {
  எனில் (auto EC = checkOffsetForWrite(Offset, Data.size()))
    திரும்பு EC;

  திரும்பு BorrowedImpl->writeBytes(ViewOffset + Offset, Data);
}

Writable_இருமம்_Stream_Ref::operator இருமம்_Stream_Ref() மாறிலி {
  திரும்பு இருமம்_Stream_Ref(*BorrowedImpl, ViewOffset, Length);
}

/// For buffered streams, commits changes to the backing store.
Error Writable_இருமம்_Stream_Ref::commit() { திரும்பு BorrowedImpl->commit(); }
