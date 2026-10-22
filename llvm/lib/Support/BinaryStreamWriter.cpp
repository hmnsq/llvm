//===- BinaryStreamWriter.cpp - Writes objects to a BinaryStream ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/BinaryStreamWriter.h"

#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/Support/BinaryStreamReader.h"
#شامل "llvm/Support/BinaryStreamRef.h"
#شامل "llvm/Support/LEB128.h"

استعمال نامخلا llvm;

ثنائی_دھارا_Writer::ثنائی_دھارا_Writer(Writable_ثنائی_دھارا_Ref Ref)
    : دھارا(Ref) {}

ثنائی_دھارا_Writer::ثنائی_دھارا_Writer(Writable_ثنائی_دھارا &دھارا)
    : دھارا(دھارا) {}

ثنائی_دھارا_Writer::ثنائی_دھارا_Writer(MutableArrayRef<uint8_t> ڈیٹا_2,
                                       llvm::endianness Endian)
    : دھارا(ڈیٹا_2, Endian) {}

Error ثنائی_دھارا_Writer::write_بائٹیں(ArrayRef<uint8_t> بفر) {
  اگر (خودکار EC = دھارا.write_بائٹیں(انحراف, بفر))
    واپس EC;
  انحراف += بفر.سائز_2();
  واپس Error::success();
}

Error ثنائی_دھارا_Writer::writeULEB128(uint64_t قدر) {
  uint8_t Encoded_بائٹیں[10] = {0};
  غیرعلامتی سائز = encodeULEB128(قدر, &Encoded_بائٹیں[0]);
  واپس write_بائٹیں({Encoded_بائٹیں, سائز});
}

Error ثنائی_دھارا_Writer::writeSLEB128(int64_t قدر) {
  uint8_t Encoded_بائٹیں[10] = {0};
  غیرعلامتی سائز = encodeSLEB128(قدر, &Encoded_بائٹیں[0]);
  واپس write_بائٹیں({Encoded_بائٹیں, سائز});
}

Error ثنائی_دھارا_Writer::writeCString(StringRef Str) {
  اگر (خودکار EC = writeFixedString(Str))
    واپس EC;
  اگر (خودکار EC = writeObject('\0'))
    واپس EC;

  واپس Error::success();
}

Error ثنائی_دھارا_Writer::writeFixedString(StringRef Str) {

  واپس write_بائٹیں(arrayRefFromStringRef(Str));
}

Error ثنائی_دھارا_Writer::write_دھارا_Ref(ثنائی_دھارا_Ref Ref) {
  واپس write_دھارا_Ref(Ref, Ref.get_لمبائی());
}

Error ثنائی_دھارا_Writer::write_دھارا_Ref(ثنائی_دھارا_Ref Ref, uint64_t لمبائی) {
  ثنائی_دھارا_Reader SrcReader(Ref.قاش(0, لمبائی));
  // This is a bit tricky.  If we just call readBytes, we are requiring that it
  // return us the entire stream as a contiguous buffer.  There is no guarantee
  // this can be satisfied by returning a reference straight from the buffer, as
  // an implementation may not store all data in a single contiguous buffer.  So
  // we iterate over each contiguous chunk, writing each one in succession.
  جب (SrcReader.بائٹیں_بقایا() > 0) {
    ArrayRef<uint8_t> Chunk;
    اگر (خودکار EC = SrcReader.readLongestContiguousChunk(Chunk))
      واپس EC;
    اگر (خودکار EC = write_بائٹیں(Chunk))
      واپس EC;
  }
  واپس Error::success();
}

std::pair<ثنائی_دھارا_Writer, ثنائی_دھارا_Writer>
ثنائی_دھارا_Writer::بانٹو(uint64_t Off) ثابت {
  أكد(get_لمبائی() >= Off);

  Writable_ثنائی_دھارا_Ref پہلا = دھارا.drop_front(انحراف);

  Writable_ثنائی_دھارا_Ref دوسرا = پہلا.drop_front(Off);
  پہلا = پہلا.keep_front(Off);
  ثنائی_دھارا_Writer W1{پہلا};
  ثنائی_دھارا_Writer W2{دوسرا};
  واپس {W1, W2};
}

Error ثنائی_دھارا_Writer::pad_مقصد_Alignment(uint32_t Align) {
  uint64_t New_انحراف = align_مقصد(انحراف, Align);
  ثابت uint64_t Zeros_سائز = 64;
  ساکن constexpr حرف Zeros[Zeros_سائز] = {};
  جب (انحراف < New_انحراف)
    اگر (خودکار E = writeArray(
            ArrayRef<حرف>(Zeros, std::min(Zeros_سائز, New_انحراف - انحراف))))
      واپس E;
  واپس Error::success();
}
