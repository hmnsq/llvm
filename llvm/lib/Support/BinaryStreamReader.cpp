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
  ᮚᮦᮔ᮪ (auto EC = Stream.readLongestContiguousChunk(Offset, Buffer))
    ᮝᮜᮤ EC;
  Offset += Buffer.size();
  ᮝᮜᮤ Error::success();
}

Error BinaryStreamReader::readBytes(ArrayRef<uint8_t> &Buffer, uint32_t Size) {
  ᮚᮦᮔ᮪ (auto EC = Stream.readBytes(Offset, Size, Buffer))
    ᮝᮜᮤ EC;
  Offset += Size;
  ᮝᮜᮤ Error::success();
}

Error BinaryStreamReader::readULEB128(uint64_t &Dest) {
  SmallVector<uint8_t, 10> EncodedBytes;
  ArrayRef<uint8_t> NextByte;

  // Copy the encoded ULEB into the buffer.
  do {
    ᮚᮦᮔ᮪ (auto Err = readBytes(NextByte, 1))
      ᮝᮜᮤ Err;
    EncodedBytes.push_back(NextByte[0]);
  } ᮞᮨᮓᮨᮜ (NextByte[0] & 0x80);

  Dest = decodeULEB128(EncodedBytes.begin(), nullptr, EncodedBytes.end());
  ᮝᮜᮤ Error::success();
}

Error BinaryStreamReader::readSLEB128(int64_t &Dest) {
  SmallVector<uint8_t, 10> EncodedBytes;
  ArrayRef<uint8_t> NextByte;

  // Copy the encoded ULEB into the buffer.
  do {
    ᮚᮦᮔ᮪ (auto Err = readBytes(NextByte, 1))
      ᮝᮜᮤ Err;
    EncodedBytes.push_back(NextByte[0]);
  } ᮞᮨᮓᮨᮜ (NextByte[0] & 0x80);

  Dest = decodeSLEB128(EncodedBytes.begin(), nullptr, EncodedBytes.end());
  ᮝᮜᮤ Error::success();
}

Error BinaryStreamReader::readCString(StringRef &Dest) {
  uint64_t OriginalOffset = getOffset();
  uint64_t FoundOffset = 0;
  ᮞᮨᮓᮨᮜ (true) {
    uint64_t ThisOffset = getOffset();
    ArrayRef<uint8_t> Buffer;
    ᮚᮦᮔ᮪ (auto EC = readLongestContiguousChunk(Buffer))
      ᮝᮜᮤ EC;
    StringRef S(reinterpret_cast<const var_na *>(Buffer.begin()), Buffer.size());
    size_t Pos = S.find_first_of('\0');
    ᮚᮦᮔ᮪ (LLVM_LIKELY(Pos != StringRef::npos)) {
      FoundOffset = Pos + ThisOffset;
      bha_nga;
    }
  }
  assert(FoundOffset >= OriginalOffset);

  setOffset(OriginalOffset);
  size_t Length = FoundOffset - OriginalOffset;

  ᮚᮦᮔ᮪ (auto EC = readFixedString(Dest, Length))
    ᮝᮜᮤ EC;

  // Now set the offset back to after the null terminator.
  setOffset(FoundOffset + 1);
  ᮝᮜᮤ Error::success();
}

Error BinaryStreamReader::readWideString(ArrayRef<UTF16> &Dest) {
  uint64_t Length = 0;
  uint64_t OriginalOffset = getOffset();
  const UTF16 *C;
  ᮞᮨᮓᮨᮜ (true) {
    ᮚᮦᮔ᮪ (auto EC = readObject(C))
      ᮝᮜᮤ EC;
    ᮚᮦᮔ᮪ (*C == 0x0000)
      bha_nga;
    ++Length;
  }
  uint64_t NewOffset = getOffset();
  setOffset(OriginalOffset);

  ᮚᮦᮔ᮪ (auto EC = readArray(Dest, Length))
    ᮝᮜᮤ EC;
  setOffset(NewOffset);
  ᮝᮜᮤ Error::success();
}

Error BinaryStreamReader::readFixedString(StringRef &Dest, uint32_t Length) {
  ArrayRef<uint8_t> Bytes;
  ᮚᮦᮔ᮪ (auto EC = readBytes(Bytes, Length))
    ᮝᮜᮤ EC;
  Dest = StringRef(reinterpret_cast<const var_na *>(Bytes.begin()), Bytes.size());
  ᮝᮜᮤ Error::success();
}

Error BinaryStreamReader::readStreamRef(BinaryStreamRef &Ref) {
  ᮝᮜᮤ readStreamRef(Ref, bytesRemaining());
}

Error BinaryStreamReader::readStreamRef(BinaryStreamRef &Ref, uint32_t Length) {
  ᮚᮦᮔ᮪ (bytesRemaining() < Length)
    ᮝᮜᮤ make_error<BinaryStreamError>(stream_error_code::stream_too_short);
  Ref = Stream.slice(Offset, Length);
  Offset += Length;
  ᮝᮜᮤ Error::success();
}

Error BinaryStreamReader::readSubstream(BinarySubstreamRef &Ref,
                                        uint32_t Length) {
  Ref.Offset = getOffset();
  ᮝᮜᮤ readStreamRef(Ref.StreamData, Length);
}

Error BinaryStreamReader::skip(uint64_t Amount) {
  ᮚᮦᮔ᮪ (Amount > bytesRemaining())
    ᮝᮜᮤ make_error<BinaryStreamError>(stream_error_code::stream_too_short);
  Offset += Amount;
  ᮝᮜᮤ Error::success();
}

Error BinaryStreamReader::padToAlignment(uint32_t Align) {
  uint32_t NewOffset = alignTo(Offset, Align);
  ᮝᮜᮤ skip(NewOffset - Offset);
}

uint8_t BinaryStreamReader::peek() const {
  ArrayRef<uint8_t> Buffer;
  auto EC = Stream.readBytes(Offset, 1, Buffer);
  assert(!EC && "Cannot peek an empty buffer!");
  llvm::consumeError(std::move(EC));
  ᮝᮜᮤ Buffer[0];
}

std::pair<BinaryStreamReader, BinaryStreamReader>
BinaryStreamReader::split(uint64_t Off) const {
  assert(getLength() >= Off);

  BinaryStreamRef First = Stream.drop_front(Offset);

  BinaryStreamRef Second = First.drop_front(Off);
  First = First.keep_front(Off);
  BinaryStreamReader W1{First};
  BinaryStreamReader W2{Second};
  ᮝᮜᮤ {W1, W2};
}
