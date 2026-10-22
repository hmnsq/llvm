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
  만약 (auto EC = Stream.readLongestContiguousChunk(Offset, Buffer))
    반환 EC;
  Offset += Buffer.size();
  반환 Error::success();
}

Error BinaryStreamReader::readBytes(ArrayRef<uint8_t> &Buffer, uint32_t Size) {
  만약 (auto EC = Stream.readBytes(Offset, Size, Buffer))
    반환 EC;
  Offset += Size;
  반환 Error::success();
}

Error BinaryStreamReader::readULEB128(uint64_t &Dest) {
  SmallVector<uint8_t, 10> EncodedBytes;
  ArrayRef<uint8_t> NextByte;

  // Copy the encoded ULEB into the buffer.
  do {
    만약 (auto Err = readBytes(NextByte, 1))
      반환 Err;
    EncodedBytes.push_back(NextByte[0]);
  } 동안 (NextByte[0] & 0x80);

  Dest = decodeULEB128(EncodedBytes.begin(), nullptr, EncodedBytes.end());
  반환 Error::success();
}

Error BinaryStreamReader::readSLEB128(int64_t &Dest) {
  SmallVector<uint8_t, 10> EncodedBytes;
  ArrayRef<uint8_t> NextByte;

  // Copy the encoded ULEB into the buffer.
  do {
    만약 (auto Err = readBytes(NextByte, 1))
      반환 Err;
    EncodedBytes.push_back(NextByte[0]);
  } 동안 (NextByte[0] & 0x80);

  Dest = decodeSLEB128(EncodedBytes.begin(), nullptr, EncodedBytes.end());
  반환 Error::success();
}

Error BinaryStreamReader::readCString(StringRef &Dest) {
  uint64_t OriginalOffset = getOffset();
  uint64_t FoundOffset = 0;
  동안 (true) {
    uint64_t ThisOffset = getOffset();
    ArrayRef<uint8_t> Buffer;
    만약 (auto EC = readLongestContiguousChunk(Buffer))
      반환 EC;
    StringRef S(reinterpret_cast<const char *>(Buffer.begin()), Buffer.size());
    size_t Pos = S.find_first_of('\0');
    만약 (LLVM_LIKELY(Pos != StringRef::npos)) {
      FoundOffset = Pos + ThisOffset;
      break;
    }
  }
  assert(FoundOffset >= OriginalOffset);

  setOffset(OriginalOffset);
  size_t Length = FoundOffset - OriginalOffset;

  만약 (auto EC = readFixedString(Dest, Length))
    반환 EC;

  // Now set the offset back to after the null terminator.
  setOffset(FoundOffset + 1);
  반환 Error::success();
}

Error BinaryStreamReader::readWideString(ArrayRef<UTF16> &Dest) {
  uint64_t Length = 0;
  uint64_t OriginalOffset = getOffset();
  const UTF16 *C;
  동안 (true) {
    만약 (auto EC = readObject(C))
      반환 EC;
    만약 (*C == 0x0000)
      break;
    ++Length;
  }
  uint64_t NewOffset = getOffset();
  setOffset(OriginalOffset);

  만약 (auto EC = readArray(Dest, Length))
    반환 EC;
  setOffset(NewOffset);
  반환 Error::success();
}

Error BinaryStreamReader::readFixedString(StringRef &Dest, uint32_t Length) {
  ArrayRef<uint8_t> Bytes;
  만약 (auto EC = readBytes(Bytes, Length))
    반환 EC;
  Dest = StringRef(reinterpret_cast<const char *>(Bytes.begin()), Bytes.size());
  반환 Error::success();
}

Error BinaryStreamReader::readStreamRef(BinaryStreamRef &Ref) {
  반환 readStreamRef(Ref, bytesRemaining());
}

Error BinaryStreamReader::readStreamRef(BinaryStreamRef &Ref, uint32_t Length) {
  만약 (bytesRemaining() < Length)
    반환 make_error<BinaryStreamError>(stream_error_code::stream_too_short);
  Ref = Stream.slice(Offset, Length);
  Offset += Length;
  반환 Error::success();
}

Error BinaryStreamReader::readSubstream(BinarySubstreamRef &Ref,
                                        uint32_t Length) {
  Ref.Offset = getOffset();
  반환 readStreamRef(Ref.StreamData, Length);
}

Error BinaryStreamReader::skip(uint64_t Amount) {
  만약 (Amount > bytesRemaining())
    반환 make_error<BinaryStreamError>(stream_error_code::stream_too_short);
  Offset += Amount;
  반환 Error::success();
}

Error BinaryStreamReader::padToAlignment(uint32_t Align) {
  uint32_t NewOffset = alignTo(Offset, Align);
  반환 skip(NewOffset - Offset);
}

uint8_t BinaryStreamReader::peek() const {
  ArrayRef<uint8_t> Buffer;
  auto EC = Stream.readBytes(Offset, 1, Buffer);
  assert(!EC && "Cannot peek an empty buffer!");
  llvm::consumeError(std::move(EC));
  반환 Buffer[0];
}

std::pair<BinaryStreamReader, BinaryStreamReader>
BinaryStreamReader::split(uint64_t Off) const {
  assert(getLength() >= Off);

  BinaryStreamRef First = Stream.drop_front(Offset);

  BinaryStreamRef Second = First.drop_front(Off);
  First = First.keep_front(Off);
  BinaryStreamReader W1{First};
  BinaryStreamReader W2{Second};
  반환 {W1, W2};
}
