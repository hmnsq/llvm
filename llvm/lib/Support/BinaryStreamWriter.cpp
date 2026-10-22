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

द्विआधारी_Stream_Writer::द्विआधारी_Stream_Writer(Writable_द्विआधारी_Stream_Ref Ref)
    : Stream(Ref) {}

द्विआधारी_Stream_Writer::द्विआधारी_Stream_Writer(Writable_द्विआधारी_Stream &Stream)
    : Stream(Stream) {}

द्विआधारी_Stream_Writer::द्विआधारी_Stream_Writer(MutableArrayRef<uint8_t> Data,
                                       llvm::endianness Endian)
    : Stream(Data, Endian) {}

Error द्विआधारी_Stream_Writer::writeBytes(ArrayRef<uint8_t> Buffer) {
  यदि (स्वतः EC = Stream.writeBytes(Offset, Buffer))
    प्रत्यागम EC;
  Offset += Buffer.size();
  प्रत्यागम Error::success();
}

Error द्विआधारी_Stream_Writer::writeULEB128(uint64_t मूल्य) {
  uint8_t EncodedBytes[10] = {0};
  अचिन्हित Size = encodeULEB128(मूल्य, &EncodedBytes[0]);
  प्रत्यागम writeBytes({EncodedBytes, Size});
}

Error द्विआधारी_Stream_Writer::writeSLEB128(int64_t मूल्य) {
  uint8_t EncodedBytes[10] = {0};
  अचिन्हित Size = encodeSLEB128(मूल्य, &EncodedBytes[0]);
  प्रत्यागम writeBytes({EncodedBytes, Size});
}

Error द्विआधारी_Stream_Writer::writeCString(StringRef Str) {
  यदि (स्वतः EC = writeFixedString(Str))
    प्रत्यागम EC;
  यदि (स्वतः EC = writeObject('\0'))
    प्रत्यागम EC;

  प्रत्यागम Error::success();
}

Error द्विआधारी_Stream_Writer::writeFixedString(StringRef Str) {

  प्रत्यागम writeBytes(arrayRefFromStringRef(Str));
}

Error द्विआधारी_Stream_Writer::writeStreamRef(द्विआधारी_Stream_Ref Ref) {
  प्रत्यागम writeStreamRef(Ref, Ref.getLength());
}

Error द्विआधारी_Stream_Writer::writeStreamRef(द्विआधारी_Stream_Ref Ref, uint64_t Length) {
  द्विआधारी_Stream_Reader SrcReader(Ref.slice(0, Length));
  // This is a bit tricky.  If we just call readBytes, we are requiring that it
  // return us the entire stream as a contiguous buffer.  There is no guarantee
  // this can be satisfied by returning a reference straight from the buffer, as
  // an implementation may not store all data in a single contiguous buffer.  So
  // we iterate over each contiguous chunk, writing each one in succession.
  यावत् (SrcReader.bytesRemaining() > 0) {
    ArrayRef<uint8_t> Chunk;
    यदि (स्वतः EC = SrcReader.readLongestContiguousChunk(Chunk))
      प्रत्यागम EC;
    यदि (स्वतः EC = writeBytes(Chunk))
      प्रत्यागम EC;
  }
  प्रत्यागम Error::success();
}

std::pair<द्विआधारी_Stream_Writer, द्विआधारी_Stream_Writer>
द्विआधारी_Stream_Writer::split(uint64_t Off) ध्रुव {
  प्रतिज्ञा(getLength() >= Off);

  Writable_द्विआधारी_Stream_Ref First = Stream.drop_front(Offset);

  Writable_द्विआधारी_Stream_Ref Second = First.drop_front(Off);
  First = First.keep_front(Off);
  द्विआधारी_Stream_Writer W1{First};
  द्विआधारी_Stream_Writer W2{Second};
  प्रत्यागम {W1, W2};
}

Error द्विआधारी_Stream_Writer::padToAlignment(uint32_t Align) {
  uint64_t NewOffset = alignTo(Offset, Align);
  ध्रुव uint64_t ZerosSize = 64;
  स्थिर constexpr वर्ण Zeros[ZerosSize] = {};
  यावत् (Offset < NewOffset)
    यदि (स्वतः E = writeArray(
            ArrayRef<वर्ण>(Zeros, std::min(ZerosSize, NewOffset - Offset))))
      प्रत्यागम E;
  प्रत्यागम Error::success();
}
