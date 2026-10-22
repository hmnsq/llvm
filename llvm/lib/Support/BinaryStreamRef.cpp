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

वर्ग ArrayRefImpl : सार्वजनिक द्विमान_Stream {
सार्वजनिक:
  ArrayRefImpl(ArrayRef<uint8_t> Data, endianness Endian) : BBS(Data, Endian) {}

  llvm::endianness getEndian() स्थिरांक override { परत BBS.getEndian(); }
  Error readBytes(uint64_t Offset, uint64_t Size,
                  ArrayRef<uint8_t> &Buffer) override {
    परत BBS.readBytes(Offset, Size, Buffer);
  }
  Error readLongestContiguousChunk(uint64_t Offset,
                                   ArrayRef<uint8_t> &Buffer) override {
    परत BBS.readLongestContiguousChunk(Offset, Buffer);
  }
  uint64_t getLength() override { परत BBS.getLength(); }

खाजगी:
  द्विमान_Byte_Stream BBS;
};

वर्ग MutableArrayRefImpl : सार्वजनिक Writable_द्विमान_Stream {
सार्वजनिक:
  MutableArrayRefImpl(MutableArrayRef<uint8_t> Data, endianness Endian)
      : BBS(Data, Endian) {}

  // Inherited via WritableBinaryStream
  llvm::endianness getEndian() स्थिरांक override { परत BBS.getEndian(); }
  Error readBytes(uint64_t Offset, uint64_t Size,
                  ArrayRef<uint8_t> &Buffer) override {
    परत BBS.readBytes(Offset, Size, Buffer);
  }
  Error readLongestContiguousChunk(uint64_t Offset,
                                   ArrayRef<uint8_t> &Buffer) override {
    परत BBS.readLongestContiguousChunk(Offset, Buffer);
  }
  uint64_t getLength() override { परत BBS.getLength(); }

  Error writeBytes(uint64_t Offset, ArrayRef<uint8_t> Data) override {
    परत BBS.writeBytes(Offset, Data);
  }
  Error commit() override { परत BBS.commit(); }

खाजगी:
  Mutable_द्विमान_Byte_Stream BBS;
};
} // namespace

द्विमान_Stream_Ref::द्विमान_Stream_Ref(द्विमान_Stream &Stream)
    : द्विमान_Stream_Ref_Base(Stream) {}
द्विमान_Stream_Ref::द्विमान_Stream_Ref(द्विमान_Stream &Stream, uint64_t Offset,
                                 std::optional<uint64_t> Length)
    : द्विमान_Stream_Ref_Base(Stream, Offset, Length) {}
द्विमान_Stream_Ref::द्विमान_Stream_Ref(ArrayRef<uint8_t> Data, endianness Endian)
    : द्विमान_Stream_Ref_Base(std::make_shared<ArrayRefImpl>(Data, Endian), 0,
                          Data.size()) {}
द्विमान_Stream_Ref::द्विमान_Stream_Ref(StringRef Data, endianness Endian)
    : द्विमान_Stream_Ref(ArrayRef(Data.bytes_begin(), Data.bytes_end()), Endian) {}

Error द्विमान_Stream_Ref::readBytes(uint64_t Offset, uint64_t Size,
                                 ArrayRef<uint8_t> &Buffer) स्थिरांक {
  जर (स्वतः EC = checkOffsetForRead(Offset, Size))
    परत EC;
  परत BorrowedImpl->readBytes(ViewOffset + Offset, Size, Buffer);
}

Error द्विमान_Stream_Ref::readLongestContiguousChunk(
    uint64_t Offset, ArrayRef<uint8_t> &Buffer) स्थिरांक {
  जर (स्वतः EC = checkOffsetForRead(Offset, 1))
    परत EC;

  जर (स्वतः EC =
          BorrowedImpl->readLongestContiguousChunk(ViewOffset + Offset, Buffer))
    परत EC;
  // This StreamRef might refer to a smaller window over a larger stream.  In
  // that case we will have read out more bytes than we should return, because
  // we should not read past the end of the current view.
  uint64_t MaxLength = getLength() - Offset;
  जर (Buffer.size() > MaxLength)
    Buffer = Buffer.slice(0, MaxLength);
  परत Error::success();
}

Writable_द्विमान_Stream_Ref::Writable_द्विमान_Stream_Ref(Writable_द्विमान_Stream &Stream)
    : द्विमान_Stream_Ref_Base(Stream) {}

Writable_द्विमान_Stream_Ref::Writable_द्विमान_Stream_Ref(Writable_द्विमान_Stream &Stream,
                                                 uint64_t Offset,
                                                 std::optional<uint64_t> Length)
    : द्विमान_Stream_Ref_Base(Stream, Offset, Length) {}

Writable_द्विमान_Stream_Ref::Writable_द्विमान_Stream_Ref(MutableArrayRef<uint8_t> Data,
                                                 endianness Endian)
    : द्विमान_Stream_Ref_Base(std::make_shared<MutableArrayRefImpl>(Data, Endian),
                          0, Data.size()) {}

Error Writable_द्विमान_Stream_Ref::writeBytes(uint64_t Offset,
                                          ArrayRef<uint8_t> Data) स्थिरांक {
  जर (स्वतः EC = checkOffsetForWrite(Offset, Data.size()))
    परत EC;

  परत BorrowedImpl->writeBytes(ViewOffset + Offset, Data);
}

Writable_द्विमान_Stream_Ref::चालक द्विमान_Stream_Ref() स्थिरांक {
  परत द्विमान_Stream_Ref(*BorrowedImpl, ViewOffset, Length);
}

/// For buffered streams, commits changes to the backing store.
Error Writable_द्विमान_Stream_Ref::commit() { परत BorrowedImpl->commit(); }
