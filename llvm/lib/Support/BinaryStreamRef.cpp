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

صنف ArrayRefImpl : عام ثنائي_Stream {
عام:
  ArrayRefImpl(ArrayRef<uint8_t> Data, endianness Endian) : BBS(Data, Endian) {}

  llvm::endianness getEndian() ثابت override { إرجاع BBS.getEndian(); }
  Error readBytes(uint64_t Offset, uint64_t Size,
                  ArrayRef<uint8_t> &Buffer) override {
    إرجاع BBS.readBytes(Offset, Size, Buffer);
  }
  Error readLongestContiguousChunk(uint64_t Offset,
                                   ArrayRef<uint8_t> &Buffer) override {
    إرجاع BBS.readLongestContiguousChunk(Offset, Buffer);
  }
  uint64_t getLength() override { إرجاع BBS.getLength(); }

خاص:
  ثنائي_Byte_Stream BBS;
};

صنف MutableArrayRefImpl : عام Writable_ثنائي_Stream {
عام:
  MutableArrayRefImpl(MutableArrayRef<uint8_t> Data, endianness Endian)
      : BBS(Data, Endian) {}

  // Inherited via WritableBinaryStream
  llvm::endianness getEndian() ثابت override { إرجاع BBS.getEndian(); }
  Error readBytes(uint64_t Offset, uint64_t Size,
                  ArrayRef<uint8_t> &Buffer) override {
    إرجاع BBS.readBytes(Offset, Size, Buffer);
  }
  Error readLongestContiguousChunk(uint64_t Offset,
                                   ArrayRef<uint8_t> &Buffer) override {
    إرجاع BBS.readLongestContiguousChunk(Offset, Buffer);
  }
  uint64_t getLength() override { إرجاع BBS.getLength(); }

  Error writeBytes(uint64_t Offset, ArrayRef<uint8_t> Data) override {
    إرجاع BBS.writeBytes(Offset, Data);
  }
  Error commit() override { إرجاع BBS.commit(); }

خاص:
  Mutable_ثنائي_Byte_Stream BBS;
};
} // namespace

ثنائي_Stream_Ref::ثنائي_Stream_Ref(ثنائي_Stream &Stream)
    : ثنائي_Stream_Ref_Base(Stream) {}
ثنائي_Stream_Ref::ثنائي_Stream_Ref(ثنائي_Stream &Stream, uint64_t Offset,
                                 std::optional<uint64_t> Length)
    : ثنائي_Stream_Ref_Base(Stream, Offset, Length) {}
ثنائي_Stream_Ref::ثنائي_Stream_Ref(ArrayRef<uint8_t> Data, endianness Endian)
    : ثنائي_Stream_Ref_Base(std::make_shared<ArrayRefImpl>(Data, Endian), 0,
                          Data.size()) {}
ثنائي_Stream_Ref::ثنائي_Stream_Ref(StringRef Data, endianness Endian)
    : ثنائي_Stream_Ref(ArrayRef(Data.bytes_begin(), Data.bytes_end()), Endian) {}

Error ثنائي_Stream_Ref::readBytes(uint64_t Offset, uint64_t Size,
                                 ArrayRef<uint8_t> &Buffer) ثابت {
  إذا (auto EC = checkOffsetForRead(Offset, Size))
    إرجاع EC;
  إرجاع BorrowedImpl->readBytes(ViewOffset + Offset, Size, Buffer);
}

Error ثنائي_Stream_Ref::readLongestContiguousChunk(
    uint64_t Offset, ArrayRef<uint8_t> &Buffer) ثابت {
  إذا (auto EC = checkOffsetForRead(Offset, 1))
    إرجاع EC;

  إذا (auto EC =
          BorrowedImpl->readLongestContiguousChunk(ViewOffset + Offset, Buffer))
    إرجاع EC;
  // This StreamRef might refer to a smaller window over a larger stream.  In
  // that case we will have read out more bytes than we should return, because
  // we should not read past the end of the current view.
  uint64_t MaxLength = getLength() - Offset;
  إذا (Buffer.size() > MaxLength)
    Buffer = Buffer.slice(0, MaxLength);
  إرجاع Error::success();
}

Writable_ثنائي_Stream_Ref::Writable_ثنائي_Stream_Ref(Writable_ثنائي_Stream &Stream)
    : ثنائي_Stream_Ref_Base(Stream) {}

Writable_ثنائي_Stream_Ref::Writable_ثنائي_Stream_Ref(Writable_ثنائي_Stream &Stream,
                                                 uint64_t Offset,
                                                 std::optional<uint64_t> Length)
    : ثنائي_Stream_Ref_Base(Stream, Offset, Length) {}

Writable_ثنائي_Stream_Ref::Writable_ثنائي_Stream_Ref(MutableArrayRef<uint8_t> Data,
                                                 endianness Endian)
    : ثنائي_Stream_Ref_Base(std::make_shared<MutableArrayRefImpl>(Data, Endian),
                          0, Data.size()) {}

Error Writable_ثنائي_Stream_Ref::writeBytes(uint64_t Offset,
                                          ArrayRef<uint8_t> Data) ثابت {
  إذا (auto EC = checkOffsetForWrite(Offset, Data.size()))
    إرجاع EC;

  إرجاع BorrowedImpl->writeBytes(ViewOffset + Offset, Data);
}

Writable_ثنائي_Stream_Ref::operator ثنائي_Stream_Ref() ثابت {
  إرجاع ثنائي_Stream_Ref(*BorrowedImpl, ViewOffset, Length);
}

/// For buffered streams, commits changes to the backing store.
Error Writable_ثنائي_Stream_Ref::commit() { إرجاع BorrowedImpl->commit(); }
