//===- BinaryStreamWriter.cpp - Writes objects to a BinaryStream ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/BinaryStreamWriter.h"

#समावेश "llvm/ADT/StringExtras.h"
#समावेश "llvm/Support/BinaryStreamReader.h"
#समावेश "llvm/Support/BinaryStreamRef.h"
#समावेश "llvm/Support/LEB128.h"

using namespace llvm;

द्विआधारी_Stream_Writer::द्विआधारी_Stream_Writer(Writable_द्विआधारी_Stream_Ref Ref)
    : Stream(Ref) {}

द्विआधारी_Stream_Writer::द्विआधारी_Stream_Writer(Writable_द्विआधारी_Stream &Stream)
    : Stream(Stream) {}

द्विआधारी_Stream_Writer::द्विआधारी_Stream_Writer(MutableArrayRef<uint8_t> तथ्य_2,
                                       llvm::endianness Endian)
    : Stream(तथ्य_2, Endian) {}

Error द्विआधारी_Stream_Writer::writeBytes(ArrayRef<uint8_t> बफर) {
  अगर (स्वतः EC = Stream.writeBytes(Offset, बफर))
    वापस EC;
  Offset += बफर.आकार_2();
  वापस Error::success();
}

Error द्विआधारी_Stream_Writer::writeULEB128(uint64_t मूल्य) {
  uint8_t EncodedBytes[10] = {0};
  अचिन्हित आकार = encodeULEB128(मूल्य, &EncodedBytes[0]);
  वापस writeBytes({EncodedBytes, आकार});
}

Error द्विआधारी_Stream_Writer::writeSLEB128(int64_t मूल्य) {
  uint8_t EncodedBytes[10] = {0};
  अचिन्हित आकार = encodeSLEB128(मूल्य, &EncodedBytes[0]);
  वापस writeBytes({EncodedBytes, आकार});
}

Error द्विआधारी_Stream_Writer::writeCString(StringRef Str) {
  अगर (स्वतः EC = writeFixedString(Str))
    वापस EC;
  अगर (स्वतः EC = writeObject('\0'))
    वापस EC;

  वापस Error::success();
}

Error द्विआधारी_Stream_Writer::writeFixedString(StringRef Str) {

  वापस writeBytes(arrayRefFromStringRef(Str));
}

Error द्विआधारी_Stream_Writer::writeStreamRef(द्विआधारी_Stream_Ref Ref) {
  वापस writeStreamRef(Ref, Ref.get_लंबाई());
}

Error द्विआधारी_Stream_Writer::writeStreamRef(द्विआधारी_Stream_Ref Ref, uint64_t लंबाई) {
  द्विआधारी_Stream_Reader SrcReader(Ref.slice(0, लंबाई));
  // This is a bit tricky.  If we just call readBytes, we are requiring that it
  // return us the entire stream as a contiguous buffer.  There is no guarantee
  // this can be satisfied by returning a reference straight from the buffer, as
  // an implementation may not store all data in a single contiguous buffer.  So
  // we iterate over each contiguous chunk, writing each one in succession.
  जबतक (SrcReader.bytesRemaining() > 0) {
    ArrayRef<uint8_t> Chunk;
    अगर (स्वतः EC = SrcReader.readLongestContiguousChunk(Chunk))
      वापस EC;
    अगर (स्वतः EC = writeBytes(Chunk))
      वापस EC;
  }
  वापस Error::success();
}

std::pair<द्विआधारी_Stream_Writer, द्विआधारी_Stream_Writer>
द्विआधारी_Stream_Writer::split(uint64_t Off) स्थिर {
  निश्चित(get_लंबाई() >= Off);

  Writable_द्विआधारी_Stream_Ref First = Stream.drop_front(Offset);

  Writable_द्विआधारी_Stream_Ref Second = First.drop_front(Off);
  First = First.keep_front(Off);
  द्विआधारी_Stream_Writer W1{First};
  द्विआधारी_Stream_Writer W2{Second};
  वापस {W1, W2};
}

Error द्विआधारी_Stream_Writer::padToAlignment(uint32_t Align) {
  uint64_t NewOffset = alignTo(Offset, Align);
  स्थिर uint64_t Zeros_आकार = 64;
  अटल constexpr अक्षर Zeros[Zeros_आकार] = {};
  जबतक (Offset < NewOffset)
    अगर (स्वतः E = writeArray(
            ArrayRef<अक्षर>(Zeros, std::min(Zeros_आकार, NewOffset - Offset))))
      वापस E;
  वापस Error::success();
}
