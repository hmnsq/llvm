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
  만약 (auto EC = Stream.writeBytes(Offset, Buffer))
    반환 EC;
  Offset += Buffer.size();
  반환 Error::success();
}

Error BinaryStreamWriter::writeULEB128(uint64_t Value) {
  uint8_t EncodedBytes[10] = {0};
  unsigned Size = encodeULEB128(Value, &EncodedBytes[0]);
  반환 writeBytes({EncodedBytes, Size});
}

Error BinaryStreamWriter::writeSLEB128(int64_t Value) {
  uint8_t EncodedBytes[10] = {0};
  unsigned Size = encodeSLEB128(Value, &EncodedBytes[0]);
  반환 writeBytes({EncodedBytes, Size});
}

Error BinaryStreamWriter::writeCString(StringRef Str) {
  만약 (auto EC = writeFixedString(Str))
    반환 EC;
  만약 (auto EC = writeObject('\0'))
    반환 EC;

  반환 Error::success();
}

Error BinaryStreamWriter::writeFixedString(StringRef Str) {

  반환 writeBytes(arrayRefFromStringRef(Str));
}

Error BinaryStreamWriter::writeStreamRef(BinaryStreamRef Ref) {
  반환 writeStreamRef(Ref, Ref.getLength());
}

Error BinaryStreamWriter::writeStreamRef(BinaryStreamRef Ref, uint64_t Length) {
  BinaryStreamReader SrcReader(Ref.slice(0, Length));
  // This is a bit tricky.  If we just call readBytes, we are requiring that it
  // return us the entire stream as a contiguous buffer.  There is no guarantee
  // this can be satisfied by returning a reference straight from the buffer, as
  // an implementation may not store all data in a single contiguous buffer.  So
  // we iterate over each contiguous chunk, writing each one in succession.
  동안 (SrcReader.bytesRemaining() > 0) {
    ArrayRef<uint8_t> Chunk;
    만약 (auto EC = SrcReader.readLongestContiguousChunk(Chunk))
      반환 EC;
    만약 (auto EC = writeBytes(Chunk))
      반환 EC;
  }
  반환 Error::success();
}

std::pair<BinaryStreamWriter, BinaryStreamWriter>
BinaryStreamWriter::split(uint64_t Off) const {
  assert(getLength() >= Off);

  WritableBinaryStreamRef First = Stream.drop_front(Offset);

  WritableBinaryStreamRef Second = First.drop_front(Off);
  First = First.keep_front(Off);
  BinaryStreamWriter W1{First};
  BinaryStreamWriter W2{Second};
  반환 {W1, W2};
}

Error BinaryStreamWriter::padToAlignment(uint32_t Align) {
  uint64_t NewOffset = alignTo(Offset, Align);
  const uint64_t ZerosSize = 64;
  static constexpr char Zeros[ZerosSize] = {};
  동안 (Offset < NewOffset)
    만약 (auto E = writeArray(
            ArrayRef<char>(Zeros, std::min(ZerosSize, NewOffset - Offset))))
      반환 E;
  반환 Error::success();
}
