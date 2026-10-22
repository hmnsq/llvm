//===- BinaryStreamWriter.cpp - Writes objects to a BinaryStream ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/BinaryStreamWriter.h"

#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/Support/BinaryStreamReader.h"
#ضمن "llvm/Support/BinaryStreamRef.h"
#ضمن "llvm/Support/LEB128.h"

using نطاق llvm;

ئىككىلىك_Stream_Writer::ئىككىلىك_Stream_Writer(Writable_ئىككىلىك_Stream_Ref Ref)
    : Stream(Ref) {}

ئىككىلىك_Stream_Writer::ئىككىلىك_Stream_Writer(Writable_ئىككىلىك_Stream &Stream)
    : Stream(Stream) {}

ئىككىلىك_Stream_Writer::ئىككىلىك_Stream_Writer(MutableArrayRef<uint8_t> Data,
                                       llvm::endianness Endian)
    : Stream(Data, Endian) {}

Error ئىككىلىك_Stream_Writer::writeBytes(ArrayRef<uint8_t> Buffer) {
  إذا (auto EC = Stream.writeBytes(Offset, Buffer))
    أرجع EC;
  Offset += Buffer.size();
  أرجع Error::success();
}

Error ئىككىلىك_Stream_Writer::writeULEB128(uint64_t قىممەت) {
  uint8_t EncodedBytes[10] = {0};
  unsigned Size = encodeULEB128(قىممەت, &EncodedBytes[0]);
  أرجع writeBytes({EncodedBytes, Size});
}

Error ئىككىلىك_Stream_Writer::writeSLEB128(int64_t قىممەت) {
  uint8_t EncodedBytes[10] = {0};
  unsigned Size = encodeSLEB128(قىممەت, &EncodedBytes[0]);
  أرجع writeBytes({EncodedBytes, Size});
}

Error ئىككىلىك_Stream_Writer::writeCString(StringRef Str) {
  إذا (auto EC = writeFixedString(Str))
    أرجع EC;
  إذا (auto EC = writeObject('\0'))
    أرجع EC;

  أرجع Error::success();
}

Error ئىككىلىك_Stream_Writer::writeFixedString(StringRef Str) {

  أرجع writeBytes(arrayRefFromStringRef(Str));
}

Error ئىككىلىك_Stream_Writer::writeStreamRef(ئىككىلىك_Stream_Ref Ref) {
  أرجع writeStreamRef(Ref, Ref.getLength());
}

Error ئىككىلىك_Stream_Writer::writeStreamRef(ئىككىلىك_Stream_Ref Ref, uint64_t Length) {
  ئىككىلىك_Stream_Reader SrcReader(Ref.slice(0, Length));
  // This is a bit tricky.  If we just call readBytes, we are requiring that it
  // return us the entire stream as a contiguous buffer.  There is no guarantee
  // this can be satisfied by returning a reference straight from the buffer, as
  // an implementation may not store all data in a single contiguous buffer.  So
  // we iterate over each contiguous chunk, writing each one in succession.
  طالما (SrcReader.bytesRemaining() > 0) {
    ArrayRef<uint8_t> Chunk;
    إذا (auto EC = SrcReader.readLongestContiguousChunk(Chunk))
      أرجع EC;
    إذا (auto EC = writeBytes(Chunk))
      أرجع EC;
  }
  أرجع Error::success();
}

std::pair<ئىككىلىك_Stream_Writer, ئىككىلىك_Stream_Writer>
ئىككىلىك_Stream_Writer::split(uint64_t Off) ثابت {
  أكد(getLength() >= Off);

  Writable_ئىككىلىك_Stream_Ref First = Stream.drop_front(Offset);

  Writable_ئىككىلىك_Stream_Ref Second = First.drop_front(Off);
  First = First.keep_front(Off);
  ئىككىلىك_Stream_Writer W1{First};
  ئىككىلىك_Stream_Writer W2{Second};
  أرجع {W1, W2};
}

Error ئىككىلىك_Stream_Writer::padToAlignment(uint32_t Align) {
  uint64_t NewOffset = alignTo(Offset, Align);
  ثابت uint64_t ZerosSize = 64;
  ساكن constexpr حرف Zeros[ZerosSize] = {};
  طالما (Offset < NewOffset)
    إذا (auto E = writeArray(
            ArrayRef<حرف>(Zeros, std::min(ZerosSize, NewOffset - Offset))))
      أرجع E;
  أرجع Error::success();
}
