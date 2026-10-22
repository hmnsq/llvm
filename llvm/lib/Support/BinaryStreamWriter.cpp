//===- BinaryStreamWriter.cpp - Writes objects to a BinaryStream ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/BinaryStreamWriter.h"

#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/Support/BinaryStreamReader.h"
#ضمن "llvm/Support/BinaryStreamRef.h"
#ضمن "llvm/Support/LEB128.h"

using نطاق llvm;

BinaryStreamWriter::BinaryStreamWriter(WritableBinaryStreamRef Ref)
    : Stream(Ref) {}

BinaryStreamWriter::BinaryStreamWriter(WritableBinaryStream &Stream)
    : Stream(Stream) {}

BinaryStreamWriter::BinaryStreamWriter(MutableArrayRef<uint8_t> Data,
                                       llvm::endianness Endian)
    : Stream(Data, Endian) {}

Error BinaryStreamWriter::writeBytes(ArrayRef<uint8_t> Buffer) {
  𐤀𐤌 (auto EC = Stream.writeBytes(Offset, Buffer))
    أرجع EC;
  Offset += Buffer.size();
  أرجع Error::success();
}

Error BinaryStreamWriter::writeULEB128(uint64_t Value) {
  uint8_t EncodedBytes[10] = {0};
  unsigned Size = encodeULEB128(Value, &EncodedBytes[0]);
  أرجع writeBytes({EncodedBytes, Size});
}

Error BinaryStreamWriter::writeSLEB128(int64_t Value) {
  uint8_t EncodedBytes[10] = {0};
  unsigned Size = encodeSLEB128(Value, &EncodedBytes[0]);
  أرجع writeBytes({EncodedBytes, Size});
}

Error BinaryStreamWriter::writeCString(StringRef Str) {
  𐤀𐤌 (auto EC = writeFixedString(Str))
    أرجع EC;
  𐤀𐤌 (auto EC = writeObject('\0'))
    أرجع EC;

  أرجع Error::success();
}

Error BinaryStreamWriter::writeFixedString(StringRef Str) {

  أرجع writeBytes(arrayRefFromStringRef(Str));
}

Error BinaryStreamWriter::writeStreamRef(BinaryStreamRef Ref) {
  أرجع writeStreamRef(Ref, Ref.getLength());
}

Error BinaryStreamWriter::writeStreamRef(BinaryStreamRef Ref, uint64_t Length) {
  BinaryStreamReader SrcReader(Ref.slice(0, Length));
  // This is a bit tricky.  If we just call readBytes, we are requiring that it
  // return us the entire stream as a contiguous buffer.  There is no guarantee
  // this can be satisfied by returning a reference straight from the buffer, as
  // an implementation may not store all data in a single contiguous buffer.  So
  // we iterate over each contiguous chunk, writing each one in succession.
  طالما (SrcReader.bytesRemaining() > 0) {
    ArrayRef<uint8_t> Chunk;
    𐤀𐤌 (auto EC = SrcReader.readLongestContiguousChunk(Chunk))
      أرجع EC;
    𐤀𐤌 (auto EC = writeBytes(Chunk))
      أرجع EC;
  }
  أرجع Error::success();
}

std::pair<BinaryStreamWriter, BinaryStreamWriter>
BinaryStreamWriter::split(uint64_t Off) ثابت {
  أكد(getLength() >= Off);

  WritableBinaryStreamRef First = Stream.drop_front(Offset);

  WritableBinaryStreamRef Second = First.drop_front(Off);
  First = First.keep_front(Off);
  BinaryStreamWriter W1{First};
  BinaryStreamWriter W2{Second};
  أرجع {W1, W2};
}

Error BinaryStreamWriter::padToAlignment(uint32_t Align) {
  uint64_t NewOffset = alignTo(Offset, Align);
  ثابت uint64_t ZerosSize = 64;
  ساكن constexpr حرف Zeros[ZerosSize] = {};
  طالما (Offset < NewOffset)
    𐤀𐤌 (auto E = writeArray(
            ArrayRef<حرف>(Zeros, std::min(ZerosSize, NewOffset - Offset))))
      أرجع E;
  أرجع Error::success();
}
