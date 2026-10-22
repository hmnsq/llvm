//===- BinaryStreamRef.cpp - ----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/Support/BinaryStreamRef.h"
#അന്തര്ഭാവ "llvm/Support/BinaryByteStream.h"

using നാമാകാശ llvm;

നാമാകാശ {

വർഗ്ഗം ArrayRefImpl : പൊതു ദ്വിമാന_Stream {
പൊതു:
  ArrayRefImpl(ArrayRef<uint8_t> Data, endianness Endian) : BBS(Data, Endian) {}

  llvm::endianness getEndian() സ്ഥിരം override { തിരികെ BBS.getEndian(); }
  Error readBytes(uint64_t Offset, uint64_t Size,
                  ArrayRef<uint8_t> &Buffer) override {
    തിരികെ BBS.readBytes(Offset, Size, Buffer);
  }
  Error readLongestContiguousChunk(uint64_t Offset,
                                   ArrayRef<uint8_t> &Buffer) override {
    തിരികെ BBS.readLongestContiguousChunk(Offset, Buffer);
  }
  uint64_t getLength() override { തിരികെ BBS.getLength(); }

സ്വകാര്യ:
  ദ്വിമാന_Byte_Stream BBS;
};

വർഗ്ഗം MutableArrayRefImpl : പൊതു Writable_ദ്വിമാന_Stream {
പൊതു:
  MutableArrayRefImpl(MutableArrayRef<uint8_t> Data, endianness Endian)
      : BBS(Data, Endian) {}

  // Inherited via WritableBinaryStream
  llvm::endianness getEndian() സ്ഥിരം override { തിരികെ BBS.getEndian(); }
  Error readBytes(uint64_t Offset, uint64_t Size,
                  ArrayRef<uint8_t> &Buffer) override {
    തിരികെ BBS.readBytes(Offset, Size, Buffer);
  }
  Error readLongestContiguousChunk(uint64_t Offset,
                                   ArrayRef<uint8_t> &Buffer) override {
    തിരികെ BBS.readLongestContiguousChunk(Offset, Buffer);
  }
  uint64_t getLength() override { തിരികെ BBS.getLength(); }

  Error writeBytes(uint64_t Offset, ArrayRef<uint8_t> Data) override {
    തിരികെ BBS.writeBytes(Offset, Data);
  }
  Error commit() override { തിരികെ BBS.commit(); }

സ്വകാര്യ:
  Mutable_ദ്വിമാന_Byte_Stream BBS;
};
} // namespace

ദ്വിമാന_Stream_Ref::ദ്വിമാന_Stream_Ref(ദ്വിമാന_Stream &Stream)
    : ദ്വിമാന_Stream_Ref_Base(Stream) {}
ദ്വിമാന_Stream_Ref::ദ്വിമാന_Stream_Ref(ദ്വിമാന_Stream &Stream, uint64_t Offset,
                                 std::optional<uint64_t> Length)
    : ദ്വിമാന_Stream_Ref_Base(Stream, Offset, Length) {}
ദ്വിമാന_Stream_Ref::ദ്വിമാന_Stream_Ref(ArrayRef<uint8_t> Data, endianness Endian)
    : ദ്വിമാന_Stream_Ref_Base(std::make_shared<ArrayRefImpl>(Data, Endian), 0,
                          Data.size()) {}
ദ്വിമാന_Stream_Ref::ദ്വിമാന_Stream_Ref(StringRef Data, endianness Endian)
    : ദ്വിമാന_Stream_Ref(ArrayRef(Data.bytes_begin(), Data.bytes_end()), Endian) {}

Error ദ്വിമാന_Stream_Ref::readBytes(uint64_t Offset, uint64_t Size,
                                 ArrayRef<uint8_t> &Buffer) സ്ഥിരം {
  എങ്കിൽ (auto EC = checkOffsetForRead(Offset, Size))
    തിരികെ EC;
  തിരികെ BorrowedImpl->readBytes(ViewOffset + Offset, Size, Buffer);
}

Error ദ്വിമാന_Stream_Ref::readLongestContiguousChunk(
    uint64_t Offset, ArrayRef<uint8_t> &Buffer) സ്ഥിരം {
  എങ്കിൽ (auto EC = checkOffsetForRead(Offset, 1))
    തിരികെ EC;

  എങ്കിൽ (auto EC =
          BorrowedImpl->readLongestContiguousChunk(ViewOffset + Offset, Buffer))
    തിരികെ EC;
  // This StreamRef might refer to a smaller window over a larger stream.  In
  // that case we will have read out more bytes than we should return, because
  // we should not read past the end of the current view.
  uint64_t MaxLength = getLength() - Offset;
  എങ്കിൽ (Buffer.size() > MaxLength)
    Buffer = Buffer.slice(0, MaxLength);
  തിരികെ Error::success();
}

Writable_ദ്വിമാന_Stream_Ref::Writable_ദ്വിമാന_Stream_Ref(Writable_ദ്വിമാന_Stream &Stream)
    : ദ്വിമാന_Stream_Ref_Base(Stream) {}

Writable_ദ്വിമാന_Stream_Ref::Writable_ദ്വിമാന_Stream_Ref(Writable_ദ്വിമാന_Stream &Stream,
                                                 uint64_t Offset,
                                                 std::optional<uint64_t> Length)
    : ദ്വിമാന_Stream_Ref_Base(Stream, Offset, Length) {}

Writable_ദ്വിമാന_Stream_Ref::Writable_ദ്വിമാന_Stream_Ref(MutableArrayRef<uint8_t> Data,
                                                 endianness Endian)
    : ദ്വിമാന_Stream_Ref_Base(std::make_shared<MutableArrayRefImpl>(Data, Endian),
                          0, Data.size()) {}

Error Writable_ദ്വിമാന_Stream_Ref::writeBytes(uint64_t Offset,
                                          ArrayRef<uint8_t> Data) സ്ഥിരം {
  എങ്കിൽ (auto EC = checkOffsetForWrite(Offset, Data.size()))
    തിരികെ EC;

  തിരികെ BorrowedImpl->writeBytes(ViewOffset + Offset, Data);
}

Writable_ദ്വിമാന_Stream_Ref::operator ദ്വിമാന_Stream_Ref() സ്ഥിരം {
  തിരികെ ദ്വിമാന_Stream_Ref(*BorrowedImpl, ViewOffset, Length);
}

/// For buffered streams, commits changes to the backing store.
Error Writable_ദ്വിമാന_Stream_Ref::commit() { തിരികെ BorrowedImpl->commit(); }
