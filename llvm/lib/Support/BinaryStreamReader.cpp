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

ورتوں نطاق llvm;

ثنائی_دھارا_Reader::ثنائی_دھارا_Reader(ثنائی_دھارا_Ref Ref) : دھارا(Ref) {}

ثنائی_دھارا_Reader::ثنائی_دھارا_Reader(ثنائی_دھارا &دھارا) : دھارا(دھارا) {}

ثنائی_دھارا_Reader::ثنائی_دھارا_Reader(ArrayRef<uint8_t> انکڑے_2,
                                       endianness Endian)
    : دھارا(انکڑے_2, Endian) {}

ثنائی_دھارا_Reader::ثنائی_دھارا_Reader(StringRef انکڑے_2, endianness Endian)
    : دھارا(انکڑے_2, Endian) {}

Error ثنائی_دھارا_Reader::readLongestContiguousChunk(
    ArrayRef<uint8_t> &بفر) {
  إذا (خودکار EC = دھارا.readLongestContiguousChunk(وتھ, بفر))
    أرجع EC;
  وتھ += بفر.ناپ_2();
  أرجع Error::success();
}

Error ثنائی_دھارا_Reader::read_بائٹاں(ArrayRef<uint8_t> &بفر, uint32_t ناپ) {
  إذا (خودکار EC = دھارا.read_بائٹاں(وتھ, ناپ, بفر))
    أرجع EC;
  وتھ += ناپ;
  أرجع Error::success();
}

Error ثنائی_دھارا_Reader::readULEB128(uint64_t &Dest) {
  Small_سمتیہ<uint8_t, 10> Encoded_بائٹاں;
  ArrayRef<uint8_t> اگلا_Byte;

  // Copy the encoded ULEB into the buffer.
  افعل {
    إذا (خودکار Err = read_بائٹاں(اگلا_Byte, 1))
      أرجع Err;
    Encoded_بائٹاں.ڈالو_پٹھ(اگلا_Byte[0]);
  } طالما (اگلا_Byte[0] & 0x80);

  Dest = decodeULEB128(Encoded_بائٹاں.begin(), خالی_سوچک, Encoded_بائٹاں.سرا());
  أرجع Error::success();
}

Error ثنائی_دھارا_Reader::readSLEB128(int64_t &Dest) {
  Small_سمتیہ<uint8_t, 10> Encoded_بائٹاں;
  ArrayRef<uint8_t> اگلا_Byte;

  // Copy the encoded ULEB into the buffer.
  افعل {
    إذا (خودکار Err = read_بائٹاں(اگلا_Byte, 1))
      أرجع Err;
    Encoded_بائٹاں.ڈالو_پٹھ(اگلا_Byte[0]);
  } طالما (اگلا_Byte[0] & 0x80);

  Dest = decodeSLEB128(Encoded_بائٹاں.begin(), خالی_سوچک, Encoded_بائٹاں.سرا());
  أرجع Error::success();
}

Error ثنائی_دھارا_Reader::readCString(StringRef &Dest) {
  uint64_t Original_وتھ = get_وتھ();
  uint64_t لبھیا_وتھ = 0;
  طالما (سچ) {
    uint64_t This_وتھ = get_وتھ();
    ArrayRef<uint8_t> بفر;
    إذا (خودکار EC = readLongestContiguousChunk(بفر))
      أرجع EC;
    StringRef S(reinterpret_cast<ثابت حرف *>(بفر.begin()), بفر.ناپ_2());
    size_t Pos = S.لبھو_پہلا_دا('\0');
    إذا (LLVM_LIKELY(Pos != StringRef::npos)) {
      لبھیا_وتھ = Pos + This_وتھ;
      اكسر;
    }
  }
  أكد(لبھیا_وتھ >= Original_وتھ);

  سیٹ_وتھ(Original_وتھ);
  size_t لمائی = لبھیا_وتھ - Original_وتھ;

  إذا (خودکار EC = readFixedString(Dest, لمائی))
    أرجع EC;

  // Now set the offset back to after the null terminator.
  سیٹ_وتھ(لبھیا_وتھ + 1);
  أرجع Error::success();
}

Error ثنائی_دھارا_Reader::readWideString(ArrayRef<UTF16> &Dest) {
  uint64_t لمائی = 0;
  uint64_t Original_وتھ = get_وتھ();
  ثابت UTF16 *C;
  طالما (سچ) {
    إذا (خودکار EC = readObject(C))
      أرجع EC;
    إذا (*C == 0x0000)
      اكسر;
    ++لمائی;
  }
  uint64_t New_وتھ = get_وتھ();
  سیٹ_وتھ(Original_وتھ);

  إذا (خودکار EC = readArray(Dest, لمائی))
    أرجع EC;
  سیٹ_وتھ(New_وتھ);
  أرجع Error::success();
}

Error ثنائی_دھارا_Reader::readFixedString(StringRef &Dest, uint32_t لمائی) {
  ArrayRef<uint8_t> بائٹاں_2;
  إذا (خودکار EC = read_بائٹاں(بائٹاں_2, لمائی))
    أرجع EC;
  Dest = StringRef(reinterpret_cast<ثابت حرف *>(بائٹاں_2.begin()), بائٹاں_2.ناپ_2());
  أرجع Error::success();
}

Error ثنائی_دھارا_Reader::read_دھارا_Ref(ثنائی_دھارا_Ref &Ref) {
  أرجع read_دھارا_Ref(Ref, بائٹاں_بچیا());
}

Error ثنائی_دھارا_Reader::read_دھارا_Ref(ثنائی_دھارا_Ref &Ref, uint32_t لمائی) {
  إذا (بائٹاں_بچیا() < لمائی)
    أرجع make_error<ثنائی_دھارا_Error>(دھارا_error_کوڈ::دھارا_too_short);
  Ref = دھارا.ٹکڑا(وتھ, لمائی);
  وتھ += لمائی;
  أرجع Error::success();
}

Error ثنائی_دھارا_Reader::readSubstream(ثنائی_Substream_Ref &Ref,
                                        uint32_t لمائی) {
  Ref.وتھ = get_وتھ();
  أرجع read_دھارا_Ref(Ref.دھارا_انکڑے, لمائی);
}

Error ثنائی_دھارا_Reader::skip(uint64_t Amount) {
  إذا (Amount > بائٹاں_بچیا())
    أرجع make_error<ثنائی_دھارا_Error>(دھارا_error_کوڈ::دھارا_too_short);
  وتھ += Amount;
  أرجع Error::success();
}

Error ثنائی_دھارا_Reader::pad_منزل_Alignment(uint32_t Align) {
  uint32_t New_وتھ = align_منزل(وتھ, Align);
  أرجع skip(New_وتھ - وتھ);
}

uint8_t ثنائی_دھارا_Reader::peek() ثابت {
  ArrayRef<uint8_t> بفر;
  خودکار EC = دھارا.read_بائٹاں(وتھ, 1, بفر);
  أكد(!EC && "Cannot peek an empty buffer!");
  llvm::consumeError(std::move(EC));
  أرجع بفر[0];
}

std::pair<ثنائی_دھارا_Reader, ثنائی_دھارا_Reader>
ثنائی_دھارا_Reader::ونڈو(uint64_t Off) ثابت {
  أكد(get_لمائی() >= Off);

  ثنائی_دھارا_Ref پہلا = دھارا.drop_front(وتھ);

  ثنائی_دھارا_Ref دوجا = پہلا.drop_front(Off);
  پہلا = پہلا.keep_front(Off);
  ثنائی_دھارا_Reader W1{پہلا};
  ثنائی_دھارا_Reader W2{دوجا};
  أرجع {W1, W2};
}
