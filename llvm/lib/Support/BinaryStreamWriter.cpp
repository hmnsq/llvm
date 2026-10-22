//===- BinaryStreamWriter.cpp - Writes objects to a BinaryStream ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/Support/BinaryStreamWriter.h"

#অন্তর্ভাব "llvm/ADT/StringExtras.h"
#অন্তর্ভাব "llvm/Support/BinaryStreamReader.h"
#অন্তর্ভাব "llvm/Support/BinaryStreamRef.h"
#অন্তর্ভাব "llvm/Support/LEB128.h"

using নামাকাশ llvm;

দ্বিমিক_Stream_Writer::দ্বিমিক_Stream_Writer(Writable_দ্বিমিক_Stream_Ref Ref)
    : Stream(Ref) {}

দ্বিমিক_Stream_Writer::দ্বিমিক_Stream_Writer(Writable_দ্বিমিক_Stream &Stream)
    : Stream(Stream) {}

দ্বিমিক_Stream_Writer::দ্বিমিক_Stream_Writer(MutableArrayRef<uint8_t> তথ্য_2,
                                       llvm::endianness Endian)
    : Stream(তথ্য_2, Endian) {}

Error দ্বিমিক_Stream_Writer::writeBytes(ArrayRef<uint8_t> বাফার) {
  যদি (auto EC = Stream.writeBytes(Offset, বাফার))
    ফেরত EC;
  Offset += বাফার.আকার_3();
  ফেরত Error::success();
}

Error দ্বিমিক_Stream_Writer::writeULEB128(uint64_t মান) {
  uint8_t EncodedBytes[10] = {0};
  unsigned আকার_2 = encodeULEB128(মান, &EncodedBytes[0]);
  ফেরত writeBytes({EncodedBytes, আকার_2});
}

Error দ্বিমিক_Stream_Writer::writeSLEB128(int64_t মান) {
  uint8_t EncodedBytes[10] = {0};
  unsigned আকার_2 = encodeSLEB128(মান, &EncodedBytes[0]);
  ফেরত writeBytes({EncodedBytes, আকার_2});
}

Error দ্বিমিক_Stream_Writer::writeCString(StringRef Str) {
  যদি (auto EC = writeFixedString(Str))
    ফেরত EC;
  যদি (auto EC = writeObject('\0'))
    ফেরত EC;

  ফেরত Error::success();
}

Error দ্বিমিক_Stream_Writer::writeFixedString(StringRef Str) {

  ফেরত writeBytes(arrayRefFromStringRef(Str));
}

Error দ্বিমিক_Stream_Writer::writeStreamRef(দ্বিমিক_Stream_Ref Ref) {
  ফেরত writeStreamRef(Ref, Ref.get_দৈর্ঘ্য());
}

Error দ্বিমিক_Stream_Writer::writeStreamRef(দ্বিমিক_Stream_Ref Ref, uint64_t দৈর্ঘ্য) {
  দ্বিমিক_Stream_Reader SrcReader(Ref.slice(0, দৈর্ঘ্য));
  // This is a bit tricky.  If we just call readBytes, we are requiring that it
  // return us the entire stream as a contiguous buffer.  There is no guarantee
  // this can be satisfied by returning a reference straight from the buffer, as
  // an implementation may not store all data in a single contiguous buffer.  So
  // we iterate over each contiguous chunk, writing each one in succession.
  যখন (SrcReader.bytesRemaining() > 0) {
    ArrayRef<uint8_t> Chunk;
    যদি (auto EC = SrcReader.readLongestContiguousChunk(Chunk))
      ফেরত EC;
    যদি (auto EC = writeBytes(Chunk))
      ফেরত EC;
  }
  ফেরত Error::success();
}

std::pair<দ্বিমিক_Stream_Writer, দ্বিমিক_Stream_Writer>
দ্বিমিক_Stream_Writer::split(uint64_t Off) ধ্রুবক {
  প্রতিজ্ঞা(get_দৈর্ঘ্য() >= Off);

  Writable_দ্বিমিক_Stream_Ref First = Stream.drop_front(Offset);

  Writable_দ্বিমিক_Stream_Ref Second = First.drop_front(Off);
  First = First.keep_front(Off);
  দ্বিমিক_Stream_Writer W1{First};
  দ্বিমিক_Stream_Writer W2{Second};
  ফেরত {W1, W2};
}

Error দ্বিমিক_Stream_Writer::padToAlignment(uint32_t Align) {
  uint64_t NewOffset = alignTo(Offset, Align);
  ধ্রুবক uint64_t Zeros_আকার = 64;
  স্থির constexpr অক্ষর Zeros[Zeros_আকার] = {};
  যখন (Offset < NewOffset)
    যদি (auto E = writeArray(
            ArrayRef<অক্ষর>(Zeros, std::min(Zeros_আকার, NewOffset - Offset))))
      ফেরত E;
  ফেরত Error::success();
}
