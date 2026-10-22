//===- BinaryStreamReader.cpp - Reads objects from a binary stream --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/BinaryStreamReader.h"

#شامل "llvm/Support/BinaryStreamError.h"
#شامل "llvm/Support/BinaryStreamRef.h"
#شامل "llvm/Support/LEB128.h"

using فضاینام llvm;

دودویی_Stream_Reader::دودویی_Stream_Reader(دودویی_Stream_Ref Ref) : Stream(Ref) {}

دودویی_Stream_Reader::دودویی_Stream_Reader(دودویی_Stream &Stream) : Stream(Stream) {}

دودویی_Stream_Reader::دودویی_Stream_Reader(ArrayRef<uint8_t> Data,
                                       endianness Endian)
    : Stream(Data, Endian) {}

دودویی_Stream_Reader::دودویی_Stream_Reader(StringRef Data, endianness Endian)
    : Stream(Data, Endian) {}

Error دودویی_Stream_Reader::readLongestContiguousChunk(
    ArrayRef<uint8_t> &Buffer) {
  اگر (auto EC = Stream.readLongestContiguousChunk(Offset, Buffer))
    بازگشت EC;
  Offset += Buffer.size();
  بازگشت Error::success();
}

Error دودویی_Stream_Reader::readBytes(ArrayRef<uint8_t> &Buffer, uint32_t Size) {
  اگر (auto EC = Stream.readBytes(Offset, Size, Buffer))
    بازگشت EC;
  Offset += Size;
  بازگشت Error::success();
}

Error دودویی_Stream_Reader::readULEB128(uint64_t &Dest) {
  SmallVector<uint8_t, 10> EncodedBytes;
  ArrayRef<uint8_t> NextByte;

  // Copy the encoded ULEB into the buffer.
  انجامبده {
    اگر (auto Err = readBytes(NextByte, 1))
      بازگشت Err;
    EncodedBytes.افزودن_back(NextByte[0]);
  } وقتی (NextByte[0] & 0x80);

  Dest = decodeULEB128(EncodedBytes.begin(), nullptr, EncodedBytes.end());
  بازگشت Error::success();
}

Error دودویی_Stream_Reader::readSLEB128(int64_t &Dest) {
  SmallVector<uint8_t, 10> EncodedBytes;
  ArrayRef<uint8_t> NextByte;

  // Copy the encoded ULEB into the buffer.
  انجامبده {
    اگر (auto Err = readBytes(NextByte, 1))
      بازگشت Err;
    EncodedBytes.افزودن_back(NextByte[0]);
  } وقتی (NextByte[0] & 0x80);

  Dest = decodeSLEB128(EncodedBytes.begin(), nullptr, EncodedBytes.end());
  بازگشت Error::success();
}

Error دودویی_Stream_Reader::readCString(StringRef &Dest) {
  uint64_t OriginalOffset = getOffset();
  uint64_t FoundOffset = 0;
  وقتی (درست) {
    uint64_t ThisOffset = getOffset();
    ArrayRef<uint8_t> Buffer;
    اگر (auto EC = readLongestContiguousChunk(Buffer))
      بازگشت EC;
    StringRef S(reinterpret_cast<ثابت نویسه *>(Buffer.begin()), Buffer.size());
    size_t Pos = S.find_first_of('\0');
    اگر (LLVM_LIKELY(Pos != StringRef::npos)) {
      FoundOffset = Pos + ThisOffset;
      بشکن;
    }
  }
  أكد(FoundOffset >= OriginalOffset);

  setOffset(OriginalOffset);
  size_t Length = FoundOffset - OriginalOffset;

  اگر (auto EC = readFixedString(Dest, Length))
    بازگشت EC;

  // Now set the offset back to after the null terminator.
  setOffset(FoundOffset + 1);
  بازگشت Error::success();
}

Error دودویی_Stream_Reader::readWideString(ArrayRef<UTF16> &Dest) {
  uint64_t Length = 0;
  uint64_t OriginalOffset = getOffset();
  ثابت UTF16 *C;
  وقتی (درست) {
    اگر (auto EC = readObject(C))
      بازگشت EC;
    اگر (*C == 0x0000)
      بشکن;
    ++Length;
  }
  uint64_t NewOffset = getOffset();
  setOffset(OriginalOffset);

  اگر (auto EC = readArray(Dest, Length))
    بازگشت EC;
  setOffset(NewOffset);
  بازگشت Error::success();
}

Error دودویی_Stream_Reader::readFixedString(StringRef &Dest, uint32_t Length) {
  ArrayRef<uint8_t> Bytes;
  اگر (auto EC = readBytes(Bytes, Length))
    بازگشت EC;
  Dest = StringRef(reinterpret_cast<ثابت نویسه *>(Bytes.begin()), Bytes.size());
  بازگشت Error::success();
}

Error دودویی_Stream_Reader::readStreamRef(دودویی_Stream_Ref &Ref) {
  بازگشت readStreamRef(Ref, bytesRemaining());
}

Error دودویی_Stream_Reader::readStreamRef(دودویی_Stream_Ref &Ref, uint32_t Length) {
  اگر (bytesRemaining() < Length)
    بازگشت make_error<دودویی_Stream_Error>(stream_error_code::stream_too_short);
  Ref = Stream.slice(Offset, Length);
  Offset += Length;
  بازگشت Error::success();
}

Error دودویی_Stream_Reader::readSubstream(دودویی_Substream_Ref &Ref,
                                        uint32_t Length) {
  Ref.Offset = getOffset();
  بازگشت readStreamRef(Ref.StreamData, Length);
}

Error دودویی_Stream_Reader::skip(uint64_t Amount) {
  اگر (Amount > bytesRemaining())
    بازگشت make_error<دودویی_Stream_Error>(stream_error_code::stream_too_short);
  Offset += Amount;
  بازگشت Error::success();
}

Error دودویی_Stream_Reader::padToAlignment(uint32_t Align) {
  uint32_t NewOffset = alignTo(Offset, Align);
  بازگشت skip(NewOffset - Offset);
}

uint8_t دودویی_Stream_Reader::peek() ثابت {
  ArrayRef<uint8_t> Buffer;
  auto EC = Stream.readBytes(Offset, 1, Buffer);
  أكد(!EC && "Cannot peek an empty buffer!");
  llvm::consumeError(std::move(EC));
  بازگشت Buffer[0];
}

std::pair<دودویی_Stream_Reader, دودویی_Stream_Reader>
دودویی_Stream_Reader::split(uint64_t Off) ثابت {
  أكد(getLength() >= Off);

  دودویی_Stream_Ref First = Stream.drop_front(Offset);

  دودویی_Stream_Ref Second = First.drop_front(Off);
  First = First.keep_front(Off);
  دودویی_Stream_Reader W1{First};
  دودویی_Stream_Reader W2{Second};
  بازگشت {W1, W2};
}
