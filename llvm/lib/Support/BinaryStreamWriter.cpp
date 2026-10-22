//===- BinaryStreamWriter.cpp - Writes objects to a BinaryStream ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/Support/BinaryStreamWriter.h"

#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/BinaryStreamReader.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/BinaryStreamRef.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/LEB128.h"

using ନାମାକାଶ llvm;

ଦ୍ୱିମିକ_Stream_Writer::ଦ୍ୱିମିକ_Stream_Writer(Writable_ଦ୍ୱିମିକ_Stream_Ref Ref)
    : Stream(Ref) {}

ଦ୍ୱିମିକ_Stream_Writer::ଦ୍ୱିମିକ_Stream_Writer(Writable_ଦ୍ୱିମିକ_Stream &Stream)
    : Stream(Stream) {}

ଦ୍ୱିମିକ_Stream_Writer::ଦ୍ୱିମିକ_Stream_Writer(MutableArrayRef<uint8_t> Data,
                                       llvm::endianness Endian)
    : Stream(Data, Endian) {}

Error ଦ୍ୱିମିକ_Stream_Writer::writeBytes(ArrayRef<uint8_t> Buffer) {
  ଯଦି (auto EC = Stream.writeBytes(Offset, Buffer))
    ଫେରନ୍ତୁ EC;
  Offset += Buffer.size();
  ଫେରନ୍ତୁ Error::success();
}

Error ଦ୍ୱିମିକ_Stream_Writer::writeULEB128(uint64_t ମୂଲ୍ୟ) {
  uint8_t EncodedBytes[10] = {0};
  unsigned Size = encodeULEB128(ମୂଲ୍ୟ, &EncodedBytes[0]);
  ଫେରନ୍ତୁ writeBytes({EncodedBytes, Size});
}

Error ଦ୍ୱିମିକ_Stream_Writer::writeSLEB128(int64_t ମୂଲ୍ୟ) {
  uint8_t EncodedBytes[10] = {0};
  unsigned Size = encodeSLEB128(ମୂଲ୍ୟ, &EncodedBytes[0]);
  ଫେରନ୍ତୁ writeBytes({EncodedBytes, Size});
}

Error ଦ୍ୱିମିକ_Stream_Writer::writeCString(StringRef Str) {
  ଯଦି (auto EC = writeFixedString(Str))
    ଫେରନ୍ତୁ EC;
  ଯଦି (auto EC = writeObject('\0'))
    ଫେରନ୍ତୁ EC;

  ଫେରନ୍ତୁ Error::success();
}

Error ଦ୍ୱିମିକ_Stream_Writer::writeFixedString(StringRef Str) {

  ଫେରନ୍ତୁ writeBytes(arrayRefFromStringRef(Str));
}

Error ଦ୍ୱିମିକ_Stream_Writer::writeStreamRef(ଦ୍ୱିମିକ_Stream_Ref Ref) {
  ଫେରନ୍ତୁ writeStreamRef(Ref, Ref.getLength());
}

Error ଦ୍ୱିମିକ_Stream_Writer::writeStreamRef(ଦ୍ୱିମିକ_Stream_Ref Ref, uint64_t Length) {
  ଦ୍ୱିମିକ_Stream_Reader SrcReader(Ref.slice(0, Length));
  // This is a bit tricky.  If we just call readBytes, we are requiring that it
  // return us the entire stream as a contiguous buffer.  There is no guarantee
  // this can be satisfied by returning a reference straight from the buffer, as
  // an implementation may not store all data in a single contiguous buffer.  So
  // we iterate over each contiguous chunk, writing each one in succession.
  ଯେତେବେଳେ (SrcReader.bytesRemaining() > 0) {
    ArrayRef<uint8_t> Chunk;
    ଯଦି (auto EC = SrcReader.readLongestContiguousChunk(Chunk))
      ଫେରନ୍ତୁ EC;
    ଯଦି (auto EC = writeBytes(Chunk))
      ଫେରନ୍ତୁ EC;
  }
  ଫେରନ୍ତୁ Error::success();
}

std::pair<ଦ୍ୱିମିକ_Stream_Writer, ଦ୍ୱିମିକ_Stream_Writer>
ଦ୍ୱିମିକ_Stream_Writer::split(uint64_t Off) ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(getLength() >= Off);

  Writable_ଦ୍ୱିମିକ_Stream_Ref First = Stream.drop_front(Offset);

  Writable_ଦ୍ୱିମିକ_Stream_Ref Second = First.drop_front(Off);
  First = First.keep_front(Off);
  ଦ୍ୱିମିକ_Stream_Writer W1{First};
  ଦ୍ୱିମିକ_Stream_Writer W2{Second};
  ଫେରନ୍ତୁ {W1, W2};
}

Error ଦ୍ୱିମିକ_Stream_Writer::padToAlignment(uint32_t Align) {
  uint64_t NewOffset = alignTo(Offset, Align);
  ଅଚଳ uint64_t ZerosSize = 64;
  ସ୍ଥିର constexpr ଅକ୍ଷର Zeros[ZerosSize] = {};
  ଯେତେବେଳେ (Offset < NewOffset)
    ଯଦି (auto E = writeArray(
            ArrayRef<ଅକ୍ଷର>(Zeros, std::min(ZerosSize, NewOffset - Offset))))
      ଫେରନ୍ତୁ E;
  ଫେରନ୍ତୁ Error::success();
}
