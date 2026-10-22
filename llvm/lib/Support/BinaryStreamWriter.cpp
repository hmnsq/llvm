//===- BinaryStreamWriter.cpp - Writes objects to a BinaryStream ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/Support/BinaryStreamWriter.h"

#અન્તર્ભાવ "llvm/ADT/StringExtras.h"
#અન્તર્ભાવ "llvm/Support/BinaryStreamReader.h"
#અન્તર્ભાવ "llvm/Support/BinaryStreamRef.h"
#અન્તર્ભાવ "llvm/Support/LEB128.h"

using નામાકાશ llvm;

દ્વિઅંકી_Stream_Writer::દ્વિઅંકી_Stream_Writer(Writable_દ્વિઅંકી_Stream_Ref Ref)
    : Stream(Ref) {}

દ્વિઅંકી_Stream_Writer::દ્વિઅંકી_Stream_Writer(Writable_દ્વિઅંકી_Stream &Stream)
    : Stream(Stream) {}

દ્વિઅંકી_Stream_Writer::દ્વિઅંકી_Stream_Writer(MutableArrayRef<uint8_t> Data,
                                       llvm::endianness Endian)
    : Stream(Data, Endian) {}

Error દ્વિઅંકી_Stream_Writer::writeBytes(ArrayRef<uint8_t> Buffer) {
  જો (auto EC = Stream.writeBytes(Offset, Buffer))
    પાછા EC;
  Offset += Buffer.size();
  પાછા Error::success();
}

Error દ્વિઅંકી_Stream_Writer::writeULEB128(uint64_t કિંમત) {
  uint8_t EncodedBytes[10] = {0};
  unsigned Size = encodeULEB128(કિંમત, &EncodedBytes[0]);
  પાછા writeBytes({EncodedBytes, Size});
}

Error દ્વિઅંકી_Stream_Writer::writeSLEB128(int64_t કિંમત) {
  uint8_t EncodedBytes[10] = {0};
  unsigned Size = encodeSLEB128(કિંમત, &EncodedBytes[0]);
  પાછા writeBytes({EncodedBytes, Size});
}

Error દ્વિઅંકી_Stream_Writer::writeCString(StringRef Str) {
  જો (auto EC = writeFixedString(Str))
    પાછા EC;
  જો (auto EC = writeObject('\0'))
    પાછા EC;

  પાછા Error::success();
}

Error દ્વિઅંકી_Stream_Writer::writeFixedString(StringRef Str) {

  પાછા writeBytes(arrayRefFromStringRef(Str));
}

Error દ્વિઅંકી_Stream_Writer::writeStreamRef(દ્વિઅંકી_Stream_Ref Ref) {
  પાછા writeStreamRef(Ref, Ref.getLength());
}

Error દ્વિઅંકી_Stream_Writer::writeStreamRef(દ્વિઅંકી_Stream_Ref Ref, uint64_t Length) {
  દ્વિઅંકી_Stream_Reader SrcReader(Ref.slice(0, Length));
  // This is a bit tricky.  If we just call readBytes, we are requiring that it
  // return us the entire stream as a contiguous buffer.  There is no guarantee
  // this can be satisfied by returning a reference straight from the buffer, as
  // an implementation may not store all data in a single contiguous buffer.  So
  // we iterate over each contiguous chunk, writing each one in succession.
  જ્યારે (SrcReader.bytesRemaining() > 0) {
    ArrayRef<uint8_t> Chunk;
    જો (auto EC = SrcReader.readLongestContiguousChunk(Chunk))
      પાછા EC;
    જો (auto EC = writeBytes(Chunk))
      પાછા EC;
  }
  પાછા Error::success();
}

std::pair<દ્વિઅંકી_Stream_Writer, દ્વિઅંકી_Stream_Writer>
દ્વિઅંકી_Stream_Writer::split(uint64_t Off) અચલ {
  પ્રતિજ્ઞા(getLength() >= Off);

  Writable_દ્વિઅંકી_Stream_Ref First = Stream.drop_front(Offset);

  Writable_દ્વિઅંકી_Stream_Ref Second = First.drop_front(Off);
  First = First.keep_front(Off);
  દ્વિઅંકી_Stream_Writer W1{First};
  દ્વિઅંકી_Stream_Writer W2{Second};
  પાછા {W1, W2};
}

Error દ્વિઅંકી_Stream_Writer::padToAlignment(uint32_t Align) {
  uint64_t NewOffset = alignTo(Offset, Align);
  અચલ uint64_t ZerosSize = 64;
  સ્થિર constexpr અક્ષર Zeros[ZerosSize] = {};
  જ્યારે (Offset < NewOffset)
    જો (auto E = writeArray(
            ArrayRef<અક્ષર>(Zeros, std::min(ZerosSize, NewOffset - Offset))))
      પાછા E;
  પાછા Error::success();
}
