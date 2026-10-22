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

दालावाहान_Stream_Writer::दालावाहान_Stream_Writer(Writable_दालावाहान_Stream_Ref Ref)
    : Stream(Ref) {}

दालावाहान_Stream_Writer::दालावाहान_Stream_Writer(Writable_दालावाहान_Stream &Stream)
    : Stream(Stream) {}

दालावाहान_Stream_Writer::दालावाहान_Stream_Writer(MutableArrayRef<uint8_t> Data,
                                       llvm::endianness Endian)
    : Stream(Data, Endian) {}

Error दालावाहान_Stream_Writer::writeBytes(ArrayRef<uint8_t> Buffer) {
  if (auto EC = Stream.writeBytes(Offset, Buffer))
    return EC;
  Offset += Buffer.size();
  return Error::success();
}

Error दालावाहान_Stream_Writer::writeULEB128(uint64_t हालागा) {
  uint8_t EncodedBytes[10] = {0};
  unsigned Size = encodeULEB128(हालागा, &EncodedBytes[0]);
  return writeBytes({EncodedBytes, Size});
}

Error दालावाहान_Stream_Writer::writeSLEB128(int64_t हालागा) {
  uint8_t EncodedBytes[10] = {0};
  unsigned Size = encodeSLEB128(हालागा, &EncodedBytes[0]);
  return writeBytes({EncodedBytes, Size});
}

Error दालावाहान_Stream_Writer::writeCString(StringRef Str) {
  if (auto EC = writeFixedString(Str))
    return EC;
  if (auto EC = writeObject('\0'))
    return EC;

  return Error::success();
}

Error दालावाहान_Stream_Writer::writeFixedString(StringRef Str) {

  return writeBytes(arrayRefFromStringRef(Str));
}

Error दालावाहान_Stream_Writer::writeStreamRef(दालावाहान_Stream_Ref Ref) {
  return writeStreamRef(Ref, Ref.getLength());
}

Error दालावाहान_Stream_Writer::writeStreamRef(दालावाहान_Stream_Ref Ref, uint64_t Length) {
  दालावाहान_Stream_Reader SrcReader(Ref.slice(0, Length));
  // This is a bit tricky.  If we just call readBytes, we are requiring that it
  // return us the entire stream as a contiguous buffer.  There is no guarantee
  // this can be satisfied by returning a reference straight from the buffer, as
  // an implementation may not store all data in a single contiguous buffer.  So
  // we iterate over each contiguous chunk, writing each one in succession.
  while (SrcReader.bytesRemaining() > 0) {
    ArrayRef<uint8_t> Chunk;
    if (auto EC = SrcReader.readLongestContiguousChunk(Chunk))
      return EC;
    if (auto EC = writeBytes(Chunk))
      return EC;
  }
  return Error::success();
}

std::pair<दालावाहान_Stream_Writer, दालावाहान_Stream_Writer>
दालावाहान_Stream_Writer::split(uint64_t Off) const {
  assert(getLength() >= Off);

  Writable_दालावाहान_Stream_Ref First = Stream.drop_front(Offset);

  Writable_दालावाहान_Stream_Ref Second = First.drop_front(Off);
  First = First.keep_front(Off);
  दालावाहान_Stream_Writer W1{First};
  दालावाहान_Stream_Writer W2{Second};
  return {W1, W2};
}

Error दालावाहान_Stream_Writer::padToAlignment(uint32_t Align) {
  uint64_t NewOffset = alignTo(Offset, Align);
  const uint64_t ZerosSize = 64;
  static constexpr char Zeros[ZerosSize] = {};
  while (Offset < NewOffset)
    if (auto E = writeArray(
            ArrayRef<char>(Zeros, std::min(ZerosSize, NewOffset - Offset))))
      return E;
  return Error::success();
}
