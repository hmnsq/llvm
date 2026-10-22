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

کارول نطاق llvm;

دوګونی_بهیر_Writer::دوګونی_بهیر_Writer(Writable_دوګونی_بهیر_Ref Ref)
    : بهیر(Ref) {}

دوګونی_بهیر_Writer::دوګونی_بهیر_Writer(Writable_دوګونی_بهیر &بهیر)
    : بهیر(بهیر) {}

دوګونی_بهیر_Writer::دوګونی_بهیر_Writer(MutableArrayRef<uint8_t> معلومات_2,
                                       llvm::endianness Endian)
    : بهیر(معلومات_2, Endian) {}

Error دوګونی_بهیر_Writer::write_بایټونه(ArrayRef<uint8_t> بفر) {
  إذا (خپلکاری EC = بهیر.write_بایټونه(دننه_واټن, بفر))
    أرجع EC;
  دننه_واټن += بفر.اندازه_2();
  أرجع Error::success();
}

Error دوګونی_بهیر_Writer::writeULEB128(uint64_t ارزښت) {
  uint8_t Encoded_بایټونه[10] = {0};
  بینښه اندازه = encodeULEB128(ارزښت, &Encoded_بایټونه[0]);
  أرجع write_بایټونه({Encoded_بایټونه, اندازه});
}

Error دوګونی_بهیر_Writer::writeSLEB128(int64_t ارزښت) {
  uint8_t Encoded_بایټونه[10] = {0};
  بینښه اندازه = encodeSLEB128(ارزښت, &Encoded_بایټونه[0]);
  أرجع write_بایټونه({Encoded_بایټونه, اندازه});
}

Error دوګونی_بهیر_Writer::writeCString(StringRef Str) {
  إذا (خپلکاری EC = writeFixedString(Str))
    أرجع EC;
  إذا (خپلکاری EC = writeObject('\0'))
    أرجع EC;

  أرجع Error::success();
}

Error دوګونی_بهیر_Writer::writeFixedString(StringRef Str) {

  أرجع write_بایټونه(arrayRefFromStringRef(Str));
}

Error دوګونی_بهیر_Writer::write_بهیر_Ref(دوګونی_بهیر_Ref Ref) {
  أرجع write_بهیر_Ref(Ref, Ref.get_اوږدتیا());
}

Error دوګونی_بهیر_Writer::write_بهیر_Ref(دوګونی_بهیر_Ref Ref, uint64_t اوږدتیا) {
  دوګونی_بهیر_Reader SrcReader(Ref.ټوټه(0, اوږدتیا));
  // This is a bit tricky.  If we just call readBytes, we are requiring that it
  // return us the entire stream as a contiguous buffer.  There is no guarantee
  // this can be satisfied by returning a reference straight from the buffer, as
  // an implementation may not store all data in a single contiguous buffer.  So
  // we iterate over each contiguous chunk, writing each one in succession.
  طالما (SrcReader.بایټونه_پاتې_شوی() > 0) {
    ArrayRef<uint8_t> Chunk;
    إذا (خپلکاری EC = SrcReader.readLongestContiguousChunk(Chunk))
      أرجع EC;
    إذا (خپلکاری EC = write_بایټونه(Chunk))
      أرجع EC;
  }
  أرجع Error::success();
}

std::pair<دوګونی_بهیر_Writer, دوګونی_بهیر_Writer>
دوګونی_بهیر_Writer::وېشل(uint64_t Off) ثابت {
  أكد(get_اوږدتیا() >= Off);

  Writable_دوګونی_بهیر_Ref لومړی = بهیر.drop_front(دننه_واټن);

  Writable_دوګونی_بهیر_Ref دوهم = لومړی.drop_front(Off);
  لومړی = لومړی.keep_front(Off);
  دوګونی_بهیر_Writer W1{لومړی};
  دوګونی_بهیر_Writer W2{دوهم};
  أرجع {W1, W2};
}

Error دوګونی_بهیر_Writer::pad_منزل_Alignment(uint32_t Align) {
  uint64_t New_دننه_واټن = align_منزل(دننه_واټن, Align);
  ثابت uint64_t Zeros_اندازه = 64;
  ساكن constexpr حرف Zeros[Zeros_اندازه] = {};
  طالما (دننه_واټن < New_دننه_واټن)
    إذا (خپلکاری E = writeArray(
            ArrayRef<حرف>(Zeros, std::min(Zeros_اندازه, New_دننه_واټن - دننه_واټن))))
      أرجع E;
  أرجع Error::success();
}
