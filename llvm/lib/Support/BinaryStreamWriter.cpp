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

استعمال نطاق llvm;

ٻٽو_وهڪرو_Writer::ٻٽو_وهڪرو_Writer(Writable_ٻٽو_وهڪرو_Ref Ref)
    : وهڪرو(Ref) {}

ٻٽو_وهڪرو_Writer::ٻٽو_وهڪرو_Writer(Writable_ٻٽو_وهڪرو &وهڪرو)
    : وهڪرو(وهڪرو) {}

ٻٽو_وهڪرو_Writer::ٻٽو_وهڪرو_Writer(MutableArrayRef<uint8_t> معلومات_2,
                                       llvm::endianness Endian)
    : وهڪرو(معلومات_2, Endian) {}

Error ٻٽو_وهڪرو_Writer::write_بائيٽون(ArrayRef<uint8_t> بفر) {
  إذا (خودڪار EC = وهڪرو.write_بائيٽون(وٿي, بفر))
    أرجع EC;
  وٿي += بفر.سائيز_2();
  أرجع Error::success();
}

Error ٻٽو_وهڪرو_Writer::writeULEB128(uint64_t قدر) {
  uint8_t Encoded_بائيٽون[10] = {0};
  بنانشان سائيز = encodeULEB128(قدر, &Encoded_بائيٽون[0]);
  أرجع write_بائيٽون({Encoded_بائيٽون, سائيز});
}

Error ٻٽو_وهڪرو_Writer::writeSLEB128(int64_t قدر) {
  uint8_t Encoded_بائيٽون[10] = {0};
  بنانشان سائيز = encodeSLEB128(قدر, &Encoded_بائيٽون[0]);
  أرجع write_بائيٽون({Encoded_بائيٽون, سائيز});
}

Error ٻٽو_وهڪرو_Writer::writeCString(StringRef Str) {
  إذا (خودڪار EC = writeFixedString(Str))
    أرجع EC;
  إذا (خودڪار EC = writeObject('\0'))
    أرجع EC;

  أرجع Error::success();
}

Error ٻٽو_وهڪرو_Writer::writeFixedString(StringRef Str) {

  أرجع write_بائيٽون(arrayRefFromStringRef(Str));
}

Error ٻٽو_وهڪرو_Writer::write_وهڪرو_Ref(ٻٽو_وهڪرو_Ref Ref) {
  أرجع write_وهڪرو_Ref(Ref, Ref.get_ڊيگهائي());
}

Error ٻٽو_وهڪرو_Writer::write_وهڪرو_Ref(ٻٽو_وهڪرو_Ref Ref, uint64_t ڊيگهائي) {
  ٻٽو_وهڪرو_Reader SrcReader(Ref.ٽڪرو(0, ڊيگهائي));
  // This is a bit tricky.  If we just call readBytes, we are requiring that it
  // return us the entire stream as a contiguous buffer.  There is no guarantee
  // this can be satisfied by returning a reference straight from the buffer, as
  // an implementation may not store all data in a single contiguous buffer.  So
  // we iterate over each contiguous chunk, writing each one in succession.
  طالما (SrcReader.بائيٽون_بچيل() > 0) {
    ArrayRef<uint8_t> Chunk;
    إذا (خودڪار EC = SrcReader.readLongestContiguousChunk(Chunk))
      أرجع EC;
    إذا (خودڪار EC = write_بائيٽون(Chunk))
      أرجع EC;
  }
  أرجع Error::success();
}

std::pair<ٻٽو_وهڪرو_Writer, ٻٽو_وهڪرو_Writer>
ٻٽو_وهڪرو_Writer::ورهايو(uint64_t Off) ثابت {
  أكد(get_ڊيگهائي() >= Off);

  Writable_ٻٽو_وهڪرو_Ref پهريون = وهڪرو.drop_front(وٿي);

  Writable_ٻٽو_وهڪرو_Ref ٻيو = پهريون.drop_front(Off);
  پهريون = پهريون.keep_front(Off);
  ٻٽو_وهڪرو_Writer W1{پهريون};
  ٻٽو_وهڪرو_Writer W2{ٻيو};
  أرجع {W1, W2};
}

Error ٻٽو_وهڪرو_Writer::pad_منزل_Alignment(uint32_t Align) {
  uint64_t New_وٿي = align_منزل(وٿي, Align);
  ثابت uint64_t Zeros_سائيز = 64;
  ساكن constexpr حرف Zeros[Zeros_سائيز] = {};
  طالما (وٿي < New_وٿي)
    إذا (خودڪار E = writeArray(
            ArrayRef<حرف>(Zeros, std::min(Zeros_سائيز, New_وٿي - وٿي))))
      أرجع E;
  أرجع Error::success();
}
