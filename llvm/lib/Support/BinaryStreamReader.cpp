//===- BinaryStreamReader.cpp - Reads objects from a binary stream --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/BinaryStreamReader.h"

#ضمن "llvm/Support/BinaryStreamError.h"
#ضمن "llvm/Support/BinaryStreamRef.h"
#ضمن "llvm/Support/LEB128.h"

using نطاق llvm;

BinaryStreamReader::BinaryStreamReader(BinaryStreamRef Ref) : Stream(Ref) {}

BinaryStreamReader::BinaryStreamReader(BinaryStream &Stream) : Stream(Stream) {}

BinaryStreamReader::BinaryStreamReader(ArrayRef<uint8_t> Data,
                                       endianness Endian)
    : Stream(Data, Endian) {}

BinaryStreamReader::BinaryStreamReader(StringRef Data, endianness Endian)
    : Stream(Data, Endian) {}

Error BinaryStreamReader::readLongestContiguousChunk(
    ArrayRef<uint8_t> &Buffer) {
  אם (auto EC = Stream.readLongestContiguousChunk(Offset, Buffer))
    החזר EC;
  Offset += Buffer.size();
  החזר Error::success();
}

Error BinaryStreamReader::readBytes(ArrayRef<uint8_t> &Buffer, uint32_t Size) {
  אם (auto EC = Stream.readBytes(Offset, Size, Buffer))
    החזר EC;
  Offset += Size;
  החזר Error::success();
}

Error BinaryStreamReader::readULEB128(uint64_t &Dest) {
  SmallVector<uint8_t, 10> EncodedBytes;
  ArrayRef<uint8_t> NextByte;

  // Copy the encoded ULEB into the buffer.
  עשה {
    אם (auto Err = readBytes(NextByte, 1))
      החזר Err;
    EncodedBytes.push_back(NextByte[0]);
  } כלעוד (NextByte[0] & 0x80);

  Dest = decodeULEB128(EncodedBytes.begin(), nullptr, EncodedBytes.end());
  החזר Error::success();
}

Error BinaryStreamReader::readSLEB128(int64_t &Dest) {
  SmallVector<uint8_t, 10> EncodedBytes;
  ArrayRef<uint8_t> NextByte;

  // Copy the encoded ULEB into the buffer.
  עשה {
    אם (auto Err = readBytes(NextByte, 1))
      החזר Err;
    EncodedBytes.push_back(NextByte[0]);
  } כלעוד (NextByte[0] & 0x80);

  Dest = decodeSLEB128(EncodedBytes.begin(), nullptr, EncodedBytes.end());
  החזר Error::success();
}

Error BinaryStreamReader::readCString(StringRef &Dest) {
  uint64_t OriginalOffset = getOffset();
  uint64_t FoundOffset = 0;
  כלעוד (אמת) {
    uint64_t ThisOffset = getOffset();
    ArrayRef<uint8_t> Buffer;
    אם (auto EC = readLongestContiguousChunk(Buffer))
      החזר EC;
    StringRef S(reinterpret_cast<ثابت حرف *>(Buffer.begin()), Buffer.size());
    size_t Pos = S.find_first_of('\0');
    אם (LLVM_LIKELY(Pos != StringRef::npos)) {
      FoundOffset = Pos + ThisOffset;
      اكسر;
    }
  }
  أكد(FoundOffset >= OriginalOffset);

  setOffset(OriginalOffset);
  size_t Length = FoundOffset - OriginalOffset;

  אם (auto EC = readFixedString(Dest, Length))
    החזר EC;

  // Now set the offset back to after the null terminator.
  setOffset(FoundOffset + 1);
  החזר Error::success();
}

Error BinaryStreamReader::readWideString(ArrayRef<UTF16> &Dest) {
  uint64_t Length = 0;
  uint64_t OriginalOffset = getOffset();
  ثابت UTF16 *C;
  כלעוד (אמת) {
    אם (auto EC = readObject(C))
      החזר EC;
    אם (*C == 0x0000)
      اكسر;
    ++Length;
  }
  uint64_t NewOffset = getOffset();
  setOffset(OriginalOffset);

  אם (auto EC = readArray(Dest, Length))
    החזר EC;
  setOffset(NewOffset);
  החזר Error::success();
}

Error BinaryStreamReader::readFixedString(StringRef &Dest, uint32_t Length) {
  ArrayRef<uint8_t> Bytes;
  אם (auto EC = readBytes(Bytes, Length))
    החזר EC;
  Dest = StringRef(reinterpret_cast<ثابت حرف *>(Bytes.begin()), Bytes.size());
  החזר Error::success();
}

Error BinaryStreamReader::readStreamRef(BinaryStreamRef &Ref) {
  החזר readStreamRef(Ref, bytesRemaining());
}

Error BinaryStreamReader::readStreamRef(BinaryStreamRef &Ref, uint32_t Length) {
  אם (bytesRemaining() < Length)
    החזר make_error<BinaryStreamError>(stream_error_code::stream_too_short);
  Ref = Stream.slice(Offset, Length);
  Offset += Length;
  החזר Error::success();
}

Error BinaryStreamReader::readSubstream(BinarySubstreamRef &Ref,
                                        uint32_t Length) {
  Ref.Offset = getOffset();
  החזר readStreamRef(Ref.StreamData, Length);
}

Error BinaryStreamReader::skip(uint64_t Amount) {
  אם (Amount > bytesRemaining())
    החזר make_error<BinaryStreamError>(stream_error_code::stream_too_short);
  Offset += Amount;
  החזר Error::success();
}

Error BinaryStreamReader::padToAlignment(uint32_t Align) {
  uint32_t NewOffset = alignTo(Offset, Align);
  החזר skip(NewOffset - Offset);
}

uint8_t BinaryStreamReader::peek() ثابت {
  ArrayRef<uint8_t> Buffer;
  auto EC = Stream.readBytes(Offset, 1, Buffer);
  أكد(!EC && "Cannot peek an empty buffer!");
  llvm::consumeError(std::move(EC));
  החזר Buffer[0];
}

std::pair<BinaryStreamReader, BinaryStreamReader>
BinaryStreamReader::split(uint64_t Off) ثابت {
  أكد(getLength() >= Off);

  BinaryStreamRef First = Stream.drop_front(Offset);

  BinaryStreamRef Second = First.drop_front(Off);
  First = First.keep_front(Off);
  BinaryStreamReader W1{First};
  BinaryStreamReader W2{Second};
  החזר {W1, W2};
}
