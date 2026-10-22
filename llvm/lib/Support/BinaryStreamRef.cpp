//===- BinaryStreamRef.cpp - ----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/BinaryStreamRef.h"
#अन्तर्भाव "llvm/Support/BinaryByteStream.h"

using नामाकाश llvm;

नामाकाश {

वर्ग ArrayRefImpl : सार्वजनिक द्विआधारी_Stream {
सार्वजनिक:
  ArrayRefImpl(ArrayRef<uint8_t> Data, endianness Endian) : BBS(Data, Endian) {}

  llvm::endianness getEndian() ध्रुव override { प्रत्यागम BBS.getEndian(); }
  Error readBytes(uint64_t Offset, uint64_t Size,
                  ArrayRef<uint8_t> &Buffer) override {
    प्रत्यागम BBS.readBytes(Offset, Size, Buffer);
  }
  Error readLongestContiguousChunk(uint64_t Offset,
                                   ArrayRef<uint8_t> &Buffer) override {
    प्रत्यागम BBS.readLongestContiguousChunk(Offset, Buffer);
  }
  uint64_t getLength() override { प्रत्यागम BBS.getLength(); }

गुप्त:
  द्विआधारी_Byte_Stream BBS;
};

वर्ग MutableArrayRefImpl : सार्वजनिक Writable_द्विआधारी_Stream {
सार्वजनिक:
  MutableArrayRefImpl(MutableArrayRef<uint8_t> Data, endianness Endian)
      : BBS(Data, Endian) {}

  // Inherited via WritableBinaryStream
  llvm::endianness getEndian() ध्रुव override { प्रत्यागम BBS.getEndian(); }
  Error readBytes(uint64_t Offset, uint64_t Size,
                  ArrayRef<uint8_t> &Buffer) override {
    प्रत्यागम BBS.readBytes(Offset, Size, Buffer);
  }
  Error readLongestContiguousChunk(uint64_t Offset,
                                   ArrayRef<uint8_t> &Buffer) override {
    प्रत्यागम BBS.readLongestContiguousChunk(Offset, Buffer);
  }
  uint64_t getLength() override { प्रत्यागम BBS.getLength(); }

  Error writeBytes(uint64_t Offset, ArrayRef<uint8_t> Data) override {
    प्रत्यागम BBS.writeBytes(Offset, Data);
  }
  Error commit() override { प्रत्यागम BBS.commit(); }

गुप्त:
  Mutable_द्विआधारी_Byte_Stream BBS;
};
} // namespace

द्विआधारी_Stream_Ref::द्विआधारी_Stream_Ref(द्विआधारी_Stream &Stream)
    : द्विआधारी_Stream_Ref_Base(Stream) {}
द्विआधारी_Stream_Ref::द्विआधारी_Stream_Ref(द्विआधारी_Stream &Stream, uint64_t Offset,
                                 std::optional<uint64_t> Length)
    : द्विआधारी_Stream_Ref_Base(Stream, Offset, Length) {}
द्विआधारी_Stream_Ref::द्विआधारी_Stream_Ref(ArrayRef<uint8_t> Data, endianness Endian)
    : द्विआधारी_Stream_Ref_Base(std::make_shared<ArrayRefImpl>(Data, Endian), 0,
                          Data.size()) {}
द्विआधारी_Stream_Ref::द्विआधारी_Stream_Ref(StringRef Data, endianness Endian)
    : द्विआधारी_Stream_Ref(ArrayRef(Data.bytes_begin(), Data.bytes_end()), Endian) {}

Error द्विआधारी_Stream_Ref::readBytes(uint64_t Offset, uint64_t Size,
                                 ArrayRef<uint8_t> &Buffer) ध्रुव {
  यदि (स्वतः EC = checkOffsetForRead(Offset, Size))
    प्रत्यागम EC;
  प्रत्यागम BorrowedImpl->readBytes(ViewOffset + Offset, Size, Buffer);
}

Error द्विआधारी_Stream_Ref::readLongestContiguousChunk(
    uint64_t Offset, ArrayRef<uint8_t> &Buffer) ध्रुव {
  यदि (स्वतः EC = checkOffsetForRead(Offset, 1))
    प्रत्यागम EC;

  यदि (स्वतः EC =
          BorrowedImpl->readLongestContiguousChunk(ViewOffset + Offset, Buffer))
    प्रत्यागम EC;
  // This StreamRef might refer to a smaller window over a larger stream.  In
  // that case we will have read out more bytes than we should return, because
  // we should not read past the end of the current view.
  uint64_t MaxLength = getLength() - Offset;
  यदि (Buffer.size() > MaxLength)
    Buffer = Buffer.slice(0, MaxLength);
  प्रत्यागम Error::success();
}

Writable_द्विआधारी_Stream_Ref::Writable_द्विआधारी_Stream_Ref(Writable_द्विआधारी_Stream &Stream)
    : द्विआधारी_Stream_Ref_Base(Stream) {}

Writable_द्विआधारी_Stream_Ref::Writable_द्विआधारी_Stream_Ref(Writable_द्विआधारी_Stream &Stream,
                                                 uint64_t Offset,
                                                 std::optional<uint64_t> Length)
    : द्विआधारी_Stream_Ref_Base(Stream, Offset, Length) {}

Writable_द्विआधारी_Stream_Ref::Writable_द्विआधारी_Stream_Ref(MutableArrayRef<uint8_t> Data,
                                                 endianness Endian)
    : द्विआधारी_Stream_Ref_Base(std::make_shared<MutableArrayRefImpl>(Data, Endian),
                          0, Data.size()) {}

Error Writable_द्विआधारी_Stream_Ref::writeBytes(uint64_t Offset,
                                          ArrayRef<uint8_t> Data) ध्रुव {
  यदि (स्वतः EC = checkOffsetForWrite(Offset, Data.size()))
    प्रत्यागम EC;

  प्रत्यागम BorrowedImpl->writeBytes(ViewOffset + Offset, Data);
}

Writable_द्विआधारी_Stream_Ref::चालक द्विआधारी_Stream_Ref() ध्रुव {
  प्रत्यागम द्विआधारी_Stream_Ref(*BorrowedImpl, ViewOffset, Length);
}

/// For buffered streams, commits changes to the backing store.
Error Writable_द्विआधारी_Stream_Ref::commit() { प्रत्यागम BorrowedImpl->commit(); }
