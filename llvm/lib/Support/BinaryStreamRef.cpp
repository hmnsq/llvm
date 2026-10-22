//===- BinaryStreamRef.cpp - ----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/BinaryStreamRef.h"
#అన్తర్భావ "llvm/Support/BinaryByteStream.h"

ఉపయోగించి నామాకాశ llvm;

నామాకాశ {

తరగతి ArrayRefImpl : బహిరంగ ద్విమాన_ప్రవాహం {
బహిరంగ:
  ArrayRefImpl(ArrayRef<uint8_t> దత్తాంశం_2, endianness Endian) : BBS(దత్తాంశం_2, Endian) {}

  llvm::endianness getEndian() స్థిరరాశి override { తిరిగి BBS.getEndian(); }
  Error read_బైట్లు(uint64_t స్థానభ్రంశం, uint64_t కొలత,
                  ArrayRef<uint8_t> &బఫరు) override {
    తిరిగి BBS.read_బైట్లు(స్థానభ్రంశం, కొలత, బఫరు);
  }
  Error readLongestContiguousChunk(uint64_t స్థానభ్రంశం,
                                   ArrayRef<uint8_t> &బఫరు) override {
    తిరిగి BBS.readLongestContiguousChunk(స్థానభ్రంశం, బఫరు);
  }
  uint64_t get_నిడివి() override { తిరిగి BBS.get_నిడివి(); }

అంతరంగ:
  ద్విమాన_Byte_ప్రవాహం BBS;
};

తరగతి MutableArrayRefImpl : బహిరంగ Writable_ద్విమాన_ప్రవాహం {
బహిరంగ:
  MutableArrayRefImpl(MutableArrayRef<uint8_t> దత్తాంశం_2, endianness Endian)
      : BBS(దత్తాంశం_2, Endian) {}

  // Inherited via WritableBinaryStream
  llvm::endianness getEndian() స్థిరరాశి override { తిరిగి BBS.getEndian(); }
  Error read_బైట్లు(uint64_t స్థానభ్రంశం, uint64_t కొలత,
                  ArrayRef<uint8_t> &బఫరు) override {
    తిరిగి BBS.read_బైట్లు(స్థానభ్రంశం, కొలత, బఫరు);
  }
  Error readLongestContiguousChunk(uint64_t స్థానభ్రంశం,
                                   ArrayRef<uint8_t> &బఫరు) override {
    తిరిగి BBS.readLongestContiguousChunk(స్థానభ్రంశం, బఫరు);
  }
  uint64_t get_నిడివి() override { తిరిగి BBS.get_నిడివి(); }

  Error write_బైట్లు(uint64_t స్థానభ్రంశం, ArrayRef<uint8_t> దత్తాంశం_2) override {
    తిరిగి BBS.write_బైట్లు(స్థానభ్రంశం, దత్తాంశం_2);
  }
  Error commit() override { తిరిగి BBS.commit(); }

అంతరంగ:
  Mutable_ద్విమాన_Byte_ప్రవాహం BBS;
};
} // namespace

ద్విమాన_ప్రవాహం_Ref::ద్విమాన_ప్రవాహం_Ref(ద్విమాన_ప్రవాహం &ప్రవాహం)
    : ద్విమాన_ప్రవాహం_Ref_పునాది(ప్రవాహం) {}
ద్విమాన_ప్రవాహం_Ref::ద్విమాన_ప్రవాహం_Ref(ద్విమాన_ప్రవాహం &ప్రవాహం, uint64_t స్థానభ్రంశం,
                                 std::optional<uint64_t> నిడివి)
    : ద్విమాన_ప్రవాహం_Ref_పునాది(ప్రవాహం, స్థానభ్రంశం, నిడివి) {}
ద్విమాన_ప్రవాహం_Ref::ద్విమాన_ప్రవాహం_Ref(ArrayRef<uint8_t> దత్తాంశం_2, endianness Endian)
    : ద్విమాన_ప్రవాహం_Ref_పునాది(std::make_shared<ArrayRefImpl>(దత్తాంశం_2, Endian), 0,
                          దత్తాంశం_2.కొలత_2()) {}
ద్విమాన_ప్రవాహం_Ref::ద్విమాన_ప్రవాహం_Ref(StringRef దత్తాంశం_2, endianness Endian)
    : ద్విమాన_ప్రవాహం_Ref(ArrayRef(దత్తాంశం_2.బైట్లు_begin(), దత్తాంశం_2.బైట్లు_చివర()), Endian) {}

Error ద్విమాన_ప్రవాహం_Ref::read_బైట్లు(uint64_t స్థానభ్రంశం, uint64_t కొలత,
                                 ArrayRef<uint8_t> &బఫరు) స్థిరరాశి {
  ఒకవేళ (స్వయంచాలక EC = check_స్థానభ్రంశం_For_Read(స్థానభ్రంశం, కొలత))
    తిరిగి EC;
  తిరిగి BorrowedImpl->read_బైట్లు(View_స్థానభ్రంశం + స్థానభ్రంశం, కొలత, బఫరు);
}

Error ద్విమాన_ప్రవాహం_Ref::readLongestContiguousChunk(
    uint64_t స్థానభ్రంశం, ArrayRef<uint8_t> &బఫరు) స్థిరరాశి {
  ఒకవేళ (స్వయంచాలక EC = check_స్థానభ్రంశం_For_Read(స్థానభ్రంశం, 1))
    తిరిగి EC;

  ఒకవేళ (స్వయంచాలక EC =
          BorrowedImpl->readLongestContiguousChunk(View_స్థానభ్రంశం + స్థానభ్రంశం, బఫరు))
    తిరిగి EC;
  // This StreamRef might refer to a smaller window over a larger stream.  In
  // that case we will have read out more bytes than we should return, because
  // we should not read past the end of the current view.
  uint64_t Max_నిడివి = get_నిడివి() - స్థానభ్రంశం;
  ఒకవేళ (బఫరు.కొలత_2() > Max_నిడివి)
    బఫరు = బఫరు.ముక్క(0, Max_నిడివి);
  తిరిగి Error::success();
}

Writable_ద్విమాన_ప్రవాహం_Ref::Writable_ద్విమాన_ప్రవాహం_Ref(Writable_ద్విమాన_ప్రవాహం &ప్రవాహం)
    : ద్విమాన_ప్రవాహం_Ref_పునాది(ప్రవాహం) {}

Writable_ద్విమాన_ప్రవాహం_Ref::Writable_ద్విమాన_ప్రవాహం_Ref(Writable_ద్విమాన_ప్రవాహం &ప్రవాహం,
                                                 uint64_t స్థానభ్రంశం,
                                                 std::optional<uint64_t> నిడివి)
    : ద్విమాన_ప్రవాహం_Ref_పునాది(ప్రవాహం, స్థానభ్రంశం, నిడివి) {}

Writable_ద్విమాన_ప్రవాహం_Ref::Writable_ద్విమాన_ప్రవాహం_Ref(MutableArrayRef<uint8_t> దత్తాంశం_2,
                                                 endianness Endian)
    : ద్విమాన_ప్రవాహం_Ref_పునాది(std::make_shared<MutableArrayRefImpl>(దత్తాంశం_2, Endian),
                          0, దత్తాంశం_2.కొలత_2()) {}

Error Writable_ద్విమాన_ప్రవాహం_Ref::write_బైట్లు(uint64_t స్థానభ్రంశం,
                                          ArrayRef<uint8_t> దత్తాంశం_2) స్థిరరాశి {
  ఒకవేళ (స్వయంచాలక EC = check_స్థానభ్రంశం_For_Write(స్థానభ్రంశం, దత్తాంశం_2.కొలత_2()))
    తిరిగి EC;

  తిరిగి BorrowedImpl->write_బైట్లు(View_స్థానభ్రంశం + స్థానభ్రంశం, దత్తాంశం_2);
}

Writable_ద్విమాన_ప్రవాహం_Ref::క్రియాకారకం ద్విమాన_ప్రవాహం_Ref() స్థిరరాశి {
  తిరిగి ద్విమాన_ప్రవాహం_Ref(*BorrowedImpl, View_స్థానభ్రంశం, నిడివి);
}

/// For buffered streams, commits changes to the backing store.
Error Writable_ద్విమాన_ప్రవాహం_Ref::commit() { తిరిగి BorrowedImpl->commit(); }
