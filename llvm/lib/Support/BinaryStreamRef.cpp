//===- BinaryStreamRef.cpp - ----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/BinaryStreamRef.h"
#समावेश "llvm/Support/BinaryByteStream.h"

using namespace llvm;

namespace {

श्रेणी ArrayRefImpl : खुला द्विआधारी_Stream {
खुला:
  ArrayRefImpl(ArrayRef<uint8_t> तथ्य_2, endianness Endian) : BBS(तथ्य_2, Endian) {}

  llvm::endianness getEndian() स्थिर override { वापस BBS.getEndian(); }
  Error readBytes(uint64_t Offset, uint64_t आकार,
                  ArrayRef<uint8_t> &बफर) override {
    वापस BBS.readBytes(Offset, आकार, बफर);
  }
  Error readLongestContiguousChunk(uint64_t Offset,
                                   ArrayRef<uint8_t> &बफर) override {
    वापस BBS.readLongestContiguousChunk(Offset, बफर);
  }
  uint64_t get_लंबाई() override { वापस BBS.get_लंबाई(); }

निजी:
  द्विआधारी_Byte_Stream BBS;
};

श्रेणी MutableArrayRefImpl : खुला Writable_द्विआधारी_Stream {
खुला:
  MutableArrayRefImpl(MutableArrayRef<uint8_t> तथ्य_2, endianness Endian)
      : BBS(तथ्य_2, Endian) {}

  // Inherited via WritableBinaryStream
  llvm::endianness getEndian() स्थिर override { वापस BBS.getEndian(); }
  Error readBytes(uint64_t Offset, uint64_t आकार,
                  ArrayRef<uint8_t> &बफर) override {
    वापस BBS.readBytes(Offset, आकार, बफर);
  }
  Error readLongestContiguousChunk(uint64_t Offset,
                                   ArrayRef<uint8_t> &बफर) override {
    वापस BBS.readLongestContiguousChunk(Offset, बफर);
  }
  uint64_t get_लंबाई() override { वापस BBS.get_लंबाई(); }

  Error writeBytes(uint64_t Offset, ArrayRef<uint8_t> तथ्य_2) override {
    वापस BBS.writeBytes(Offset, तथ्य_2);
  }
  Error commit() override { वापस BBS.commit(); }

निजी:
  Mutable_द्विआधारी_Byte_Stream BBS;
};
} // namespace

द्विआधारी_Stream_Ref::द्विआधारी_Stream_Ref(द्विआधारी_Stream &Stream)
    : द्विआधारी_Stream_Ref_Base(Stream) {}
द्विआधारी_Stream_Ref::द्विआधारी_Stream_Ref(द्विआधारी_Stream &Stream, uint64_t Offset,
                                 std::optional<uint64_t> लंबाई)
    : द्विआधारी_Stream_Ref_Base(Stream, Offset, लंबाई) {}
द्विआधारी_Stream_Ref::द्विआधारी_Stream_Ref(ArrayRef<uint8_t> तथ्य_2, endianness Endian)
    : द्विआधारी_Stream_Ref_Base(std::make_shared<ArrayRefImpl>(तथ्य_2, Endian), 0,
                          तथ्य_2.आकार_2()) {}
द्विआधारी_Stream_Ref::द्विआधारी_Stream_Ref(StringRef तथ्य_2, endianness Endian)
    : द्विआधारी_Stream_Ref(ArrayRef(तथ्य_2.bytes_begin(), तथ्य_2.bytes_end()), Endian) {}

Error द्विआधारी_Stream_Ref::readBytes(uint64_t Offset, uint64_t आकार,
                                 ArrayRef<uint8_t> &बफर) स्थिर {
  अगर (स्वतः EC = checkOffsetForRead(Offset, आकार))
    वापस EC;
  वापस BorrowedImpl->readBytes(ViewOffset + Offset, आकार, बफर);
}

Error द्विआधारी_Stream_Ref::readLongestContiguousChunk(
    uint64_t Offset, ArrayRef<uint8_t> &बफर) स्थिर {
  अगर (स्वतः EC = checkOffsetForRead(Offset, 1))
    वापस EC;

  अगर (स्वतः EC =
          BorrowedImpl->readLongestContiguousChunk(ViewOffset + Offset, बफर))
    वापस EC;
  // This StreamRef might refer to a smaller window over a larger stream.  In
  // that case we will have read out more bytes than we should return, because
  // we should not read past the end of the current view.
  uint64_t Max_लंबाई = get_लंबाई() - Offset;
  अगर (बफर.आकार_2() > Max_लंबाई)
    बफर = बफर.slice(0, Max_लंबाई);
  वापस Error::success();
}

Writable_द्विआधारी_Stream_Ref::Writable_द्विआधारी_Stream_Ref(Writable_द्विआधारी_Stream &Stream)
    : द्विआधारी_Stream_Ref_Base(Stream) {}

Writable_द्विआधारी_Stream_Ref::Writable_द्विआधारी_Stream_Ref(Writable_द्विआधारी_Stream &Stream,
                                                 uint64_t Offset,
                                                 std::optional<uint64_t> लंबाई)
    : द्विआधारी_Stream_Ref_Base(Stream, Offset, लंबाई) {}

Writable_द्विआधारी_Stream_Ref::Writable_द्विआधारी_Stream_Ref(MutableArrayRef<uint8_t> तथ्य_2,
                                                 endianness Endian)
    : द्विआधारी_Stream_Ref_Base(std::make_shared<MutableArrayRefImpl>(तथ्य_2, Endian),
                          0, तथ्य_2.आकार_2()) {}

Error Writable_द्विआधारी_Stream_Ref::writeBytes(uint64_t Offset,
                                          ArrayRef<uint8_t> तथ्य_2) स्थिर {
  अगर (स्वतः EC = checkOffsetForWrite(Offset, तथ्य_2.आकार_2()))
    वापस EC;

  वापस BorrowedImpl->writeBytes(ViewOffset + Offset, तथ्य_2);
}

Writable_द्विआधारी_Stream_Ref::चालक द्विआधारी_Stream_Ref() स्थिर {
  वापस द्विआधारी_Stream_Ref(*BorrowedImpl, ViewOffset, लंबाई);
}

/// For buffered streams, commits changes to the backing store.
Error Writable_द्विआधारी_Stream_Ref::commit() { वापस BorrowedImpl->commit(); }
