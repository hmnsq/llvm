//===- BinaryStreamRef.cpp - ----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/BinaryStreamRef.h"
#अन्तर्भाव "llvm/Support/BinaryByteStream.h"

उपयुज्य नामाकाश llvm;

नामाकाश {

वर्ग ArrayRefImpl : सार्वजनिक द्विमान_प्रवाहः {
सार्वजनिक:
  ArrayRefImpl(ArrayRef<uint8_t> दत्त_2, endianness Endian) : BBS(दत्त_2, Endian) {}

  llvm::endianness getEndian() ध्रुव override { प्रत्यागम BBS.getEndian(); }
  Error read_अष्टकानि(uint64_t विस्थापनम्, uint64_t आकार,
                  ArrayRef<uint8_t> &बफर) override {
    प्रत्यागम BBS.read_अष्टकानि(विस्थापनम्, आकार, बफर);
  }
  Error readLongestContiguousChunk(uint64_t विस्थापनम्,
                                   ArrayRef<uint8_t> &बफर) override {
    प्रत्यागम BBS.readLongestContiguousChunk(विस्थापनम्, बफर);
  }
  uint64_t get_दीर्घता() override { प्रत्यागम BBS.get_दीर्घता(); }

गुप्त:
  द्विमान_Byte_प्रवाहः BBS;
};

वर्ग MutableArrayRefImpl : सार्वजनिक Writable_द्विमान_प्रवाहः {
सार्वजनिक:
  MutableArrayRefImpl(MutableArrayRef<uint8_t> दत्त_2, endianness Endian)
      : BBS(दत्त_2, Endian) {}

  // Inherited via WritableBinaryStream
  llvm::endianness getEndian() ध्रुव override { प्रत्यागम BBS.getEndian(); }
  Error read_अष्टकानि(uint64_t विस्थापनम्, uint64_t आकार,
                  ArrayRef<uint8_t> &बफर) override {
    प्रत्यागम BBS.read_अष्टकानि(विस्थापनम्, आकार, बफर);
  }
  Error readLongestContiguousChunk(uint64_t विस्थापनम्,
                                   ArrayRef<uint8_t> &बफर) override {
    प्रत्यागम BBS.readLongestContiguousChunk(विस्थापनम्, बफर);
  }
  uint64_t get_दीर्घता() override { प्रत्यागम BBS.get_दीर्घता(); }

  Error write_अष्टकानि(uint64_t विस्थापनम्, ArrayRef<uint8_t> दत्त_2) override {
    प्रत्यागम BBS.write_अष्टकानि(विस्थापनम्, दत्त_2);
  }
  Error commit() override { प्रत्यागम BBS.commit(); }

गुप्त:
  Mutable_द्विमान_Byte_प्रवाहः BBS;
};
} // namespace

द्विमान_प्रवाहः_Ref::द्विमान_प्रवाहः_Ref(द्विमान_प्रवाहः &प्रवाहः)
    : द्विमान_प्रवाहः_Ref_आधारः(प्रवाहः) {}
द्विमान_प्रवाहः_Ref::द्विमान_प्रवाहः_Ref(द्विमान_प्रवाहः &प्रवाहः, uint64_t विस्थापनम्,
                                 std::optional<uint64_t> दीर्घता)
    : द्विमान_प्रवाहः_Ref_आधारः(प्रवाहः, विस्थापनम्, दीर्घता) {}
द्विमान_प्रवाहः_Ref::द्विमान_प्रवाहः_Ref(ArrayRef<uint8_t> दत्त_2, endianness Endian)
    : द्विमान_प्रवाहः_Ref_आधारः(std::make_shared<ArrayRefImpl>(दत्त_2, Endian), 0,
                          दत्त_2.आकार_2()) {}
द्विमान_प्रवाहः_Ref::द्विमान_प्रवाहः_Ref(StringRef दत्त_2, endianness Endian)
    : द्विमान_प्रवाहः_Ref(ArrayRef(दत्त_2.अष्टकानि_begin(), दत्त_2.अष्टकानि_अन्तः()), Endian) {}

Error द्विमान_प्रवाहः_Ref::read_अष्टकानि(uint64_t विस्थापनम्, uint64_t आकार,
                                 ArrayRef<uint8_t> &बफर) ध्रुव {
  यदि (स्वचालित EC = check_विस्थापनम्_For_Read(विस्थापनम्, आकार))
    प्रत्यागम EC;
  प्रत्यागम BorrowedImpl->read_अष्टकानि(View_विस्थापनम् + विस्थापनम्, आकार, बफर);
}

Error द्विमान_प्रवाहः_Ref::readLongestContiguousChunk(
    uint64_t विस्थापनम्, ArrayRef<uint8_t> &बफर) ध्रुव {
  यदि (स्वचालित EC = check_विस्थापनम्_For_Read(विस्थापनम्, 1))
    प्रत्यागम EC;

  यदि (स्वचालित EC =
          BorrowedImpl->readLongestContiguousChunk(View_विस्थापनम् + विस्थापनम्, बफर))
    प्रत्यागम EC;
  // This StreamRef might refer to a smaller window over a larger stream.  In
  // that case we will have read out more bytes than we should return, because
  // we should not read past the end of the current view.
  uint64_t Max_दीर्घता = get_दीर्घता() - विस्थापनम्;
  यदि (बफर.आकार_2() > Max_दीर्घता)
    बफर = बफर.खण्डिका(0, Max_दीर्घता);
  प्रत्यागम Error::success();
}

Writable_द्विमान_प्रवाहः_Ref::Writable_द्विमान_प्रवाहः_Ref(Writable_द्विमान_प्रवाहः &प्रवाहः)
    : द्विमान_प्रवाहः_Ref_आधारः(प्रवाहः) {}

Writable_द्विमान_प्रवाहः_Ref::Writable_द्विमान_प्रवाहः_Ref(Writable_द्विमान_प्रवाहः &प्रवाहः,
                                                 uint64_t विस्थापनम्,
                                                 std::optional<uint64_t> दीर्घता)
    : द्विमान_प्रवाहः_Ref_आधारः(प्रवाहः, विस्थापनम्, दीर्घता) {}

Writable_द्विमान_प्रवाहः_Ref::Writable_द्विमान_प्रवाहः_Ref(MutableArrayRef<uint8_t> दत्त_2,
                                                 endianness Endian)
    : द्विमान_प्रवाहः_Ref_आधारः(std::make_shared<MutableArrayRefImpl>(दत्त_2, Endian),
                          0, दत्त_2.आकार_2()) {}

Error Writable_द्विमान_प्रवाहः_Ref::write_अष्टकानि(uint64_t विस्थापनम्,
                                          ArrayRef<uint8_t> दत्त_2) ध्रुव {
  यदि (स्वचालित EC = check_विस्थापनम्_For_Write(विस्थापनम्, दत्त_2.आकार_2()))
    प्रत्यागम EC;

  प्रत्यागम BorrowedImpl->write_अष्टकानि(View_विस्थापनम् + विस्थापनम्, दत्त_2);
}

Writable_द्विमान_प्रवाहः_Ref::संक्रियकः द्विमान_प्रवाहः_Ref() ध्रुव {
  प्रत्यागम द्विमान_प्रवाहः_Ref(*BorrowedImpl, View_विस्थापनम्, दीर्घता);
}

/// For buffered streams, commits changes to the backing store.
Error Writable_द्विमान_प्रवाहः_Ref::commit() { प्रत्यागम BorrowedImpl->commit(); }
