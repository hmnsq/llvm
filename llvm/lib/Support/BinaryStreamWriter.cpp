//===- BinaryStreamWriter.cpp - Writes objects to a BinaryStream ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/BinaryStreamWriter.h"

#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/Support/BinaryStreamReader.h"
#ضمن "llvm/Support/BinaryStreamRef.h"
#ضمن "llvm/Support/LEB128.h"

استفاده نطاق llvm;

دودویی_جریان_Writer::دودویی_جریان_Writer(Writable_دودویی_جریان_Ref Ref)
    : جریان(Ref) {}

دودویی_جریان_Writer::دودویی_جریان_Writer(Writable_دودویی_جریان &جریان)
    : جریان(جریان) {}

دودویی_جریان_Writer::دودویی_جریان_Writer(MutableArrayRef<uint8_t> داده_2,
                                       llvm::endianness Endian)
    : جریان(داده_2, Endian) {}

Error دودویی_جریان_Writer::write_بایتها(ArrayRef<uint8_t> میانگیر) {
  إذا (خودکار EC = جریان.write_بایتها(جابه_جایی, میانگیر))
    أرجع EC;
  جابه_جایی += میانگیر.اندازه_2();
  أرجع Error::success();
}

Error دودویی_جریان_Writer::writeULEB128(uint64_t مقدار) {
  uint8_t Encoded_بایتها[10] = {0};
  بیعلامت اندازه = encodeULEB128(مقدار, &Encoded_بایتها[0]);
  أرجع write_بایتها({Encoded_بایتها, اندازه});
}

Error دودویی_جریان_Writer::writeSLEB128(int64_t مقدار) {
  uint8_t Encoded_بایتها[10] = {0};
  بیعلامت اندازه = encodeSLEB128(مقدار, &Encoded_بایتها[0]);
  أرجع write_بایتها({Encoded_بایتها, اندازه});
}

Error دودویی_جریان_Writer::writeCString(StringRef Str) {
  إذا (خودکار EC = writeFixedString(Str))
    أرجع EC;
  إذا (خودکار EC = writeObject('\0'))
    أرجع EC;

  أرجع Error::success();
}

Error دودویی_جریان_Writer::writeFixedString(StringRef Str) {

  أرجع write_بایتها(arrayRefFromStringRef(Str));
}

Error دودویی_جریان_Writer::write_جریان_Ref(دودویی_جریان_Ref Ref) {
  أرجع write_جریان_Ref(Ref, Ref.get_درازی());
}

Error دودویی_جریان_Writer::write_جریان_Ref(دودویی_جریان_Ref Ref, uint64_t درازی) {
  دودویی_جریان_Reader SrcReader(Ref.برش(0, درازی));
  // This is a bit tricky.  If we just call readBytes, we are requiring that it
  // return us the entire stream as a contiguous buffer.  There is no guarantee
  // this can be satisfied by returning a reference straight from the buffer, as
  // an implementation may not store all data in a single contiguous buffer.  So
  // we iterate over each contiguous chunk, writing each one in succession.
  طالما (SrcReader.بایتها_مانده() > 0) {
    ArrayRef<uint8_t> Chunk;
    إذا (خودکار EC = SrcReader.readLongestContiguousChunk(Chunk))
      أرجع EC;
    إذا (خودکار EC = write_بایتها(Chunk))
      أرجع EC;
  }
  أرجع Error::success();
}

std::pair<دودویی_جریان_Writer, دودویی_جریان_Writer>
دودویی_جریان_Writer::تقسیم(uint64_t Off) ثابت {
  أكد(get_درازی() >= Off);

  Writable_دودویی_جریان_Ref اولی = جریان.drop_front(جابه_جایی);

  Writable_دودویی_جریان_Ref دومی = اولی.drop_front(Off);
  اولی = اولی.keep_front(Off);
  دودویی_جریان_Writer W1{اولی};
  دودویی_جریان_Writer W2{دومی};
  أرجع {W1, W2};
}

Error دودویی_جریان_Writer::pad_مقصد_Alignment(uint32_t Align) {
  uint64_t New_جابه_جایی = align_مقصد(جابه_جایی, Align);
  ثابت uint64_t Zeros_اندازه = 64;
  ساكن constexpr حرف Zeros[Zeros_اندازه] = {};
  طالما (جابه_جایی < New_جابه_جایی)
    إذا (خودکار E = writeArray(
            ArrayRef<حرف>(Zeros, std::min(Zeros_اندازه, New_جابه_جایی - جابه_جایی))))
      أرجع E;
  أرجع Error::success();
}
