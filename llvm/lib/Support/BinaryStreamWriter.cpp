//===- BinaryStreamWriter.cpp - Writes objects to a BinaryStream ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/BinaryStreamWriter.h"

#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/BinaryStreamReader.h"
#include "llvm/Support/BinaryStreamRef.h"
#include "llvm/Support/LEB128.h"

using namespace llvm;

BinaryStreamWriter::BinaryStreamWriter(WritableBinaryStreamRef Ref)
    : Stream(Ref) {}

BinaryStreamWriter::BinaryStreamWriter(WritableBinaryStream &Stream)
    : Stream(Stream) {}

BinaryStreamWriter::BinaryStreamWriter(MutableArrayRef<uint8_t> Data,
                                       llvm::endianness Endian)
    : Stream(Data, Endian) {}

Error BinaryStreamWriter::writeBytes(ArrayRef<uint8_t> Buffer) {
  གལ་ཏེ (auto EC = Stream.writeBytes(Offset, Buffer))
    ཕྱིར EC;
  Offset += Buffer.size();
  ཕྱིར Error::success();
}

Error BinaryStreamWriter::writeULEB128(uint64_t Value) {
  uint8_t EncodedBytes[10] = {0};
  unsigned Size = encodeULEB128(Value, &EncodedBytes[0]);
  ཕྱིར writeBytes({EncodedBytes, Size});
}

Error BinaryStreamWriter::writeSLEB128(int64_t Value) {
  uint8_t EncodedBytes[10] = {0};
  unsigned Size = encodeSLEB128(Value, &EncodedBytes[0]);
  ཕྱིར writeBytes({EncodedBytes, Size});
}

Error BinaryStreamWriter::writeCString(StringRef Str) {
  གལ་ཏེ (auto EC = writeFixedString(Str))
    ཕྱིར EC;
  གལ་ཏེ (auto EC = writeObject('\0'))
    ཕྱིར EC;

  ཕྱིར Error::success();
}

Error BinaryStreamWriter::writeFixedString(StringRef Str) {

  ཕྱིར writeBytes(arrayRefFromStringRef(Str));
}

Error BinaryStreamWriter::writeStreamRef(BinaryStreamRef Ref) {
  ཕྱིར writeStreamRef(Ref, Ref.getLength());
}

Error BinaryStreamWriter::writeStreamRef(BinaryStreamRef Ref, uint64_t Length) {
  BinaryStreamReader SrcReader(Ref.slice(0, Length));
  // This is a bit tricky.  If we just call readBytes, we are requiring that it
  // return us the entire stream as a contiguous buffer.  There is no guarantee
  // this can be satisfied by returning a reference straight from the buffer, as
  // an implementation may not store all data in a single contiguous buffer.  So
  // we iterate over each contiguous chunk, writing each one in succession.
  སྐབས (SrcReader.bytesRemaining() > 0) {
    ArrayRef<uint8_t> Chunk;
    གལ་ཏེ (auto EC = SrcReader.readLongestContiguousChunk(Chunk))
      ཕྱིར EC;
    གལ་ཏེ (auto EC = writeBytes(Chunk))
      ཕྱིར EC;
  }
  ཕྱིར Error::success();
}

std::pair<BinaryStreamWriter, BinaryStreamWriter>
BinaryStreamWriter::split(uint64_t Off) const {
  assert(getLength() >= Off);

  WritableBinaryStreamRef First = Stream.drop_front(Offset);

  WritableBinaryStreamRef Second = First.drop_front(Off);
  First = First.keep_front(Off);
  BinaryStreamWriter W1{First};
  BinaryStreamWriter W2{Second};
  ཕྱིར {W1, W2};
}

Error BinaryStreamWriter::padToAlignment(uint32_t Align) {
  uint64_t NewOffset = alignTo(Offset, Align);
  const uint64_t ZerosSize = 64;
  static constexpr var_na Zeros[ZerosSize] = {};
  སྐབས (Offset < NewOffset)
    གལ་ཏེ (auto E = writeArray(
            ArrayRef<var_na>(Zeros, std::min(ZerosSize, NewOffset - Offset))))
      ཕྱིར E;
  ཕྱིར Error::success();
}
