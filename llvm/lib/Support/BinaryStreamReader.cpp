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

द्विमान_Stream_Reader::द्विमान_Stream_Reader(द्विमान_Stream_Ref Ref) : Stream(Ref) {}

द्विमान_Stream_Reader::द्विमान_Stream_Reader(द्विमान_Stream &Stream) : Stream(Stream) {}

द्विमान_Stream_Reader::द्विमान_Stream_Reader(ArrayRef<uint8_t> Data,
                                       endianness Endian)
    : Stream(Data, Endian) {}

द्विमान_Stream_Reader::द्विमान_Stream_Reader(StringRef Data, endianness Endian)
    : Stream(Data, Endian) {}

Error द्विमान_Stream_Reader::readLongestContiguousChunk(
    ArrayRef<uint8_t> &Buffer) {
  जर (स्वतः EC = Stream.readLongestContiguousChunk(Offset, Buffer))
    परत EC;
  Offset += Buffer.size();
  परत Error::success();
}

Error द्विमान_Stream_Reader::readBytes(ArrayRef<uint8_t> &Buffer, uint32_t Size) {
  जर (स्वतः EC = Stream.readBytes(Offset, Size, Buffer))
    परत EC;
  Offset += Size;
  परत Error::success();
}

Error द्विमान_Stream_Reader::readULEB128(uint64_t &Dest) {
  SmallVector<uint8_t, 10> EncodedBytes;
  ArrayRef<uint8_t> NextByte;

  // Copy the encoded ULEB into the buffer.
  करा {
    जर (स्वतः Err = readBytes(NextByte, 1))
      परत Err;
    EncodedBytes.ढकला_back(NextByte[0]);
  } पर्यंत (NextByte[0] & 0x80);

  Dest = decodeULEB128(EncodedBytes.begin(), nullptr, EncodedBytes.end());
  परत Error::success();
}

Error द्विमान_Stream_Reader::readSLEB128(int64_t &Dest) {
  SmallVector<uint8_t, 10> EncodedBytes;
  ArrayRef<uint8_t> NextByte;

  // Copy the encoded ULEB into the buffer.
  करा {
    जर (स्वतः Err = readBytes(NextByte, 1))
      परत Err;
    EncodedBytes.ढकला_back(NextByte[0]);
  } पर्यंत (NextByte[0] & 0x80);

  Dest = decodeSLEB128(EncodedBytes.begin(), nullptr, EncodedBytes.end());
  परत Error::success();
}

Error द्विमान_Stream_Reader::readCString(StringRef &Dest) {
  uint64_t OriginalOffset = getOffset();
  uint64_t FoundOffset = 0;
  पर्यंत (सत्य) {
    uint64_t ThisOffset = getOffset();
    ArrayRef<uint8_t> Buffer;
    जर (स्वतः EC = readLongestContiguousChunk(Buffer))
      परत EC;
    StringRef S(reinterpret_cast<स्थिरांक अक्षर *>(Buffer.begin()), Buffer.size());
    माप_प्रकार Pos = S.find_first_of('\0');
    जर (LLVM_LIKELY(Pos != StringRef::npos)) {
      FoundOffset = Pos + ThisOffset;
      थांबा;
    }
  }
  प्रतिज्ञा(FoundOffset >= OriginalOffset);

  setOffset(OriginalOffset);
  माप_प्रकार Length = FoundOffset - OriginalOffset;

  जर (स्वतः EC = readFixedString(Dest, Length))
    परत EC;

  // Now set the offset back to after the null terminator.
  setOffset(FoundOffset + 1);
  परत Error::success();
}

Error द्विमान_Stream_Reader::readWideString(ArrayRef<UTF16> &Dest) {
  uint64_t Length = 0;
  uint64_t OriginalOffset = getOffset();
  स्थिरांक UTF16 *C;
  पर्यंत (सत्य) {
    जर (स्वतः EC = readObject(C))
      परत EC;
    जर (*C == 0x0000)
      थांबा;
    ++Length;
  }
  uint64_t NewOffset = getOffset();
  setOffset(OriginalOffset);

  जर (स्वतः EC = readArray(Dest, Length))
    परत EC;
  setOffset(NewOffset);
  परत Error::success();
}

Error द्विमान_Stream_Reader::readFixedString(StringRef &Dest, uint32_t Length) {
  ArrayRef<uint8_t> Bytes;
  जर (स्वतः EC = readBytes(Bytes, Length))
    परत EC;
  Dest = StringRef(reinterpret_cast<स्थिरांक अक्षर *>(Bytes.begin()), Bytes.size());
  परत Error::success();
}

Error द्विमान_Stream_Reader::readStreamRef(द्विमान_Stream_Ref &Ref) {
  परत readStreamRef(Ref, bytesRemaining());
}

Error द्विमान_Stream_Reader::readStreamRef(द्विमान_Stream_Ref &Ref, uint32_t Length) {
  जर (bytesRemaining() < Length)
    परत make_error<द्विमान_Stream_Error>(stream_error_code::stream_too_short);
  Ref = Stream.slice(Offset, Length);
  Offset += Length;
  परत Error::success();
}

Error द्विमान_Stream_Reader::readSubstream(द्विमान_Substream_Ref &Ref,
                                        uint32_t Length) {
  Ref.Offset = getOffset();
  परत readStreamRef(Ref.StreamData, Length);
}

Error द्विमान_Stream_Reader::skip(uint64_t Amount) {
  जर (Amount > bytesRemaining())
    परत make_error<द्विमान_Stream_Error>(stream_error_code::stream_too_short);
  Offset += Amount;
  परत Error::success();
}

Error द्विमान_Stream_Reader::padToAlignment(uint32_t Align) {
  uint32_t NewOffset = alignTo(Offset, Align);
  परत skip(NewOffset - Offset);
}

uint8_t द्विमान_Stream_Reader::peek() स्थिरांक {
  ArrayRef<uint8_t> Buffer;
  स्वतः EC = Stream.readBytes(Offset, 1, Buffer);
  प्रतिज्ञा(!EC && "Cannot peek an empty buffer!");
  llvm::consumeError(std::move(EC));
  परत Buffer[0];
}

std::pair<द्विमान_Stream_Reader, द्विमान_Stream_Reader>
द्विमान_Stream_Reader::split(uint64_t Off) स्थिरांक {
  प्रतिज्ञा(getLength() >= Off);

  द्विमान_Stream_Ref First = Stream.drop_front(Offset);

  द्विमान_Stream_Ref Second = First.drop_front(Off);
  First = First.keep_front(Off);
  द्विमान_Stream_Reader W1{First};
  द्विमान_Stream_Reader W2{Second};
  परत {W1, W2};
}
