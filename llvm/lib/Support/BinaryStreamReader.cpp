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

استعمال نطاق llvm;

ٻٽو_وهڪرو_Reader::ٻٽو_وهڪرو_Reader(ٻٽو_وهڪرو_Ref Ref) : وهڪرو(Ref) {}

ٻٽو_وهڪرو_Reader::ٻٽو_وهڪرو_Reader(ٻٽو_وهڪرو &وهڪرو) : وهڪرو(وهڪرو) {}

ٻٽو_وهڪرو_Reader::ٻٽو_وهڪرو_Reader(ArrayRef<uint8_t> معلومات_2,
                                       endianness Endian)
    : وهڪرو(معلومات_2, Endian) {}

ٻٽو_وهڪرو_Reader::ٻٽو_وهڪرو_Reader(StringRef معلومات_2, endianness Endian)
    : وهڪرو(معلومات_2, Endian) {}

Error ٻٽو_وهڪرو_Reader::readLongestContiguousChunk(
    ArrayRef<uint8_t> &بفر) {
  إذا (خودڪار EC = وهڪرو.readLongestContiguousChunk(وٿي, بفر))
    أرجع EC;
  وٿي += بفر.سائيز_2();
  أرجع Error::success();
}

Error ٻٽو_وهڪرو_Reader::read_بائيٽون(ArrayRef<uint8_t> &بفر, uint32_t سائيز) {
  إذا (خودڪار EC = وهڪرو.read_بائيٽون(وٿي, سائيز, بفر))
    أرجع EC;
  وٿي += سائيز;
  أرجع Error::success();
}

Error ٻٽو_وهڪرو_Reader::readULEB128(uint64_t &Dest) {
  Small_ويڪٽر<uint8_t, 10> Encoded_بائيٽون;
  ArrayRef<uint8_t> ايندڙ_Byte;

  // Copy the encoded ULEB into the buffer.
  افعل {
    إذا (خودڪار Err = read_بائيٽون(ايندڙ_Byte, 1))
      أرجع Err;
    Encoded_بائيٽون.ڌڪو_پٺ(ايندڙ_Byte[0]);
  } طالما (ايندڙ_Byte[0] & 0x80);

  Dest = decodeULEB128(Encoded_بائيٽون.begin(), خالي_اشارو, Encoded_بائيٽون.پڇاڙي());
  أرجع Error::success();
}

Error ٻٽو_وهڪرو_Reader::readSLEB128(int64_t &Dest) {
  Small_ويڪٽر<uint8_t, 10> Encoded_بائيٽون;
  ArrayRef<uint8_t> ايندڙ_Byte;

  // Copy the encoded ULEB into the buffer.
  افعل {
    إذا (خودڪار Err = read_بائيٽون(ايندڙ_Byte, 1))
      أرجع Err;
    Encoded_بائيٽون.ڌڪو_پٺ(ايندڙ_Byte[0]);
  } طالما (ايندڙ_Byte[0] & 0x80);

  Dest = decodeSLEB128(Encoded_بائيٽون.begin(), خالي_اشارو, Encoded_بائيٽون.پڇاڙي());
  أرجع Error::success();
}

Error ٻٽو_وهڪرو_Reader::readCString(StringRef &Dest) {
  uint64_t Original_وٿي = get_وٿي();
  uint64_t مليو_وٿي = 0;
  طالما (سچ) {
    uint64_t This_وٿي = get_وٿي();
    ArrayRef<uint8_t> بفر;
    إذا (خودڪار EC = readLongestContiguousChunk(بفر))
      أرجع EC;
    StringRef S(reinterpret_cast<ثابت حرف *>(بفر.begin()), بفر.سائيز_2());
    size_t Pos = S.ڳوليو_پهريون_جو('\0');
    إذا (LLVM_LIKELY(Pos != StringRef::npos)) {
      مليو_وٿي = Pos + This_وٿي;
      اكسر;
    }
  }
  أكد(مليو_وٿي >= Original_وٿي);

  سيٽ_وٿي(Original_وٿي);
  size_t ڊيگهائي = مليو_وٿي - Original_وٿي;

  إذا (خودڪار EC = readFixedString(Dest, ڊيگهائي))
    أرجع EC;

  // Now set the offset back to after the null terminator.
  سيٽ_وٿي(مليو_وٿي + 1);
  أرجع Error::success();
}

Error ٻٽو_وهڪرو_Reader::readWideString(ArrayRef<UTF16> &Dest) {
  uint64_t ڊيگهائي = 0;
  uint64_t Original_وٿي = get_وٿي();
  ثابت UTF16 *C;
  طالما (سچ) {
    إذا (خودڪار EC = readObject(C))
      أرجع EC;
    إذا (*C == 0x0000)
      اكسر;
    ++ڊيگهائي;
  }
  uint64_t New_وٿي = get_وٿي();
  سيٽ_وٿي(Original_وٿي);

  إذا (خودڪار EC = readArray(Dest, ڊيگهائي))
    أرجع EC;
  سيٽ_وٿي(New_وٿي);
  أرجع Error::success();
}

Error ٻٽو_وهڪرو_Reader::readFixedString(StringRef &Dest, uint32_t ڊيگهائي) {
  ArrayRef<uint8_t> بائيٽون_2;
  إذا (خودڪار EC = read_بائيٽون(بائيٽون_2, ڊيگهائي))
    أرجع EC;
  Dest = StringRef(reinterpret_cast<ثابت حرف *>(بائيٽون_2.begin()), بائيٽون_2.سائيز_2());
  أرجع Error::success();
}

Error ٻٽو_وهڪرو_Reader::read_وهڪرو_Ref(ٻٽو_وهڪرو_Ref &Ref) {
  أرجع read_وهڪرو_Ref(Ref, بائيٽون_بچيل());
}

Error ٻٽو_وهڪرو_Reader::read_وهڪرو_Ref(ٻٽو_وهڪرو_Ref &Ref, uint32_t ڊيگهائي) {
  إذا (بائيٽون_بچيل() < ڊيگهائي)
    أرجع make_error<ٻٽو_وهڪرو_Error>(وهڪرو_error_ڪوڊ::وهڪرو_too_short);
  Ref = وهڪرو.ٽڪرو(وٿي, ڊيگهائي);
  وٿي += ڊيگهائي;
  أرجع Error::success();
}

Error ٻٽو_وهڪرو_Reader::readSubstream(ٻٽو_Substream_Ref &Ref,
                                        uint32_t ڊيگهائي) {
  Ref.وٿي = get_وٿي();
  أرجع read_وهڪرو_Ref(Ref.وهڪرو_معلومات, ڊيگهائي);
}

Error ٻٽو_وهڪرو_Reader::skip(uint64_t Amount) {
  إذا (Amount > بائيٽون_بچيل())
    أرجع make_error<ٻٽو_وهڪرو_Error>(وهڪرو_error_ڪوڊ::وهڪرو_too_short);
  وٿي += Amount;
  أرجع Error::success();
}

Error ٻٽو_وهڪرو_Reader::pad_منزل_Alignment(uint32_t Align) {
  uint32_t New_وٿي = align_منزل(وٿي, Align);
  أرجع skip(New_وٿي - وٿي);
}

uint8_t ٻٽو_وهڪرو_Reader::peek() ثابت {
  ArrayRef<uint8_t> بفر;
  خودڪار EC = وهڪرو.read_بائيٽون(وٿي, 1, بفر);
  أكد(!EC && "Cannot peek an empty buffer!");
  llvm::consumeError(std::move(EC));
  أرجع بفر[0];
}

std::pair<ٻٽو_وهڪرو_Reader, ٻٽو_وهڪرو_Reader>
ٻٽو_وهڪرو_Reader::ورهايو(uint64_t Off) ثابت {
  أكد(get_ڊيگهائي() >= Off);

  ٻٽو_وهڪرو_Ref پهريون = وهڪرو.drop_front(وٿي);

  ٻٽو_وهڪرو_Ref ٻيو = پهريون.drop_front(Off);
  پهريون = پهريون.keep_front(Off);
  ٻٽو_وهڪرو_Reader W1{پهريون};
  ٻٽو_وهڪرو_Reader W2{ٻيو};
  أرجع {W1, W2};
}
