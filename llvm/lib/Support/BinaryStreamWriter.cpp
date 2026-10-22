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

ثنائي_Stream_Writer::ثنائي_Stream_Writer(Writable_ثنائي_Stream_Ref Ref)
    : Stream(Ref) {}

ثنائي_Stream_Writer::ثنائي_Stream_Writer(Writable_ثنائي_Stream &Stream)
    : Stream(Stream) {}

ثنائي_Stream_Writer::ثنائي_Stream_Writer(MutableArrayRef<uint8_t> Data,
                                       llvm::endianness Endian)
    : Stream(Data, Endian) {}

Error ثنائي_Stream_Writer::writeBytes(ArrayRef<uint8_t> Buffer) {
  إذا (auto EC = Stream.writeBytes(Offset, Buffer))
    إرجاع EC;
  Offset += Buffer.size();
  إرجاع Error::success();
}

Error ثنائي_Stream_Writer::writeULEB128(uint64_t قيمة) {
  uint8_t EncodedBytes[10] = {0};
  unsigned Size = encodeULEB128(قيمة, &EncodedBytes[0]);
  إرجاع writeBytes({EncodedBytes, Size});
}

Error ثنائي_Stream_Writer::writeSLEB128(int64_t قيمة) {
  uint8_t EncodedBytes[10] = {0};
  unsigned Size = encodeSLEB128(قيمة, &EncodedBytes[0]);
  إرجاع writeBytes({EncodedBytes, Size});
}

Error ثنائي_Stream_Writer::writeCString(StringRef Str) {
  إذا (auto EC = writeFixedString(Str))
    إرجاع EC;
  إذا (auto EC = writeObject('\0'))
    إرجاع EC;

  إرجاع Error::success();
}

Error ثنائي_Stream_Writer::writeFixedString(StringRef Str) {

  إرجاع writeBytes(arrayRefFromStringRef(Str));
}

Error ثنائي_Stream_Writer::writeStreamRef(ثنائي_Stream_Ref Ref) {
  إرجاع writeStreamRef(Ref, Ref.getLength());
}

Error ثنائي_Stream_Writer::writeStreamRef(ثنائي_Stream_Ref Ref, uint64_t Length) {
  ثنائي_Stream_Reader SrcReader(Ref.slice(0, Length));
  // This is a bit tricky.  If we just call readBytes, we are requiring that it
  // return us the entire stream as a contiguous buffer.  There is no guarantee
  // this can be satisfied by returning a reference straight from the buffer, as
  // an implementation may not store all data in a single contiguous buffer.  So
  // we iterate over each contiguous chunk, writing each one in succession.
  بينما (SrcReader.bytesRemaining() > 0) {
    ArrayRef<uint8_t> Chunk;
    إذا (auto EC = SrcReader.readLongestContiguousChunk(Chunk))
      إرجاع EC;
    إذا (auto EC = writeBytes(Chunk))
      إرجاع EC;
  }
  إرجاع Error::success();
}

std::pair<ثنائي_Stream_Writer, ثنائي_Stream_Writer>
ثنائي_Stream_Writer::split(uint64_t Off) ثابت {
  أكد(getLength() >= Off);

  Writable_ثنائي_Stream_Ref First = Stream.drop_front(Offset);

  Writable_ثنائي_Stream_Ref Second = First.drop_front(Off);
  First = First.keep_front(Off);
  ثنائي_Stream_Writer W1{First};
  ثنائي_Stream_Writer W2{Second};
  إرجاع {W1, W2};
}

Error ثنائي_Stream_Writer::padToAlignment(uint32_t Align) {
  uint64_t NewOffset = alignTo(Offset, Align);
  ثابت uint64_t ZerosSize = 64;
  ساكن constexpr حرف Zeros[ZerosSize] = {};
  بينما (Offset < NewOffset)
    إذا (auto E = writeArray(
            ArrayRef<حرف>(Zeros, std::min(ZerosSize, NewOffset - Offset))))
      إرجاع E;
  إرجاع Error::success();
}
