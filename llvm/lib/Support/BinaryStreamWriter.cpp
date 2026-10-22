//===- BinaryStreamWriter.cpp - Writes objects to a BinaryStream ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/Support/BinaryStreamWriter.h"

#அந்தர்பாவ "llvm/ADT/StringExtras.h"
#அந்தர்பாவ "llvm/Support/BinaryStreamReader.h"
#அந்தர்பாவ "llvm/Support/BinaryStreamRef.h"
#அந்தர்பாவ "llvm/Support/LEB128.h"

using நாமாகாஶ llvm;

இருமம்_Stream_Writer::இருமம்_Stream_Writer(Writable_இருமம்_Stream_Ref Ref)
    : Stream(Ref) {}

இருமம்_Stream_Writer::இருமம்_Stream_Writer(Writable_இருமம்_Stream &Stream)
    : Stream(Stream) {}

இருமம்_Stream_Writer::இருமம்_Stream_Writer(MutableArrayRef<uint8_t> Data,
                                       llvm::endianness Endian)
    : Stream(Data, Endian) {}

Error இருமம்_Stream_Writer::writeBytes(ArrayRef<uint8_t> Buffer) {
  எனில் (auto EC = Stream.writeBytes(Offset, Buffer))
    திரும்பு EC;
  Offset += Buffer.size();
  திரும்பு Error::success();
}

Error இருமம்_Stream_Writer::writeULEB128(uint64_t மதிப்பு) {
  uint8_t EncodedBytes[10] = {0};
  unsigned Size = encodeULEB128(மதிப்பு, &EncodedBytes[0]);
  திரும்பு writeBytes({EncodedBytes, Size});
}

Error இருமம்_Stream_Writer::writeSLEB128(int64_t மதிப்பு) {
  uint8_t EncodedBytes[10] = {0};
  unsigned Size = encodeSLEB128(மதிப்பு, &EncodedBytes[0]);
  திரும்பு writeBytes({EncodedBytes, Size});
}

Error இருமம்_Stream_Writer::writeCString(StringRef Str) {
  எனில் (auto EC = writeFixedString(Str))
    திரும்பு EC;
  எனில் (auto EC = writeObject('\0'))
    திரும்பு EC;

  திரும்பு Error::success();
}

Error இருமம்_Stream_Writer::writeFixedString(StringRef Str) {

  திரும்பு writeBytes(arrayRefFromStringRef(Str));
}

Error இருமம்_Stream_Writer::writeStreamRef(இருமம்_Stream_Ref Ref) {
  திரும்பு writeStreamRef(Ref, Ref.getLength());
}

Error இருமம்_Stream_Writer::writeStreamRef(இருமம்_Stream_Ref Ref, uint64_t Length) {
  இருமம்_Stream_Reader SrcReader(Ref.slice(0, Length));
  // This is a bit tricky.  If we just call readBytes, we are requiring that it
  // return us the entire stream as a contiguous buffer.  There is no guarantee
  // this can be satisfied by returning a reference straight from the buffer, as
  // an implementation may not store all data in a single contiguous buffer.  So
  // we iterate over each contiguous chunk, writing each one in succession.
  போது (SrcReader.bytesRemaining() > 0) {
    ArrayRef<uint8_t> Chunk;
    எனில் (auto EC = SrcReader.readLongestContiguousChunk(Chunk))
      திரும்பு EC;
    எனில் (auto EC = writeBytes(Chunk))
      திரும்பு EC;
  }
  திரும்பு Error::success();
}

std::pair<இருமம்_Stream_Writer, இருமம்_Stream_Writer>
இருமம்_Stream_Writer::split(uint64_t Off) மாறிலி {
  ப்ரதிஜ்ஞா(getLength() >= Off);

  Writable_இருமம்_Stream_Ref First = Stream.drop_front(Offset);

  Writable_இருமம்_Stream_Ref Second = First.drop_front(Off);
  First = First.keep_front(Off);
  இருமம்_Stream_Writer W1{First};
  இருமம்_Stream_Writer W2{Second};
  திரும்பு {W1, W2};
}

Error இருமம்_Stream_Writer::padToAlignment(uint32_t Align) {
  uint64_t NewOffset = alignTo(Offset, Align);
  மாறிலி uint64_t ZerosSize = 64;
  நிலையான constexpr எழுத்து Zeros[ZerosSize] = {};
  போது (Offset < NewOffset)
    எனில் (auto E = writeArray(
            ArrayRef<எழுத்து>(Zeros, std::min(ZerosSize, NewOffset - Offset))))
      திரும்பு E;
  திரும்பு Error::success();
}
