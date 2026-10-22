//===- BinaryStreamWriter.cpp - Writes objects to a BinaryStream ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/BinaryStreamWriter.h"

#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/Support/BinaryStreamReader.h"
#अन्तर्भाव "llvm/Support/BinaryStreamRef.h"
#अन्तर्भाव "llvm/Support/LEB128.h"

उपयुज्य नामाकाश llvm;

द्विमान_प्रवाहः_Writer::द्विमान_प्रवाहः_Writer(Writable_द्विमान_प्रवाहः_Ref Ref)
    : प्रवाहः(Ref) {}

द्विमान_प्रवाहः_Writer::द्विमान_प्रवाहः_Writer(Writable_द्विमान_प्रवाहः &प्रवाहः)
    : प्रवाहः(प्रवाहः) {}

द्विमान_प्रवाहः_Writer::द्विमान_प्रवाहः_Writer(MutableArrayRef<uint8_t> दत्त_2,
                                       llvm::endianness Endian)
    : प्रवाहः(दत्त_2, Endian) {}

Error द्विमान_प्रवाहः_Writer::write_अष्टकानि(ArrayRef<uint8_t> बफर) {
  यदि (स्वचालित EC = प्रवाहः.write_अष्टकानि(विस्थापनम्, बफर))
    प्रत्यागम EC;
  विस्थापनम् += बफर.आकार_2();
  प्रत्यागम Error::success();
}

Error द्विमान_प्रवाहः_Writer::writeULEB128(uint64_t मानम्) {
  uint8_t Encoded_अष्टकानि[10] = {0};
  अचिह्नित आकार = encodeULEB128(मानम्, &Encoded_अष्टकानि[0]);
  प्रत्यागम write_अष्टकानि({Encoded_अष्टकानि, आकार});
}

Error द्विमान_प्रवाहः_Writer::writeSLEB128(int64_t मानम्) {
  uint8_t Encoded_अष्टकानि[10] = {0};
  अचिह्नित आकार = encodeSLEB128(मानम्, &Encoded_अष्टकानि[0]);
  प्रत्यागम write_अष्टकानि({Encoded_अष्टकानि, आकार});
}

Error द्विमान_प्रवाहः_Writer::writeCString(StringRef Str) {
  यदि (स्वचालित EC = writeFixedString(Str))
    प्रत्यागम EC;
  यदि (स्वचालित EC = writeObject('\0'))
    प्रत्यागम EC;

  प्रत्यागम Error::success();
}

Error द्विमान_प्रवाहः_Writer::writeFixedString(StringRef Str) {

  प्रत्यागम write_अष्टकानि(arrayRefFromStringRef(Str));
}

Error द्विमान_प्रवाहः_Writer::write_प्रवाहः_Ref(द्विमान_प्रवाहः_Ref Ref) {
  प्रत्यागम write_प्रवाहः_Ref(Ref, Ref.get_दीर्घता());
}

Error द्विमान_प्रवाहः_Writer::write_प्रवाहः_Ref(द्विमान_प्रवाहः_Ref Ref, uint64_t दीर्घता) {
  द्विमान_प्रवाहः_Reader SrcReader(Ref.खण्डिका(0, दीर्घता));
  // This is a bit tricky.  If we just call readBytes, we are requiring that it
  // return us the entire stream as a contiguous buffer.  There is no guarantee
  // this can be satisfied by returning a reference straight from the buffer, as
  // an implementation may not store all data in a single contiguous buffer.  So
  // we iterate over each contiguous chunk, writing each one in succession.
  यावत् (SrcReader.अष्टकानि_Remaining() > 0) {
    ArrayRef<uint8_t> Chunk;
    यदि (स्वचालित EC = SrcReader.readLongestContiguousChunk(Chunk))
      प्रत्यागम EC;
    यदि (स्वचालित EC = write_अष्टकानि(Chunk))
      प्रत्यागम EC;
  }
  प्रत्यागम Error::success();
}

std::pair<द्विमान_प्रवाहः_Writer, द्विमान_प्रवाहः_Writer>
द्विमान_प्रवाहः_Writer::विभज(uint64_t Off) ध्रुव {
  प्रतिज्ञा(get_दीर्घता() >= Off);

  Writable_द्विमान_प्रवाहः_Ref First = प्रवाहः.drop_front(विस्थापनम्);

  Writable_द्विमान_प्रवाहः_Ref Second = First.drop_front(Off);
  First = First.keep_front(Off);
  द्विमान_प्रवाहः_Writer W1{First};
  द्विमान_प्रवाहः_Writer W2{Second};
  प्रत्यागम {W1, W2};
}

Error द्विमान_प्रवाहः_Writer::pad_गन्तव्यम्_Alignment(uint32_t Align) {
  uint64_t New_विस्थापनम् = align_गन्तव्यम्(विस्थापनम्, Align);
  ध्रुव uint64_t Zeros_आकार = 64;
  स्थिर constexpr वर्ण Zeros[Zeros_आकार] = {};
  यावत् (विस्थापनम् < New_विस्थापनम्)
    यदि (स्वचालित E = writeArray(
            ArrayRef<वर्ण>(Zeros, std::min(Zeros_आकार, New_विस्थापनम् - विस्थापनम्))))
      प्रत्यागम E;
  प्रत्यागम Error::success();
}
