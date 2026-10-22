//===- BinaryStreamReader.cpp - Reads objects from a binary stream --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/BinaryStreamReader.h"

#अन्तर्भाव "llvm/Support/BinaryStreamError.h"
#अन्तर्भाव "llvm/Support/BinaryStreamRef.h"
#अन्तर्भाव "llvm/Support/LEB128.h"

using नामाकाश llvm;

द्विआधारी_Stream_Reader::द्विआधारी_Stream_Reader(द्विआधारी_Stream_Ref Ref) : Stream(Ref) {}

द्विआधारी_Stream_Reader::द्विआधारी_Stream_Reader(द्विआधारी_Stream &Stream) : Stream(Stream) {}

द्विआधारी_Stream_Reader::द्विआधारी_Stream_Reader(ArrayRef<uint8_t> Data,
                                       endianness Endian)
    : Stream(Data, Endian) {}

द्विआधारी_Stream_Reader::द्विआधारी_Stream_Reader(StringRef Data, endianness Endian)
    : Stream(Data, Endian) {}

Error द्विआधारी_Stream_Reader::readLongestContiguousChunk(
    ArrayRef<uint8_t> &Buffer) {
  यदि (स्वतः EC = Stream.readLongestContiguousChunk(Offset, Buffer))
    फर्काऊ EC;
  Offset += Buffer.size();
  फर्काऊ Error::success();
}

Error द्विआधारी_Stream_Reader::readBytes(ArrayRef<uint8_t> &Buffer, uint32_t Size) {
  यदि (स्वतः EC = Stream.readBytes(Offset, Size, Buffer))
    फर्काऊ EC;
  Offset += Size;
  फर्काऊ Error::success();
}

Error द्विआधारी_Stream_Reader::readULEB128(uint64_t &Dest) {
  SmallVector<uint8_t, 10> EncodedBytes;
  ArrayRef<uint8_t> NextByte;

  // Copy the encoded ULEB into the buffer.
  गर {
    यदि (स्वतः Err = readBytes(NextByte, 1))
      फर्काऊ Err;
    EncodedBytes.थप_back(NextByte[0]);
  } जबसम्म (NextByte[0] & 0x80);

  Dest = decodeULEB128(EncodedBytes.begin(), nullptr, EncodedBytes.end());
  फर्काऊ Error::success();
}

Error द्विआधारी_Stream_Reader::readSLEB128(int64_t &Dest) {
  SmallVector<uint8_t, 10> EncodedBytes;
  ArrayRef<uint8_t> NextByte;

  // Copy the encoded ULEB into the buffer.
  गर {
    यदि (स्वतः Err = readBytes(NextByte, 1))
      फर्काऊ Err;
    EncodedBytes.थप_back(NextByte[0]);
  } जबसम्म (NextByte[0] & 0x80);

  Dest = decodeSLEB128(EncodedBytes.begin(), nullptr, EncodedBytes.end());
  फर्काऊ Error::success();
}

Error द्विआधारी_Stream_Reader::readCString(StringRef &Dest) {
  uint64_t OriginalOffset = getOffset();
  uint64_t FoundOffset = 0;
  जबसम्म (सत्य) {
    uint64_t ThisOffset = getOffset();
    ArrayRef<uint8_t> Buffer;
    यदि (स्वतः EC = readLongestContiguousChunk(Buffer))
      फर्काऊ EC;
    StringRef S(reinterpret_cast<अचर वर्ण *>(Buffer.begin()), Buffer.size());
    माप_प्रकार Pos = S.find_first_of('\0');
    यदि (LLVM_LIKELY(Pos != StringRef::npos)) {
      FoundOffset = Pos + ThisOffset;
      रोक;
    }
  }
  प्रतिज्ञा(FoundOffset >= OriginalOffset);

  setOffset(OriginalOffset);
  माप_प्रकार Length = FoundOffset - OriginalOffset;

  यदि (स्वतः EC = readFixedString(Dest, Length))
    फर्काऊ EC;

  // Now set the offset back to after the null terminator.
  setOffset(FoundOffset + 1);
  फर्काऊ Error::success();
}

Error द्विआधारी_Stream_Reader::readWideString(ArrayRef<UTF16> &Dest) {
  uint64_t Length = 0;
  uint64_t OriginalOffset = getOffset();
  अचर UTF16 *C;
  जबसम्म (सत्य) {
    यदि (स्वतः EC = readObject(C))
      फर्काऊ EC;
    यदि (*C == 0x0000)
      रोक;
    ++Length;
  }
  uint64_t NewOffset = getOffset();
  setOffset(OriginalOffset);

  यदि (स्वतः EC = readArray(Dest, Length))
    फर्काऊ EC;
  setOffset(NewOffset);
  फर्काऊ Error::success();
}

Error द्विआधारी_Stream_Reader::readFixedString(StringRef &Dest, uint32_t Length) {
  ArrayRef<uint8_t> Bytes;
  यदि (स्वतः EC = readBytes(Bytes, Length))
    फर्काऊ EC;
  Dest = StringRef(reinterpret_cast<अचर वर्ण *>(Bytes.begin()), Bytes.size());
  फर्काऊ Error::success();
}

Error द्विआधारी_Stream_Reader::readStreamRef(द्विआधारी_Stream_Ref &Ref) {
  फर्काऊ readStreamRef(Ref, bytesRemaining());
}

Error द्विआधारी_Stream_Reader::readStreamRef(द्विआधारी_Stream_Ref &Ref, uint32_t Length) {
  यदि (bytesRemaining() < Length)
    फर्काऊ make_error<द्विआधारी_Stream_Error>(stream_error_code::stream_too_short);
  Ref = Stream.slice(Offset, Length);
  Offset += Length;
  फर्काऊ Error::success();
}

Error द्विआधारी_Stream_Reader::readSubstream(द्विआधारी_Substream_Ref &Ref,
                                        uint32_t Length) {
  Ref.Offset = getOffset();
  फर्काऊ readStreamRef(Ref.StreamData, Length);
}

Error द्विआधारी_Stream_Reader::skip(uint64_t Amount) {
  यदि (Amount > bytesRemaining())
    फर्काऊ make_error<द्विआधारी_Stream_Error>(stream_error_code::stream_too_short);
  Offset += Amount;
  फर्काऊ Error::success();
}

Error द्विआधारी_Stream_Reader::padToAlignment(uint32_t Align) {
  uint32_t NewOffset = alignTo(Offset, Align);
  फर्काऊ skip(NewOffset - Offset);
}

uint8_t द्विआधारी_Stream_Reader::peek() अचर {
  ArrayRef<uint8_t> Buffer;
  स्वतः EC = Stream.readBytes(Offset, 1, Buffer);
  प्रतिज्ञा(!EC && "Cannot peek an empty buffer!");
  llvm::consumeError(std::move(EC));
  फर्काऊ Buffer[0];
}

std::pair<द्विआधारी_Stream_Reader, द्विआधारी_Stream_Reader>
द्विआधारी_Stream_Reader::split(uint64_t Off) अचर {
  प्रतिज्ञा(getLength() >= Off);

  द्विआधारी_Stream_Ref First = Stream.drop_front(Offset);

  द्विआधारी_Stream_Ref Second = First.drop_front(Off);
  First = First.keep_front(Off);
  द्विआधारी_Stream_Reader W1{First};
  द्विआधारी_Stream_Reader W2{Second};
  फर्काऊ {W1, W2};
}
