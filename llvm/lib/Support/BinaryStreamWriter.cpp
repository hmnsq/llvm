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

ورتوں نطاق llvm;

ثنائی_دھارا_Writer::ثنائی_دھارا_Writer(Writable_ثنائی_دھارا_Ref Ref)
    : دھارا(Ref) {}

ثنائی_دھارا_Writer::ثنائی_دھارا_Writer(Writable_ثنائی_دھارا &دھارا)
    : دھارا(دھارا) {}

ثنائی_دھارا_Writer::ثنائی_دھارا_Writer(MutableArrayRef<uint8_t> انکڑے_2,
                                       llvm::endianness Endian)
    : دھارا(انکڑے_2, Endian) {}

Error ثنائی_دھارا_Writer::write_بائٹاں(ArrayRef<uint8_t> بفر) {
  إذا (خودکار EC = دھارا.write_بائٹاں(وتھ, بفر))
    أرجع EC;
  وتھ += بفر.ناپ_2();
  أرجع Error::success();
}

Error ثنائی_دھارا_Writer::writeULEB128(uint64_t قدر) {
  uint8_t Encoded_بائٹاں[10] = {0};
  بنانشانی ناپ = encodeULEB128(قدر, &Encoded_بائٹاں[0]);
  أرجع write_بائٹاں({Encoded_بائٹاں, ناپ});
}

Error ثنائی_دھارا_Writer::writeSLEB128(int64_t قدر) {
  uint8_t Encoded_بائٹاں[10] = {0};
  بنانشانی ناپ = encodeSLEB128(قدر, &Encoded_بائٹاں[0]);
  أرجع write_بائٹاں({Encoded_بائٹاں, ناپ});
}

Error ثنائی_دھارا_Writer::writeCString(StringRef Str) {
  إذا (خودکار EC = writeFixedString(Str))
    أرجع EC;
  إذا (خودکار EC = writeObject('\0'))
    أرجع EC;

  أرجع Error::success();
}

Error ثنائی_دھارا_Writer::writeFixedString(StringRef Str) {

  أرجع write_بائٹاں(arrayRefFromStringRef(Str));
}

Error ثنائی_دھارا_Writer::write_دھارا_Ref(ثنائی_دھارا_Ref Ref) {
  أرجع write_دھارا_Ref(Ref, Ref.get_لمائی());
}

Error ثنائی_دھارا_Writer::write_دھارا_Ref(ثنائی_دھارا_Ref Ref, uint64_t لمائی) {
  ثنائی_دھارا_Reader SrcReader(Ref.ٹکڑا(0, لمائی));
  // This is a bit tricky.  If we just call readBytes, we are requiring that it
  // return us the entire stream as a contiguous buffer.  There is no guarantee
  // this can be satisfied by returning a reference straight from the buffer, as
  // an implementation may not store all data in a single contiguous buffer.  So
  // we iterate over each contiguous chunk, writing each one in succession.
  طالما (SrcReader.بائٹاں_بچیا() > 0) {
    ArrayRef<uint8_t> Chunk;
    إذا (خودکار EC = SrcReader.readLongestContiguousChunk(Chunk))
      أرجع EC;
    إذا (خودکار EC = write_بائٹاں(Chunk))
      أرجع EC;
  }
  أرجع Error::success();
}

std::pair<ثنائی_دھارا_Writer, ثنائی_دھارا_Writer>
ثنائی_دھارا_Writer::ونڈو(uint64_t Off) ثابت {
  أكد(get_لمائی() >= Off);

  Writable_ثنائی_دھارا_Ref پہلا = دھارا.drop_front(وتھ);

  Writable_ثنائی_دھارا_Ref دوجا = پہلا.drop_front(Off);
  پہلا = پہلا.keep_front(Off);
  ثنائی_دھارا_Writer W1{پہلا};
  ثنائی_دھارا_Writer W2{دوجا};
  أرجع {W1, W2};
}

Error ثنائی_دھارا_Writer::pad_منزل_Alignment(uint32_t Align) {
  uint64_t New_وتھ = align_منزل(وتھ, Align);
  ثابت uint64_t Zeros_ناپ = 64;
  ساكن constexpr حرف Zeros[Zeros_ناپ] = {};
  طالما (وتھ < New_وتھ)
    إذا (خودکار E = writeArray(
            ArrayRef<حرف>(Zeros, std::min(Zeros_ناپ, New_وتھ - وتھ))))
      أرجع E;
  أرجع Error::success();
}
