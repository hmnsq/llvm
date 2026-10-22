//===- BinaryStreamReader.cpp - Reads objects from a binary stream --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/BinaryStreamReader.h"

#समावेश "llvm/Support/BinaryStreamError.h"
#समावेश "llvm/Support/BinaryStreamRef.h"
#समावेश "llvm/Support/LEB128.h"

using namespace llvm;

द्विआधारी_Stream_Reader::द्विआधारी_Stream_Reader(द्विआधारी_Stream_Ref Ref) : Stream(Ref) {}

द्विआधारी_Stream_Reader::द्विआधारी_Stream_Reader(द्विआधारी_Stream &Stream) : Stream(Stream) {}

द्विआधारी_Stream_Reader::द्विआधारी_Stream_Reader(ArrayRef<uint8_t> तथ्य_2,
                                       endianness Endian)
    : Stream(तथ्य_2, Endian) {}

द्विआधारी_Stream_Reader::द्विआधारी_Stream_Reader(StringRef तथ्य_2, endianness Endian)
    : Stream(तथ्य_2, Endian) {}

Error द्विआधारी_Stream_Reader::readLongestContiguousChunk(
    ArrayRef<uint8_t> &बफर) {
  अगर (स्वतः EC = Stream.readLongestContiguousChunk(Offset, बफर))
    वापस EC;
  Offset += बफर.आकार_2();
  वापस Error::success();
}

Error द्विआधारी_Stream_Reader::readBytes(ArrayRef<uint8_t> &बफर, uint32_t आकार) {
  अगर (स्वतः EC = Stream.readBytes(Offset, आकार, बफर))
    वापस EC;
  Offset += आकार;
  वापस Error::success();
}

Error द्विआधारी_Stream_Reader::readULEB128(uint64_t &Dest) {
  SmallVector<uint8_t, 10> EncodedBytes;
  ArrayRef<uint8_t> अग्र_Byte;

  // Copy the encoded ULEB into the buffer.
  करो {
    अगर (स्वतः Err = readBytes(अग्र_Byte, 1))
      वापस Err;
    EncodedBytes.धकेलो_back(अग्र_Byte[0]);
  } जबतक (अग्र_Byte[0] & 0x80);

  Dest = decodeULEB128(EncodedBytes.begin(), nullptr, EncodedBytes.end());
  वापस Error::success();
}

Error द्विआधारी_Stream_Reader::readSLEB128(int64_t &Dest) {
  SmallVector<uint8_t, 10> EncodedBytes;
  ArrayRef<uint8_t> अग्र_Byte;

  // Copy the encoded ULEB into the buffer.
  करो {
    अगर (स्वतः Err = readBytes(अग्र_Byte, 1))
      वापस Err;
    EncodedBytes.धकेलो_back(अग्र_Byte[0]);
  } जबतक (अग्र_Byte[0] & 0x80);

  Dest = decodeSLEB128(EncodedBytes.begin(), nullptr, EncodedBytes.end());
  वापस Error::success();
}

Error द्विआधारी_Stream_Reader::readCString(StringRef &Dest) {
  uint64_t OriginalOffset = getOffset();
  uint64_t FoundOffset = 0;
  जबतक (true) {
    uint64_t ThisOffset = getOffset();
    ArrayRef<uint8_t> बफर;
    अगर (स्वतः EC = readLongestContiguousChunk(बफर))
      वापस EC;
    StringRef S(reinterpret_cast<स्थिर अक्षर *>(बफर.begin()), बफर.आकार_2());
    माप_प्रकार Pos = S.find_first_of('\0');
    अगर (LLVM_LIKELY(Pos != StringRef::npos)) {
      FoundOffset = Pos + ThisOffset;
      अवरोध;
    }
  }
  निश्चित(FoundOffset >= OriginalOffset);

  setOffset(OriginalOffset);
  माप_प्रकार लंबाई = FoundOffset - OriginalOffset;

  अगर (स्वतः EC = readFixedString(Dest, लंबाई))
    वापस EC;

  // Now set the offset back to after the null terminator.
  setOffset(FoundOffset + 1);
  वापस Error::success();
}

Error द्विआधारी_Stream_Reader::readWideString(ArrayRef<UTF16> &Dest) {
  uint64_t लंबाई = 0;
  uint64_t OriginalOffset = getOffset();
  स्थिर UTF16 *C;
  जबतक (true) {
    अगर (स्वतः EC = readObject(C))
      वापस EC;
    अगर (*C == 0x0000)
      अवरोध;
    ++लंबाई;
  }
  uint64_t NewOffset = getOffset();
  setOffset(OriginalOffset);

  अगर (स्वतः EC = readArray(Dest, लंबाई))
    वापस EC;
  setOffset(NewOffset);
  वापस Error::success();
}

Error द्विआधारी_Stream_Reader::readFixedString(StringRef &Dest, uint32_t लंबाई) {
  ArrayRef<uint8_t> Bytes;
  अगर (स्वतः EC = readBytes(Bytes, लंबाई))
    वापस EC;
  Dest = StringRef(reinterpret_cast<स्थिर अक्षर *>(Bytes.begin()), Bytes.आकार_2());
  वापस Error::success();
}

Error द्विआधारी_Stream_Reader::readStreamRef(द्विआधारी_Stream_Ref &Ref) {
  वापस readStreamRef(Ref, bytesRemaining());
}

Error द्विआधारी_Stream_Reader::readStreamRef(द्विआधारी_Stream_Ref &Ref, uint32_t लंबाई) {
  अगर (bytesRemaining() < लंबाई)
    वापस make_error<द्विआधारी_Stream_Error>(stream_error_code::stream_too_short);
  Ref = Stream.slice(Offset, लंबाई);
  Offset += लंबाई;
  वापस Error::success();
}

Error द्विआधारी_Stream_Reader::readSubstream(द्विआधारी_Substream_Ref &Ref,
                                        uint32_t लंबाई) {
  Ref.Offset = getOffset();
  वापस readStreamRef(Ref.Stream_तथ्य, लंबाई);
}

Error द्विआधारी_Stream_Reader::skip(uint64_t Amount) {
  अगर (Amount > bytesRemaining())
    वापस make_error<द्विआधारी_Stream_Error>(stream_error_code::stream_too_short);
  Offset += Amount;
  वापस Error::success();
}

Error द्विआधारी_Stream_Reader::padToAlignment(uint32_t Align) {
  uint32_t NewOffset = alignTo(Offset, Align);
  वापस skip(NewOffset - Offset);
}

uint8_t द्विआधारी_Stream_Reader::peek() स्थिर {
  ArrayRef<uint8_t> बफर;
  स्वतः EC = Stream.readBytes(Offset, 1, बफर);
  निश्चित(!EC && "Cannot peek an empty buffer!");
  llvm::consumeError(std::move(EC));
  वापस बफर[0];
}

std::pair<द्विआधारी_Stream_Reader, द्विआधारी_Stream_Reader>
द्विआधारी_Stream_Reader::split(uint64_t Off) स्थिर {
  निश्चित(get_लंबाई() >= Off);

  द्विआधारी_Stream_Ref First = Stream.drop_front(Offset);

  द्विआधारी_Stream_Ref Second = First.drop_front(Off);
  First = First.keep_front(Off);
  द्विआधारी_Stream_Reader W1{First};
  द्विआधारी_Stream_Reader W2{Second};
  वापस {W1, W2};
}
