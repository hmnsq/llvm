//===- BinaryStreamWriter.cpp - Writes objects to a BinaryStream ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/BinaryStreamWriter.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/BinaryStreamReader.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/BinaryStreamRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/LEB128.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਦੁਆਧਾਰੀ_ਧਾਰਾ_Writer::ਦੁਆਧਾਰੀ_ਧਾਰਾ_Writer(Writable_ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref Ref)
    : ਧਾਰਾ(Ref) {}

ਦੁਆਧਾਰੀ_ਧਾਰਾ_Writer::ਦੁਆਧਾਰੀ_ਧਾਰਾ_Writer(Writable_ਦੁਆਧਾਰੀ_ਧਾਰਾ &ਧਾਰਾ)
    : ਧਾਰਾ(ਧਾਰਾ) {}

ਦੁਆਧਾਰੀ_ਧਾਰਾ_Writer::ਦੁਆਧਾਰੀ_ਧਾਰਾ_Writer(MutableArrayRef<uint8_t> ਅੰਕੜੇ_2,
                                       llvm::endianness Endian)
    : ਧਾਰਾ(ਅੰਕੜੇ_2, Endian) {}

Error ਦੁਆਧਾਰੀ_ਧਾਰਾ_Writer::write_ਬਾਈਟਾਂ(ArrayRef<uint8_t> ਬਫਰ) {
  ਜੇ (ਸਵੈਚਾਲਿਤ EC = ਧਾਰਾ.write_ਬਾਈਟਾਂ(ਵਿੱਥ, ਬਫਰ))
    ਵਾਪਸ EC;
  ਵਿੱਥ += ਬਫਰ.ਮਾਪ_2();
  ਵਾਪਸ Error::success();
}

Error ਦੁਆਧਾਰੀ_ਧਾਰਾ_Writer::writeULEB128(uint64_t ਮੁੱਲ) {
  uint8_t Encoded_ਬਾਈਟਾਂ[10] = {0};
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ = encodeULEB128(ਮੁੱਲ, &Encoded_ਬਾਈਟਾਂ[0]);
  ਵਾਪਸ write_ਬਾਈਟਾਂ({Encoded_ਬਾਈਟਾਂ, ਮਾਪ});
}

Error ਦੁਆਧਾਰੀ_ਧਾਰਾ_Writer::writeSLEB128(int64_t ਮੁੱਲ) {
  uint8_t Encoded_ਬਾਈਟਾਂ[10] = {0};
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ = encodeSLEB128(ਮੁੱਲ, &Encoded_ਬਾਈਟਾਂ[0]);
  ਵਾਪਸ write_ਬਾਈਟਾਂ({Encoded_ਬਾਈਟਾਂ, ਮਾਪ});
}

Error ਦੁਆਧਾਰੀ_ਧਾਰਾ_Writer::writeCString(StringRef Str) {
  ਜੇ (ਸਵੈਚਾਲਿਤ EC = writeFixedString(Str))
    ਵਾਪਸ EC;
  ਜੇ (ਸਵੈਚਾਲਿਤ EC = writeObject('\0'))
    ਵਾਪਸ EC;

  ਵਾਪਸ Error::success();
}

Error ਦੁਆਧਾਰੀ_ਧਾਰਾ_Writer::writeFixedString(StringRef Str) {

  ਵਾਪਸ write_ਬਾਈਟਾਂ(arrayRefFromStringRef(Str));
}

Error ਦੁਆਧਾਰੀ_ਧਾਰਾ_Writer::write_ਧਾਰਾ_Ref(ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref Ref) {
  ਵਾਪਸ write_ਧਾਰਾ_Ref(Ref, Ref.get_ਲੰਮਾਈ());
}

Error ਦੁਆਧਾਰੀ_ਧਾਰਾ_Writer::write_ਧਾਰਾ_Ref(ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref Ref, uint64_t ਲੰਮਾਈ) {
  ਦੁਆਧਾਰੀ_ਧਾਰਾ_Reader SrcReader(Ref.ਟੁਕੜਾ(0, ਲੰਮਾਈ));
  // This is a bit tricky.  If we just call readBytes, we are requiring that it
  // return us the entire stream as a contiguous buffer.  There is no guarantee
  // this can be satisfied by returning a reference straight from the buffer, as
  // an implementation may not store all data in a single contiguous buffer.  So
  // we iterate over each contiguous chunk, writing each one in succession.
  ਜਦੋਂ (SrcReader.ਬਾਈਟਾਂ_ਬਚਿਆ() > 0) {
    ArrayRef<uint8_t> Chunk;
    ਜੇ (ਸਵੈਚਾਲਿਤ EC = SrcReader.readLongestContiguousChunk(Chunk))
      ਵਾਪਸ EC;
    ਜੇ (ਸਵੈਚਾਲਿਤ EC = write_ਬਾਈਟਾਂ(Chunk))
      ਵਾਪਸ EC;
  }
  ਵਾਪਸ Error::success();
}

std::pair<ਦੁਆਧਾਰੀ_ਧਾਰਾ_Writer, ਦੁਆਧਾਰੀ_ਧਾਰਾ_Writer>
ਦੁਆਧਾਰੀ_ਧਾਰਾ_Writer::ਵੰਡੋ(uint64_t Off) ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(get_ਲੰਮਾਈ() >= Off);

  Writable_ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref ਪਹਿਲਾ = ਧਾਰਾ.drop_front(ਵਿੱਥ);

  Writable_ਦੁਆਧਾਰੀ_ਧਾਰਾ_Ref ਦੂਜਾ = ਪਹਿਲਾ.drop_front(Off);
  ਪਹਿਲਾ = ਪਹਿਲਾ.keep_front(Off);
  ਦੁਆਧਾਰੀ_ਧਾਰਾ_Writer W1{ਪਹਿਲਾ};
  ਦੁਆਧਾਰੀ_ਧਾਰਾ_Writer W2{ਦੂਜਾ};
  ਵਾਪਸ {W1, W2};
}

Error ਦੁਆਧਾਰੀ_ਧਾਰਾ_Writer::pad_ਮੰਜ਼ਿਲ_Alignment(uint32_t Align) {
  uint64_t New_ਵਿੱਥ = align_ਮੰਜ਼ਿਲ(ਵਿੱਥ, Align);
  ਅਚਲ uint64_t Zeros_ਮਾਪ = 64;
  ਸਥਿਰ constexpr ਅੱਖਰ Zeros[Zeros_ਮਾਪ] = {};
  ਜਦੋਂ (ਵਿੱਥ < New_ਵਿੱਥ)
    ਜੇ (ਸਵੈਚਾਲਿਤ E = writeArray(
            ArrayRef<ਅੱਖਰ>(Zeros, std::min(Zeros_ਮਾਪ, New_ਵਿੱਥ - ਵਿੱਥ))))
      ਵਾਪਸ E;
  ਵਾਪਸ Error::success();
}
