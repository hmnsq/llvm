//===- BinaryStreamWriter.cpp - Writes objects to a BinaryStream ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/BinaryStreamWriter.h"

#అన్తర్భావ "llvm/ADT/StringExtras.h"
#అన్తర్భావ "llvm/Support/BinaryStreamReader.h"
#అన్తర్భావ "llvm/Support/BinaryStreamRef.h"
#అన్తర్భావ "llvm/Support/LEB128.h"

ఉపయోగించి నామాకాశ llvm;

ద్విమాన_ప్రవాహం_Writer::ద్విమాన_ప్రవాహం_Writer(Writable_ద్విమాన_ప్రవాహం_Ref Ref)
    : ప్రవాహం(Ref) {}

ద్విమాన_ప్రవాహం_Writer::ద్విమాన_ప్రవాహం_Writer(Writable_ద్విమాన_ప్రవాహం &ప్రవాహం)
    : ప్రవాహం(ప్రవాహం) {}

ద్విమాన_ప్రవాహం_Writer::ద్విమాన_ప్రవాహం_Writer(MutableArrayRef<uint8_t> దత్తాంశం_2,
                                       llvm::endianness Endian)
    : ప్రవాహం(దత్తాంశం_2, Endian) {}

Error ద్విమాన_ప్రవాహం_Writer::write_బైట్లు(ArrayRef<uint8_t> బఫరు) {
  ఒకవేళ (స్వయంచాలక EC = ప్రవాహం.write_బైట్లు(స్థానభ్రంశం, బఫరు))
    తిరిగి EC;
  స్థానభ్రంశం += బఫరు.కొలత_2();
  తిరిగి Error::success();
}

Error ద్విమాన_ప్రవాహం_Writer::writeULEB128(uint64_t విలువ) {
  uint8_t Encoded_బైట్లు[10] = {0};
  గుర్తులేని కొలత = encodeULEB128(విలువ, &Encoded_బైట్లు[0]);
  తిరిగి write_బైట్లు({Encoded_బైట్లు, కొలత});
}

Error ద్విమాన_ప్రవాహం_Writer::writeSLEB128(int64_t విలువ) {
  uint8_t Encoded_బైట్లు[10] = {0};
  గుర్తులేని కొలత = encodeSLEB128(విలువ, &Encoded_బైట్లు[0]);
  తిరిగి write_బైట్లు({Encoded_బైట్లు, కొలత});
}

Error ద్విమాన_ప్రవాహం_Writer::writeCString(StringRef Str) {
  ఒకవేళ (స్వయంచాలక EC = writeFixedString(Str))
    తిరిగి EC;
  ఒకవేళ (స్వయంచాలక EC = writeObject('\0'))
    తిరిగి EC;

  తిరిగి Error::success();
}

Error ద్విమాన_ప్రవాహం_Writer::writeFixedString(StringRef Str) {

  తిరిగి write_బైట్లు(arrayRefFromStringRef(Str));
}

Error ద్విమాన_ప్రవాహం_Writer::write_ప్రవాహం_Ref(ద్విమాన_ప్రవాహం_Ref Ref) {
  తిరిగి write_ప్రవాహం_Ref(Ref, Ref.get_నిడివి());
}

Error ద్విమాన_ప్రవాహం_Writer::write_ప్రవాహం_Ref(ద్విమాన_ప్రవాహం_Ref Ref, uint64_t నిడివి) {
  ద్విమాన_ప్రవాహం_Reader SrcReader(Ref.ముక్క(0, నిడివి));
  // This is a bit tricky.  If we just call readBytes, we are requiring that it
  // return us the entire stream as a contiguous buffer.  There is no guarantee
  // this can be satisfied by returning a reference straight from the buffer, as
  // an implementation may not store all data in a single contiguous buffer.  So
  // we iterate over each contiguous chunk, writing each one in succession.
  అయితే (SrcReader.బైట్లు_మిగిలిన() > 0) {
    ArrayRef<uint8_t> Chunk;
    ఒకవేళ (స్వయంచాలక EC = SrcReader.readLongestContiguousChunk(Chunk))
      తిరిగి EC;
    ఒకవేళ (స్వయంచాలక EC = write_బైట్లు(Chunk))
      తిరిగి EC;
  }
  తిరిగి Error::success();
}

std::pair<ద్విమాన_ప్రవాహం_Writer, ద్విమాన_ప్రవాహం_Writer>
ద్విమాన_ప్రవాహం_Writer::విభజించు(uint64_t Off) స్థిరరాశి {
  ప్రతిజ్ఞా(get_నిడివి() >= Off);

  Writable_ద్విమాన_ప్రవాహం_Ref మొదటి = ప్రవాహం.drop_front(స్థానభ్రంశం);

  Writable_ద్విమాన_ప్రవాహం_Ref రెండవ = మొదటి.drop_front(Off);
  మొదటి = మొదటి.keep_front(Off);
  ద్విమాన_ప్రవాహం_Writer W1{మొదటి};
  ద్విమాన_ప్రవాహం_Writer W2{రెండవ};
  తిరిగి {W1, W2};
}

Error ద్విమాన_ప్రవాహం_Writer::pad_గమ్యం_Alignment(uint32_t Align) {
  uint64_t New_స్థానభ్రంశం = align_గమ్యం(స్థానభ్రంశం, Align);
  స్థిరరాశి uint64_t Zeros_కొలత = 64;
  స్థిర constexpr అక్షరం Zeros[Zeros_కొలత] = {};
  అయితే (స్థానభ్రంశం < New_స్థానభ్రంశం)
    ఒకవేళ (స్వయంచాలక E = writeArray(
            ArrayRef<అక్షరం>(Zeros, std::min(Zeros_కొలత, New_స్థానభ్రంశం - స్థానభ్రంశం))))
      తిరిగి E;
  తిరిగి Error::success();
}
