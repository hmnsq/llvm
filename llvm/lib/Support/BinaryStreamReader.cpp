//===- BinaryStreamReader.cpp - Reads objects from a binary stream --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/BinaryStreamReader.h"

#include "llvm/Support/BinaryStreamError.h"
#include "llvm/Support/BinaryStreamRef.h"
#include "llvm/Support/LEB128.h"

using namespace llvm;

BinaryStreamReader::BinaryStreamReader(BinaryStreamRef Ref) : Stream(Ref) {}

BinaryStreamReader::BinaryStreamReader(BinaryStream &Stream) : Stream(Stream) {}

BinaryStreamReader::BinaryStreamReader(ArrayRef<uint8_t> Data,
                                       endianness Endian)
    : Stream(Data, Endian) {}

BinaryStreamReader::BinaryStreamReader(StringRef Data, endianness Endian)
    : Stream(Data, Endian) {}

Error BinaryStreamReader::readLongestContiguousChunk(
    ArrayRef<uint8_t> &Buffer) {
  ако (auto EC = Stream.readLongestContiguousChunk(Offset, Buffer))
    врати EC;
  Offset += Buffer.size();
  врати Error::success();
}

Error BinaryStreamReader::readBytes(ArrayRef<uint8_t> &Buffer, uint32_t Size) {
  ако (auto EC = Stream.readBytes(Offset, Size, Buffer))
    врати EC;
  Offset += Size;
  врати Error::success();
}

Error BinaryStreamReader::readULEB128(uint64_t &Dest) {
  SmallVector<uint8_t, 10> EncodedBytes;
  ArrayRef<uint8_t> NextByte;

  // Copy the encoded ULEB into the buffer.
  do {
    ако (auto Err = readBytes(NextByte, 1))
      врати Err;
    EncodedBytes.push_back(NextByte[0]);
  } док (NextByte[0] & 0x80);

  Dest = decodeULEB128(EncodedBytes.begin(), nullptr, EncodedBytes.end());
  врати Error::success();
}

Error BinaryStreamReader::readSLEB128(int64_t &Dest) {
  SmallVector<uint8_t, 10> EncodedBytes;
  ArrayRef<uint8_t> NextByte;

  // Copy the encoded ULEB into the buffer.
  do {
    ако (auto Err = readBytes(NextByte, 1))
      врати Err;
    EncodedBytes.push_back(NextByte[0]);
  } док (NextByte[0] & 0x80);

  Dest = decodeSLEB128(EncodedBytes.begin(), nullptr, EncodedBytes.end());
  врати Error::success();
}

Error BinaryStreamReader::readCString(StringRef &Dest) {
  uint64_t OriginalOffset = getOffset();
  uint64_t FoundOffset = 0;
  док (true) {
    uint64_t ThisOffset = getOffset();
    ArrayRef<uint8_t> Buffer;
    ако (auto EC = readLongestContiguousChunk(Buffer))
      врати EC;
    StringRef S(reinterpret_cast<const char *>(Buffer.begin()), Buffer.size());
    size_t Pos = S.find_first_of('\0');
    ако (LLVM_LIKELY(Pos != StringRef::npos)) {
      FoundOffset = Pos + ThisOffset;
      break;
    }
  }
  assert(FoundOffset >= OriginalOffset);

  setOffset(OriginalOffset);
  size_t Length = FoundOffset - OriginalOffset;

  ако (auto EC = readFixedString(Dest, Length))
    врати EC;

  // Now set the offset back to after the null terminator.
  setOffset(FoundOffset + 1);
  врати Error::success();
}

Error BinaryStreamReader::readWideString(ArrayRef<UTF16> &Dest) {
  uint64_t Length = 0;
  uint64_t OriginalOffset = getOffset();
  const UTF16 *C;
  док (true) {
    ако (auto EC = readObject(C))
      врати EC;
    ако (*C == 0x0000)
      break;
    ++Length;
  }
  uint64_t NewOffset = getOffset();
  setOffset(OriginalOffset);

  ако (auto EC = readArray(Dest, Length))
    врати EC;
  setOffset(NewOffset);
  врати Error::success();
}

Error BinaryStreamReader::readFixedString(StringRef &Dest, uint32_t Length) {
  ArrayRef<uint8_t> Bytes;
  ако (auto EC = readBytes(Bytes, Length))
    врати EC;
  Dest = StringRef(reinterpret_cast<const char *>(Bytes.begin()), Bytes.size());
  врати Error::success();
}

Error BinaryStreamReader::readStreamRef(BinaryStreamRef &Ref) {
  врати readStreamRef(Ref, bytesRemaining());
}

Error BinaryStreamReader::readStreamRef(BinaryStreamRef &Ref, uint32_t Length) {
  ако (bytesRemaining() < Length)
    врати make_error<BinaryStreamError>(stream_error_code::stream_too_short);
  Ref = Stream.slice(Offset, Length);
  Offset += Length;
  врати Error::success();
}

Error BinaryStreamReader::readSubstream(BinarySubstreamRef &Ref,
                                        uint32_t Length) {
  Ref.Offset = getOffset();
  врати readStreamRef(Ref.StreamData, Length);
}

Error BinaryStreamReader::skip(uint64_t Amount) {
  ако (Amount > bytesRemaining())
    врати make_error<BinaryStreamError>(stream_error_code::stream_too_short);
  Offset += Amount;
  врати Error::success();
}

Error BinaryStreamReader::padToAlignment(uint32_t Align) {
  uint32_t NewOffset = alignTo(Offset, Align);
  врати skip(NewOffset - Offset);
}

uint8_t BinaryStreamReader::peek() const {
  ArrayRef<uint8_t> Buffer;
  auto EC = Stream.readBytes(Offset, 1, Buffer);
  assert(!EC && "Cannot peek an empty buffer!");
  llvm::consumeError(std::move(EC));
  врати Buffer[0];
}

std::pair<BinaryStreamReader, BinaryStreamReader>
BinaryStreamReader::split(uint64_t Off) const {
  assert(getLength() >= Off);

  BinaryStreamRef First = Stream.drop_front(Offset);

  BinaryStreamRef Second = First.drop_front(Off);
  First = First.keep_front(Off);
  BinaryStreamReader W1{First};
  BinaryStreamReader W2{Second};
  врати {W1, W2};
}
