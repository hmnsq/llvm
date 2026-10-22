//===- BinaryStreamReader.cpp - Reads objects from a binary stream --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/Support/BinaryStreamReader.h"

#அந்தர்பாவ "llvm/Support/BinaryStreamError.h"
#அந்தர்பாவ "llvm/Support/BinaryStreamRef.h"
#அந்தர்பாவ "llvm/Support/LEB128.h"

using நாமாகாஶ llvm;

இருமம்_Stream_Reader::இருமம்_Stream_Reader(இருமம்_Stream_Ref Ref) : Stream(Ref) {}

இருமம்_Stream_Reader::இருமம்_Stream_Reader(இருமம்_Stream &Stream) : Stream(Stream) {}

இருமம்_Stream_Reader::இருமம்_Stream_Reader(ArrayRef<uint8_t> Data,
                                       endianness Endian)
    : Stream(Data, Endian) {}

இருமம்_Stream_Reader::இருமம்_Stream_Reader(StringRef Data, endianness Endian)
    : Stream(Data, Endian) {}

Error இருமம்_Stream_Reader::readLongestContiguousChunk(
    ArrayRef<uint8_t> &Buffer) {
  எனில் (auto EC = Stream.readLongestContiguousChunk(Offset, Buffer))
    திரும்பு EC;
  Offset += Buffer.size();
  திரும்பு Error::success();
}

Error இருமம்_Stream_Reader::readBytes(ArrayRef<uint8_t> &Buffer, uint32_t Size) {
  எனில் (auto EC = Stream.readBytes(Offset, Size, Buffer))
    திரும்பு EC;
  Offset += Size;
  திரும்பு Error::success();
}

Error இருமம்_Stream_Reader::readULEB128(uint64_t &Dest) {
  SmallVector<uint8_t, 10> EncodedBytes;
  ArrayRef<uint8_t> NextByte;

  // Copy the encoded ULEB into the buffer.
  செய் {
    எனில் (auto Err = readBytes(NextByte, 1))
      திரும்பு Err;
    EncodedBytes.தள்ளு_back(NextByte[0]);
  } போது (NextByte[0] & 0x80);

  Dest = decodeULEB128(EncodedBytes.begin(), nullptr, EncodedBytes.end());
  திரும்பு Error::success();
}

Error இருமம்_Stream_Reader::readSLEB128(int64_t &Dest) {
  SmallVector<uint8_t, 10> EncodedBytes;
  ArrayRef<uint8_t> NextByte;

  // Copy the encoded ULEB into the buffer.
  செய் {
    எனில் (auto Err = readBytes(NextByte, 1))
      திரும்பு Err;
    EncodedBytes.தள்ளு_back(NextByte[0]);
  } போது (NextByte[0] & 0x80);

  Dest = decodeSLEB128(EncodedBytes.begin(), nullptr, EncodedBytes.end());
  திரும்பு Error::success();
}

Error இருமம்_Stream_Reader::readCString(StringRef &Dest) {
  uint64_t OriginalOffset = getOffset();
  uint64_t FoundOffset = 0;
  போது (உண்மை) {
    uint64_t ThisOffset = getOffset();
    ArrayRef<uint8_t> Buffer;
    எனில் (auto EC = readLongestContiguousChunk(Buffer))
      திரும்பு EC;
    StringRef S(reinterpret_cast<மாறிலி எழுத்து *>(Buffer.begin()), Buffer.size());
    size_t Pos = S.find_first_of('\0');
    எனில் (LLVM_LIKELY(Pos != StringRef::npos)) {
      FoundOffset = Pos + ThisOffset;
      நிறுத்து;
    }
  }
  ப்ரதிஜ்ஞா(FoundOffset >= OriginalOffset);

  setOffset(OriginalOffset);
  size_t Length = FoundOffset - OriginalOffset;

  எனில் (auto EC = readFixedString(Dest, Length))
    திரும்பு EC;

  // Now set the offset back to after the null terminator.
  setOffset(FoundOffset + 1);
  திரும்பு Error::success();
}

Error இருமம்_Stream_Reader::readWideString(ArrayRef<UTF16> &Dest) {
  uint64_t Length = 0;
  uint64_t OriginalOffset = getOffset();
  மாறிலி UTF16 *C;
  போது (உண்மை) {
    எனில் (auto EC = readObject(C))
      திரும்பு EC;
    எனில் (*C == 0x0000)
      நிறுத்து;
    ++Length;
  }
  uint64_t NewOffset = getOffset();
  setOffset(OriginalOffset);

  எனில் (auto EC = readArray(Dest, Length))
    திரும்பு EC;
  setOffset(NewOffset);
  திரும்பு Error::success();
}

Error இருமம்_Stream_Reader::readFixedString(StringRef &Dest, uint32_t Length) {
  ArrayRef<uint8_t> Bytes;
  எனில் (auto EC = readBytes(Bytes, Length))
    திரும்பு EC;
  Dest = StringRef(reinterpret_cast<மாறிலி எழுத்து *>(Bytes.begin()), Bytes.size());
  திரும்பு Error::success();
}

Error இருமம்_Stream_Reader::readStreamRef(இருமம்_Stream_Ref &Ref) {
  திரும்பு readStreamRef(Ref, bytesRemaining());
}

Error இருமம்_Stream_Reader::readStreamRef(இருமம்_Stream_Ref &Ref, uint32_t Length) {
  எனில் (bytesRemaining() < Length)
    திரும்பு make_error<இருமம்_Stream_Error>(stream_error_code::stream_too_short);
  Ref = Stream.slice(Offset, Length);
  Offset += Length;
  திரும்பு Error::success();
}

Error இருமம்_Stream_Reader::readSubstream(இருமம்_Substream_Ref &Ref,
                                        uint32_t Length) {
  Ref.Offset = getOffset();
  திரும்பு readStreamRef(Ref.StreamData, Length);
}

Error இருமம்_Stream_Reader::skip(uint64_t Amount) {
  எனில் (Amount > bytesRemaining())
    திரும்பு make_error<இருமம்_Stream_Error>(stream_error_code::stream_too_short);
  Offset += Amount;
  திரும்பு Error::success();
}

Error இருமம்_Stream_Reader::padToAlignment(uint32_t Align) {
  uint32_t NewOffset = alignTo(Offset, Align);
  திரும்பு skip(NewOffset - Offset);
}

uint8_t இருமம்_Stream_Reader::peek() மாறிலி {
  ArrayRef<uint8_t> Buffer;
  auto EC = Stream.readBytes(Offset, 1, Buffer);
  ப்ரதிஜ்ஞா(!EC && "Cannot peek an empty buffer!");
  llvm::consumeError(std::move(EC));
  திரும்பு Buffer[0];
}

std::pair<இருமம்_Stream_Reader, இருமம்_Stream_Reader>
இருமம்_Stream_Reader::split(uint64_t Off) மாறிலி {
  ப்ரதிஜ்ஞா(getLength() >= Off);

  இருமம்_Stream_Ref First = Stream.drop_front(Offset);

  இருமம்_Stream_Ref Second = First.drop_front(Off);
  First = First.keep_front(Off);
  இருமம்_Stream_Reader W1{First};
  இருமம்_Stream_Reader W2{Second};
  திரும்பு {W1, W2};
}
