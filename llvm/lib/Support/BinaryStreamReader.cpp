//===- BinaryStreamReader.cpp - Reads objects from a binary stream --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/BinaryStreamReader.h"

#ضمن "llvm/Support/BinaryStreamError.h"
#ضمن "llvm/Support/BinaryStreamRef.h"
#ضمن "llvm/Support/LEB128.h"

کارول نطاق llvm;

دوګونی_بهیر_Reader::دوګونی_بهیر_Reader(دوګونی_بهیر_Ref Ref) : بهیر(Ref) {}

دوګونی_بهیر_Reader::دوګونی_بهیر_Reader(دوګونی_بهیر &بهیر) : بهیر(بهیر) {}

دوګونی_بهیر_Reader::دوګونی_بهیر_Reader(ArrayRef<uint8_t> معلومات_2,
                                       endianness Endian)
    : بهیر(معلومات_2, Endian) {}

دوګونی_بهیر_Reader::دوګونی_بهیر_Reader(StringRef معلومات_2, endianness Endian)
    : بهیر(معلومات_2, Endian) {}

Error دوګونی_بهیر_Reader::readLongestContiguousChunk(
    ArrayRef<uint8_t> &بفر) {
  إذا (خپلکاری EC = بهیر.readLongestContiguousChunk(دننه_واټن, بفر))
    أرجع EC;
  دننه_واټن += بفر.اندازه_2();
  أرجع Error::success();
}

Error دوګونی_بهیر_Reader::read_بایټونه(ArrayRef<uint8_t> &بفر, uint32_t اندازه) {
  إذا (خپلکاری EC = بهیر.read_بایټونه(دننه_واټن, اندازه, بفر))
    أرجع EC;
  دننه_واټن += اندازه;
  أرجع Error::success();
}

Error دوګونی_بهیر_Reader::readULEB128(uint64_t &Dest) {
  Small_ویکټور<uint8_t, 10> Encoded_بایټونه;
  ArrayRef<uint8_t> بل_Byte;

  // Copy the encoded ULEB into the buffer.
  افعل {
    إذا (خپلکاری Err = read_بایټونه(بل_Byte, 1))
      أرجع Err;
    Encoded_بایټونه.ټېل_شا(بل_Byte[0]);
  } طالما (بل_Byte[0] & 0x80);

  Dest = decodeULEB128(Encoded_بایټونه.begin(), تش_اشاره, Encoded_بایټونه.پای_ټکی());
  أرجع Error::success();
}

Error دوګونی_بهیر_Reader::readSLEB128(int64_t &Dest) {
  Small_ویکټور<uint8_t, 10> Encoded_بایټونه;
  ArrayRef<uint8_t> بل_Byte;

  // Copy the encoded ULEB into the buffer.
  افعل {
    إذا (خپلکاری Err = read_بایټونه(بل_Byte, 1))
      أرجع Err;
    Encoded_بایټونه.ټېل_شا(بل_Byte[0]);
  } طالما (بل_Byte[0] & 0x80);

  Dest = decodeSLEB128(Encoded_بایټونه.begin(), تش_اشاره, Encoded_بایټونه.پای_ټکی());
  أرجع Error::success();
}

Error دوګونی_بهیر_Reader::readCString(StringRef &Dest) {
  uint64_t Original_دننه_واټن = get_دننه_واټن();
  uint64_t وموندل_دننه_واټن = 0;
  طالما (ریښتیا) {
    uint64_t This_دننه_واټن = get_دننه_واټن();
    ArrayRef<uint8_t> بفر;
    إذا (خپلکاری EC = readLongestContiguousChunk(بفر))
      أرجع EC;
    StringRef S(reinterpret_cast<ثابت حرف *>(بفر.begin()), بفر.اندازه_2());
    size_t Pos = S.پیدا_کړه_لومړی_د('\0');
    إذا (LLVM_LIKELY(Pos != StringRef::npos)) {
      وموندل_دننه_واټن = Pos + This_دننه_واټن;
      اكسر;
    }
  }
  أكد(وموندل_دننه_واټن >= Original_دننه_واټن);

  سېټ_دننه_واټن(Original_دننه_واټن);
  size_t اوږدتیا = وموندل_دننه_واټن - Original_دننه_واټن;

  إذا (خپلکاری EC = readFixedString(Dest, اوږدتیا))
    أرجع EC;

  // Now set the offset back to after the null terminator.
  سېټ_دننه_واټن(وموندل_دننه_واټن + 1);
  أرجع Error::success();
}

Error دوګونی_بهیر_Reader::readWideString(ArrayRef<UTF16> &Dest) {
  uint64_t اوږدتیا = 0;
  uint64_t Original_دننه_واټن = get_دننه_واټن();
  ثابت UTF16 *C;
  طالما (ریښتیا) {
    إذا (خپلکاری EC = readObject(C))
      أرجع EC;
    إذا (*C == 0x0000)
      اكسر;
    ++اوږدتیا;
  }
  uint64_t New_دننه_واټن = get_دننه_واټن();
  سېټ_دننه_واټن(Original_دننه_واټن);

  إذا (خپلکاری EC = readArray(Dest, اوږدتیا))
    أرجع EC;
  سېټ_دننه_واټن(New_دننه_واټن);
  أرجع Error::success();
}

Error دوګونی_بهیر_Reader::readFixedString(StringRef &Dest, uint32_t اوږدتیا) {
  ArrayRef<uint8_t> بایټونه_2;
  إذا (خپلکاری EC = read_بایټونه(بایټونه_2, اوږدتیا))
    أرجع EC;
  Dest = StringRef(reinterpret_cast<ثابت حرف *>(بایټونه_2.begin()), بایټونه_2.اندازه_2());
  أرجع Error::success();
}

Error دوګونی_بهیر_Reader::read_بهیر_Ref(دوګونی_بهیر_Ref &Ref) {
  أرجع read_بهیر_Ref(Ref, بایټونه_پاتې_شوی());
}

Error دوګونی_بهیر_Reader::read_بهیر_Ref(دوګونی_بهیر_Ref &Ref, uint32_t اوږدتیا) {
  إذا (بایټونه_پاتې_شوی() < اوږدتیا)
    أرجع make_error<دوګونی_بهیر_Error>(بهیر_error_کوډ::بهیر_too_short);
  Ref = بهیر.ټوټه(دننه_واټن, اوږدتیا);
  دننه_واټن += اوږدتیا;
  أرجع Error::success();
}

Error دوګونی_بهیر_Reader::readSubstream(دوګونی_Substream_Ref &Ref,
                                        uint32_t اوږدتیا) {
  Ref.دننه_واټن = get_دننه_واټن();
  أرجع read_بهیر_Ref(Ref.بهیر_معلومات, اوږدتیا);
}

Error دوګونی_بهیر_Reader::skip(uint64_t Amount) {
  إذا (Amount > بایټونه_پاتې_شوی())
    أرجع make_error<دوګونی_بهیر_Error>(بهیر_error_کوډ::بهیر_too_short);
  دننه_واټن += Amount;
  أرجع Error::success();
}

Error دوګونی_بهیر_Reader::pad_منزل_Alignment(uint32_t Align) {
  uint32_t New_دننه_واټن = align_منزل(دننه_واټن, Align);
  أرجع skip(New_دننه_واټن - دننه_واټن);
}

uint8_t دوګونی_بهیر_Reader::peek() ثابت {
  ArrayRef<uint8_t> بفر;
  خپلکاری EC = بهیر.read_بایټونه(دننه_واټن, 1, بفر);
  أكد(!EC && "Cannot peek an empty buffer!");
  llvm::consumeError(std::move(EC));
  أرجع بفر[0];
}

std::pair<دوګونی_بهیر_Reader, دوګونی_بهیر_Reader>
دوګونی_بهیر_Reader::وېشل(uint64_t Off) ثابت {
  أكد(get_اوږدتیا() >= Off);

  دوګونی_بهیر_Ref لومړی = بهیر.drop_front(دننه_واټن);

  دوګونی_بهیر_Ref دوهم = لومړی.drop_front(Off);
  لومړی = لومړی.keep_front(Off);
  دوګونی_بهیر_Reader W1{لومړی};
  دوګونی_بهیر_Reader W2{دوهم};
  أرجع {W1, W2};
}
