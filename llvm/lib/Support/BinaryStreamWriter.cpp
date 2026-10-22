//===- BinaryStreamWriter.cpp - Writes objects to a BinaryStream ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/BinaryStreamWriter.h"

#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/Support/BinaryStreamReader.h"
#شامل "llvm/Support/BinaryStreamRef.h"
#شامل "llvm/Support/LEB128.h"

using فضاینام llvm;

دودویی_Stream_Writer::دودویی_Stream_Writer(Writable_دودویی_Stream_Ref Ref)
    : Stream(Ref) {}

دودویی_Stream_Writer::دودویی_Stream_Writer(Writable_دودویی_Stream &Stream)
    : Stream(Stream) {}

دودویی_Stream_Writer::دودویی_Stream_Writer(MutableArrayRef<uint8_t> Data,
                                       llvm::endianness Endian)
    : Stream(Data, Endian) {}

Error دودویی_Stream_Writer::writeBytes(ArrayRef<uint8_t> Buffer) {
  اگر (auto EC = Stream.writeBytes(Offset, Buffer))
    بازگشت EC;
  Offset += Buffer.size();
  بازگشت Error::success();
}

Error دودویی_Stream_Writer::writeULEB128(uint64_t مقدار) {
  uint8_t EncodedBytes[10] = {0};
  unsigned Size = encodeULEB128(مقدار, &EncodedBytes[0]);
  بازگشت writeBytes({EncodedBytes, Size});
}

Error دودویی_Stream_Writer::writeSLEB128(int64_t مقدار) {
  uint8_t EncodedBytes[10] = {0};
  unsigned Size = encodeSLEB128(مقدار, &EncodedBytes[0]);
  بازگشت writeBytes({EncodedBytes, Size});
}

Error دودویی_Stream_Writer::writeCString(StringRef Str) {
  اگر (auto EC = writeFixedString(Str))
    بازگشت EC;
  اگر (auto EC = writeObject('\0'))
    بازگشت EC;

  بازگشت Error::success();
}

Error دودویی_Stream_Writer::writeFixedString(StringRef Str) {

  بازگشت writeBytes(arrayRefFromStringRef(Str));
}

Error دودویی_Stream_Writer::writeStreamRef(دودویی_Stream_Ref Ref) {
  بازگشت writeStreamRef(Ref, Ref.getLength());
}

Error دودویی_Stream_Writer::writeStreamRef(دودویی_Stream_Ref Ref, uint64_t Length) {
  دودویی_Stream_Reader SrcReader(Ref.slice(0, Length));
  // This is a bit tricky.  If we just call readBytes, we are requiring that it
  // return us the entire stream as a contiguous buffer.  There is no guarantee
  // this can be satisfied by returning a reference straight from the buffer, as
  // an implementation may not store all data in a single contiguous buffer.  So
  // we iterate over each contiguous chunk, writing each one in succession.
  وقتی (SrcReader.bytesRemaining() > 0) {
    ArrayRef<uint8_t> Chunk;
    اگر (auto EC = SrcReader.readLongestContiguousChunk(Chunk))
      بازگشت EC;
    اگر (auto EC = writeBytes(Chunk))
      بازگشت EC;
  }
  بازگشت Error::success();
}

std::pair<دودویی_Stream_Writer, دودویی_Stream_Writer>
دودویی_Stream_Writer::split(uint64_t Off) ثابت {
  أكد(getLength() >= Off);

  Writable_دودویی_Stream_Ref First = Stream.drop_front(Offset);

  Writable_دودویی_Stream_Ref Second = First.drop_front(Off);
  First = First.keep_front(Off);
  دودویی_Stream_Writer W1{First};
  دودویی_Stream_Writer W2{Second};
  بازگشت {W1, W2};
}

Error دودویی_Stream_Writer::padToAlignment(uint32_t Align) {
  uint64_t NewOffset = alignTo(Offset, Align);
  ثابت uint64_t ZerosSize = 64;
  ایستا constexpr نویسه Zeros[ZerosSize] = {};
  وقتی (Offset < NewOffset)
    اگر (auto E = writeArray(
            ArrayRef<نویسه>(Zeros, std::min(ZerosSize, NewOffset - Offset))))
      بازگشت E;
  بازگشت Error::success();
}
