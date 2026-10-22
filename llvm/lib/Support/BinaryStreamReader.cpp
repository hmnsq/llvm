//===- BinaryStreamReader.cpp - Reads objects from a binary stream --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/BinaryStreamReader.h"

#अन्तर्भाव "llvm/Support/BinaryStreamError.h"
#अन्तर्भाव "llvm/Support/BinaryStreamRef.h"
#अन्तर्भाव "llvm/Support/LEB128.h"

उपयुज्य नामाकाश llvm;

द्विमान_प्रवाहः_Reader::द्विमान_प्रवाहः_Reader(द्विमान_प्रवाहः_Ref Ref) : प्रवाहः(Ref) {}

द्विमान_प्रवाहः_Reader::द्विमान_प्रवाहः_Reader(द्विमान_प्रवाहः &प्रवाहः) : प्रवाहः(प्रवाहः) {}

द्विमान_प्रवाहः_Reader::द्विमान_प्रवाहः_Reader(ArrayRef<uint8_t> दत्त_2,
                                       endianness Endian)
    : प्रवाहः(दत्त_2, Endian) {}

द्विमान_प्रवाहः_Reader::द्विमान_प्रवाहः_Reader(StringRef दत्त_2, endianness Endian)
    : प्रवाहः(दत्त_2, Endian) {}

Error द्विमान_प्रवाहः_Reader::readLongestContiguousChunk(
    ArrayRef<uint8_t> &बफर) {
  यदि (स्वचालित EC = प्रवाहः.readLongestContiguousChunk(विस्थापनम्, बफर))
    प्रत्यागम EC;
  विस्थापनम् += बफर.आकार_2();
  प्रत्यागम Error::success();
}

Error द्विमान_प्रवाहः_Reader::read_अष्टकानि(ArrayRef<uint8_t> &बफर, uint32_t आकार) {
  यदि (स्वचालित EC = प्रवाहः.read_अष्टकानि(विस्थापनम्, आकार, बफर))
    प्रत्यागम EC;
  विस्थापनम् += आकार;
  प्रत्यागम Error::success();
}

Error द्विमान_प्रवाहः_Reader::readULEB128(uint64_t &Dest) {
  Small_सदिशम्<uint8_t, 10> Encoded_अष्टकानि;
  ArrayRef<uint8_t> अग्र_Byte;

  // Copy the encoded ULEB into the buffer.
  कुरु {
    यदि (स्वचालित Err = read_अष्टकानि(अग्र_Byte, 1))
      प्रत्यागम Err;
    Encoded_अष्टकानि.प्रक्षिप_पृष्ठभागः(अग्र_Byte[0]);
  } यावत् (अग्र_Byte[0] & 0x80);

  Dest = decodeULEB128(Encoded_अष्टकानि.begin(), शून्यसूचकः, Encoded_अष्टकानि.अन्तः());
  प्रत्यागम Error::success();
}

Error द्विमान_प्रवाहः_Reader::readSLEB128(int64_t &Dest) {
  Small_सदिशम्<uint8_t, 10> Encoded_अष्टकानि;
  ArrayRef<uint8_t> अग्र_Byte;

  // Copy the encoded ULEB into the buffer.
  कुरु {
    यदि (स्वचालित Err = read_अष्टकानि(अग्र_Byte, 1))
      प्रत्यागम Err;
    Encoded_अष्टकानि.प्रक्षिप_पृष्ठभागः(अग्र_Byte[0]);
  } यावत् (अग्र_Byte[0] & 0x80);

  Dest = decodeSLEB128(Encoded_अष्टकानि.begin(), शून्यसूचकः, Encoded_अष्टकानि.अन्तः());
  प्रत्यागम Error::success();
}

Error द्विमान_प्रवाहः_Reader::readCString(StringRef &Dest) {
  uint64_t Original_विस्थापनम् = get_विस्थापनम्();
  uint64_t प्राप्तम्_विस्थापनम् = 0;
  यावत् (सत्य) {
    uint64_t This_विस्थापनम् = get_विस्थापनम्();
    ArrayRef<uint8_t> बफर;
    यदि (स्वचालित EC = readLongestContiguousChunk(बफर))
      प्रत्यागम EC;
    StringRef S(reinterpret_cast<ध्रुव वर्ण *>(बफर.begin()), बफर.आकार_2());
    size_t Pos = S.find_first_सम्बन्धि('\0');
    यदि (LLVM_LIKELY(Pos != StringRef::npos)) {
      प्राप्तम्_विस्थापनम् = Pos + This_विस्थापनम्;
      भङ्ग;
    }
  }
  प्रतिज्ञा(प्राप्तम्_विस्थापनम् >= Original_विस्थापनम्);

  समुच्चयः_विस्थापनम्(Original_विस्थापनम्);
  size_t दीर्घता = प्राप्तम्_विस्थापनम् - Original_विस्थापनम्;

  यदि (स्वचालित EC = readFixedString(Dest, दीर्घता))
    प्रत्यागम EC;

  // Now set the offset back to after the null terminator.
  समुच्चयः_विस्थापनम्(प्राप्तम्_विस्थापनम् + 1);
  प्रत्यागम Error::success();
}

Error द्विमान_प्रवाहः_Reader::readWideString(ArrayRef<UTF16> &Dest) {
  uint64_t दीर्घता = 0;
  uint64_t Original_विस्थापनम् = get_विस्थापनम्();
  ध्रुव UTF16 *C;
  यावत् (सत्य) {
    यदि (स्वचालित EC = readObject(C))
      प्रत्यागम EC;
    यदि (*C == 0x0000)
      भङ्ग;
    ++दीर्घता;
  }
  uint64_t New_विस्थापनम् = get_विस्थापनम्();
  समुच्चयः_विस्थापनम्(Original_विस्थापनम्);

  यदि (स्वचालित EC = readArray(Dest, दीर्घता))
    प्रत्यागम EC;
  समुच्चयः_विस्थापनम्(New_विस्थापनम्);
  प्रत्यागम Error::success();
}

Error द्विमान_प्रवाहः_Reader::readFixedString(StringRef &Dest, uint32_t दीर्घता) {
  ArrayRef<uint8_t> अष्टकानि_2;
  यदि (स्वचालित EC = read_अष्टकानि(अष्टकानि_2, दीर्घता))
    प्रत्यागम EC;
  Dest = StringRef(reinterpret_cast<ध्रुव वर्ण *>(अष्टकानि_2.begin()), अष्टकानि_2.आकार_2());
  प्रत्यागम Error::success();
}

Error द्विमान_प्रवाहः_Reader::read_प्रवाहः_Ref(द्विमान_प्रवाहः_Ref &Ref) {
  प्रत्यागम read_प्रवाहः_Ref(Ref, अष्टकानि_Remaining());
}

Error द्विमान_प्रवाहः_Reader::read_प्रवाहः_Ref(द्विमान_प्रवाहः_Ref &Ref, uint32_t दीर्घता) {
  यदि (अष्टकानि_Remaining() < दीर्घता)
    प्रत्यागम make_error<द्विमान_प्रवाहः_Error>(प्रवाहः_error_संहिता::प्रवाहः_too_short);
  Ref = प्रवाहः.खण्डिका(विस्थापनम्, दीर्घता);
  विस्थापनम् += दीर्घता;
  प्रत्यागम Error::success();
}

Error द्विमान_प्रवाहः_Reader::readSubstream(द्विमान_Substream_Ref &Ref,
                                        uint32_t दीर्घता) {
  Ref.विस्थापनम् = get_विस्थापनम्();
  प्रत्यागम read_प्रवाहः_Ref(Ref.प्रवाहः_दत्त, दीर्घता);
}

Error द्विमान_प्रवाहः_Reader::skip(uint64_t Amount) {
  यदि (Amount > अष्टकानि_Remaining())
    प्रत्यागम make_error<द्विमान_प्रवाहः_Error>(प्रवाहः_error_संहिता::प्रवाहः_too_short);
  विस्थापनम् += Amount;
  प्रत्यागम Error::success();
}

Error द्विमान_प्रवाहः_Reader::pad_गन्तव्यम्_Alignment(uint32_t Align) {
  uint32_t New_विस्थापनम् = align_गन्तव्यम्(विस्थापनम्, Align);
  प्रत्यागम skip(New_विस्थापनम् - विस्थापनम्);
}

uint8_t द्विमान_प्रवाहः_Reader::peek() ध्रुव {
  ArrayRef<uint8_t> बफर;
  स्वचालित EC = प्रवाहः.read_अष्टकानि(विस्थापनम्, 1, बफर);
  प्रतिज्ञा(!EC && "Cannot peek an empty buffer!");
  llvm::consumeError(std::move(EC));
  प्रत्यागम बफर[0];
}

std::pair<द्विमान_प्रवाहः_Reader, द्विमान_प्रवाहः_Reader>
द्विमान_प्रवाहः_Reader::विभज(uint64_t Off) ध्रुव {
  प्रतिज्ञा(get_दीर्घता() >= Off);

  द्विमान_प्रवाहः_Ref First = प्रवाहः.drop_front(विस्थापनम्);

  द्विमान_प्रवाहः_Ref Second = First.drop_front(Off);
  First = First.keep_front(Off);
  द्विमान_प्रवाहः_Reader W1{First};
  द्विमान_प्रवाहः_Reader W2{Second};
  प्रत्यागम {W1, W2};
}
