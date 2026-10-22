//===- BinaryStreamWriter.cpp - Writes objects to a BinaryStream ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/BinaryStreamWriter.h"

#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/Support/BinaryStreamReader.h"
#अन्तर्भाव "llvm/Support/BinaryStreamRef.h"
#अन्तर्भाव "llvm/Support/LEB128.h"

using नामाकाश llvm;

द्विमान_Stream_Writer::द्विमान_Stream_Writer(Writable_द्विमान_Stream_Ref Ref)
    : Stream(Ref) {}

द्विमान_Stream_Writer::द्विमान_Stream_Writer(Writable_द्विमान_Stream &Stream)
    : Stream(Stream) {}

द्विमान_Stream_Writer::द्विमान_Stream_Writer(MutableArrayRef<uint8_t> Data,
                                       llvm::endianness Endian)
    : Stream(Data, Endian) {}

Error द्विमान_Stream_Writer::writeBytes(ArrayRef<uint8_t> Buffer) {
  सचे (auto EC = Stream.writeBytes(Offset, Buffer))
    पटिनिवत्त EC;
  Offset += Buffer.size();
  पटिनिवत्त Error::success();
}

Error द्विमान_Stream_Writer::writeULEB128(uint64_t मानम्) {
  uint8_t EncodedBytes[10] = {0};
  unsigned Size = encodeULEB128(मानम्, &EncodedBytes[0]);
  पटिनिवत्त writeBytes({EncodedBytes, Size});
}

Error द्विमान_Stream_Writer::writeSLEB128(int64_t मानम्) {
  uint8_t EncodedBytes[10] = {0};
  unsigned Size = encodeSLEB128(मानम्, &EncodedBytes[0]);
  पटिनिवत्त writeBytes({EncodedBytes, Size});
}

Error द्विमान_Stream_Writer::writeCString(StringRef Str) {
  सचे (auto EC = writeFixedString(Str))
    पटिनिवत्त EC;
  सचे (auto EC = writeObject('\0'))
    पटिनिवत्त EC;

  पटिनिवत्त Error::success();
}

Error द्विमान_Stream_Writer::writeFixedString(StringRef Str) {

  पटिनिवत्त writeBytes(arrayRefFromStringRef(Str));
}

Error द्विमान_Stream_Writer::writeStreamRef(द्विमान_Stream_Ref Ref) {
  पटिनिवत्त writeStreamRef(Ref, Ref.getLength());
}

Error द्विमान_Stream_Writer::writeStreamRef(द्विमान_Stream_Ref Ref, uint64_t Length) {
  द्विमान_Stream_Reader SrcReader(Ref.slice(0, Length));
  // This is a bit tricky.  If we just call readBytes, we are requiring that it
  // return us the entire stream as a contiguous buffer.  There is no guarantee
  // this can be satisfied by returning a reference straight from the buffer, as
  // an implementation may not store all data in a single contiguous buffer.  So
  // we iterate over each contiguous chunk, writing each one in succession.
  याव (SrcReader.bytesRemaining() > 0) {
    ArrayRef<uint8_t> Chunk;
    सचे (auto EC = SrcReader.readLongestContiguousChunk(Chunk))
      पटिनिवत्त EC;
    सचे (auto EC = writeBytes(Chunk))
      पटिनिवत्त EC;
  }
  पटिनिवत्त Error::success();
}

std::pair<द्विमान_Stream_Writer, द्विमान_Stream_Writer>
द्विमान_Stream_Writer::split(uint64_t Off) ध्रुव {
  प्रतिज्ञा(getLength() >= Off);

  Writable_द्विमान_Stream_Ref First = Stream.drop_front(Offset);

  Writable_द्विमान_Stream_Ref Second = First.drop_front(Off);
  First = First.keep_front(Off);
  द्विमान_Stream_Writer W1{First};
  द्विमान_Stream_Writer W2{Second};
  पटिनिवत्त {W1, W2};
}

Error द्विमान_Stream_Writer::padToAlignment(uint32_t Align) {
  uint64_t NewOffset = alignTo(Offset, Align);
  ध्रुव uint64_t ZerosSize = 64;
  स्थिर constexpr वर्ण Zeros[ZerosSize] = {};
  याव (Offset < NewOffset)
    सचे (auto E = writeArray(
            ArrayRef<वर्ण>(Zeros, std::min(ZerosSize, NewOffset - Offset))))
      पटिनिवत्त E;
  पटिनिवत्त Error::success();
}
