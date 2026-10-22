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

استفاده نطاق llvm;

دودویی_جریان_Reader::دودویی_جریان_Reader(دودویی_جریان_Ref Ref) : جریان(Ref) {}

دودویی_جریان_Reader::دودویی_جریان_Reader(دودویی_جریان &جریان) : جریان(جریان) {}

دودویی_جریان_Reader::دودویی_جریان_Reader(ArrayRef<uint8_t> داده_2,
                                       endianness Endian)
    : جریان(داده_2, Endian) {}

دودویی_جریان_Reader::دودویی_جریان_Reader(StringRef داده_2, endianness Endian)
    : جریان(داده_2, Endian) {}

Error دودویی_جریان_Reader::readLongestContiguousChunk(
    ArrayRef<uint8_t> &میانگیر) {
  إذا (خودکار EC = جریان.readLongestContiguousChunk(جابه_جایی, میانگیر))
    أرجع EC;
  جابه_جایی += میانگیر.اندازه_2();
  أرجع Error::success();
}

Error دودویی_جریان_Reader::read_بایتها(ArrayRef<uint8_t> &میانگیر, uint32_t اندازه) {
  إذا (خودکار EC = جریان.read_بایتها(جابه_جایی, اندازه, میانگیر))
    أرجع EC;
  جابه_جایی += اندازه;
  أرجع Error::success();
}

Error دودویی_جریان_Reader::readULEB128(uint64_t &Dest) {
  Small_وکتور<uint8_t, 10> Encoded_بایتها;
  ArrayRef<uint8_t> بعدی_Byte;

  // Copy the encoded ULEB into the buffer.
  افعل {
    إذا (خودکار Err = read_بایتها(بعدی_Byte, 1))
      أرجع Err;
    Encoded_بایتها.افزودن_پشت(بعدی_Byte[0]);
  } طالما (بعدی_Byte[0] & 0x80);

  Dest = decodeULEB128(Encoded_بایتها.begin(), اشاره_تهی, Encoded_بایتها.انتها());
  أرجع Error::success();
}

Error دودویی_جریان_Reader::readSLEB128(int64_t &Dest) {
  Small_وکتور<uint8_t, 10> Encoded_بایتها;
  ArrayRef<uint8_t> بعدی_Byte;

  // Copy the encoded ULEB into the buffer.
  افعل {
    إذا (خودکار Err = read_بایتها(بعدی_Byte, 1))
      أرجع Err;
    Encoded_بایتها.افزودن_پشت(بعدی_Byte[0]);
  } طالما (بعدی_Byte[0] & 0x80);

  Dest = decodeSLEB128(Encoded_بایتها.begin(), اشاره_تهی, Encoded_بایتها.انتها());
  أرجع Error::success();
}

Error دودویی_جریان_Reader::readCString(StringRef &Dest) {
  uint64_t Original_جابه_جایی = get_جابه_جایی();
  uint64_t یافت_جابه_جایی = 0;
  طالما (درست) {
    uint64_t This_جابه_جایی = get_جابه_جایی();
    ArrayRef<uint8_t> میانگیر;
    إذا (خودکار EC = readLongestContiguousChunk(میانگیر))
      أرجع EC;
    StringRef S(reinterpret_cast<ثابت حرف *>(میانگیر.begin()), میانگیر.اندازه_2());
    size_t Pos = S.پیدا_کن_اولی_از('\0');
    إذا (LLVM_LIKELY(Pos != StringRef::npos)) {
      یافت_جابه_جایی = Pos + This_جابه_جایی;
      اكسر;
    }
  }
  أكد(یافت_جابه_جایی >= Original_جابه_جایی);

  مجموعه_جابه_جایی(Original_جابه_جایی);
  size_t درازی = یافت_جابه_جایی - Original_جابه_جایی;

  إذا (خودکار EC = readFixedString(Dest, درازی))
    أرجع EC;

  // Now set the offset back to after the null terminator.
  مجموعه_جابه_جایی(یافت_جابه_جایی + 1);
  أرجع Error::success();
}

Error دودویی_جریان_Reader::readWideString(ArrayRef<UTF16> &Dest) {
  uint64_t درازی = 0;
  uint64_t Original_جابه_جایی = get_جابه_جایی();
  ثابت UTF16 *C;
  طالما (درست) {
    إذا (خودکار EC = readObject(C))
      أرجع EC;
    إذا (*C == 0x0000)
      اكسر;
    ++درازی;
  }
  uint64_t New_جابه_جایی = get_جابه_جایی();
  مجموعه_جابه_جایی(Original_جابه_جایی);

  إذا (خودکار EC = readArray(Dest, درازی))
    أرجع EC;
  مجموعه_جابه_جایی(New_جابه_جایی);
  أرجع Error::success();
}

Error دودویی_جریان_Reader::readFixedString(StringRef &Dest, uint32_t درازی) {
  ArrayRef<uint8_t> بایتها_2;
  إذا (خودکار EC = read_بایتها(بایتها_2, درازی))
    أرجع EC;
  Dest = StringRef(reinterpret_cast<ثابت حرف *>(بایتها_2.begin()), بایتها_2.اندازه_2());
  أرجع Error::success();
}

Error دودویی_جریان_Reader::read_جریان_Ref(دودویی_جریان_Ref &Ref) {
  أرجع read_جریان_Ref(Ref, بایتها_مانده());
}

Error دودویی_جریان_Reader::read_جریان_Ref(دودویی_جریان_Ref &Ref, uint32_t درازی) {
  إذا (بایتها_مانده() < درازی)
    أرجع make_error<دودویی_جریان_Error>(جریان_error_کد::جریان_too_short);
  Ref = جریان.برش(جابه_جایی, درازی);
  جابه_جایی += درازی;
  أرجع Error::success();
}

Error دودویی_جریان_Reader::readSubstream(دودویی_Substream_Ref &Ref,
                                        uint32_t درازی) {
  Ref.جابه_جایی = get_جابه_جایی();
  أرجع read_جریان_Ref(Ref.جریان_داده, درازی);
}

Error دودویی_جریان_Reader::skip(uint64_t Amount) {
  إذا (Amount > بایتها_مانده())
    أرجع make_error<دودویی_جریان_Error>(جریان_error_کد::جریان_too_short);
  جابه_جایی += Amount;
  أرجع Error::success();
}

Error دودویی_جریان_Reader::pad_مقصد_Alignment(uint32_t Align) {
  uint32_t New_جابه_جایی = align_مقصد(جابه_جایی, Align);
  أرجع skip(New_جابه_جایی - جابه_جایی);
}

uint8_t دودویی_جریان_Reader::peek() ثابت {
  ArrayRef<uint8_t> میانگیر;
  خودکار EC = جریان.read_بایتها(جابه_جایی, 1, میانگیر);
  أكد(!EC && "Cannot peek an empty buffer!");
  llvm::consumeError(std::move(EC));
  أرجع میانگیر[0];
}

std::pair<دودویی_جریان_Reader, دودویی_جریان_Reader>
دودویی_جریان_Reader::تقسیم(uint64_t Off) ثابت {
  أكد(get_درازی() >= Off);

  دودویی_جریان_Ref اولی = جریان.drop_front(جابه_جایی);

  دودویی_جریان_Ref دومی = اولی.drop_front(Off);
  اولی = اولی.keep_front(Off);
  دودویی_جریان_Reader W1{اولی};
  دودویی_جریان_Reader W2{دومی};
  أرجع {W1, W2};
}
