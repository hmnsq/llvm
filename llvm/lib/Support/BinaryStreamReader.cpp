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

ثنائی_Stream_Reader::ثنائی_Stream_Reader(ثنائی_Stream_Ref Ref) : Stream(Ref) {}

ثنائی_Stream_Reader::ثنائی_Stream_Reader(ثنائی_Stream &Stream) : Stream(Stream) {}

ثنائی_Stream_Reader::ثنائی_Stream_Reader(ArrayRef<uint8_t> Data,
                                       endianness Endian)
    : Stream(Data, Endian) {}

ثنائی_Stream_Reader::ثنائی_Stream_Reader(StringRef Data, endianness Endian)
    : Stream(Data, Endian) {}

Error ثنائی_Stream_Reader::readLongestContiguousChunk(
    ArrayRef<uint8_t> &Buffer) {
  إذا (auto EC = Stream.readLongestContiguousChunk(Offset, Buffer))
    أرجع EC;
  Offset += Buffer.size();
  أرجع Error::success();
}

Error ثنائی_Stream_Reader::readBytes(ArrayRef<uint8_t> &Buffer, uint32_t Size) {
  إذا (auto EC = Stream.readBytes(Offset, Size, Buffer))
    أرجع EC;
  Offset += Size;
  أرجع Error::success();
}

Error ثنائی_Stream_Reader::readULEB128(uint64_t &Dest) {
  SmallVector<uint8_t, 10> EncodedBytes;
  ArrayRef<uint8_t> NextByte;

  // Copy the encoded ULEB into the buffer.
  افعل {
    إذا (auto Err = readBytes(NextByte, 1))
      أرجع Err;
    EncodedBytes.ڈالو_back(NextByte[0]);
  } طالما (NextByte[0] & 0x80);

  Dest = decodeULEB128(EncodedBytes.begin(), nullptr, EncodedBytes.end());
  أرجع Error::success();
}

Error ثنائی_Stream_Reader::readSLEB128(int64_t &Dest) {
  SmallVector<uint8_t, 10> EncodedBytes;
  ArrayRef<uint8_t> NextByte;

  // Copy the encoded ULEB into the buffer.
  افعل {
    إذا (auto Err = readBytes(NextByte, 1))
      أرجع Err;
    EncodedBytes.ڈالو_back(NextByte[0]);
  } طالما (NextByte[0] & 0x80);

  Dest = decodeSLEB128(EncodedBytes.begin(), nullptr, EncodedBytes.end());
  أرجع Error::success();
}

Error ثنائی_Stream_Reader::readCString(StringRef &Dest) {
  uint64_t OriginalOffset = getOffset();
  uint64_t FoundOffset = 0;
  طالما (true) {
    uint64_t ThisOffset = getOffset();
    ArrayRef<uint8_t> Buffer;
    إذا (auto EC = readLongestContiguousChunk(Buffer))
      أرجع EC;
    StringRef S(reinterpret_cast<ثابت حرف *>(Buffer.begin()), Buffer.size());
    size_t Pos = S.find_first_of('\0');
    إذا (LLVM_LIKELY(Pos != StringRef::npos)) {
      FoundOffset = Pos + ThisOffset;
      اكسر;
    }
  }
  أكد(FoundOffset >= OriginalOffset);

  setOffset(OriginalOffset);
  size_t Length = FoundOffset - OriginalOffset;

  إذا (auto EC = readFixedString(Dest, Length))
    أرجع EC;

  // Now set the offset back to after the null terminator.
  setOffset(FoundOffset + 1);
  أرجع Error::success();
}

Error ثنائی_Stream_Reader::readWideString(ArrayRef<UTF16> &Dest) {
  uint64_t Length = 0;
  uint64_t OriginalOffset = getOffset();
  ثابت UTF16 *C;
  طالما (true) {
    إذا (auto EC = readObject(C))
      أرجع EC;
    إذا (*C == 0x0000)
      اكسر;
    ++Length;
  }
  uint64_t NewOffset = getOffset();
  setOffset(OriginalOffset);

  إذا (auto EC = readArray(Dest, Length))
    أرجع EC;
  setOffset(NewOffset);
  أرجع Error::success();
}

Error ثنائی_Stream_Reader::readFixedString(StringRef &Dest, uint32_t Length) {
  ArrayRef<uint8_t> Bytes;
  إذا (auto EC = readBytes(Bytes, Length))
    أرجع EC;
  Dest = StringRef(reinterpret_cast<ثابت حرف *>(Bytes.begin()), Bytes.size());
  أرجع Error::success();
}

Error ثنائی_Stream_Reader::readStreamRef(ثنائی_Stream_Ref &Ref) {
  أرجع readStreamRef(Ref, bytesRemaining());
}

Error ثنائی_Stream_Reader::readStreamRef(ثنائی_Stream_Ref &Ref, uint32_t Length) {
  إذا (bytesRemaining() < Length)
    أرجع make_error<ثنائی_Stream_Error>(stream_error_code::stream_too_short);
  Ref = Stream.slice(Offset, Length);
  Offset += Length;
  أرجع Error::success();
}

Error ثنائی_Stream_Reader::readSubstream(ثنائی_Substream_Ref &Ref,
                                        uint32_t Length) {
  Ref.Offset = getOffset();
  أرجع readStreamRef(Ref.StreamData, Length);
}

Error ثنائی_Stream_Reader::skip(uint64_t Amount) {
  إذا (Amount > bytesRemaining())
    أرجع make_error<ثنائی_Stream_Error>(stream_error_code::stream_too_short);
  Offset += Amount;
  أرجع Error::success();
}

Error ثنائی_Stream_Reader::padToAlignment(uint32_t Align) {
  uint32_t NewOffset = alignTo(Offset, Align);
  أرجع skip(NewOffset - Offset);
}

uint8_t ثنائی_Stream_Reader::peek() ثابت {
  ArrayRef<uint8_t> Buffer;
  auto EC = Stream.readBytes(Offset, 1, Buffer);
  أكد(!EC && "Cannot peek an empty buffer!");
  llvm::consumeError(std::move(EC));
  أرجع Buffer[0];
}

std::pair<ثنائی_Stream_Reader, ثنائی_Stream_Reader>
ثنائی_Stream_Reader::split(uint64_t Off) ثابت {
  أكد(getLength() >= Off);

  ثنائی_Stream_Ref First = Stream.drop_front(Offset);

  ثنائی_Stream_Ref Second = First.drop_front(Off);
  First = First.keep_front(Off);
  ثنائی_Stream_Reader W1{First};
  ثنائی_Stream_Reader W2{Second};
  أرجع {W1, W2};
}
