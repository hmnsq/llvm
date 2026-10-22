//===- BinaryStreamRef.cpp - ----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/Support/BinaryStreamRef.h"
#অন্তর্ভাব "llvm/Support/BinaryByteStream.h"

using নামাকাশ llvm;

নামাকাশ {

শ্রেণী ArrayRefImpl : সর্বজনীন দ্বিমিক_Stream {
সর্বজনীন:
  ArrayRefImpl(ArrayRef<uint8_t> তথ্য_2, endianness Endian) : BBS(তথ্য_2, Endian) {}

  llvm::endianness getEndian() ধ্রুবক override { ফেরত BBS.getEndian(); }
  Error readBytes(uint64_t Offset, uint64_t আকার_2,
                  ArrayRef<uint8_t> &বাফার) override {
    ফেরত BBS.readBytes(Offset, আকার_2, বাফার);
  }
  Error readLongestContiguousChunk(uint64_t Offset,
                                   ArrayRef<uint8_t> &বাফার) override {
    ফেরত BBS.readLongestContiguousChunk(Offset, বাফার);
  }
  uint64_t get_দৈর্ঘ্য() override { ফেরত BBS.get_দৈর্ঘ্য(); }

ব্যক্তিগত:
  দ্বিমিক_Byte_Stream BBS;
};

শ্রেণী MutableArrayRefImpl : সর্বজনীন Writable_দ্বিমিক_Stream {
সর্বজনীন:
  MutableArrayRefImpl(MutableArrayRef<uint8_t> তথ্য_2, endianness Endian)
      : BBS(তথ্য_2, Endian) {}

  // Inherited via WritableBinaryStream
  llvm::endianness getEndian() ধ্রুবক override { ফেরত BBS.getEndian(); }
  Error readBytes(uint64_t Offset, uint64_t আকার_2,
                  ArrayRef<uint8_t> &বাফার) override {
    ফেরত BBS.readBytes(Offset, আকার_2, বাফার);
  }
  Error readLongestContiguousChunk(uint64_t Offset,
                                   ArrayRef<uint8_t> &বাফার) override {
    ফেরত BBS.readLongestContiguousChunk(Offset, বাফার);
  }
  uint64_t get_দৈর্ঘ্য() override { ফেরত BBS.get_দৈর্ঘ্য(); }

  Error writeBytes(uint64_t Offset, ArrayRef<uint8_t> তথ্য_2) override {
    ফেরত BBS.writeBytes(Offset, তথ্য_2);
  }
  Error commit() override { ফেরত BBS.commit(); }

ব্যক্তিগত:
  Mutable_দ্বিমিক_Byte_Stream BBS;
};
} // namespace

দ্বিমিক_Stream_Ref::দ্বিমিক_Stream_Ref(দ্বিমিক_Stream &Stream)
    : দ্বিমিক_Stream_Ref_Base(Stream) {}
দ্বিমিক_Stream_Ref::দ্বিমিক_Stream_Ref(দ্বিমিক_Stream &Stream, uint64_t Offset,
                                 std::optional<uint64_t> দৈর্ঘ্য)
    : দ্বিমিক_Stream_Ref_Base(Stream, Offset, দৈর্ঘ্য) {}
দ্বিমিক_Stream_Ref::দ্বিমিক_Stream_Ref(ArrayRef<uint8_t> তথ্য_2, endianness Endian)
    : দ্বিমিক_Stream_Ref_Base(std::make_shared<ArrayRefImpl>(তথ্য_2, Endian), 0,
                          তথ্য_2.আকার_3()) {}
দ্বিমিক_Stream_Ref::দ্বিমিক_Stream_Ref(StringRef তথ্য_2, endianness Endian)
    : দ্বিমিক_Stream_Ref(ArrayRef(তথ্য_2.bytes_begin(), তথ্য_2.bytes_end()), Endian) {}

Error দ্বিমিক_Stream_Ref::readBytes(uint64_t Offset, uint64_t আকার_2,
                                 ArrayRef<uint8_t> &বাফার) ধ্রুবক {
  যদি (auto EC = checkOffsetForRead(Offset, আকার_2))
    ফেরত EC;
  ফেরত BorrowedImpl->readBytes(ViewOffset + Offset, আকার_2, বাফার);
}

Error দ্বিমিক_Stream_Ref::readLongestContiguousChunk(
    uint64_t Offset, ArrayRef<uint8_t> &বাফার) ধ্রুবক {
  যদি (auto EC = checkOffsetForRead(Offset, 1))
    ফেরত EC;

  যদি (auto EC =
          BorrowedImpl->readLongestContiguousChunk(ViewOffset + Offset, বাফার))
    ফেরত EC;
  // This StreamRef might refer to a smaller window over a larger stream.  In
  // that case we will have read out more bytes than we should return, because
  // we should not read past the end of the current view.
  uint64_t Max_দৈর্ঘ্য = get_দৈর্ঘ্য() - Offset;
  যদি (বাফার.আকার_3() > Max_দৈর্ঘ্য)
    বাফার = বাফার.slice(0, Max_দৈর্ঘ্য);
  ফেরত Error::success();
}

Writable_দ্বিমিক_Stream_Ref::Writable_দ্বিমিক_Stream_Ref(Writable_দ্বিমিক_Stream &Stream)
    : দ্বিমিক_Stream_Ref_Base(Stream) {}

Writable_দ্বিমিক_Stream_Ref::Writable_দ্বিমিক_Stream_Ref(Writable_দ্বিমিক_Stream &Stream,
                                                 uint64_t Offset,
                                                 std::optional<uint64_t> দৈর্ঘ্য)
    : দ্বিমিক_Stream_Ref_Base(Stream, Offset, দৈর্ঘ্য) {}

Writable_দ্বিমিক_Stream_Ref::Writable_দ্বিমিক_Stream_Ref(MutableArrayRef<uint8_t> তথ্য_2,
                                                 endianness Endian)
    : দ্বিমিক_Stream_Ref_Base(std::make_shared<MutableArrayRefImpl>(তথ্য_2, Endian),
                          0, তথ্য_2.আকার_3()) {}

Error Writable_দ্বিমিক_Stream_Ref::writeBytes(uint64_t Offset,
                                          ArrayRef<uint8_t> তথ্য_2) ধ্রুবক {
  যদি (auto EC = checkOffsetForWrite(Offset, তথ্য_2.আকার_3()))
    ফেরত EC;

  ফেরত BorrowedImpl->writeBytes(ViewOffset + Offset, তথ্য_2);
}

Writable_দ্বিমিক_Stream_Ref::operator দ্বিমিক_Stream_Ref() ধ্রুবক {
  ফেরত দ্বিমিক_Stream_Ref(*BorrowedImpl, ViewOffset, দৈর্ঘ্য);
}

/// For buffered streams, commits changes to the backing store.
Error Writable_দ্বিমিক_Stream_Ref::commit() { ফেরত BorrowedImpl->commit(); }
