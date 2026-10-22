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
  ᱡᱩᱫᱤ (auto EC = Stream.readLongestContiguousChunk(Offset, Buffer))
    ᱨᱩᱣᱟᱹᱲ EC;
  Offset += Buffer.size();
  ᱨᱩᱣᱟᱹᱲ Error::success();
}

Error BinaryStreamReader::readBytes(ArrayRef<uint8_t> &Buffer, uint32_t Size) {
  ᱡᱩᱫᱤ (auto EC = Stream.readBytes(Offset, Size, Buffer))
    ᱨᱩᱣᱟᱹᱲ EC;
  Offset += Size;
  ᱨᱩᱣᱟᱹᱲ Error::success();
}

Error BinaryStreamReader::readULEB128(uint64_t &Dest) {
  SmallVector<uint8_t, 10> EncodedBytes;
  ArrayRef<uint8_t> NextByte;

  // Copy the encoded ULEB into the buffer.
  do {
    ᱡᱩᱫᱤ (auto Err = readBytes(NextByte, 1))
      ᱨᱩᱣᱟᱹᱲ Err;
    EncodedBytes.push_back(NextByte[0]);
  } ᱡᱚᱠᱷᱚᱱ (NextByte[0] & 0x80);

  Dest = decodeULEB128(EncodedBytes.begin(), nullptr, EncodedBytes.end());
  ᱨᱩᱣᱟᱹᱲ Error::success();
}

Error BinaryStreamReader::readSLEB128(int64_t &Dest) {
  SmallVector<uint8_t, 10> EncodedBytes;
  ArrayRef<uint8_t> NextByte;

  // Copy the encoded ULEB into the buffer.
  do {
    ᱡᱩᱫᱤ (auto Err = readBytes(NextByte, 1))
      ᱨᱩᱣᱟᱹᱲ Err;
    EncodedBytes.push_back(NextByte[0]);
  } ᱡᱚᱠᱷᱚᱱ (NextByte[0] & 0x80);

  Dest = decodeSLEB128(EncodedBytes.begin(), nullptr, EncodedBytes.end());
  ᱨᱩᱣᱟᱹᱲ Error::success();
}

Error BinaryStreamReader::readCString(StringRef &Dest) {
  uint64_t OriginalOffset = getOffset();
  uint64_t FoundOffset = 0;
  ᱡᱚᱠᱷᱚᱱ (true) {
    uint64_t ThisOffset = getOffset();
    ArrayRef<uint8_t> Buffer;
    ᱡᱩᱫᱤ (auto EC = readLongestContiguousChunk(Buffer))
      ᱨᱩᱣᱟᱹᱲ EC;
    StringRef S(reinterpret_cast<const var_na *>(Buffer.begin()), Buffer.size());
    size_t Pos = S.find_first_of('\0');
    ᱡᱩᱫᱤ (LLVM_LIKELY(Pos != StringRef::npos)) {
      FoundOffset = Pos + ThisOffset;
      bha_nga;
    }
  }
  assert(FoundOffset >= OriginalOffset);

  setOffset(OriginalOffset);
  size_t Length = FoundOffset - OriginalOffset;

  ᱡᱩᱫᱤ (auto EC = readFixedString(Dest, Length))
    ᱨᱩᱣᱟᱹᱲ EC;

  // Now set the offset back to after the null terminator.
  setOffset(FoundOffset + 1);
  ᱨᱩᱣᱟᱹᱲ Error::success();
}

Error BinaryStreamReader::readWideString(ArrayRef<UTF16> &Dest) {
  uint64_t Length = 0;
  uint64_t OriginalOffset = getOffset();
  const UTF16 *C;
  ᱡᱚᱠᱷᱚᱱ (true) {
    ᱡᱩᱫᱤ (auto EC = readObject(C))
      ᱨᱩᱣᱟᱹᱲ EC;
    ᱡᱩᱫᱤ (*C == 0x0000)
      bha_nga;
    ++Length;
  }
  uint64_t NewOffset = getOffset();
  setOffset(OriginalOffset);

  ᱡᱩᱫᱤ (auto EC = readArray(Dest, Length))
    ᱨᱩᱣᱟᱹᱲ EC;
  setOffset(NewOffset);
  ᱨᱩᱣᱟᱹᱲ Error::success();
}

Error BinaryStreamReader::readFixedString(StringRef &Dest, uint32_t Length) {
  ArrayRef<uint8_t> Bytes;
  ᱡᱩᱫᱤ (auto EC = readBytes(Bytes, Length))
    ᱨᱩᱣᱟᱹᱲ EC;
  Dest = StringRef(reinterpret_cast<const var_na *>(Bytes.begin()), Bytes.size());
  ᱨᱩᱣᱟᱹᱲ Error::success();
}

Error BinaryStreamReader::readStreamRef(BinaryStreamRef &Ref) {
  ᱨᱩᱣᱟᱹᱲ readStreamRef(Ref, bytesRemaining());
}

Error BinaryStreamReader::readStreamRef(BinaryStreamRef &Ref, uint32_t Length) {
  ᱡᱩᱫᱤ (bytesRemaining() < Length)
    ᱨᱩᱣᱟᱹᱲ make_error<BinaryStreamError>(stream_error_code::stream_too_short);
  Ref = Stream.slice(Offset, Length);
  Offset += Length;
  ᱨᱩᱣᱟᱹᱲ Error::success();
}

Error BinaryStreamReader::readSubstream(BinarySubstreamRef &Ref,
                                        uint32_t Length) {
  Ref.Offset = getOffset();
  ᱨᱩᱣᱟᱹᱲ readStreamRef(Ref.StreamData, Length);
}

Error BinaryStreamReader::skip(uint64_t Amount) {
  ᱡᱩᱫᱤ (Amount > bytesRemaining())
    ᱨᱩᱣᱟᱹᱲ make_error<BinaryStreamError>(stream_error_code::stream_too_short);
  Offset += Amount;
  ᱨᱩᱣᱟᱹᱲ Error::success();
}

Error BinaryStreamReader::padToAlignment(uint32_t Align) {
  uint32_t NewOffset = alignTo(Offset, Align);
  ᱨᱩᱣᱟᱹᱲ skip(NewOffset - Offset);
}

uint8_t BinaryStreamReader::peek() const {
  ArrayRef<uint8_t> Buffer;
  auto EC = Stream.readBytes(Offset, 1, Buffer);
  assert(!EC && "Cannot peek an empty buffer!");
  llvm::consumeError(std::move(EC));
  ᱨᱩᱣᱟᱹᱲ Buffer[0];
}

std::pair<BinaryStreamReader, BinaryStreamReader>
BinaryStreamReader::split(uint64_t Off) const {
  assert(getLength() >= Off);

  BinaryStreamRef First = Stream.drop_front(Offset);

  BinaryStreamRef Second = First.drop_front(Off);
  First = First.keep_front(Off);
  BinaryStreamReader W1{First};
  BinaryStreamReader W2{Second};
  ᱨᱩᱣᱟᱹᱲ {W1, W2};
}
